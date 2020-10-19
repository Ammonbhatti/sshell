#include "cmd_parser.h"
#include <stdio.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <sys/stat.h>
#include <dirent.h>

//Pass struct by reference
void cmd_parser(cmd_t* vessel, char* raw)
{

	char *argument, *ui;		//used to process args including argv[0] 
	char pwd[]= "pwd", cd[]="cd", sls[] = "sls", exit[]="exit"; 
 
	vessel->mallocs= 0;
	vessel->child1_status = 0; 
	vessel->child2_status = 0; 
	/* Get rid of '\n' at the end of the command */
	ui  = strchr(raw, '\n');
        if (ui)	*ui = '\0';
	ui  = strchr(vessel->raw_input, '\n');
        if (ui) *ui = '\0';	
	
	if(strchr(raw, '>') == NULL)
	{
		if(strchr(raw, '|') == NULL)
		{

			//Regular command with arguments
			vessel->which_command = NORMAL; 

			int count = 0;
			
			argument = strtok(raw, " "); 
			strcpy(vessel->exec, argument);
		    vessel->args[count++] = argument;
			//if pwd command
			if(!strcmp(argument, pwd))
                vessel->which_command = PWD;
			//if cd command
			//Note args[1] path to change to 
            else if(!strcmp(argument, cd))
                vessel->which_command = CD;
		    else if (!strcmp(argument, sls))
				vessel->which_command = SLS; 
			else if(!strcmp(argument, exit))
				vessel->which_command = EXIT; 
			argument = strtok(NULL, " ");
		    while(argument != NULL)
			{
				vessel->args[count++] = argument;
				argument = strtok(NULL, " "); 
			}	

		}
		else
		{
			//Handle pipes
			int index = 0; 	
			/*Assumes that exit will not be passed in 
			 * as a piped command. */
			vessel->args[0] = "Random command";	
			argument = strtok_r(raw, "|", &raw);
		       	while(argument != NULL)
			{	
				//recursize call to handle NORMAL command
				vessel->pipe_cmds[index] = (cmd_t*) malloc(sizeof(cmd_t));
				cmd_parser(vessel->pipe_cmds[index++], argument); 	
				argument = strtok_r(raw, "|", &raw); 	
			}
			vessel->mallocs = index; 
			if(index==2)
				vessel->which_command = PIPE_TWO; 
			else if(index == 3)
				vessel->which_command = PIPE_THREE; 
			else
				fprintf(stderr, "Incorrect number of pipes"); 
		}		

	}

	else
	{
		//Handle file redirection
		int count =0; 
		char *command, *out_file;
		// check for append command
		if (strstr(raw, ">>") == NULL)
			vessel->which_command = REDIRECT_NORMAL;
		else
			vessel->which_command = REDIRECT_APPEND; 
		
		/* Extract command and arguments */
		command = strtok_r(raw, ">", &raw);
		argument = strtok(command, " ");
		strcpy(vessel->exec, argument);
		vessel->args[count++] = argument;        	  
		argument = strtok(NULL, " ");
		while(argument != NULL)
		{
			vessel->args[count++] = argument;
			argument = strtok(NULL, " "); 	
		}

		/* Extract output file name  */
		out_file = strtok_r(raw, ">", &raw);
	      	out_file = strtok(out_file, " "); 	
		strcpy(vessel->output_file, out_file);      	

	}
}

void pipeline_2(cmd_t* cmd)
{

	int status1 =0, pid; 
	int fd[2]; 
	pipe(fd);	
	/* Note the program outputing to the terminal must be in the  *
	 * parent process. The children cannot output to the terminal *
	 * due to the SIGTTIN/ SIGTTOU                                */
	pid = fork();
	if(pid > 0)
	{
		//Parent
                //No need for write access
                close(fd[1]);
                //Replace stdin with pipe
                dup2(fd[0], STDIN_FILENO);
                //close now unused FD
                close(fd[0]);
                //child becomes process 2
		waitpid(pid, &status1, 0); 
		cmd->child1_status = WEXITSTATUS(status1); 
                execvp(cmd->pipe_cmds[1]->exec, cmd->pipe_cmds[1]->args); 
	}	
	else
	{

		//Child	
                //no need for read access
                close(fd[0]);
                //replace stdout with pipe
                dup2(fd[1], STDOUT_FILENO);
                //Close now unused FD
                close(fd[1]);
                //child becomes process 1 
                execvp(cmd->pipe_cmds[0]->exec, cmd->pipe_cmds[0]->args);
	}

}

