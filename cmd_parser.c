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
	char pwd[]= "pwd", cd[]="cd", sls[] = "sls"; 
 
	vessel->mallocs= 0;
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

			int count =0;
			
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

	int fd[2]; 
	pipe(fd); 
	if(fork() != 0)
	{
		//parent
		//no need for read access
		close(fd[0]); 
		//replace stdout with pipe
		dup2(fd[1], STDOUT_FILENO); 
		//Close now unused FD
		close(fd[1]);
		//parent becomes process 1 

		execvp(cmd->pipe_cmds[0]->exec, cmd->pipe_cmds[0]->args); 
	}	
	else
	{

		//Child
		//No need for write access
		close(fd[1]); 
		//Replace stdin with pipe
		dup2(fd[0], STDIN_FILENO); 
		//close now unused FD
		close(fd[0]); 
		//child becomes process 2
		execvp(cmd->pipe_cmds[1]->exec, cmd->pipe_cmds[1]->args); 	

	}

}
void pipeline_3(cmd_t* cmd)
{

	int fd[2]; 
	int fd_2[2]; 
	pipe(fd); 
	if(fork() != 0)
	{
		//Grandparent
		//no need for read access
		close(fd[0]); 
		//replace stdout with pipe
		dup2(fd[1], STDOUT_FILENO); 
		//Close now unused FD
		close(fd[1]);
		//grandparent parent becomes process 1 

		execvp(cmd->pipe_cmds[0]->exec, cmd->pipe_cmds[0]->args); 
	}	
	else
	{
		pipe(fd_2); 

		if(fork() !=0)
		{
			//Parent
			//No need for write access pipe #1
			close(fd[1]);
			//No need for read access pipe #2 
			close(fd_2[0]);  
			//Replace stdin with pipe
			dup2(fd[0], STDIN_FILENO);
			//Replace stdout with pipe2 
			dup2(fd_2[1], STDOUT_FILENO); 
			//close now unused FDs 
			close(fd[0]); 
			close(fd_2[1]); 
			//parent becomes process 2
			execvp(cmd->pipe_cmds[1]->exec, cmd->pipe_cmds[1]->args); 

		}
		else
		{
			//Child
			//No need for write access
			close(fd_2[1]); 
			//Replace stdin with pipe
			dup2(fd_2[0], STDIN_FILENO); 
			//close now unused FD
			close(fd_2[0]); 
			//child becomes process 3
			execvp(cmd->pipe_cmds[2]->exec, cmd->pipe_cmds[2]->args);
		} 	
	}
}

/*These commands need to run in the child process*/
void execute_command_c(cmd_t* cmd)
{
	int fd;

	if(cmd == NULL)
		fprintf(stderr, "NULL pointer passed in !");
       	
	switch(cmd->which_command)
	{
		case NORMAL:
			execvp(cmd->exec, cmd->args); 	
			break; 
		case REDIRECT_NORMAL: 
			fd = open(cmd->output_file, O_WRONLY | O_CREAT, 0644);	
			dup2(fd, STDOUT_FILENO);
			close(fd);
			execvp(cmd->exec, cmd->args);
		    	break; 
		case REDIRECT_APPEND:
			// case when output file does not exist
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



