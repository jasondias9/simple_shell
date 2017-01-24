all: simple_shell.c
	gcc -o simple_shell simple_shell.c

clean:
	rm -f simple_shell
