#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>   
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include "cmd_parser.h"		//contains parsing utilites and macros

int main(void)
{

	while(1)
	{ 
		int parent =0; 
		cmd_t parser;
		char raw_input[MAX_BUFFER]; 
		for(int i =0; i<MAX_ARGS; ++i)		//fill arguments with zeroes 
			parser.args[i] = NULL; 

		printf("sshell@ucd$ "); 
		fflush(stdout); 

		fgets(raw_input, 512, stdin);
		strcpy(parser.raw_input, raw_input); 
		cmd_parser(&parser,raw_input); 
		if(parser.which_command == CD || parser.which_command == SLS)
			parent=1; 

		if(!strcmp(parser.args[0], "exit"))
		{
			fprintf(stderr, "Bye...\n");
			break;  
		}

		int pid = fork(); 
		if (pid == 0) 
		{
			/* Child */
			execute_command_c(&parser); 
			perror("execv");
			exit(1);
		} 
		else if (pid > 0) 
		{
			/* Parent */
			int status;
			waitpid(pid, &status, 0);
			if(parent)
				execute_command_p(&parser);

			fprintf(stderr, "\n+ Completed '%s' [%d]\n",
					parser.raw_input, WEXITSTATUS(status));
		}
		for(unsigned i = 0; i<parser.mallocs; i++)
			free(parser.pipe_cmds[i]); 

	}

	return 0; 
}
