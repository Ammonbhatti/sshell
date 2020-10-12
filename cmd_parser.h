#ifndef CMDPARSER_H
#define CMDPARSER_H
#include <string.h>
#include <stdbool.h>
#define MAX_BUFFER 512
#define MAX_ARGS 17
#define DEFAULT_SIZE 100      

/*This structure handles the raw input from the terminal *
 *and holds the results of the parser function.          */

typedef struct pipe_t 
{
	unsigned count; 
	char first_raw[DEFAULT_SIZE]; 
	char second_raw[DEFAULT_SIZE]; 
	char third_raw[DEFAULT_SIZE]; 

} pipe_t;

typedef struct cmd_t 
{
	
	char raw_input[MAX_BUFFER];
        char exec[DEFAULT_SIZE];		 	
	char* args[MAX_ARGS];
	
	//handling redirection to output file
	bool output_file_flag; 
	char output_file[DEFAULT_SIZE]; 		
	
	//handling pipes 
	bool pipe_flag; 
	pipe_t pipe_struct; 


} cmd_t; 


void cmd_parser(cmd_t* vessel, char* raw); 



#endif 
