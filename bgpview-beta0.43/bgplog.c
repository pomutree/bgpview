/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1997/12                         */
/*                                                              */
/* Logging  Routine                                             */
/****************************************************************/
/* bgplog.c,v 1.6 2005/08/18 02:01:22 kuniaki Exp */

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

/*
 * Copyright (c) 2003 Intec NetCore, Inc.
 * All rights reserved.
 */

#include "bgplog.h"

/* System Log File Poingter */
FILE	*sfd;
char	logstr[LOGSTRSIZE];

void pr_time(fd)
FILE *fd;
{
	time_t 		clock;
	struct tm	*tm;
	char		buf[TIME_BUF+1];
	int		ret;

	bzero(buf, TIME_BUF+1);
	time(&clock);
	tm = localtime(&clock);
	ret = strftime(buf, TIME_BUF, "%Y/%m/%d %H:%M:%S", tm);
	if (ret != 0) {
		fprintf(fd, "%s: ", buf);
	}
}

void open_syslog(logname)
char *logname;
{
	/* System Log Open */
	if (facility == -1) {
		if ((sfd = fopen(logname, "a")) == NULL) {
			printf("System Log Could Not Opened\n");
			return;
		}

		fprintf(sfd, "====== Open System Log File at ");
		pr_time(sfd);
		fprintf(sfd, "======\n");
		fflush(sfd);
	} else {
		openlog("BGPView", LOG_PID | LOG_NDELAY, facility);
		syslog(LOG_INFO , "====== Starting BGPView System Log ======");
	}
}

void open_log(logname, pn)
char *logname;
int  pn;
{
	if ((peer[pn].logfd = fopen(logname, "a")) == NULL) {
		printf("Log File Could Not Opend for peer %d\n",pn);
		return;
	}

	fprintf(peer[pn].logfd, "====== Open Log File Peer No.%d at ", pn);
	pr_time(peer[pn].logfd);
	fprintf(peer[pn].logfd, "======\n");
	fflush(peer[pn].logfd);
}

void close_log()
{
	int pn;

	for(pn = 0; pn < MAXPEERS && peer[pn].neighbor != 0; pn++) {
		if (peer[pn].logfd != NULL) {
			fclose(peer[pn].logfd);
		}
	}
	if (facility == -1) {
		if (sfd != NULL) {
			fclose(sfd);
		}
	} else {
		closelog();
	}
}

void pr_log(typ, pn, str, timep)
u_char typ;
int    pn;
char   *str;
int    timep;
{
	int	cnt;

	if (pn == LOG_ALL) {
		for(cnt = 0; cnt < MAXPEERS && peer[cnt].neighbor != 0; cnt++) {
			if((LG_ISSET(typ, peer[cnt].logtype)) &&
			   (peer[cnt].logfd != NULL)) {
				if (timep == ON) pr_time(peer[cnt].logfd);
				fprintf(peer[cnt].logfd, str);
				fflush(peer[cnt].logfd);
			}
		}
		if (facility == -1) {
			if (sfd != NULL) {
				if (timep == ON) pr_time(sfd);
				fprintf(sfd, str);
				fflush(sfd);
			}
		} else {
			syslog(LOG_INFO, str);
		}
	} else {
		if ((LG_ISSET(typ, peer[pn].logtype)) &&
	   	    (peer[pn].logfd != NULL)) {
			if (timep == ON) pr_time(peer[pn].logfd);
			fprintf(peer[pn].logfd, str);
			fflush(peer[pn].logfd);
		}
	}
}

void pr_log2(u_char type, int pn, int timep, char *str, ...)
{
        va_list argp;
        int     cnt;
        char    temp[6000];

	bzero(temp, 6000);
	va_start(argp, str);

#ifdef HAVE_VSNPRINTF
        vsnprintf(temp, sizeof(temp), str, argp);
#else
        vsprintf(temp, str, argp);
#endif

	va_end(argp);

        pr_log(type, pn, temp, timep);
}

void restart_log()
{

	int	pn;
	char	subfname[FNAMESIZE];

	pr_log(VLOG_GLOB, LOG_ALL, "Logfile Restarted\n", ON);
	close_log();

	for(pn = 0; pn < MAXPEERS && peer[pn].neighbor != 0; pn++) {
		if (peer[pn].logfd != NULL) {
			strcpy(subfname, peer[pn].logname);
			strcat(subfname, ".bak");
			rename(peer[pn].logname, subfname);
		}
		open_log(peer[pn].logname, pn);
	}

	strcpy(subfname, syslogname);
	strcat(subfname, ".bak");
	rename(syslogname, subfname);
	open_syslog(syslogname);
}

