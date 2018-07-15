/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1999/01                         */
/*                                                              */
/* Dummy Route Sender Header File                               */
/****************************************************************/
/* dummyroute.h,v 1.1 2003/03/31 05:51:04 kuniaki Exp */

/*
 * Copyright (c) 1998-2001 Internet Initiative Japan Inc.
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

#ifndef _DUMMYROUTE_H
#define _DUMMYROUTE_H

#include <ctype.h>

#include "bgp.h"

/* Dummy Route Inject Mode */
#define	DMY_WITHDRAW	0
#define	DMY_UPDATE	1
#define	DMY_PLAYBACK	2

/* AS Type */
#define NORMAL		0
#define INCREMENTAL	1

typedef struct {
	int		pn;
	net_ulong	neighbor;
	int		type;
	u_long		start;
	u_long		cnt;
	int		pack;
	char		aspath[PATHSETSIZE];
	u_long		current;
	int		astype;
	char		dump[128];
	FILE		*dumpfp;
} DUMMYINFO;

extern char		dmyroutef[];
extern DUMMYINFO	dmyroute;

extern void init_dummy();
extern int  send_dummy();
extern int  getdmyinfo();
extern void sigexec_getdmyinfo();


#endif /* _DUMMYROUTE_H */
