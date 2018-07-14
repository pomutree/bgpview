/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1997/12                         */
/*                                                              */
/* IPv6 Routeing Information Definition                         */
/****************************************************************/
/* route_ipv6.h,v 1.1 2003/03/31 05:51:04 kuniaki Exp */

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

#ifndef _ROUTE_IPV6_H
#define _ROUTE_IPV6_H

#include <sys/time.h>

/*
#include "bgp.h"
#include "route.h"
*/

#define IPV6_ADDR_LEN	16

typedef struct {
	char		src_afi;
        char		source[IPV6_ADDR_LEN];
        struct timeval  lastupdate;
        struct timeval  firstupdate;
        char		prefix[IPV6_ADDR_LEN];
        u_char          length;
        u_short         attribute_flag;
        char            attr_type;
        char            transitive;
        char            partial;
        char            origin_type;
        char            path_type;
        char            *pathset_ptr;
	char		nexthop_afi;
        char		next_hop[IPV6_ADDR_LEN];
        u_long          metric;
        u_long          local_perf;
        u_short         aggregate_as;
        net_ulong       aggregate_address;
        u_long          communities[MAXCOMMUNITYLIST];
        int             comlist_cnt;
        net_ulong       originator_id;
        net_ulong       cluster_list[MAXCLUSTERLIST];

#ifdef RTHISTORY
        RT_HISTORY      *history;
        RT_HISTORY      *histend;
        boolean         active;
#endif /* RTHISTORY */

} ROUTE_INFO_V6;

typedef struct TMP_V6ROUTE_LIST {
	struct TMP_V6ROUTE_LIST		*next;
	char				v6prefix[IPV6_ADDR_LEN];
	u_char				length;
} V6ROUTE_LIST;

typedef struct {
	char		nofprefix;
	char		nexthop[IPV6_ADDR_LEN];
	V6ROUTE_LIST	*preflist;
} V6ROUTE_BUF;

int     rthist_clean_v6;
time_t  cleaned_time_v6;


#endif /* _ROUTE_IPV6_H */
