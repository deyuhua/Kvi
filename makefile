all:
	cc kvi.c cmd.c frame.c screen.c -g -Wall -Werror -o kvi

clean:
	rm kvi
