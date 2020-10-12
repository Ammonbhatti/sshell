#include "cmd_parser.h"

//Pass struct by reference
void cmd_parser(cmd_t* vessel, char* raw)
{
	
	char *argument, *ui;		//used to process args including argv[0]

	/* Get rid of '\n' at the end of the command */
	ui  = strchr(raw, '\n');        //ui points to '\n'
        if (ui)
        	*ui = '\0';
	
	if(strchr(raw, '>') == NULL)
	{
		if(strchr(raw, '|') == NULL)
		{
			//Regular command with arguments
			int count =0;
			
			argument = strtok(raw, " ");
			strcpy(vessel->exec, "/bin/");
			strcat(vessel->exec, argument);
		        vessel->args[count++] = argument;
		        	
			strcpy(vessel->args[0], argument);  
			argument = strtok(NULL, " ");
		        while(argument != NULL)
			{
				vessel->args[count++] = argument;
				argument = strtok(NULL, " "); 	
			}	

		}
		else
		{
			//Handle pipes
		}		

	}

	else
	{
		//Handle file redirection
	}
}
