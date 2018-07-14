/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1997/12                         */
/*                                                              */
/* Radix Tree Routine                          			*/
/****************************************************************/
/* radix.h,v 1.1 2003/03/31 05:51:04 kuniaki Exp */

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

#ifndef _RADIX_H
#define _RADIX_H

#include <stdlib.h>
#include <string.h>
#include "bgp.h"
#include "route.h"
/* #include "route_ipv6.h" */
#include "bgplog.h"
#include "bvshell.h"

#ifdef RTHISTORY
/** Route Delete Type **/
#define		DELETE	0
#define		DFLAG	1
#endif

typedef struct _R_LIST {
	struct _R_LIST	*next;
	ROUTE_INFO	*route_data;
} R_LIST;

typedef struct _RADIX_T {
	struct _RADIX_T		*upleaf;
	struct _RADIX_T		*r_leaf;
	struct _RADIX_T		*l_leaf;
	int			check_bit;
	net_ulong		key;
	u_char			masklen;
	R_LIST			*rinfo;
} RADIX_T;

/*
RADIX_T	*radix_top;
u_long	nodes;
u_long	routes;
u_long	prefixes;
*/
extern RADIX_T	*radix_top;
extern u_long	nodes;
extern u_long	routes;
extern u_long	prefixes;

#ifdef RTHISTORY
extern int rtclear_id;
#endif

extern int 	init_radix();
extern u_long 	radix_prefixes();
extern u_long 	radix_routes();
extern u_long 	radix_nodes();
extern int 	add_route(/* ROUTE_INFO * */);
extern RADIX_T 	*show_route_all(/* RADIX_T *curnode */);
extern int 	withdraw_peer(/* int pn */);
extern RADIX_T	*search_route(/* net_ulong prefix, int masklen */);

#ifdef RTHISTORY
extern RADIX_T	*clear_rt_history(/* RADIX_T *curnode, int *upcode */);
extern void	rthisttimerclear();
extern int	start_rthistclear_timer();
#endif


#endif /* _RADIX_H */
