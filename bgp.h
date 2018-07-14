/****************************************************************/
/* BGP-4 Protocol Viewer					*/
/*    Programmed By K.Kondo IIJ	1997/12				*/
/*								*/
/* Some Define and Structs					*/
/****************************************************************/
/* bgp.h,v 1.7 2005/10/21 02:24:50 kuniaki Exp */

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

/*
 * Copyright (c) 2003-2005 Intec NetCore, Inc.
 * All rights reserved.
 */

/*
 * Copyright (c) 2003-2008 Mahoroba Kobo / Bugest-Network
 * All rights reserved.
 */

#ifndef _BGP_H
#define _BGP_H

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
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#include "bvtypes.h"
#include "route.h"
#include "route_ipv6.h"
#include "utils.h"
#include "bgpdata.h"
#include "bgplog.h"
#include "timer.h"
#include "dummyroute.h"

#ifdef RADIX
#include "radix.h"
#include "radix_v6.h"
#endif

#ifdef TCP_MD5SIG
#include "md5sig.h"
#endif

#ifdef IRRCHK
#include "irrdif.h"
#endif

#define BV_VERSION	"0.43Beta"
#define BV_COPYRIGHT	"Copyright (c) 1998-2002 Internet Initiative Japan Inc.\r\nCopyright (c) 2003-2005 Intec NetCore, Inc.\r\nCopyright (c) 2006-2015 Mahoroba Kobo, Inc."

/* BGP Header Type field defines */
#define BGPTYPES                5
#define BGP_UNDEFINE            0
#define BGP_OPEN                1
#define BGP_UPDATE              2
#define BGP_NOTIFICATION        3
#define BGP_KEEPALIVE           4

/* BGP States */
#define IDLE                    1
#define CONNECT                 2
#define ACTIVE                  3
#define OPENSENT                4
#define OPENCONFIRM             5
#define ESTABLISHED             6

/* BGP Events */
#define BGP_NONE                0
#define BGP_START               1
#define BGP_STOP                2
#define BGP_TRANS_CNT_OPEN      3
#define BGP_TRANS_CNT_CLOSE     4
#define BGP_TRANS_CNT_OPEN_FAIL 5
#define BGP_TRANS_FATAL_ERR     6
#define CNT_RETRY_TIME_EXP      7
#define HOLD_TIME_EXP           8
#define KEEPALIVE_TIME_EXP      9
#define RECV_OPEN               10
#define RECV_KEEPALIVE          11
#define RECV_UPDATE             12
#define RECV_NOTIFICATION       13

/* Timer Default Value */
#define CNT_RETRY_TIMER		120
#define HOLD_TIMER		90
#define KEEPALIVE_TIMER		30
#define MINORIGIN_INTEVAL	15
#define MINROUTEADV_INTERVAL	30
#define RE_INITIAL_TIMER	60
#ifdef RTHISTORY
#define DEFAULT_CLEARHISTORY	86400 /* Second = 24Hour */
#endif

/* TIMER IDs */
#define ID_CNT_RETRY_TIMER	1
#define ID_HOLD_TIMER		2
#define ID_KEEPALIVE_TIMER	3
#define ID_MINORIGIN_INTERVAL	4
#define ID_MINROUTEADV_INTERVAL	5
#define ID_RE_INITIAL_TIMER	6

#define ID_BGPINFO_TIMER	10

/* Socket Status */
#define SOCK_ACCEPTING		1
#define SOCK_CONNECTED		2

/* OPEN Option Types */
#define BGPOPT_AUTHENTICATION		1
#define BGPOPT_CAPABILITY		2	/* RFC2842 */

/* Capability Code -- RFC2842 */
#define CAP_RESERVE			0	/* RFC2842 */
#define CAP_MPBGP			1	/* RFC2858 */
#define CAP_ROUTEREFRESH		2	/* RFC2918 */
#define CAP_CRFC			3	/* Rekter  */
#define CAP_MULTIROUTE			4	/* RFC3107 */
#define CAP_GRACEFUL			64	/* Chen    */
#define CAP_4OCTAS			65	/* Chen    */
#define CAP_DCAP			66	/* Chen    */

