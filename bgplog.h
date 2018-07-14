/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1998/1                          */
/*                                                              */
/* BGP Logging Routine Include File                             */
/****************************************************************/
/* bgplog.h,v 1.5 2005/05/10 03:39:45 kuniaki Exp */

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

/*
 * Copyright (c) 2003 Intec NetCore, Inc.
 * All rights reserved.
 */

#ifndef _BGPLOG_H
#define _BGPLOG_H

#include <syslog.h>
/* #include <varargs.h> */
#include <stdarg.h>
#include <stdio.h>
#include "bgp.h"
#include "timer.h"
#include "route.h"

#define LOG_ALL		-1

#define VLOG_GLOB	0x01
#define VLOG_OPEN	0x02
#define VLOG_UPDE	0x04
#define VLOG_NOTI	0x08
#define VLOG_KEEP	0x10
#define VLOG_UPAT	0x20
#define VLOG_UPDU	0x40

#define LG_SET(n, p)    ((p) |= (n))
#define LG_CLR(n, p)    ((p) &= ~(n))
#define LG_ISSET(n, p)  ((p) & (n))
#define LG_ZERO(p)	((p) = 0x00)

#define TIME_BUF 	25
#define LOGSTRSIZE	128

extern char	logstr[];
extern FILE	*sfd;

extern void restart_log();
extern void pr_log(/*typ, pn, str, timep*/);
extern void pr_log2(u_char type, int pn, int timep, char *str, ...);
extern void close_log(/**/);
extern void open_log(/*logname, pn*/);
extern void init_prefixlog(/**/);
extern int write_prefixlog(/*typ, prefix, pn*/);
extern int write_v6prefixlog(/*typ, prefix, pn*/);
extern void close_prefixlog(/**/);
extern void open_syslog(/*logname*/);

#endif	/* _BGPLOG_H */
