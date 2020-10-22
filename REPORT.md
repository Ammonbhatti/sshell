# Project 1
## Authors: 

Ammon Bhatti, arbhatti [at] ucdavis [dot] edu 

Benjamin Sher, bosher [at] ucdavis [dot] edu

## Overview
We split our code into 3 files, sshell.c, cmd_parser.h, and cmd_parser.c.
sshell.c contains the main function and the input, execute while loop. The
other two are used to define and run, structures and functions to parse and
execute the shell commands. Running our shell inside of a while loop allowed
us to ask the user for a command, execute it, and then ask the user for another
command repeatedly. Since the tester.sh inputs the commands into stdin as a
single string delimited by '\n' characters we decided to split the command
string into command tokens and process each of the commands individually on
every iteration of the inner while loop. The nested while loop uses fork() and
executes a command from inside a child where we can use execvp() function. If
the command isn't able to run properly in the child due to restricted access to
the terminal, like cd or sls, we simply ran the command in the parent process.
We used one big struct called 'cmd_t' to store the input from the user. After
getting the raw input from the user, a parser function (cmd_parser()) is called to
populate the struct based on the parsed raw input. The parser function checks
to see if the commmand is one of three types normal, redirect, or pipe.
Depending on which category the command falls under we call a specialized
utility function to check for parser errors and further classify which command
we are working with. There are two types of functions in cmd_parser.c command
processors and command executers. After the input is parsed our
execute_command_c and execute_command_p will run depending on the
command type. Most of the commands are run within execute_command_c which
is for the child, but SLS and CD commands are within execute_command_p which
is for the parent. These executer functions are esstially a big switch
statement that run the command that was detected in the parser. We have a
enumeration type that we made for each type of command we had  to run.
When the processing and execution is finished a print_main function is called
to print to stderr what the status of the execution was. Then the program
returns to sshell.c and another command is processed. This process repeats
until exit is inputed. 

## Data Structures

### Enumerations: 
What type of command are we executing. Used in switch statement to execute
the specific command. 
'''enum CMD_TYPE {NORMAL, REDIRECT_NORMAL, REDIRECT_APPEND, PIPE_TWO,
PIPE_THREE, SLS, PWD, CD, EXIT};'''

### Structure: cmd_t
Holds all of the relavent command information. In the case of pipe commands
it holds the an array of pointers to other other cmd_t structures. Each of the
pointers will point to a pipe sub command. cmd_parser() is called recursively
on the subcommands to figure out what type of commands they are whether
they are valid or not.
 
## Makefile
The make file builds and executable out of two targets sshell.o and
cmd_parser.o. Both of these depend on cmd_parser.h which holds
all of the declarations. 

## Testing

Initially when we tested our executable on csif everything worked correctly.
But when we used the tester.sh file a lot of the commands didn't work. And
so we looked through the tester.sh file and noticed that all of the commands
were input in stdin all at once delimited by '\n' characters. We changed our
code to process these bulk commands. Then we passed the tests. 
