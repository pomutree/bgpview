# BGP View Makefile
# Makefile,v 1.9 2005/10/21 02:24:50 kuniaki Exp

PROGNAME	= bgpview
BVSPASSWD	= bvspasswd
VERSION		= beta0.43
DISTNAME	= $(PROGNAME)-$(VERSION)
CVSTAG		= b043
CVS_COMMAND	= cvs
#CVS_COMMAND	= cvs -d /usr/home/kuniaki/cvsroot
INSTALLCMD	= /usr/bin/install


CFILES	= bgpview.c	fsm.c		timer.c		update.c	\
	  config.c	bgplog.c	bgpdata.c	dummyroute.c	\
	  utils.c	bvshell.c	bvcommand.c	routertt.c	\
	  mailannounce.c		radix.c		mpnlri.c	\
	  radix_v6.c	bvcommand_v6.c	md5sig.c	irrdif.c

BVSPWC  = bvspasswd.c

IFILES  = bgp.h		bgpdata.h	bgplog.h	route.h		\
	  timer.h	dummyroute.h	bvshell.h	bvcommand.h	\
	  routertt.h	mailannounce.h	radix.h		utils.h		\
	  mpnlri.h	route_ipv6.h	bvtypes.h 	radix_v6.h	\
	  md5sig.h	irrdif.h

DOCUMENT= Makefile 	readme.txt 	change.txt 	readme-bvs.txt	\
	  bgpview.cfg.sample 		dummyroute.cfg.sample		\
	  readme-e.txt	readme-bvs-e.txt				\
	  bgpview.cron.sample		NOTICE

TOOL	= tool/mrtgout/mrtgout.pl			\
          tool/mrtgout/mrtgout.doc			\
	  tool/checkprefix/checkprefix.pl		\
	  tool/checkprefix/abuseprefix.dat.sample	\
	  tool/checkprefix/check.sh			\
	  tool/checkprefix/prefixcheck.doc		\
	  tool/mlcnt/mlcnt.pl				\
	  tool/mlcnt/mlcnt.doc				\
	  tool/mlcnt/makegraph.pl			\
	  tool/mlcnt/makegraph.doc			\
	  tool/sumroute/sumroute.doc			\
	  tool/sumroute/sumroute.pl			\
	  tool/historycheck/cronrefresh.sh		\
	  tool/historycheck/flapsum.pl			\
	  tool/historycheck/flapsum_v6.pl		\
	  tool/historycheck/historycheck.doc		\
	  tool/irrcheck/Makefile			\
	  tool/irrcheck/irrcheck.c			\
	  tool/irrcheck/irrcheck.h			\
	  tool/mkrtimage/README.txt			\
	  tool/mkrtimage/coord.pl			\
	  tool/mkrtimage/makerib.pl			\
	  tool/mkrtimage/mkimage.pl			\
	  tool/mkrtimage/routetoimage.sh.sample

	  

OFILES	= ${CFILES:.c=.o}

SHELL	=/bin/sh
TARCMD	=/usr/bin/tar
.SUFFIXES: .c .o

CC	= gcc

BINDIR	= /usr/local/bin
SBINDIR = /usr/local/sbin
CONFDIR = /usr/local/etc
CONFFILE= bgpview.cfg.sample

MANDIR	= /usr/local/man/man1

## FreeBSD, NetBSD
LIBS	= -lcrypt
## SunOS :: 'bind' library is required
#LIBS	= -L/usr/local/lib -lbind -lcrypt
## Solaris2
#LIBS	= -lsocket -lnsl -lresolv -lcrypt

# CFLAGS	= -DDEBUG -g -O -Wall
# CFLAGS	= -DLINUX -DHAVE_TOP -DRADIX -DDEBUG -g -O
# CFLAGS	= -DHAVE_TOP -DRADIX -DDEBUG -g -O
# CFLAGS	= -DHAVE_TOP -DRADIX -DRTHISTORY -DDEBUG -g -O
# CFLAGS	= -DHAVE_TOP -DRADIX -DRTHISTORY -DDEBUG -DGETPASS -g -O
# CFLAGS	= -DHAVE_TOP -DRADIX -DRTHISTORY -DDEBUG -DGETPASS -DTCP_MD5SIG -g -O
# CFLAGS	= -DHAVE_TOP -DRADIX -DRTHISTORY -DDEBUG -DDVERV -DTCP_MD5SIG -g -O
# CFLAGS	= -DHAVE_TOP -DRADIX -DRTHISTORY -DDEBUG -DDVERV -DTCP_MD5SIG -DIRRCHK -g -O
#CFLAGS	= -DHAVE_TOP -DRADIX -DRTHISTORY -DDVERV -DTCP_MD5SIG -DIRRCHK -g -O
CFLAGS	= -DDEBUG -DHAVE_TOP -DRADIX -DRTHISTORY -DDVERV -DTCP_MD5SIG -g -O
INCDIR	= -I./

all:		$(PROGNAME) $(BVSPASSWD)

$(PROGNAME):	$(OFILES)
		@echo "Linking ... "
		$(CC) $(CFLAGS) $(INCDIR) -o $(PROGNAME) $(OFILES) $(LIBS)
		@echo "done"

$(BVSPASSWD):	$(BVSPWDC)
		@echo "Linking BVSPasswd ..."
		$(CC) $(CFLAGS) $(INCDIR) -o $(BVSPASSWD) $(BVSPWC) utils.o $(LIBS)
		@echo "done"

depend:
		-@makedepend -- $(CFLAGS) -- $(CFILES) -f Makefile

.c.o:		$(CFILES)
		$(CC) -c $(CFLAGS) $<

install:	
		@echo "Installing $(PROGNAME) ..."
		$(INSTALLCMD) -c -m 4555 -o root -g bin $(PROGNAME) $(SBINDIR)/
		$(INSTALLCMD) -c -m 4555 -o root -g bin $(BVSPASSWD) $(BINDIR)/
		$(INSTALLCMD) -c -m 644 -o root -g bin $(CONFFILE) $(CONFDIR)/
		@echo "done."

clean:;		rm -f $(OFILES) $(PROGNAME) $(BVSPASSWD) core

tar:
	@if [ -f $(DISTNAME).tar.gz ]; then rm -f $(DISTNAME).tar.gz ; fi
	@if [ -d $(DISTNAME) ]; then rm -rf $(DISTNAME) ; fi
	@$(CVS_COMMAND) export -r $(CVSTAG) -kv -d $(DISTNAME) $(PROGNAME)
	@tar cvf - $(DISTNAME) | gzip -9 > $(DISTNAME).tar.gz
	@rm -rf $(DISTNAME)

# DO NOT DELETE THIS LINE -- make depend depends on it.
