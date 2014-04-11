#!/bin/sh

gcc -fPIC -g -c sysvipc.c
gcc -fPIC -g -c posixipc.c

gcc -g -shared -Wl,-soname,posixstuff.so.1 \
	-o posixstuff.so.1 posixipc.o -lc

gcc -g -shared -Wl,-soname,sysvstuff.so.1 \
	-o sysvstuff.so.1 sysvipc.o -lc