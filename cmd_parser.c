#include "cmd_parser.h"
#include <stdio.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

//Pass struct by reference
void cmd_parser(cmd_t* vessel, char* raw)
{
	
	char *argument, *ui;		//used to process args including argv[0] 
	vessel->mallocs= 0; 		//for heap memory management

	/* Get rid of '\n' at the end of the command */
	ui  = strchr(raw, '\n');        //ui points to '\n'
        if (ui)
        	*ui = '\0';
	
	if(strchr(raw, '>') == NULL)
	{
		if(strchr(raw, '|') == NULL)
		{
			//Regular command with arguments
			vessel->which_command = NORMAL; 
			int count =0;
			
			argument = strtok(raw, " ");
			strcpy(vessel->exec, "/bin/");
			strcat(vessel->exec, argument);
		        vessel->args[count++] = argument;
		        	
			strcpy(vessel->args[0], argument);  
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
			       	//problemuoccurs here (possible argument is changed)
				//strcpy(placeholder, argument); 	
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
		// int fd;
		char *command;
		char *nameOfOutputFile;
		vessel->which_command = REDIRECT_NORMAL; 

		command = strtok_r(raw, ">", &raw);
		
		argument = strtok(command, " ");
		strcpy(vessel->exec, "/bin/");
		strcat(vessel->exec, argument);
		vessel->args[count++] = argument;
		        	
		strcpy(vessel->args[0], argument);  
		argument = strtok(NULL, " ");
		while(argument != NULL)
		{
			vessel->args[count++] = argument;
			argument = strtok(NULL, " "); 	
		}

			// pointing output file name
		nameOfOutputFile = strtok_r(raw, ">", &raw);  
		strcpy(vessel->output_file, nameOfOutputFile);      	
		// printf("%s\n", nameOfOutputFile);

		

	}
}

void pipeline(cmd_t* process1, cmd_t* process2)
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
		execv(process1->exec, process1->args); 
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
		execv(process2->exec, process2->args); 	

	}

}


void execute_command(cmd_t* cmd)
{
	int fd;

	if(cmd == NULL)
		fprintf(stderr, "NULL pointer passed in !");
       	
	switch(cmd->which_command)
	{
		case NORMAL:
			execv(cmd->exec, cmd->args); 	
			break; 
		case REDIRECT_NORMAL:
			// redirect code
			 
			fd = open(cmd->output_file, O_WRONLY | O_CREAT, 0644);	
			dup2(fd, STDOUT_FILENO);
			close(fd);
			execv(cmd->exec, cmd->args);
		    break; 
		case REDIRECT_APPEND:
			break; 
	        case PIPE_TWO: 
			pipeline(cmd->pipe_cmds[0],cmd->pipe_cmds[1]);  
			break; 
		case PIPE_THREE: 
			break; 
		case SLS: 
			break; 
		case FILES:
			break; 
		default: 
			break; 

	}

		
}





















