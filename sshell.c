#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>   
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include "cmd_parser.h"		//contains parsing utilities
#define MAX_BUFFER 512
#define MAX_ARGS 17		//Includes argv[0]

int main(void)
{



	while(1)
	{

		
		cmd_t parser;
		for(int i =0; i<MAX_ARGS; ++i)		//fill arguments with zeroes 
			parser.args[i] = NULL; 

		printf("sshell@ucd$ "); 
		fflush(stdout); 

		fgets(parser.raw_input, 512, stdin);
		//ret_cmd = process_command(user_input);
		//Currently Phase1: basic commands
		
		cmd_parser(&parser, parser.raw_input); 
		 	
		if(!strcmp(parser.args[0], "exit"))
		{
			fprintf(stderr, "Bye...\n");
			break;  
		}

		int pid = fork(); 
		if (pid == 0) 
		{
			/* Child */
			execv(parser.exec, parser.args);
			perror("execv");
			exit(1);
		} 
		else if (pid > 0) 
		{
			/* Parent */
			int status;
			waitpid(pid, &status, 0);
			fprintf(stderr, "\n+ Completed '%s' [%d]\n",
					parser.raw_input, WEXITSTATUS(status));
		}

	}

	return 0; 
}
