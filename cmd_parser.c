#include "cmd_parser.h"

/* 
 * Function: cmd_parser
 * -----------------------
 * Populates the internal variables of the cmd_t struct
 * by parsing the string in raw. 
 * 
 * vessel: pointer to cmd_t object (not on the heap)
 * raw : string containing raw input from the terminal
 *  
 * */
void cmd_parser(cmd_t* vessel, char* raw)
{
	/*String processing variables */
	int pipe_flag = 0, file_redirect_flag =0;
 	char* ui;

	/*Initializes cmd_t struct*/
	vessel->mallocs= 0;
	vessel->parser_error = 0; 
	vessel->child1_status = 0; 
	vessel->child2_status = 0;

	/* Get rid of '\n' at the end of the command */
	ui  = strchr(raw, '\n');
    if (ui)	
	{
		*ui = '\0';
	}
	ui  = strchr(vessel->raw_input, '\n');
    if (ui)
	{ 
		*ui = '\0'; 
	}	
	pipe_flag = strchr(raw, '|') != NULL;
	file_redirect_flag = strchr(raw, '>') != NULL;
	if(pipe_flag && file_redirect_flag)
	{
		fprintf(stderr, "Error: mislocated output redirection\n"); 
		vessel->parser_error = 1;
		return; 
	}
	if(!file_redirect_flag)
	{
		if(!pipe_flag)
		{
			handle_normal(vessel, raw); 
		}
		else
		{
			handle_pipes(vessel, raw); 
		}		
	}
	else
	{     	
		handle_redirects(vessel, raw); 
	} 
}

void handle_normal(cmd_t* vessel, char* raw)
{
	/*Regular command with arguments*/
	int count = 0;
	char *argument;	 
	char pwd[]= "pwd", cd[]="cd", sls[] = "sls", exit[]="exit"; 
	vessel->which_command = NORMAL; 
	argument = strtok(raw, " "); 
	strcpy(vessel->exec, argument);
	vessel->args[count++] = argument;

	/* checks for commands that need to be implemented*/
	if(!strcmp(argument, pwd))
        vessel->which_command = PWD;

	/*Note args[1] will be the output file name*/ 
    else if(!strcmp(argument, cd))
        vessel->which_command = CD;
	else if (!strcmp(argument, sls))
		vessel->which_command = SLS; 
	else if(!strcmp(argument, exit))
		vessel->which_command = EXIT; 
	argument = strtok(NULL, " ");
	while(argument != NULL)
	{
		/*Too many arguments check*/
		if(count -1 > MAX_ARGS)
		{
			fprintf(stderr, "Error: too many process arguements\n");
            vessel->parser_error = 1;
			return; 
		}
		vessel->args[count++] = argument;
		argument = strtok(NULL, " "); 
	}
}

void handle_redirects(cmd_t* vessel, char* raw)
{
	/*Handle file redirection*/
	int count =0, first =0, fd; 
	char *command, *out_file, *argument, *symbol;

	/* symbol will be used to catch an incomplete command*/
	symbol = strchr(raw, '>'); 
	if (strstr(raw, ">>") == NULL)
		vessel->which_command = REDIRECT_NORMAL;
	else
		vessel->which_command = REDIRECT_APPEND; 
		
	/* Extract command and arguments */
	command = strtok_r(raw, ">", &raw);
	if(symbol < command) first =1 ; 	
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

	/* Parser error checking */
	if(out_file == NULL && first)
	{
		fprintf(stderr, "Error: missing command\n");
        vessel->parser_error = 1;
		return; 
	}
	if(out_file == NULL && !first)
    {
        fprintf(stderr, "Error: no output file\n");
    	vessel->parser_error = 1;
		return; 
	}
	out_file = strtok(out_file, " "); 
	strcpy(vessel->output_file, out_file);
	fd = open(vessel->output_file, O_WRONLY | O_CREAT, 0644); 
	if(fd == -1)
	{
		fprintf(stderr, "Error: cannot open output file\n");
        vessel->parser_error = 1;
        return;
	}
	close(fd); 
}

void handle_pipes(cmd_t* vessel, char* raw)
{
	/*Process pipe commands*/
	int index = 0; 	
	char* argument;

	/*Assumes that exit will not be passed in     *
	 * as a piped command. The cmd_t.exec and     *
	 * cmd_t.args[0] is filled with a dummy string*/
	vessel->args[0] = "Random command";	
	argument = strtok_r(raw, "|", &raw);
	while(argument != NULL)
	{	
		vessel->pipe_cmds[index] = (cmd_t*) malloc(sizeof(cmd_t));

		/* Recursive call to cmd_parser() to parse normal sub commad */
		cmd_parser(vessel->pipe_cmds[index++], argument); 	
		argument = strtok_r(raw, "|", &raw); 	
	}
	vessel->mallocs = index; 
	if(index==2)
	{
		vessel->which_command = PIPE_TWO; 
	}
	else if(index == 3)
	{
		vessel->which_command = PIPE_THREE; 
	}
	else
	{
		fprintf(stderr, "Error: missing commands\n");
		vessel->parser_error = 1; 
	}	
}

