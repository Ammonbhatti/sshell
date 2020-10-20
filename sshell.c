#include "cmd_parser.h"

int main(void)
{
	while(1)
	{ 
		int parent_cmd =0;
		char raw_input[MAX_BUFFER]; 
		cmd_t parser;
		/*Initialize args pointer array*/
		for(int i =0; i<MAX_ARGS; ++i) 
			parser.args[i] = NULL; 

		printf("sshell@ucd$ "); 
		fflush(stdout); 
		if(!fgets(raw_input, MAX_BUFFER, stdin))
		{
			fprintf(stderr, "Error: input error occured");
		       	continue; 	
		}
		strcpy(parser.raw_input, raw_input); 
		cmd_parser(&parser,raw_input); 
		if(parser.which_command == CD || parser.which_command == SLS)
			parent_cmd=1; 
		if(parser.parser_error)
			continue; 
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
			if(parent_cmd)
				/*sls, cd*/
				execute_command_p(&parser);
			/*Prints based on different cases */
			print_main(&parser, status); 
		}
		for(unsigned i = 0; i<parser.mallocs; i++)
			free(parser.pipe_cmds[i]); 
		if(parser.which_command == EXIT)
			break; 
	}
	return 0; 
}
