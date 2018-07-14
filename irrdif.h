/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo, Atelier Mahoroba 2006/08           */
/*                                                              */
/* IRRd Interface Header File                                   */
/****************************************************************/
/* irrdif.h,v 1.8 2008/06/26 05:24:08 kuniaki Exp */

/*
 * Copyright (c) 2006 Atelier Mahoroba
 * All rights Reserved.
 */

#ifndef _IRRDIF_H
#define _IRRDIF_H

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* #include "radix.h" */
/* #include "route.h" */
/* #include "timer.h" */
#include "bvtypes.h"
/* #include "bgplog.h" */


#define	STRBUFLEN		4096
#define IRR_CACHE_EXPTIME	36000 /* Sec: 36000 = 10H */
/* #define	IRR_WALK_TIMER		900   /* 900 Sec = 15min */
#define	IRR_WALK_TIMER		300   /* 60 Sec = 1min */
#define IRR_SEARCH_TIMEOUT	5	/* Search Wait 5 Sec */

/* Proc Flag */
#define IRRCHK_ACTIVE		1
#define IRRCHK_TIMERWAIT	2

/* Defult IRR Host */
#define DEF_IRR_HOST	"localhost"
#define DEF_IRR_PORT	43

/* IRR Parameter Setting Types */
#define SET_IRR_DISABLE	0
#define SET_IRR_HOST	1
#define	SET_IRR_PORT	2

/* IRR Parameter Setting type for PEER */
#define SET_P_IRRCOMPARE	1
#define SET_P_IRRNOTIFY		2
#define	SET_P_IRRNOTELEVEL	3

/* Route Cache Status */
#define	CA_ENTRIED	0
#define	CA_UNENTRIED	1

/* IRR Check Result Code */
#define IRR_SEARCH_FAIL	0
#define IRR_EXACT_AS	1
#define IRR_EXACT_NOTAS	2
#define IRR_MORES_AS	3
#define IRR_MORES_NOTAS	4
#define IRR_NOTMATCH	5

typedef struct _IRR_ENTRY {
	struct in_addr	prefix;
	u_short		p_len;
	u_long		originas;
	u_short		status;
	struct timeval	entry_time;
} IRR_ENTRY;

extern int	irr_check_enable;

extern void walk_bgp_chk_irr();
extern void activate_bgp_chk_irr(int start);
extern int set_irrparam(int type, char *val);
extern int set_irr_peerparam(int type, char *val, int pn);
/* extern int irrchk_r_list(R_LIST *rlist); */
extern int irr_walker_check();
extern int irr_routecmp(struct in_addr *in, int plen, u_long oasn, IRR_ENTRY *irrent);

#endif /* _IRRDIF_H */