/* Notification Err Codes */
#define MESSAGE_HEADER_ERROR		1
#define OPEN_MESSAGE_ERROR		2
#define UPDATE_MESSAGE_ERROR		3
#define HOLD_TIME_EXPIERD		4
#define FINITE_STATE_MACHINE_ERROR	5
#define CEASE				6

/* Notification ErrCodes : Message Header Error subcodes */
#define CONNECTION_NOT_SYNCRONIZED	1
#define BAD_MESSAGE_LENGTH		2
#define BAD_MESSAGE_TYPE		3

/* Notification ErrCodes : OPEN Message Error subcodes */
#define UNSUPPORTED_VERSION_NUMBER	1
#define BAD_PEER_AS			2
#define BAD_BGP_IDENTIFIER		3
#define UNSUPPORTED_OPTIONAL_PARAMETER	4
#define AUTHENTICATION_FAILURE		5
#define UNACCEPTABLE_HOLD_TIME		6
#define UNSUPPORTED_CAPABILITY		7	/* RFC2842 */

/* Notification ErrCodes : UPDATE Message Error subcodes */
#define MALFORMED_ATTRIBUTE_LIST	1
#define UNRECOGNIZED_WELL_KNOWN_ATTRIBUTE	2
#define MISSING_WELL_KNOWN_ATTRIBUTE	3
#define ATTRIBUTE_FLAGS_ERROR		4
#define ATTRIBUTE_LENGTH_ERROR		5
#define INVALID_ORIGIN_ATTRIBUTE	6
#define AS_ROUTING_LOOP			7
#define INVALID_NEXT_HOP_ATTRIBUTE	8
#define OPTIONAL_ATTRIBUTE_ERROR	9
#define INVALID_NETWORK_FIELD		10
#define MALFORMED_AS_PATH		11

/* PATH ATTRIBUTE Type Code */
#define ATTR_MAX			18
#define ATTR_UNKNOWN			0
#define ATTR_ORIGIN			1
#define ATTR_AS_PATH			2
#define ATTR_NEXT_HOP			3
#define ATTR_MULTI_EXIT_DISC		4
#define ATTR_LOCAL_PREF			5
#define ATTR_ATOMIC_AGGREGATE		6
#define ATTR_AGGREGATOR			7
#define ATTR_COMMUNITIES		8
#define ATTR_ORIGINATOR_ID		9
#define ATTR_CLUSTER_LIST		10
#define ATTR_DPA			11	/* Chen */
#define ATTR_ADVERTISER			12	/* RFC1863 */
#define ATTR_RCIDPATH_CLUSTERID		13	/* RFC1863 */
#define ATTR_MP_REACH_NLRI		14	/* RFC2283 */
#define ATTR_MP_UNREACH_NLRI		15	/* RFC2283 */
#define ATTR_EXTENDED_COMMUNITIES	16	/* Rosen */
#define ATTR_NEW_AS_PATH		17	/* E.Chen */
#define ATTR_NEW_AGGREGATOR		18	/* E.Chen */

/* AFI / IANA Address Family Numbers */
#define AF_IPV4			1
#define AF_IPV6			2

/* SAFI / IANA Subsequence Address Family Identifier RFC2858 */
#define SAFI_NLRI_UNI		1
#define SAFI_NLRI_MULTI		2
#define SAFI_NLRI_BOTH		3
#define SAFI_NLRI_MPLS		4

/* Default BGP port */
#define BGP_PORT		179
#define BGP_MAX_PACKET_SIZE	4096

#define MAXPEERS		128
#define HEADERLENGTH		19
#define MARKERLENGTH		16
#define CONF_FILE		"/usr/local/etc/bgpview.cfg"
#define SPOOL_DEFAULT		"/tmp"
#define DESCSIZE		128