void pipeline_2(cmd_t* cmd)
{
	int status1 =0, pid; 
	int fd[2]; 
	if(pipe(fd) == -1)
	{
		fprintf(stderr, "Error: cannot get create pipe\n");
        exit(1);
	}	

	/* Note the program outputing to the terminal must be in the  *
	 * parent process. The children cannot output to the terminal *
	 * due to the SIGTTIN/ SIGTTOU signals.                       */
	pid = fork();
	if(pid > 0)
	{
		/*Parent */
        /*No need for write access to pipe*/
        close(fd[1]);

        /* stdin is received from the pipe*/
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);

        /* Catches errors from children */
		waitpid(pid, &status1, 0); 
		cmd->child1_status = WEXITSTATUS(status1); 
        execvp(cmd->pipe_cmds[1]->exec, cmd->pipe_cmds[1]->args); 
	}	
	else
	{
		/*Child*/
        /* No need for read access from pipe */
    	close(fd[0]);

        /* stdout goes to pipe */
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        execvp(cmd->pipe_cmds[0]->exec, cmd->pipe_cmds[0]->args);
	}
}

void pipeline_3(cmd_t* cmd)
{
	int fd_1[2], fd_2[2]; 
	int status1, status2, pid1, pid2;
	if(pipe(fd_1) == -1)
	{
		fprintf(stderr, "Error: cannot get create pipe\n");
        exit(1);
	}
	if((pid1 =fork()) > 0)
	{
		/*Grandparent*/
        close(fd_1[1]);
        dup2(fd_1[0], STDIN_FILENO);
        close(fd_1[0]);

        /*grandparent catches error from child*/
		waitpid(pid1, &status1, 0);
        cmd->child1_status = WEXITSTATUS(status1);
        execvp(cmd->pipe_cmds[2]->exec, cmd->pipe_cmds[2]->args); 
	}	
	else
	{
		if(pipe(fd_2) == -1)
		{
			fprintf(stderr, "Error: cannot get create pipe\n");
            exit(1);
		}	
		if((pid2= fork()) > 0)
		{
			/*Parent*/
			/*No need for write access pipe #1*/
			close(fd_2[1]);

			/*No need for read access pipe #2 */
			close(fd_1[0]);  
			dup2(fd_2[0], STDIN_FILENO); 
			dup2(fd_1[1], STDOUT_FILENO); 

			/* Close unused file descriptors  */
			close(fd_2[0]); 
			close(fd_1[1]); 

			/*Parent checks for errors from child*/
			waitpid(pid2, &status2, 0);
	        cmd->child2_status = WEXITSTATUS(status2);
			execvp(cmd->pipe_cmds[1]->exec, cmd->pipe_cmds[1]->args); 
		}
		else
		{
			/*Child*/
	        close(fd_2[0]);
            dup2(fd_2[1], STDOUT_FILENO);
            close(fd_2[1]); 
            execvp(cmd->pipe_cmds[0]->exec, cmd->pipe_cmds[0]->args);
		} 	
	}
}

/*These commands need to run in the child process*/
void execute_command_c(cmd_t* cmd)
{
	int fd;
	if(cmd == NULL)
	{
		fprintf(stderr, "NULL pointer passed in !");
	}	
	switch(cmd->which_command)
	{
		case NORMAL:	
			if(execvp(cmd->exec, cmd->args) == -1)
			{
				fprintf(stderr, "Error: command not found\n");
                exit(1);
			}	
			break; 
		case REDIRECT_NORMAL: 
			fd = open(cmd->output_file, O_WRONLY | O_CREAT, 0644);	
			dup2(fd, STDOUT_FILENO);
			close(fd);
			execvp(cmd->exec, cmd->args);
		    break; 
		case REDIRECT_APPEND:
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
			if(!getcwd(cmd->cwd, sizeof(cmd->cwd)))
			{
				fprintf(stderr, "Error: cannot get current directory\n"); 
				exit(1); 
			}	
			printf("%s\n", cmd->cwd);
		    fflush(stdout);
			exit(0);		
			break;
		case CD: 
			if(chdir(cmd->args[1]) == -1) exit(1);
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
			if(!getcwd(cmd->cwd, sizeof(cmd->cwd)))
			{
				fprintf(stderr, "Error: cannot get current directory\n");
                exit(1);
			}
			execute_sls(cmd); 
			break; 
		case CD: 
			if(chdir(cmd->args[1]) == -1)
			{
				fprintf(stderr, "Error: cannot cd into directory\n");
			}
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
	if(!getcwd(cmd->cwd, sizeof(cmd->cwd)))
	{
		fprintf(stderr, "Error: cannot get current directory\n");
		exit(1);
	}
	cur_dir = opendir(cmd->cwd);
	dp = readdir(cur_dir); 
	if (dp == NULL)
	{
		fprintf(stderr, "Error: cannot open directory"); 
		exit(1); 
	}
	do
	{
		/*Don't read hidden files  */
		if(dp->d_name[0] == '.')
		{
			dp = readdir(cur_dir);
			continue; 
		}
		stat(dp->d_name, &st); 
        printf("%s (%ld bytes)\n",dp->d_name, st.st_size);
		dp = readdir(cur_dir); 

	}while(dp !=NULL); 	
}

void print_main(cmd_t* parser, int status)
{
	/*Prints based on the different cases */
	switch(parser->which_command)
	{
		case PIPE_TWO:
			fprintf(stderr, "+ completed '%s' [%d][%d]\n",
                    parser->raw_input, 
					parser->child1_status,
					WEXITSTATUS(status)); 
			break; 
		case PIPE_THREE:
			fprintf(stderr, "+ completed '%s' [%d][%d][%d]\n",
                    parser->raw_input,
				    parser->child1_status,
                    parser->child2_status,	
                    WEXITSTATUS(status));
			break;
		case EXIT:
            fprintf(stderr, "Bye...\n");
			fprintf(stderr, "+ completed '%s' [%d]\n",
                    parser->raw_input, WEXITSTATUS(status));
			break; 
		default:
			fprintf(stderr, "+ completed '%s' [%d]\n",
                    parser->raw_input, WEXITSTATUS(status));
			break;
	}
}

