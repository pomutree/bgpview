/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 2000/03                         */
/*                                                              */
/* Route Update RTT Estimation                                  */
/****************************************************************/
/* routertt.h,v 1.1 2003/03/31 05:51:04 kuniaki Exp */

/*
 * Copyright (c) 1998-2000 Internet Initiative Japan Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistribution with functional modification must include
 *    prominent notice stating how and when and by whom it is
 *    modified.
 * 3. Redistributions in binary form have to be along with the source
 *    code or documentation which include above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 4. All commercial advertising materials mentioning features or use
 *    of this software must display the following acknowledgement:
 *      This product includes software developed by Internet
 *      Initiative Japan Inc.
 *
 * THIS SOFTWARE IS PROVIDED BY ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 */

#ifndef _ROUTERTT_H
#define _ROUTERTT_H

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
#include "bgp.h"
#include "bvcommand.h"
#include "bvshell.h"
#include "timer.h"

#define RTTCHECK_MAX	5

#define RTR_NOTACTIVE	0
#define RTR_READY	1
#define RTR_SENT_UPD	2
#define RTR_RECV_UPD	3
#define RTR_SENT_WDN	4
#define RTR_RECV_WDN	5
#define RTR_FINISHED	6

typedef struct {
	int		vtyno;
	int		timer_id;
	net_ulong	neighbor;
	net_ulong	prefix;
	int		prefixlen;
	int		timeout;
	int		status;
	struct timeval	upd_injecttime;
	struct timeval	upd_receivetime;
	struct timeval	wdn_injecttime;
	struct timeval	wdn_receivetime;
} ROUTERTT;

extern ROUTERTT	routertt[];

extern int 	init_routertt();
extern int 	newcheckrtt();
extern void 	recvprefix();
extern void 	routerttexpier();

#endif /* _ROUTERTT_H */
