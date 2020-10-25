#!/usr/bin/env make -f

DATE=$(shell date +%Y%m%d%H%M%S)
AUTHOR=adidegani
BASE=c
GCC=gcc -I.
CFLAGS=-Wall
LFLAGS=-lreadline

release:	CFLAGS+=-Ofast
release:	version link
	strip $(BASE)
	rm -f *.o

debug:	CFLAGS+=-D_DEBUG_
debug:	BASE=c_d
debug:	version link

link:	_version.o a_debug.o buffer.o calc.o calcex.o childproc.o dlist.o fstack.o hashtable.o lexer.o main.o userinput.o utils.o 
	$(GCC) $(CFLAGS) $(LFLAGS) -o $(BASE) *.o

_version.o:	_version.c 
	$(GCC) $(CFLAGS) -c _version.c

a_debug.o:	a_debug.c utils.h a_debug.h
	$(GCC) $(CFLAGS) -c a_debug.c

buffer.o:	buffer.c utils.h buffer.h
	$(GCC) $(CFLAGS) -c buffer.c

calc.o:	calc.c a_debug.h fstack.h buffer.h dlist.h lexer.h calc.h
	$(GCC) $(CFLAGS) -c calc.c

calcex.o:	calcex.c buffer.h hashtable.h a_debug.h childproc.h calcex.h calc.h
	$(GCC) $(CFLAGS) -c calcex.c

childproc.o:	childproc.c childproc.h
	$(GCC) $(CFLAGS) -c childproc.c

dlist.o:	dlist.c dlist.h
	$(GCC) $(CFLAGS) -c dlist.c

fstack.o:	fstack.c utils.h fstack.h
	$(GCC) $(CFLAGS) -c fstack.c

hashtable.o:	hashtable.c utils.h hashtable.h
	$(GCC) $(CFLAGS) -c hashtable.c

lexer.o:	lexer.c a_debug.h dlist.h fstack.h buffer.h lexer.h
	$(GCC) $(CFLAGS) -c lexer.c

main.o:	main.c utils.h userinput.h calc.h calcex.h calc.h _version.h
	$(GCC) $(CFLAGS) -c main.c

userinput.o:	userinput.c userinput.h
	$(GCC) $(CFLAGS) -c userinput.c

utils.o:	utils.c utils.h
	$(GCC) $(CFLAGS) -c utils.c

clean:
	rm -f *.o $(BASE) $(BASE)_d

version:
	@echo "const char* appversion() {return \"$(DATE)\";}\nconst char* appauthor() { return \"$(USER)\";}" > _version.c
	@echo "#ifndef _VERSION_H_\n#define _VERSION_H_\nconst char* appversion();\nconst char* appauthor();\n#endif\n" > _version.h
