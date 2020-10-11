#ifndef CMDPARSER_H
#define CMDPARSER_H
#include <string.h>
#define MAX_BUFFER 512
#define MAX_ARGS 17
#define DEFAULT_SIZE 100      

/*This structure handles the raw input from the terminal *
 *and holds the results of the parser function.          */
typedef struct cmd_t 
{
	
	char raw_input[MAX_BUFFER];
        char exec[DEFAULT_SIZE];		 	
	char *args[MAX_ARGS] = {0};
	
	//handling redirection to output file
	bool output_file_flag = false; 
	char output_file[DEFAULT_SIZE]; 		
	
	//handling pipes 
	bool pipe_flag = false; 
	pipe_t pipe_struct; 


}; 

typedef struct pipe_t 
{
	unsigned count = 0; 
	char first_raw[DEFAULT_SIZE]; 
	char second_raw[DEFAULT_SIZE]; 
	char third_raw[DEFAULT_SIZE]; 

};

cmd_t command_parser(cmd_t vessel, char* raw); 



#endif 
