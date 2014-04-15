CC=gcc 
CFLAGS=-Wall

all: main sysvstuff.so.1 posixstuff.so.1

main: main.o arghandle.o dynlib.o helper.h -lrt -lpthread -ldl

sysvstuff.so.1: sysvipc.o
	gcc -g -shared -Wl,-soname,sysvstuff.so.1 -o sysvstuff.so.1 sysvipc.o -lc

sysvipc.o: sysvipc.c sysvipc.h
	gcc -fPIC -g -c sysvipc.c;

posixstuff.so.1: posixipc.o
	gcc -g -shared -Wl,-soname,posixstuff.so.1 -o posixstuff.so.1 posixipc.o -lc

posixipc.o: posixipc.c posixipc.h
	gcc -fPIC -g -c posixipc.c;

	