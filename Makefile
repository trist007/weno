CFLAGS=-Wall -g3

all: main

main: 
	cc $(CFLAGS) main.c functions.c ncurses-main.c -lncurses -lcrypto -o main

clean:
	rm -rf main
