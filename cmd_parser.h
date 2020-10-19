#ifndef CMDPARSER_H
#define CMDPARSER_H
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#define MAX_BUFFER 512
#define MAX_ARGS 16
#define DEFAULT_SIZE 100   
#define MAX_PIPE_SIZE 3

/*This structure handles the raw input from the terminal *
 *and holds the results of the parser function.          */

enum CMD_TYPE {NORMAL, REDIRECT_NORMAL, REDIRECT_APPEND, PIPE_TWO, PIPE_THREE, SLS, PWD, CD}; 

typedef struct cmd_t 
{
	
	enum CMD_TYPE which_command; 

	char raw_input[MAX_BUFFER];
        char exec[DEFAULT_SIZE];		 	
	char* args[MAX_ARGS];
	
	//handling redirection to output file 
	char output_file[DEFAULT_SIZE]; 		
	
	//handling pipes
	unsigned mallocs; 
	struct cmd_t* pipe_cmds[MAX_PIPE_SIZE];
	
	//file commands
	char cwd[DEFAULT_SIZE];	
	/*Note args[1] is pointing to path for cd*/

} cmd_t; 


void cmd_parser(cmd_t* vessel, char* raw); 
void execute_command_c(cmd_t* cmd);
void execute_command_p(cmd_t* cmd); 
void execute_sls(cmd_t* cmd); 



#endif 
