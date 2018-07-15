/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 2000/01                         */
/*                                                              */
/* BGPView Shell Commands Header File                           */
/****************************************************************/
/* bvcommand.h,v 1.1 2003/03/31 05:51:04 kuniaki Exp */

/*
 * Copyright (c) 2000-2001 Internet Initiative Japan Inc.
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

#ifndef _BVCOMMAND_H
#define _BVCOMMAND_H

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "bvtypes.h"
/* #include "bgp.h" */
#include "bvshell.h"
#include "bgplog.h"
#include "bgpdata.h"
#include "dummyroute.h"
#include "mailannounce.h"
#include "timer.h"

#ifdef RADIX
#include "radix.h"
#include "radix_v6.h"
#endif


/* Defines */

#define	TOPCMD	"/usr/bin/top"

#define ROUTERTT_TIMEOUT	60

/** Route History Print Command Display Type **/
#define RTV_ALL		0
#define RTV_ACTIVE	1
#define RTV_INACTIVE	2

typedef struct nexthop_list {
	struct nexthop_list	*next;
	u_long			next_hop;
	int			count;
} NEXTHOP_LIST;

/* functions */

extern int bvc_shutdown();
extern int bvc_reboot();
extern int bvc_setannouncestatus();
extern int bvc_setpeerdisable();
extern int bvc_setpeerenable();
extern int bvc_setinfologstatus();
extern int bvc_bgpsum();
extern int bvs_clearbgp();
extern int bvc_shousers();
extern int bvc_shoneighbor();
#ifdef HAVE_TOP
extern int bvc_shoproc();
#endif /* HAVE_TOP */
extern int bvc_shostatics();
extern int bvc_clrstatics();
extern int bvc_clrstaticsall();
extern int bvc_shoversion();
extern int bvc_shoconfig();
extern int bvc_doannouncedummy();
extern int bvc_shoannouncedummy();
extern int bvc_dorefleshsystemlog();
extern int bvc_dorefleshprefixlogall();
extern int bvc_doroutertt();
extern int bvc_doroutertt_notimeout();
extern int bvc_set_vtyrowsize();

#ifdef RADIX
extern int bvc_show_bgproute_all();
extern int bvc_show_bgproute_all_v6();
extern int bvc_output_bgproute_all();
extern int bvc_output_bgproute_all_v6();
extern int bvc_output_bgproute_all_nei();
extern int bvc_output_bgproute_all_nei_v6();
extern int bvc_show_bgproute_neighbor();
extern int bvc_show_bgproute_neighbor_v6();
extern int bvc_showipbgproute();
extern int bvc_showipbgproute_v6();
extern int bvc_show_bgproute_sum();
extern int bvc_showipbgproutetime_eq();
extern int bvc_showipbgproutetime_le();
extern int bvc_showipbgproutetime_ge();
#ifdef RTHISTORY
extern int bvc_clear_route_history();
extern int bvc_clear_route_history_v6();
extern int bvc_next_rthistory_clear_set();
extern int bvc_next_rthistory_clear_set_v6();
extern int bvc_show_bgproute_active();
extern int bvc_show_bgproute_active_v6();
extern int bvc_show_bgproute_inactive();
extern int bvc_show_bgproute_inactive_v6();
extern int bvc_show_bgproute_all_detail();
extern int bvc_show_bgproute_all_detail_v6();
extern int bvc_show_bgproute_active_detail();
extern int bvc_show_bgproute_active_detail_v6();
extern int bvc_show_bgproute_inactive_detail();
extern int bvc_show_bgproute_inactive_detail_v6();
extern int bvc_show_bgproute_neighbor_active();
extern int bvc_show_bgproute_neighbor_active_v6();
extern int bvc_show_bgproute_neighbor_inactive();
extern int bvc_show_bgproute_neighbor_inactive_v6();
extern int bvc_output_bgproute_active();
extern int bvc_output_bgproute_active_v6();
extern int bvc_output_bgproute_active_nei();
extern int bvc_output_bgproute_active_nei_v6();
extern int bvc_output_bgproute_inactive();
extern int bvc_output_bgproute_inactive_v6();
extern int bvc_output_bgproute_inactive_nei();
extern int bvc_output_bgproute_inactive_nei_v6();
extern int bvc_output_bgproute_all_detail();
extern int bvc_output_bgproute_all_detail_v6();
extern int bvc_output_bgproute_all_detail_nei();
extern int bvc_output_bgproute_all_detail_nei_v6();
extern int bvc_output_bgproute_active_detail();
extern int bvc_output_bgproute_active_detail_v6();
extern int bvc_output_bgproute_active_detail_nei();
extern int bvc_output_bgproute_active_detail_nei_v6();
extern int bvc_output_bgproute_inactive_detail();
extern int bvc_output_bgproute_inactive_detail_v6();
extern int bvc_output_bgproute_inactive_detail_nei();
extern int bvc_output_bgproute_inactive_detail_nei_v6();
#endif
#endif

#ifdef IRRCHK
extern int bvc_show_irr_status();
extern int bvc_show_irr_status_active();
extern int bvc_do_output_irr_status();
extern int bvc_do_output_irr_status_active();
extern int bvc_show_irr_status_summary();
#endif

extern int bvc_output_aslist_nei();

extern int bvc_readcronfile();
extern int bvc_showcronlist();
extern int bvc_show_bgpnexthop_sum();
extern int bvc_show_bgpnexthop_sum_neighbor();

#ifdef DEBUG
#ifdef RADIX
extern int bvc_dodeleteroute();
extern int bvc_doclearpeerroute();
#endif
extern int bvc_dotimertest();
extern void testfunc();
#endif

#endif /* _BVCOMMAND_H */
