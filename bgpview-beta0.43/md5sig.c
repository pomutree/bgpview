/****************************************************************/
/* BGPView                                                      */
/*   TCP MD5 Support Code                                       */
/*   However, this code support one way. it depends on FreeBSD  */
/*                                                              */
/* Programmed By K.Kondo Intec NetCore, Inc. 2004/06            */
/****************************************************************/
/* md5sig.c,v 1.2 2004/06/24 07:59:51 kuniaki Exp */

/*
 * Copyright (c) 2004 Intec NetCore, Inc.
 * All rights reserved.
 */

#ifdef TCP_MD5SIG

#include "md5sig.h"

/* Set Socket option for TCP/MD5 */
int sockopt_tcpmd5(int pn, int enable)
{
	int ret;

	ret = setsockopt(peer[pn].soc, IPPROTO_TCP, TCP_MD5SIG,
			(void *)&enable, sizeof(int));
	if (ret < 0) {
#ifdef DEBUG
		printf("DEBUG: MD5 Signature Set Fail\n");
#endif
		return(1);
	}
	return(0);
}

/* Set password */
int md5passwd_set(int pn)
{
	if (peer[pn].md5sig == false) {
		return(1);
	}

	/* Need to do PF_Key operation here to add an SA entry */

	if (peer[pn].soc != 0) {
		sockopt_tcpmd5(pn, TCP_SIG_SPI_BASE + peer[pn].port);
	} else {
		/* Peer Not Active */
		return(1);
	}
	return(0);
}

/* Unset Password */
int md5passwd_unset(int pn)
{
	if (peer[pn].soc != 0) {
		sockopt_tcpmd5(pn, 0);
	} else {
		/* Peer Not Active */
		return(1);
	}
	return(0);
}

#endif /* TCP_MD5SIG */

