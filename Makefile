CFLAGS=-g -std=c11 -pthread
CC=gcc

all:	main.o
	${CC} main.o -o main ${CFLAGS}
main.o:	main.c
	${CC} -c main.c -o main.o ${CFLAGS}
clean:
	rm main main.o
install :
	cp main /usr/bin/main
uninstall	:
	rm /usr/bin/main
