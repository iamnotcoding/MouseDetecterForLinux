CFLAGS=-g -std=c11 -Wall -Wextra -Wpedantic -O0 -pthread
CC=gcc

all:	MouseDetecter.o
	${CC} main.o -o MouseDetecter ${CFLAGS}
main.o:	main.c
	${CC} -c main.c -o main.o ${CFLAGS}
clean:
	rm  MouseDetecter main.o
install :
	cp MouseDetecter /usr/bin/
uninstall	:
	rm /usr/bin/MouseDetecter
