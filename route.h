/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1997/12                         */
/*                                                              */
/* Routeing Information Update Routine                          */
/****************************************************************/
/* route.h,v 1.1 2003/03/31 05:51:04 kuniaki Exp */

/*
 * Copyright (c) 1998-2002 Internet Initiative Japan Inc.
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

#ifndef _ROUTE_H
#define _ROUTE_H

#include <sys/time.h>
/* #include "bgp.h" */
#ifdef IRRCHK
#include "irrdif.h"
#endif

/* Route Information Definition */
/* Attribute Types */
#define OPTIONAL	1
#define WELL_KNOWN	0

/* Transitive type */
#define TRANSITIVE	1
#define NON_TRANSITIVE	0

/* Partial Type */
#define PARTIAL		1
#define COMPLETE	0

/* Origin Type */
#define ORIGIN_IGP		0
#define ORIGIN_EGP		1
#define ORIGIN_INCOMPLETE	2

/* AS Path Segment Type */
#define AS_SET			1
#define AS_SEQUENCE		2
#define AS_CONFED_SET		3
#define AS_CONFED_SEQUENCE	4

#define PATHSETSIZE             512
#define MAXCLUSTERLIST		96
#define MAXCOMMUNITYLIST	96

/* SET Attribute Flags */
#define SET_ORIGIN		0x0001
#define SET_PATHTYPE		0x0002
#define SET_NEXTHOP		0x0004
#define SET_METRIC		0x0008
#define SET_LOCALPREF		0x0010
#define SET_ATOMICAGGREGATE	0x0020
#define SET_AGGREGATOR		0x0040
#define SET_COMMUNITIES		0x0080
#define SET_ORIGINATOR		0x0100
#define SET_CLUSTER_LIST	0x0200

/* Attribute Flag Control Macros */
#define ATRFL_SET(n, p)		((p) |= (n))
#define ATRFL_CLR(n, p)		((p) &= ~(n))
#define ATRFL_ISSET(n, p)	((p) & (n))
#define ATRFL_ZERO(p)		((p) = 0x0000))

#ifdef RTHISTORY
/* RTHISTORY Clearing Flag */
#define RTHIST_NOTCLEANING	0
#define RTHIST_CLEANING		1

/* RTHISTORY Type */
#define UPDATE		0
#define WITHDRAW	1

typedef struct _HIST {
	struct _HIST	*next;
	struct _HIST	*before;
	char		type;
	struct timeval	update;
} RT_HISTORY;

int	rthist_clean;
time_t	cleaned_time;
#endif /* RTHISTORY */

typedef struct {
	u_long		source;
	struct timeval	lastupdate;
	struct timeval  firstupdate;
	net_ulong	prefix;
	u_char		length;
	u_short		attribute_flag;
	char		attr_type;
	char		transitive;
	char		partial;
	char		origin_type;
	char		path_type;
	char		*pathset_ptr;
	/* char		path_set[PATHSETSIZE]; */
	net_ulong	next_hop;
	u_long		metric;
	u_long		local_perf;
	u_short		aggregate_as;
	net_ulong	aggregate_address;
	u_long		communities[MAXCOMMUNITYLIST];
	int		comlist_cnt;
	net_ulong	originator_id;
	net_ulong	cluster_list[MAXCLUSTERLIST];

#ifdef RTHISTORY
	RT_HISTORY	*history;
	RT_HISTORY	*histend;
	boolean		active;
#endif /* RTHISTORY */

#ifdef IRRCHK
	IRR_ENTRY	*irr;
#endif

} ROUTE_INFO;

#endif /* _ROUTE_H */
