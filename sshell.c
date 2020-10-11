#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>   
#include <unistd.h>
#include <strings.h>
#include <string.h>
#define MAX_BUFFER 512
#define MAX_ARGS 17		//Includes argv[0]

int main(void)
{



	while(1)
	{

		char user_input[MAX_BUFFER]; 
		char cmd[MAX_BUFFER] = "/bin/";  
		char* args[MAX_ARGS] = {0}; 
		char* ui;
		printf("sshell@ucd$ "); 
		fflush(stdout); 

		fgets(user_input, 512, stdin);
		//ret_cmd = process_command(user_input);
		//Currently Phase1: basic commands
		ui  = strchr(user_input, '\n');
		if (ui)
                        *ui = '\0';
		strcat(cmd, user_input);
	       	args[0] = cmd; 	
		if(!strcmp(user_input, "exit"))
		{
			fprintf(stderr, "Bye...\n");
			break;  
		}

		unsigned pid = fork(); 
		if (pid == 0) 
		{
			/* Child */
			execv(cmd, args);
			perror("execv");
			exit(1);
		} 
		else if (pid > 0) 
		{
			/* Parent */
			int status;
			waitpid(pid, &status, 0);
			printf(stderr, "\n+ Completed '%s' [%d]\n",user_input, WEXITSTATUS(status));
		}

	}

	return 0; 
}