/* finish command parameter */
#define PROC_SHUTDOWN		1
#define PROC_REBOOT		2
#define PROC_CHILDEND		3

/* BGPView Peer NLRI Types */
#define NLRI_DEFAULT		0
#define IPv4_UNICAST		1
#define IPv6_UNICAST		2

#define AS_TRANS		(u_short)23456

#define DEFSYSNAME		"BGPView"

extern	net_ulong	IDENT;
/* extern  u_short		LOCAL_AS; */
extern  u_long		LOCAL_AS;
extern	struct timeval	uptime;

typedef struct {
        u_short length;
        u_char  type;
} BGPV4_HEADER;

typedef struct {
	char		description[DESCSIZE];	/* Description */
	boolean		peerstatus;		/* Peer Status (Shutdown) */
	boolean		as4_enable;
	boolean		as4_remote;
	net_ulong	neighbor;		/* Neighbor Address */
	/* u_short		remote_as;		/* REMOTE AS */
	u_long		remote_as;		/* Configured REMOTE AS */
	u_long		remote_as4;		/* Received REMOTE 4Byte-AS */
	net_ulong	local_id;		/* local IDENT */
	/* u_short		local_as;		/* local AS */
	u_long		local_as;		/* local AS */
	char		admin_addr[FNAMESIZE];  /* Peer Admin Email */
	char		nlri;			/* NLRI Type */
	int		soc;			/* Socket Discripter */
	int		flag;			/* Socket Flags	*/
	int		status;			/* BGP Status (FSM) */
	int		event;			/* BGP Event (FSM) */
	int		resume;			/* RESUME FLAG */
	int		datalen;		/* Recv Data Length */
	char		*data;			/* Recv Data */
	int		holdtimer;		/* Hold Timer Value */
	int		keepalive;		/* Keepalive Timer Value */
	int		holdtimer_conf;		/* Configured Hold Timer */
	int		keepalive_conf;		/* Configured Keepalive Timer */
	char		logname[FNAMESIZE];	/* Log File Name */
	u_char		logtype;		/* Logging Type */
	boolean		status_notify;		/* statys notify flag */
	FILE		*logfd;			/* Log File Filedescripter */
	char		prefixout[FNAMESIZE];	/* Prefix Log File Name */
	boolean		prefixsum;		/* Add prefix sum to Prefix log */
	FILE		*pfd;			/* Prefix Log File Descripter */
	u_short		port;			/* Destination TCP Socket Num */
#ifdef TCP_MD5SIG
	boolean		md5sig;			/* Enable/Disable TCP/MD5 SIG */
	char		md5pass[MD5SIG_PASWORD_MAXLEN];
						/* MD5 Password */
#endif
	int             session_re_init_timer;  /* Sec */
#ifdef IRRCHK
	boolean		irr_compare;		/* true:enable, false:disable */
	char		irr_notify[FNAMESIZE];	/* Notify Email Address */
	char		irr_notelevel;		/* Notification Level */
#endif
} PEERS;

typedef struct {
	net_ulong prefix;	/* Prefix */
	char	  length;	/* Mask Length */
} prefix_t;

static char *fsm_state_names[] = {
	"", "Idle", "Connect", "Active", "OpenSent",
	"OpenConfirm", "Established"
};

/* Global Valiables */
extern char 	*bgptypestring[];
extern int	bgp_state;
extern int	bgp_event;
extern int	timeout_id;
extern PEERS	peer[];
extern fd_set	readfds;
extern char	syslogname[];
extern int	facility;
extern char	configfile[];
extern char	spooldir[];
extern char	sysname[16];

#ifdef RTHISTORY
extern struct timeval	rthistcleartime;
extern struct timeval	rthistcleartime_v6;
#endif

/* bgpview.c */
extern void finish();
/* extern int sourcetopn(/* char *v6adrs, char afi ); */
extern int check_id1topn(/* net_ulong id1 */);

#endif /* _BGP_H */
