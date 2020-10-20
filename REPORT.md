# Project 1
### Authors: 

Ammon Bhatti, arbhatti [at] ucdavis [dot] edu 

Benjamin Sher, bosher [at] ucdavis [dot] edu

Running our shell inside of a while loop allowed us to ask the user for a command, 
execute it, and then ask the user for another command repeatedly. The while loop 
uses fork() and executes a command from inside a child where we can use execv(). 
After getting the command from the user input we call our parser function. The 
parser function seperates the command and the arguments according to the project 
specifications. The parser function uses a struct in order to store the executable, 
arguments, input, and output file. The parser struct also keeps track of the 
command type. Our execute_command functions use the information in the parser 
struct to execute the correct commands. 