void pipeline_3(cmd_t* cmd)
{

	int fd_1[2], fd_2[2]; 
	int status1, status2, pid1, pid2; 
	pipe(fd_1); 
	if((pid1 =fork()) > 0)
	{
		//Grandparent
		//No need for write access
                close(fd_1[1]);
                //Replace stdin with pipe
                dup2(fd_1[0], STDIN_FILENO);
                //close now unused FD
                close(fd_1[0]);
                //grandparent becomes process 3
		waitpid(pid1, &status1, 0);
               	cmd->child1_status = WEXITSTATUS(status1);
                execvp(cmd->pipe_cmds[2]->exec, cmd->pipe_cmds[2]->args); 
	}	
	else
	{
		pipe(fd_2); 

		if((pid2= fork()) > 0)
		{
			//Parent
			//No need for write access pipe #1
			close(fd_2[1]);
			//No need for read access pipe #2 
			close(fd_1[0]);  
			//Replace stdin with pipe
			dup2(fd_2[0], STDIN_FILENO);
			//Replace stdout with pipe2 
			dup2(fd_1[1], STDOUT_FILENO); 
			//close now unused FDs 
			close(fd_2[0]); 
			close(fd_1[1]); 
			//parent becomes process 2
			waitpid(pid2, &status2, 0);
	                cmd->child2_status = WEXITSTATUS(status2);
			execvp(cmd->pipe_cmds[1]->exec, cmd->pipe_cmds[1]->args); 

		}
		else
		{
			//Child
			//no need for read access
	                close(fd_2[0]);
                	//replace stdout with pipe
                	dup2(fd_2[1], STDOUT_FILENO);
                	//Close now unused FD
                	close(fd_2[1]);
			//childe becomes process 1
                	execvp(cmd->pipe_cmds[0]->exec, cmd->pipe_cmds[0]->args);
		} 	
	}
}

/*These commands need to run in the child process*/
void execute_command_c(cmd_t* cmd)
{
	int fd;
	int argument_count = 0;
	int argument_index = 0;

	if(cmd == NULL)
		fprintf(stderr, "NULL pointer passed in !");
       	
	switch(cmd->which_command)
	{
		case NORMAL:	
			// count number of arguments
			while (cmd->args[argument_index] != NULL)
			{
				argument_count++;
				argument_index++;
			}

			// detect error for too many arguments
			if (argument_count > 16)
			{
				fprintf(stderr, "Error: too many process arguements\n");
				exit(1); 
			}

			execvp(cmd->exec, cmd->args); 	
			break; 
		case REDIRECT_NORMAL: 
			fd = open(cmd->output_file, O_WRONLY | O_CREAT, 0644);	
			dup2(fd, STDOUT_FILENO);
			close(fd);
			execvp(cmd->exec, cmd->args);
		    	break; 
		case REDIRECT_APPEND:
			// case when output file does not exist ERROR
			if (access(cmd->output_file, F_OK) == -1)
			{
				fprintf(stderr, "Error: cannot open output file\n");
				exit(1); 
			}
			fd = open(cmd->output_file, O_WRONLY | O_APPEND, 0644);	
			dup2(fd, STDOUT_FILENO);
			close(fd);
			execvp(cmd->exec, cmd->args);
			break; 
	        case PIPE_TWO: 
			pipeline_2(cmd);  
			break; 
		case PIPE_THREE: 
			pipeline_3(cmd); 
			break; 
		case SLS: 
			exit(0); 
			break; 	
		case PWD:
			getcwd(cmd->cwd, sizeof(cmd->cwd)); 
			printf("%s \n", cmd->cwd);
		       	fflush(stdout);
			exit(0);		
			break;
		case CD: 
			exit(0); 
			break;
		case EXIT: 
			exit(0); 
			break;	
		default: 
			break; 

	}
	
}

void execute_command_p(cmd_t* cmd)
{
	/*These commands need to be run in the parent*/
	switch(cmd->which_command)
	{
		case SLS: 
			getcwd(cmd->cwd, sizeof(cmd->cwd));
			execute_sls(cmd); 
			break; 
		case CD: 
			chdir(cmd->args[1]);
			break;
		default:
			break; 

	}
}

void execute_sls(cmd_t* cmd)
{
	DIR * cur_dir;
        struct dirent *dp;
	struct stat st;

	cur_dir = opendir(cmd->cwd);
	dp = readdir(cur_dir); 
	if (dp == NULL)
	{
		fprintf(stderr, "Error: cannot open directory"); 
		exit(1); 
	}
	do
	{
		stat(dp->d_name, &st); 
        	printf("%s (%ld bytes) \n",dp->d_name, st.st_size);
		dp = readdir(cur_dir); 

	}while(dp !=NULL); 	

}

void print_main(cmd_t* parser, int status)
{

	switch(parser->which_command)
	{
		case PIPE_TWO:
			fprintf(stderr, "\n+ Completed '%s' [%d][%d]\n",
                                        parser->raw_input, 
					parser->child1_status,
					WEXITSTATUS(status)); 
			break; 
		case PIPE_THREE:
			fprintf(stderr, "\n+ Completed '%s' [%d][%d][%d]\n",
                                        parser->raw_input,
				        parser->child1_status,
                                        parser->child2_status,	
                                        WEXITSTATUS(status));
			break;
		case EXIT:
                        fprintf(stderr, "Bye...\n");
			fprintf(stderr, "\n+ Completed '%s' [%d]\n",
                                        parser->raw_input, WEXITSTATUS(status));
			break; 
		default:
			fprintf(stderr, "\n+ Completed '%s' [%d]\n",
                                        parser->raw_input, WEXITSTATUS(status));
			break;
	}
}

