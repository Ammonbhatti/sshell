# --Makefile--

all: sshell

sshell: sshell.c
	gcc -g -Wall -Wextra -Werror sshell.c -o sshell

clean: 
	rm -f *.o sshell
