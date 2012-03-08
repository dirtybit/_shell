all: myshell.c
	gcc -o myshell myshell.c -lreadline -I./

debug: myshell.c
	gcc -o myshell myshell.c -lreadline -I./ -g

clean:
	rm *.o myshell