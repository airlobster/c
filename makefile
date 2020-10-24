
DATE=$(shell date)
AUTHOR=${USER}
BASE=c
OUTNAME=c
CFLAGS=-I.  -Wall
GCC=gcc $(CFLAGS)

# RELEASE
release:	CFLAGS+=-Ofast
release:	version link
	strip $(BASE)
	rm -f *.o

# DEBUG
debug:	CFLAGS+=-g -D_DEBUG_
debug:	OUTNAME=$(BASE)_d
debug:	version link

# LINK
link:	_version.o a_debug.o buffer.o calc.o calcex.o childproc.o dlist.o fstack.o hashtable.o lexer.o main.o userinput.o utils.o 
	rm -f $(OUTNAME)
	$(GCC) -o $(OUTNAME) -lreadline *.o

# COMPILE

_version.o:	_version.c _version.h
	$(GCC) -c _version.c

a_debug.o:	a_debug.c a_debug.h
	$(GCC) -c a_debug.c

buffer.o:	buffer.c buffer.h
	$(GCC) -c buffer.c

calc.o:	calc.c calc.h
	$(GCC) -c calc.c

calcex.o:	calcex.c calcex.h
	$(GCC) -c calcex.c

childproc.o:	childproc.c childproc.h
	$(GCC) -c childproc.c

dlist.o:	dlist.c dlist.h
	$(GCC) -c dlist.c

fstack.o:	fstack.c fstack.h
	$(GCC) -c fstack.c

hashtable.o:	hashtable.c hashtable.h
	$(GCC) -c hashtable.c

lexer.o:	lexer.c lexer.h
	$(GCC) -c lexer.c

main.o:	main.c
	$(GCC) -c main.c

userinput.o:	userinput.c userinput.h
	$(GCC) -c userinput.c

utils.o:	utils.c utils.h
	$(GCC) -c utils.c


# UTILITIES
clean:
	rm -f *.o

version:
	@echo "const char __version[]=\"$(DATE)\";\nconst char __author[]=\"$(USER)\";" > _version.c
	@echo "#ifndef _VERSION_H_\n#define _VERSION_H_\nextern const char __version[];\nextern const char __author[];\n#endif\n" > _version.h

