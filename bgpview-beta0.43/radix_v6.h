/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 2002/04                         */
/*                                                              */
/* Radix Tree Routine for IPv6              			*/
/****************************************************************/
/* radix_v6.h,v 1.1 2003/03/31 05:51:04 kuniaki Exp */

/*
 * Copyright (c) 2002 Internet Initiative Japan Inc.
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

#ifndef _RADIX_V6_H
#define _RADIX_V6_H

#include <stdlib.h>
#include <string.h>
#include "bgp.h"
#include "radix.h"
#include "route_ipv6.h"
#include "bgplog.h"
#include "bvshell.h"

typedef struct _R_LIST_V6 {
        struct _R_LIST_V6  *next;
	ROUTE_INFO_V6      *route_data;
} R_LIST_V6;


typedef struct _RADIX_V6_T {
	struct _RADIX_V6_T	*upleaf;
	struct _RADIX_V6_T	*r_leaf;
	struct _RADIX_V6_T	*l_leaf;
	int			check_bit;
	char			key[IPV6_ADDR_LEN];
	u_char			masklen;
	R_LIST_V6		*rinfo;
} RADIX_V6_T;

RADIX_V6_T	*radixv6_top;
u_long	nodes_v6;
u_long	routes_v6;
u_long	prefixes_v6;

#ifdef RTHISTORY
extern int rtclear_id_v6;
#endif /* RTHISTORY */

extern int 		init_radix_v6();
extern u_long	 	radix_prefixes_v6();
extern u_long 		radix_routes_v6();
extern u_long 		radix_nodes_v6();
extern int 		add_route_v6(/* ROUTE_INFO_V6 * */);
extern RADIX_V6_T 	*show_route_all_v6(/* RADIX_V6_T *curnode */);
extern int 		withdraw_peer_v6(/* int pn */);
extern RADIX_V6_T	*search_route_v6(/* char *prefix, int masklen */);

#ifdef RTHISTORY
extern RADIX_V6_T	*clear_rt_history_v6(/* RADIX_V6_T *curnode, int *upcode */);
extern void		rthisttimerclear_v6();
extern int		start_rthistclear_timer_v6();
#endif /* RTHISTORY */

#endif /* _RADIX_H */
