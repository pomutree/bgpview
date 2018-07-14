
# IrrCheck Makefile
# Makefile,v 1.1 2004/12/08 05:41:07 kuniaki Exp

PROGS	= irrcheck
VERSION = alpha0.1

CFILES	= irrcheck.c

IFILES  = irrcheck.h

DOCUMENT= Makefile

OFILES	= ${CFILES:.c=.o}

TARCMD	=/usr/bin/tar
SHELL	=/bin/sh
.SUFFIXES: .c .o

CC	= gcc

BINDIR	=/usr/local/bin

MANDIR	=/usr/local/man/man1

TARFILE = $(PROGS)-$(VERSION).tar.gz
DISTDIR = $(PROGS)-$(VERSION)

CFLAGS  = -g -O -Wall
#CFLAGS  = -g -O -Wall -DDEBUG

INCDIR	= -I./

LIB =

all:		$(PROGS)

$(PROGS):	$(OFILES)
		@echo "Linking ... "
		$(CC) $(CFLAGS) $(INCDIR) $(LIB) -o $(PROGS) $(OFILES)
		@echo "done"

.c.o:		$(CFILES)
		$(CC) -c $(CFLAGS) $<

install:	
		@echo "Installing IrrCheck ..."
		cp $(PROGS) $(BINDIR)/$(PROGS)
		@echo "done"

clean:;		rm -f $(OFILES) $(PROGS)

tar:		$(CFILES) $(IFILES) $(DOCUMENT) $(TOOL)
		@if [ -d ../$(DISTDIR) ] ; then rm -f ../$(DISTDIR); fi
		@ln -s aslookup ../$(DISTDIR)
		@echo $(TARCMD) cvzf $(DISTDIR)/$(TARFILE) "\\" > tartmp.sh
		@for file in $> ; \
		do \
		  echo $(DISTDIR)/$${file} "\\" >> tartmp.sh ; \
		done
		@echo >> tartmp.sh
		@cd .. ; sh < $(DISTDIR)/tartmp.sh
		@rm -f $(DISTDIR)/tartmp.sh