void init_prefixlog()
{
	int	pn;
	char	subfname[FNAMESIZE];

	pr_log(VLOG_GLOB, LOG_ALL, "Opening Prefix Logfile\n", ON);

	for(pn = 0; pn < MAXPEERS && peer[pn].neighbor != 0; pn++) {
		if (strlen(peer[pn].prefixout) == 0) {
			peer[pn].pfd = NULL;
			continue;
		}

		if (peer[pn].pfd != NULL) {
			fclose(peer[pn].pfd);
			strcpy(subfname, peer[pn].prefixout);
			strcat(subfname, ".bak");
			rename(peer[pn].prefixout, subfname);
		}
		if ((peer[pn].pfd = fopen(peer[pn].prefixout, "a")) == NULL) {
			peer[pn].pfd = NULL;
			pr_log(VLOG_GLOB, pn, "Could not open Prefix Log\n",ON);
#ifdef DEBUG
			printf("PREFIX LOG NOT OPEN:%s\n",peer[pn].prefixout);
#endif
			continue;

		}

#ifdef RADIX
		if (peer[pn].prefixsum) {
			fprintf(peer[pn].pfd, 
				"BGPView Total Routes Summary =====\n");
			fprintf(peer[pn].pfd, 
				" Received Prefixes : %ld\n", radix_prefixes());
			fprintf(peer[pn].pfd, 
				" Received Routes   : %ld\n", radix_routes());
			fprintf(peer[pn].pfd, 
				" RADIX Tree Nodes  : %ld\n", radix_nodes());
			fprintf(peer[pn].pfd, 
				"==================================\n");
		}
#endif /* RADIX */
	}
}

void close_prefixlog()
{
	int     pn;

	pr_log(VLOG_GLOB, LOG_ALL, "Closing Prefix Logfile\n", ON);

	for(pn = 0; pn < MAXPEERS && peer[pn].neighbor != 0; pn++) {
		if (peer[pn].pfd != NULL) {
			fclose(peer[pn].pfd);
		}
		peer[pn].pfd = NULL;
	}
}

int write_prefixlog(typ, prefix, pn)
int		typ;	/* 0:Update 1:Withdraw */
void		*prefix;
int		pn;	/* Peer number */
{
	prefix_t	*withdrawn;
	ROUTE_INFO	*update;
	struct in_addr	inaddr;
	char		lbuf[30];
	char		stmp[4096];

	if (peer[pn].pfd == NULL) return 1;
	pr_time(peer[pn].pfd);
	if (typ == 0) {
		/* Update */
		fprintf(peer[pn].pfd, "UPDATE  : ");
		update = (ROUTE_INFO *)prefix;
		inaddr.s_addr = update->prefix;
		sprintf(lbuf, "%s/%d", inet_ntoa(inaddr), update->length);
		inaddr.s_addr = update->next_hop;
		if (update->pathset_ptr == NULL) {
			stmp[0] = '\0';
		} else {
			strcpy(stmp, update->pathset_ptr);
		}
		fprintf(peer[pn].pfd, "%-18s %-15s %s\n",
			lbuf,
			inet_ntoa(inaddr),
			/* update->path_set); */
			stmp);
	} else {
		/* WITHDRAW */
		fprintf(peer[pn].pfd, "WITHDRAW: ");
		withdrawn = (prefix_t *)prefix;
		inaddr.s_addr = withdrawn->prefix;
		fprintf(peer[pn].pfd, "%s/%d\n",
				inet_ntoa(inaddr),
				withdrawn->length);
	}
	return 0;
}

int write_v6prefixlog(int typ, void *prefix, int pn)
/*
int		typ;	/* 0:Update 1:Withdraw 
void		*prefix;
int		pn;	/* Peer number 
*/
{
	V6ROUTE_BUF	*withdrawn;
	ROUTE_INFO_V6	*update;
	struct in_addr	inaddr;
	char		v6addr[IPV6_ADDR_LEN];
	char		lbuf[50], lbuf2[50];
	char		stmp[4096];
	V6ROUTE_LIST	*v6list;

	if (peer[pn].pfd == NULL) return 1;
	pr_time(peer[pn].pfd);
	if (typ == 0) {
		/* Update */
		fprintf(peer[pn].pfd, "UPDATE  : ");
		update = (ROUTE_INFO_V6 *)prefix;
		bintostrv6(update->prefix, lbuf2);
		sprintf(lbuf, "%s/%d", lbuf2, update->length);
		if (update->nexthop_afi == AF_IPV4) {
			inaddr.s_addr = *(net_ulong *)update->next_hop;
			strcpy(lbuf2, inet_ntoa(inaddr));
		} else if (update->nexthop_afi == AF_IPV6) {
			bintostrv6(update->next_hop, lbuf2);
		} else {
			strcpy(lbuf2, "::");
		}
		if (update->pathset_ptr == NULL) {
			stmp[0] = '\0';
		} else {
			strcpy(stmp, update->pathset_ptr);
		}
		fprintf(peer[pn].pfd, "%-18s %-15s %s\n",
			lbuf,
			lbuf2,
			/* update->path_set); */
			stmp);
	} else {
		/* WITHDRAW */
		fprintf(peer[pn].pfd, "WITHDRAW: ");
		withdrawn = (V6ROUTE_BUF *)prefix;
		v6list = withdrawn->preflist;
		while(v6list != NULL) {
			bintostrv6(v6list->v6prefix, lbuf2);
			fprintf(peer[pn].pfd, "%s/%d\n", lbuf2,
							 v6list->length);
			v6list = v6list->next;
		}
	}
	return 0;
}
