/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1997/12                         */
/*                                                              */
/* Timer Routine Include File                                   */
/****************************************************************/
/* timer.h,v 1.1 2003/03/31 05:51:04 kuniaki Exp */

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

#ifndef _TIMER_H
#define _TIMER_H

#include <sys/time.h>
#include <time.h>
#include "bgp.h"
#include "bgplog.h"
#include "bvshell.h"

#define TIME_SEC_USEC   1000000
#define COMMON_ID1	0xFFFFFFFF
#define MAX_TIMER_TABLE	32
#define MAX_CRON	MAX_TIMER_TABLE
#define DEFAULT_CRONF	"/usr/local/etc/bgpview.cron"
#define CRON_QRETRY	5

typedef struct _tlist {
	struct _tlist   *prev, *next;
	struct timeval  timeout;
	net_ulong       id1;
	int             id2;
} TIMERLIST;

typedef struct {
	int	param;
	void	(*function)(int, int);
} COMMON_TIMER_LIST;

typedef struct _CRON_LIST {
	char	min[64];
	char	hour[64];
	char	day[64];
	char	month[64];
	char	year[64];
	char	command[1024];
} CRON_LIST;


/* local valiables */
char	cmd_queue[MAX_CRON][1024];
int	cron_id;
int	qretry_id;

/* common valiables */
extern CRON_LIST	*cron_list[];
extern char		cron_file[];

/* Functions */
extern void 	timer_diff(/* t1, t2, tv */);
extern void	clear_timer();
extern boolean	entry_timer();
extern int	off_timer();
extern void	check_timer(/*id1, id2, tv*/);

extern int	common_entry_timer(/*tv, param, function*/);
extern int	common_off_timer(/*common_id*/);
extern void 	common_timer_expire(/*int id2*/);

extern void	init_cron();
extern void	set_cronfile();
extern int	read_cron_file();
extern void	exec_cmdqueue();
extern void	execute_cron();

#endif /* _TIMER_H */
