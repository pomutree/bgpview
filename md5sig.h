/****************************************************************/
/* BGPView                                                      */
/*   TCP MD5 Support Code					*/
/*   However, this code support one way. it depends on FreeBSD  */
/*                                                              */
/* Programmed By K.Kondo Intec NetCore, Inc. 2004/06            */
/****************************************************************/
/* md5sig.h,v 1.2 2004/06/24 07:59:51 kuniaki Exp */

/*
 * Copyright (c) 2004 Intec NetCore, Inc.
 * All rights reserved.
 */

#ifdef TCP_MD5SIG

#ifndef _MD5SIG_
#define _MD5SIG_

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#include "bvtypes.h"

#define MD5SIG_PASWORD_MINLEN	1
#define MD5SIG_PASWORD_MAXLEN	80

#ifndef TCP_SIG_SPI_BASE
#define TCP_SIG_SPI_BASE	1000 /* this will go away */
#endif

#include "bgp.h"

int sockopt_tcpmd5(int pn, int enable);
int md5passwd_set(int pn);
int md5passwd_unset(int pn);

#endif /* _MD5SIG_ */

#endif /* _TCP_MD5SIG */

