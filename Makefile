# --Makefile--

all: sshell

sshell: sshell.o cmd_parser.o cmd_parser.h
	gcc -O2 -Wall -Wextra -Werror sshell.o cmd_parser.o -o sshell

sshell.o: sshell.c cmd_parser.h
	gcc -c -O2 -Wall -Wextra -Werror sshell.c

cmd_parser.o: cmd_parser.c cmd_parser.h
	gcc -c -O2 -Wall -Wextra -Werror cmd_parser.c

clean: 
	rm -f *.o sshell
