CROSS_COMPILE=

all: aesdsocket aesdsocket.o
default: aesdsocket aesdsocket.o

aesdsocket: aesdsocket.o
	$(CROSS_COMPILE)gcc -g -Wall -Werror -I/ aesdsocket.c -o aesdsocket

aesdsocket.o: aesdsocket.c
	$(CROSS_COMPILE)gcc -g -Wall -Werror -c -o aesdsocket.o aesdsocket.c

clean:
	rm -f aesdsocket.o aesdsocket
