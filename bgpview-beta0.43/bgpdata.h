/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1997/12                         */
/*                                                              */
/* BGP Message Inormations                                      */
/****************************************************************/
/* bgpdata.h,v 1.2 2003/09/25 02:04:06 kuniaki Exp */

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

#ifndef _BGPDATA_H
#define _BGPDATA_H

#include <sys/time.h>
#include <time.h>

#include "bgp.h"
#include "bgplog.h"


#define INFO_HEAD "Time                Message Update  Notific Open    Keepali Prefix  Withdra Totalpr\n"
#define INFO_FORM "%s %7lu %7lu %7lu %7lu %7lu %7lu %7lu %7lu\n"

#define DEF_INT	60

typedef struct {
	u_long		msgcnt;
	u_long		updatecnt;
	u_long		notificationcnt;
	u_long		opencnt;
	u_long		keepalivecnt;
	u_long		prefixcnt;
	u_long		withdrawcnt;
	u_long		totalprefix;
	int		interval;
	boolean		infostatus;
	char		fname[FNAMESIZE];
	struct timeval	update;
} BGP_INFO;

extern BGP_INFO bgpinfo[];

extern int write_info(/*pn*/);
extern void clear_info(/*pn*/);
extern void recount_info(/*pn*/);
extern void recount_all();

#endif /* _BGPDATA_H */
