#ifndef CMDPARSER_H
#define CMDPARSER_H
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#define MAX_BUFFER 512
#define MAX_ARGS 16
#define DEFAULT_SIZE 100   
#define MAX_PIPE_SIZE 3

/*This structure handles the raw input from the terminal *
 *and holds the results of the parser function.          */
enum CMD_TYPE {NORMAL, REDIRECT_NORMAL, REDIRECT_APPEND, PIPE_TWO, PIPE_THREE, SLS, PWD, CD, EXIT}; 

typedef struct cmd_t 
{
	enum CMD_TYPE which_command; 
	char raw_input[MAX_BUFFER];
    char exec[DEFAULT_SIZE];		 	
	char* args[MAX_ARGS];

	/*Used to hold output file name redirection */
	char output_file[DEFAULT_SIZE]; 

	/*Pipe command variables*/
	unsigned mallocs; 
	int child1_status, child2_status; 
	struct cmd_t* pipe_cmds[MAX_PIPE_SIZE];

	/*Current working directory*/
	char cwd[DEFAULT_SIZE];	

	/*Note args[1] is pointing to path for cd */

	/*Useful for not printing +completed */
	int parser_error; 
	
	/*Specfically for tester*/
	int command_finished; 

} cmd_t; 

void cmd_parser(cmd_t* vessel, char* raw);
void handle_normal(cmd_t* vessel, char* raw); 
void handle_redirects(cmd_t* vessel, char* raw);
void handle_pipes(cmd_t* vessel, char* raw);
void handle_errors(cmd_t* vessel, char* raw);
void execute_command_c(cmd_t* cmd);
void execute_command_p(cmd_t* cmd); 
void execute_sls(cmd_t* cmd); 
void print_main(cmd_t* cmd, int status); 

#endif 
