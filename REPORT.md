# Project 1
### Authors: 

Ammon Bhatti, arbhatti [at] ucdavis [dot] edu 

Benjamin Sher, bosher [at] ucdavis [dot] edu

### Overview

Running our shell inside of a while loop allowed us to ask the user for a
command, execute it, and then ask the user for another command
repeatedly. The while loop uses fork() and executes a command from inside a
child where we can use execv(). If execv() was used inside of the parent then 
our parent program would never return to the shell. We use a struct called
'vessel' that is used to store the input from the user. After storing the raw
input from the user, a parser function is called to parse the raw input. The
parser function has access to our vessel where the input is stored. The parser
struct also keeps track of the command type. The parser function first checks to
see if there is a pipe command or a redirect command, and sets the appropriate
flags inside the vessel. If the command does not contain the pipe or redirect
commands then our fucntion handle_normal is called with the vessel passed in as
a paremeter. There are seperate functions that the parser will call in order to
handle pipes and redirects. After the input is parsed our execute_command_c and
execute_command_p will run depending on the depend command type. Most
commands are ran within execute_command_c which is for the child, but SLS and CD
commands are within execute_command_p which is for the parent.

### Testing

Our testing was first done manually on our local machines using the examples
that were given in the project file. After passing these tests on our local
machines, we ran the same tests manually on the CSIF machines. Testing on the
CSIF machines provided us a way to have a common point of reference with our
peers. Once we were passing the examples in the project file we started testing
using the tester.sh file that was provided. 
