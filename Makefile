all: simple_shell.c
	rm -f simple_shell
	gcc -o simple_shell simple_shell.c

clean:
	rm -f simple_shell
