CC=gcc
PROJECTDIR=dsync
INSTALLDIR=/bin/dsync
CFLAGS=
DEBUGCFLAGS=-g

default: sync.c
	$(CC) -o $(PROJECTDIR) $^ $(CFLAGS)

debug: sync.c
	$(CC) -o $(PROJECTDIR) $^ $(DEBUGCFLAGS)

install:
	make default
	sudo cp $(PROJECTDIR) $(INSTALLDIR)
