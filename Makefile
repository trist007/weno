CFLAGS=-Wall -g3

all: weno

weno: 
	cc $(CFLAGS) main.c functions.c -o weno

clean:
	rm -rf weno
