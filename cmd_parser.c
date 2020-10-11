#include "cmd_parser.h"


void cmd_parser(cmd_t& vessel, char* raw)
{
	
	char * argument; 
	/* Get rid of '\n' at the end of the command */
	ui  = strchr(raw, '\n');        //ui points to '\n'
        if (ui)
        	*ui = '\0';
	
	strcpy(vessel.raw_input, raw); 
	if(strchr(raw, '>') == NULL)
	{
		if(strchr(raw, '|') == NULL)
		{
			//Regular command
			strcpy(vessel.exec, strtok(raw, " "));  
			argument = strtok(NULL, " ");
		        while(argument != NULL)
			{
				
			}	

		}		


	}

	else
	{

	}















	return vessel; 

}
