#include "cmd_parser.h"

int main(void)
{
	while(1)
	{ 
		char raw_input[MAX_BUFFER];
	    char* sub_command; 	
		printf("sshell@ucd$ ");
	    printf("\n");	
		fflush(stdout); 
		if(!fgets(raw_input, MAX_BUFFER, stdin))
		{
			fprintf(stderr, "Error: input error occured");
		    continue; 	
		}  
		sub_command = strtok(raw_input, "\n");
		do
		{
			cmd_t parser; 	
			int parent_cmd = 0;
			/*Initialize args pointer array*/
	        for(int i =0; i<MAX_ARGS; ++i)
			{
                parser.args[i] = NULL;
			}
	        strcpy(parser.raw_input, sub_command);
		    strcat(parser.raw_input, "\n");
			strcat(sub_command, "\n"); 	
			cmd_parser(&parser,sub_command); 
			if(parser.which_command == CD || parser.which_command == SLS)
			{
				parent_cmd=1; 
			}
			if(parser.parser_error)
			{
				continue; 
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
				if(parent_cmd)
				{
					/*sls, cd*/
					execute_command_p(&parser);
				}
				/*Prints based on different cases */
				print_main(&parser, status);
				fflush(stdout); 
			}
			for(unsigned i = 0; i<parser.mallocs; i++)
			{
				free(parser.pipe_cmds[i]); 
			}
			if(parser.which_command == EXIT)
			{
				return 0; 
			}
			sub_command = strtok(NULL, "\n"); 
			fflush(stdout); 
		}while(sub_command != NULL); 	
	}
	return 0; 
}
