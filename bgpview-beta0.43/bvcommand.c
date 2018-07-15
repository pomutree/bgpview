/****************************************************************/
/*    Programmed By K.Kondo IIJ 2000/01                         */
/*                                                              */
/* BGPView Shell Commands                                       */
/****************************************************************/
/* bvcommand.c,v 1.2 2004/07/13 08:58:44 kuniaki Exp */

/*
 * Copyright (c) 2000 - 2001 Internet Initiative Japan Inc.
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

#include "bvcommand.h"

/****** Functions **********/
int bvc_shutdown(BVS_CMD_PARAM *prm)
{
	notify_systemnotify("SHUTDOWN", "Shutdown by BGPView Shell Command");
	finish(PROC_SHUTDOWN);
	return(0);
}

int bvc_reboot(BVS_CMD_PARAM *prm)
{
	notify_systemnotify("REBOOT", "Reboot by BGPView Shell Command");
	finish(PROC_REBOOT);
	return(0);
}

char *sectoptm(long tvsec, char *str)
{
	int	hh;
	int	mm;
	int	ss;

	if (tvsec <= 86400) {
		hh = (int)(tvsec/3600);
		mm = (int)((tvsec % 3600)/60);
		ss = (int)(tvsec % 60);
		sprintf(str, "%02d:%02d:%02d", hh, mm, ss);
	} else if (tvsec <= 604800) {
		hh = (int)(tvsec/86400);
		mm = (int)((tvsec % 86400)/3600);
		sprintf(str, "%dd%dh", hh,mm);
	} else {
		hh = (int)(tvsec / 604800);
		mm = (int)((tvsec % 604800) / 86400);
		sprintf(str, "%dw%dd", hh, mm);
	}
	return(str);
}

long ptmtosec(char *str, long *tvsec, long *range)
{
	char	tsbuf[128];
	int	cnt,ccnt;
	int	flag;
	int	cherr;

	cnt = 0;
	flag = 0;
	*tvsec = 0;
	cherr = 0;
	ccnt = 0;
	bzero(tsbuf, 128);
	while(*(str+cnt) != '\0') {
		switch (*(str+cnt)) {
		case ':':
			flag++;
			switch(flag) {
			case 1:
				*tvsec += atoi(tsbuf)*3600;
				bzero(tsbuf, 128);
				ccnt = 0;
				break;
			case 2:
				*tvsec += atoi(tsbuf)*60;
				bzero(tsbuf, 128);
				ccnt = 0;
				break;
			/***** Don't NEED *****
			case 3:
				*tvsec += atoi(tsbuf);
				bzero(tsbuf, 128);
				ccnt = 0;
				break;
			***********************/
			default:
				cherr = 1;
			}
			break;
		case 'w':
			*tvsec += atoi(tsbuf)*604800;
			bzero(tsbuf, 128);
			ccnt = 0;
			*range = 604800;
			break;
		case 'd':
			*tvsec += atoi(tsbuf)*86400;
			bzero(tsbuf, 128);
			ccnt = 0;
			*range = 86400;
			break;
		case 'h':
			*tvsec += atoi(tsbuf)*3600;
			bzero(tsbuf, 128);
			ccnt = 0;
			*range = 3600;
			break;
		default:
			if ((*(str+cnt) >= '0') && (*(str+cnt) <= '9')) {
				tsbuf[ccnt] = *(str+cnt);
			} else {
				cherr = 1;
			}
		}
		if (cherr == 1) break;
		cnt++;
	}
	if ((cherr != 1) && (flag == 2)) {
		*tvsec += atoi(tsbuf);
		*range = 1;
	}
	if (cherr == 1) {
		*range = 0;
		return(0);
	}
	return(*tvsec);
}

int bvc_setannouncestatus(BVS_CMD_PARAM *prm)
{
	int		clno;
	u_long		adrs;
	boolean		action;
	int		pn;

	clno = (prm->adrs_boolean).clno;
	adrs = (prm->adrs_boolean).addr;
	action = (prm->adrs_boolean).swc;

	pn = check_id1topn(adrs);
	if (pn < 0) {
		bvs_write(clno, "Neighbor Address is not configured\r\n");
		return(BVS_CNT_END);
	}
	if (action == 1) {
		if (peer[pn].status_notify == true) {
			bvs_write(clno, "Notifying peer status already ON\r\n");
		} else {
			peer[pn].status_notify = true;
			bvs_write(clno, "Notifying peer status changed to ON\r\n");
		}
	} else {
		if (peer[pn].status_notify == false) {
			bvs_write(clno, "Notifying peer status already OFF\r\n");
		} else {
			peer[pn].status_notify = false;
			bvs_write(clno, "Notifying peer status changed to OFF\r\n");
		}
	}
	return(BVS_CNT_END);
}

int bvc_setpeerdisable(BVS_CMD_PARAM *prm)
{
	int		clno;
	u_long		adrs;
	int		pn;
	char		prline[128];
	struct in_addr  sin;

	/****
	clno = (prm->adrs).clno;
	adrs = (prm->adrs).addr;
	*****/

	clno = *(int *)prm;
	adrs = *(u_long *)((char *)prm + sizeof(int));
	sin.s_addr = adrs;

	pn = check_id1topn(adrs);
	if (pn < 0) {
		sprintf(prline, "Neighbor %s was not configured\r\n",
							inet_ntoa(sin));
		bvs_write(clno, prline);
		return(BVS_CNT_END);
	}

	if (peer[pn].peerstatus == false) {
		sprintf(prline, "Neighbor %s already disabled.\r\n",
							inet_ntoa(sin));
		bvs_write(clno, prline);
		return(BVS_CNT_END);
	}
	peer[pn].peerstatus = false;
	bgpinfo[pn].infostatus = false;
	clear_peer(pn);

	sprintf(prline, "Disable neighbor %s\r\n", inet_ntoa(sin));
	bvs_write(clno, prline);

	return(BVS_CNT_END);
}

int bvc_setpeerenable(BVS_CMD_PARAM *prm)
{
	int		clno;
	u_long		adrs;
	int		pn;
	char		prline[128];
	struct in_addr	sin;
	struct timeval	timer_value;

	/****
	clno = (prm->adrs).clno;
	adrs = (prm->adrs).addr;
	****/

	clno = *(int *)prm;
	adrs = *(u_long *)((char *)prm + sizeof(int));
	sin.s_addr = adrs;

	pn = check_id1topn(adrs);
	if (pn < 0) {
		sprintf(prline, "Neighbor %s was not configured\r\n",
							inet_ntoa(sin));
		bvs_write(clno, prline);
		return(BVS_CNT_END);
	}

	if (peer[pn].peerstatus == true) {
		sprintf(prline, "Neighbor %s already enabled.\r\n",
							inet_ntoa(sin));
		bvs_write(clno, prline);
		return(BVS_CNT_END);
	}
	peer[pn].peerstatus = true;
	peer[pn].event = BGP_NONE;
	peer[pn].status = IDLE;
	bgpinfo[pn].infostatus = true;
	recount_info(pn);
	timer_value.tv_sec = bgpinfo[pn].interval;
	timer_value.tv_usec = 0;
	if (!entry_timer(&timer_value, peer[pn].neighbor, ID_BGPINFO_TIMER)) {
		pr_log(VLOG_GLOB, LOG_ALL, "failed to allocate memory.\n", ON);
		notify_systemnotify("STOP",
			"Memory Allocation Error : bvc_setinfologstatus()");
		finish(PROC_SHUTDOWN);
		return(BVS_CNT_END);
	}

	sprintf(prline, "Enable neighbor %s\r\n", inet_ntoa(sin));
	bvs_write(clno, prline);

	return(BVS_CNT_END);
}

int bvc_setinfologstatus(BVS_CMD_PARAM *prm)
{
	int		clno;
	u_long		adrs;
	boolean		sw;
	char		prline[128];
	int		pn;
	struct in_addr	sin;
	struct timeval	timer_value;

	clno = (prm->adrs_boolean).clno;
	adrs = (prm->adrs_boolean).addr;
	sw   = (prm->adrs_boolean).swc;
	sin.s_addr = adrs;

	pn = check_id1topn(adrs);
	if (pn < 0) {
		sprintf(prline, "Neighbor %s was not configured\r\n",
						inet_ntoa(sin));
		bvs_write(clno, prline);
		return(BVS_CNT_END);
	}

	if (bgpinfo[pn].infostatus == false) {
		if (sw == false) {
			bvs_write(clno, "Infomation log already disabled.\r\n");
		} else {
			bgpinfo[pn].infostatus = true;
			recount_info(pn);
			timer_value.tv_sec = bgpinfo[pn].interval;
			timer_value.tv_usec = 0;
			if (!entry_timer(&timer_value, peer[pn].neighbor,
							ID_BGPINFO_TIMER)) {
				pr_log(VLOG_GLOB, LOG_ALL,
					"failed to allocate memory.\n", ON);
				notify_systemnotify("STOP",
				"Memory Allocation Error : bvc_setinfologstatus()");
				finish(PROC_SHUTDOWN);
				return(BVS_CNT_END);
			}
			bvs_write(clno, "Infomation log was set enable.\r\n");
		}
	} else {
		if (sw == true) {
			bvs_write(clno, "Infomation log already enabled.\r\n");
		} else {
			bgpinfo[pn].infostatus = false;
			bvs_write(clno, "Infomation log was set disable.\r\n");
		}
	}
	return(BVS_CNT_END);
}

int bvc_bgpsum(BVS_CMD_PARAM *prm)
{
	int		clno;
	int		pn;
	char		prline[128];
	struct in_addr	adr;
	char		localident[20];
	char		tempstr[20];


	clno = (prm->noparam).clno;

	bvs_write(clno, "\r\nRoute Summary---\r\n");
	bvs_write(clno, "                     IPv4      IPv6\r\n");
	sprintf(prline, " Received Routes   : %8ld  %8ld\r\n", 
					radix_prefixes(), radix_prefixes_v6());
	bvs_write(clno, prline);
	sprintf(prline, " Received Prefixes : %8ld  %8ld\r\n",
					 radix_routes(), radix_routes_v6());
	bvs_write(clno, prline);
	sprintf(prline, " RADIX Tree Nodes  : %8ld  %8ld\r\n",
					 radix_nodes(), radix_nodes_v6());
	bvs_write(clno, prline);

	bvs_write(clno, "\r\nNo LocalIdentifier LoASN Neighbor        ASN   Status\r\n");
	for(pn = 0; pn < MAXPEERS && peer[pn].neighbor != 0; pn++) {
		adr.s_addr = peer[pn].local_id;
		strcpy(localident, inet_ntoa(adr));
		adr.s_addr = peer[pn].neighbor;
		sprintf(tempstr, "%lu", (u_long)peer[pn].local_as);
		/* strcpy(tempstr, ulongtofourbyteas(peer[pn].local_as)); */
		if (peer[pn].peerstatus == true) {
			/***
			sprintf(prline, "%2d %-15s %-11s %-15s %-11s %-12s\r\n",
				pn, localident, tempstr,
				inet_ntoa(adr), ulongtofourbyteas(peer[pn].remote_as),
				fsm_state_names[peer[pn].status]);
			***/
			sprintf(prline, "%2d %-15s %-11s %-15s %-11lu %-12s\r\n",
				pn, localident, tempstr,
				inet_ntoa(adr), (u_long)(peer[pn].remote_as),
				fsm_state_names[peer[pn].status]);
		} else {
			/***
			sprintf(prline, "%2d %-15s %-11s %-15s %-11s Admin Down\r\n",
				pn, localident, tempstr,
				inet_ntoa(adr), ulongtofourbyteas(peer[pn].remote_as));
			***/
			sprintf(prline, "%2d %-15s %-11s %-15s %-11lu Admin Down\r\n",
				pn, localident, tempstr,
				inet_ntoa(adr), (u_long)(peer[pn].remote_as));
		}
		bvs_write(clno, prline);
	}
	return(BVS_CNT_END);
}

int bvs_clearbgp(BVS_CMD_PARAM *prm)
{
	int		clno;
	struct in_addr	sin;
	char		prline[128];
	int		pn;

	/****
	clno = (prm->adrs).clno;
	sin.s_addr = (prm->adrs).addr;
	****/

	clno = *(int *)prm;
	sin.s_addr = *(u_long *)((char *)prm + sizeof(int));

	sprintf(prline, "Reset Peer : %s\r\n", inet_ntoa(sin));
	bvs_write(clno, prline);
	pn = check_id1topn(sin.s_addr);
	if (pn < 0) {
		bvs_write(clno, "Not found peer\r\n");
	} else {
		if (peer[pn].soc == -1) {
			bvs_write(clno, "Peer is not established\r\n");
		} else {
			clear_peer(pn);
		}
	}
	return(BVS_CNT_END);
}

int bvc_shousers(BVS_CMD_PARAM *prm)
{
	int		clno;
	char		prline[128];
	int		cnt;
	struct in_addr	sin;
	struct timeval	nowtime;
	long		btmin;
	char		mark;

	clno = (prm->noparam).clno;
	cnt = 0;
	bvs_write(clno, "  No Address         LoginTime\r\n");
	while(cnt < (bvs_maxcon-1)) {
		/* if ((bvs_clients + cnt)->clsock != (int)NULL) { */
		if ((bvs_clients + cnt)->clsock != 0) {
			sin.s_addr = htonl((bvs_clients + cnt)->addr);
			gettimeofday(&nowtime, (struct timezone *)0);
			btmin = (nowtime.tv_sec -
				((bvs_clients + cnt)->lgtime).tv_sec)/60;
			if (cnt == clno) {
				mark = '*';
			} else {
				mark = ' ';
			}
			sprintf(prline, "%c %2d %-15s %ld min\r\n",
					mark, cnt, inet_ntoa(sin), btmin);
			bvs_write(clno, prline);
		}
		cnt++;
	}
	return(BVS_CNT_END);
}

int bvc_shoneighbor(BVS_CMD_PARAM *prm)
{
	int		clno;
	struct in_addr	sin;
	char		prline[128];
	int		pn;

	char		*BOOL[] = { "OFF", "ON" };

	/****
	clno = (prm->adrs).clno;
	sin.s_addr = (prm->adrs).addr;
	****/

	clno = *(int *)prm;
	sin.s_addr = *(u_long *)((char *)prm + sizeof(int));

	pn = check_id1topn(sin.s_addr);

#ifdef DEBUG
	printf("DEBUG: SEARCH PEER(bvc_shoneighbor): %08X\n", ntohl(sin.s_addr));
#endif
	if (pn < 0) {
		bvs_write(clno, "Not found peer\r\n");
	} else {
		bvs_write(clno, "\r\nBGP Peer Information:\r\n");
		sprintf(prline, "    Peer Name        : %s\r\n",
					peer[pn].description);
		bvs_write(clno, prline);
		sprintf(prline, "    Neighbor Address : %s\r\n",
					inet_ntoa(sin));
		bvs_write(clno, prline);
		/***
		sprintf(prline, "    Remote AS        : %s\r\n",
					ulongtofourbyteas(peer[pn].remote_as));
		***/
		sprintf(prline, "    Remote AS        : %lu\r\n",
					(u_long)(peer[pn].remote_as));
		bvs_write(clno, prline);
		sin.s_addr = peer[pn].local_id;
		sprintf(prline, "    Individual IDENT : %s\r\n",
					inet_ntoa(sin));
		bvs_write(clno, prline);
		/***
		sprintf(prline, "    Individual AS    : %s\r\n",
					ulongtofourbyteas(peer[pn].local_as));
		***/
		sprintf(prline, "    Individual AS    : %lu\r\n",
					(u_long)(peer[pn].local_as));
		bvs_write(clno, prline);
		if (peer[pn].peerstatus == false) {
			bvs_write(clno, 
				"    BGP Peer Status  : Admin Down\r\n");
		} else {
			sprintf(prline, "    BGP Peer Status  : %s\r\n",
					fsm_state_names[peer[pn].status]);
			bvs_write(clno, prline);
		}
		sprintf(prline, "    Admin Address    : %s\r\n",
					peer[pn].admin_addr);
		bvs_write(clno, prline);
		sprintf(prline, "    Hold Timer       : %d sec\r\n",
					peer[pn].holdtimer);
		bvs_write(clno, prline);
		sprintf(prline, "    Keepalive Timer  : %d sec\r\n",
					peer[pn].keepalive);
		bvs_write(clno, prline);
		sprintf(prline, "    Peer Logfile     : %s\r\n",
					peer[pn].logname);
		bvs_write(clno, prline);
		sprintf(prline, "    Prefix Logfile   : %s\r\n",
					peer[pn].prefixout);
		bvs_write(clno, prline);
		sprintf(prline, "    Notifying Status : %s\r\n",
						BOOL[peer[pn].status_notify]);
		bvs_write(clno, prline);
		sprintf(prline, "    INFO Log Status  : %s\r\n",
						BOOL[bgpinfo[pn].infostatus]);
		bvs_write(clno, prline);
		bvs_write(clno, "    Log Type:\r\n");

		sprintf(prline, "        Global Log               : %s\r\n",
				BOOL[(LG_ISSET(peer[pn].logtype, VLOG_GLOB)!=0)]);
		bvs_write(clno, prline);
		sprintf(prline, "        Open Message Log         : %s\r\n",
				BOOL[(LG_ISSET(peer[pn].logtype, VLOG_OPEN)!=0)]);
		bvs_write(clno, prline);
		sprintf(prline, "        Updata Message Log       : %s\r\n",
				BOOL[(LG_ISSET(peer[pn].logtype, VLOG_UPDE)!=0)]);
		bvs_write(clno, prline);
		sprintf(prline, "        Notification Message Log : %s\r\n",
				BOOL[(LG_ISSET(peer[pn].logtype, VLOG_NOTI)!=0)]);
		bvs_write(clno, prline);
		sprintf(prline, "        Keep Alive Message Log   : %s\r\n",
				BOOL[(LG_ISSET(peer[pn].logtype, VLOG_KEEP)!=0)]);
		bvs_write(clno, prline);
		sprintf(prline, "        Updata Attribute Log     : %s\r\n",
				BOOL[(LG_ISSET(peer[pn].logtype, VLOG_UPAT)!=0)]);
		bvs_write(clno, prline);
		sprintf(prline, "        Updata Packet Dump Log   : %s\r\n\r\n",
				BOOL[(LG_ISSET(peer[pn].logtype, VLOG_UPDU)!=0)]);
		bvs_write(clno, prline);
	}
	return(BVS_CNT_END);
}

#ifdef HAVE_TOP
int bvc_shoproc(BVS_CMD_PARAM *prm)
{
        int             clno;
	char            prline[128];
	int             wri[2],rea[2];
	int		pid;
	int		wfp,rfp;
	int		bcnt, m;
	char		cc;

	clno = (prm->noparam).clno;

	if (pipe(wri) < 0) {
		return(BVS_CNT_END);
	}
	if (pipe(rea) < 0) {
		close(wri[0]);
		close(wri[1]);
		return(BVS_CNT_END);
	}
	if ((pid = fork()) < 0) {
		close(wri[0]);
		close(wri[1]);
		close(rea[0]);
		close(rea[1]);
		return(BVS_CNT_END);
	}

	if (pid == 0) {
		close(wri[1]);
		close(rea[0]);

		close(0);
		dup(wri[0]);
		close(wri[0]);

		close(1);
		dup(rea[1]);
		close(rea[1]);

		execl(TOPCMD, "top", "-b", (char *)NULL );
		perror("top command exec error");
		pr_log(VLOG_GLOB, LOG_ALL, 
				"top command exec error\n", ON);
		bvs_write(clno, "top command exec error\r\n");
		return(BVS_CNT_END);
	}

	close(wri[0]);
	close(rea[1]);
	rfp = rea[0];
	wfp = wri[1];

	bcnt = 0;
	bzero(prline, 128);
	while(m > -1) {
		cc = '\0';
		m = read(rfp, &cc, 1);
		if ((cc == '\0') && (m == 0)) break;
		if (cc == '\n') {
			prline[bcnt++] = '\r';
			prline[bcnt++] = '\n';
		} else {
			prline[bcnt++] = cc;
		}
		if (bcnt >= 120) {
			bvs_write(clno, prline);
			bcnt = 0;
			bzero(prline, 128);
		}
	}
	bvs_write(clno, prline);

	close(wfp);
	close(rfp);
	/* while (wait((int *)0) != pid); */
	return(BVS_CNT_END);

}
#endif /* HAVE_TOP */

int bvc_shostatics(BVS_CMD_PARAM *prm)
{
	int		clno;
	struct in_addr	sin;
	char		prline[128];
	int		pn;
	struct timeval	now;
	long		dift;

	/***
	clno = (prm->adrs).clno;
	sin.s_addr = (prm->adrs).addr;
	***/

	clno = *(int *)prm;
	sin.s_addr = *(u_long *)((char *)prm + sizeof(int));

	pn = check_id1topn(sin.s_addr);
	if (pn < 0) {
		bvs_write(clno, "Not found peer\r\n");
	} else {
		bvs_write(clno, "\r\n  BGP Peer Message Statics:\r\n");
		sprintf(prline, "    Neighbor Address   : %s\r\n",
							inet_ntoa(sin));
		bvs_write(clno, prline);
		sprintf(prline, "    Log File Name      : %s\r\n",
							bgpinfo[pn].fname);
		bvs_write(clno, prline);
		if (strlen(bgpinfo[pn].fname) == 0) {
			sprintf(prline, "    Log Write Interval : Never\r\n");
		} else {
			sprintf(prline, "    Log Write Interval : %d sec\r\n",
							bgpinfo[pn].interval);
		}
		bvs_write(clno, prline);
		sprintf(prline, "    Received Messages  : %d\r\n",
							bgpinfo[pn].msgcnt);
		bvs_write(clno, prline);
		sprintf(prline, "      Update           : %d\r\n",
							bgpinfo[pn].updatecnt);
		bvs_write(clno, prline);
		sprintf(prline, "      Notification     : %d\r\n",
						bgpinfo[pn].notificationcnt);
		bvs_write(clno, prline);
		sprintf(prline, "      Open             : %d\r\n",
							bgpinfo[pn].opencnt);
		bvs_write(clno, prline);
		sprintf(prline, "      KeepAlive        : %d\r\n",
							bgpinfo[pn].keepalivecnt);
		bvs_write(clno, prline);
		sprintf(prline, "    Total Prefix       : %d\r\n",
							bgpinfo[pn].totalprefix);
		bvs_write(clno, prline);
		sprintf(prline, "      Update           : %d\r\n",
							bgpinfo[pn].prefixcnt);
		bvs_write(clno, prline);
		sprintf(prline, "      Withdraw         : %d\r\n",
							bgpinfo[pn].withdrawcnt);
		bvs_write(clno, prline);
		if (bgpinfo[pn].update.tv_sec != 0) {
			gettimeofday(&now, (struct timezone *)0);
			dift = now.tv_sec - bgpinfo[pn].update.tv_sec;
			sprintf(prline,
			   "    Last Update Clear  : %02d:%02d:%02d ago\r\n",
						dift/3600, (dift%3600)/60,
						dift%60);
			bvs_write(clno, prline);
		} else {
			bvs_write(clno, "    Last Update Clear  : Never\r\n");
		}
		bvs_write(clno, "\r\n");
	}
	return(BVS_CNT_END);
}

int bvc_clrstatics(BVS_CMD_PARAM *prm)
{
	int		clno;
	struct in_addr	sin;
	char		prline[128];
	int		pn;

	/***
	clno = (prm->adrs).clno;
	sin.s_addr = (prm->adrs).addr;
	****/

	clno = *(int *)prm;
	sin.s_addr = *(u_long *)((char *)prm + sizeof(int));

	pn = check_id1topn(sin.s_addr);
	if (pn < 0) {
		bvs_write(clno, "Not found peer\r\n");
	} else {
		sprintf(prline, "Clearing BGP Statics for %s\r\n",
							inet_ntoa(sin));
		bvs_write(clno, prline);
		recount_info(pn);
	}
	return(BVS_CNT_END);
}

int bvc_clrstaticsall(BVS_CMD_PARAM *prm)
{
	int		clno;

	clno = (prm->noparam).clno;

	bvs_write(clno, "Clearing All BGP Statics\r\n");
	recount_all();
	return(BVS_CNT_END);
}

int bvc_shoversion(BVS_CMD_PARAM *prm)
{
	int		clno;
	struct in_addr	sin;
	char		prline[2048];
	char		bufline[2048];
	int		cnt;
	struct timeval	curtime;
	long		timediff;
#ifdef RTHISTORY
	struct tm	*tm;
#endif

	clno = (prm->noparam).clno;

	sprintf(prline, "\r\nBGPView Veriosn %s\r\n", BV_VERSION);
	bvs_write(clno, prline);
	sprintf(prline, "%s\r\n", BV_COPYRIGHT);
	bvs_write(clno, prline);
	sprintf(prline, "All rights reserved.\r\n");
	bvs_write(clno, prline);
	gettimeofday(&curtime, (struct timezone *)0);
	timediff = curtime.tv_sec - uptime.tv_sec;
	sprintf(prline, "System up to %s\r\n\r\n", sectoptm(timediff, bufline));
	bvs_write(clno, prline);
	sprintf(prline, "System Configurations:\r\n");
	bvs_write(clno, prline);
	sprintf(prline, "  Configuration File      : %s\r\n", configfile);
	bvs_write(clno, prline);
	sin.s_addr = IDENT;
	sprintf(prline, "  Local BGP Identifier    : %s\r\n", inet_ntoa(sin));
	bvs_write(clno, prline);
	/***
	sprintf(prline, "  Local AS Number         : %d\r\n", 
						ulongtofourbyteas(LOCAL_AS));
	***/
	sprintf(prline, "  Local AS Number         : %lu\r\n", 
						(u_long)(LOCAL_AS));
	bvs_write(clno, prline);
	sprintf(prline, "  Sendmail Command        : %s\r\n", sendmail_command);
	bvs_write(clno, prline);
	sprintf(prline, "  Administrator Address   : %s\r\n", admin_addr);
	bvs_write(clno, prline);
	sprintf(prline, "  Notify Address          : %s\r\n", mail_addr);
	bvs_write(clno, prline);
	sprintf(prline, "  Default Spool Directory : %s\r\n", spooldir);
	bvs_write(clno, prline);
	if (facility == -1) {
		sprintf(prline, "  System Logfile Name     : %s\r\n", 
							syslogname);
	} else {
		sprintf(prline, "  Syslog Facility         : %s\r\n",
							&syslogname[1]);
	}
	bvs_write(clno, prline);
	sprintf(prline, "  Dummuy Route Config     : %s\r\n", dmyroutef);
	bvs_write(clno, prline);
	cnt = 0;
	while(peer[cnt].neighbor != 0) {
		cnt++;
	}
	sprintf(prline, "  Configurated Peer Num.  : %d\r\n", cnt);
	bvs_write(clno, prline);
#ifdef RTHISTORY
	if (cleaned_time == 0) {
		bvs_write(clno, "  Last route history clr         : Never\r\n");
	} else {
		tm = localtime(&cleaned_time);
		strftime(bufline, TIME_BUF, "%Y/%m/%d %H:%M:%S", tm);
		sprintf(prline, "  Last route history clr: %s\r\n", bufline);
		bvs_write(clno, prline);
	}
	sprintf(prline, "  Cleaning route history interval: %ld\r\n",
							rthistcleartime.tv_sec);
	bvs_write(clno, prline);
	if (cleaned_time_v6 == 0) {
		bvs_write(clno, "  Last IPv6 route history clear  : Never\r\n");
	} else {
		tm = localtime(&cleaned_time_v6);
		strftime(bufline, TIME_BUF, "%Y/%m/%d %H:%M:%S", tm);
		sprintf(prline, "  Last IPv6 route history clr: %s\r\n", bufline);
		bvs_write(clno, prline);
	}
	sprintf(prline, "  Cleaning IPv6 route histry int : %ld\r\n",
						rthistcleartime_v6.tv_sec);
	bvs_write(clno, prline);
#endif
	bvs_write(clno, "\r\n");
	return(BVS_CNT_END);
}

int bvc_shoconfig(BVS_CMD_PARAM *prm)
{
	int		clno;
	FILE		*fd;
	char		prline[128];
	int		ccnt;

	clno = (prm->noparam).clno;

	if ((fd = fopen(configfile, "r")) == NULL) {
		bvs_write(clno, "Configuration File Could no opened\r\n");
		return(BVS_CNT_END);
	}
	while(!feof(fd)) {
		bzero(prline, 128);
		fgets(prline, 120, fd);
		ccnt = strlen(prline);
		if (prline[ccnt-1] == '\n') {
			prline[ccnt-1] = '\r';
			prline[ccnt] = '\n';
		}
		bvs_write(clno, prline);
	}
	fclose(fd);
	bvs_write(clno, "\r\n");
	return(BVS_CNT_END);
}

int bvc_doannouncedummy(BVS_CMD_PARAM *prm)
{
	int		clno;
	int		ret;

	clno = (prm->noparam).clno;

	ret = getdmyinfo();
	switch(ret) {
	case 1:
		bvs_write(clno, "Data File Could Not read\r\n");
		break;
	case 2:
		bvs_write(clno, "Target Peer is not Established\r\n");
		break;
	case 3:
		bvs_write(clno, "Now Sending\r\n");
		break;
	case 4:
		bvs_write(clno, "Dump File Not Configured\r\n");
		break;
	case 5:
		bvs_write(clno, "Dump File Could Not Opened\r\n");
		break;
	default:
		bvs_write(clno, "Advertising Dummy Route\r\n");
	}
	return(BVS_CNT_END);
}

int bvc_shoannouncedummy(BVS_CMD_PARAM *prm)
{
	int		clno;
	struct in_addr	sin;
	char		prline[128];

	clno = (prm->noparam).clno;

	bvs_write(clno, "\r\nBGPView Dummy Route Advertisment Status:\r\n");
	if (dmyroute.pn == -1) {
		bvs_write(clno, "  Not be advertising\r\n\r\n");
		return(BVS_CNT_END);
	}

	sin.s_addr = dmyroute.neighbor;
	sprintf(prline, "  Neighbor Address     : %s\r\n", inet_ntoa(sin));
	bvs_write(clno, prline);

	strcpy(prline, "  Annouce Type         : ");
	switch(dmyroute.type) {
	case DMY_WITHDRAW: strcat(prline, "WITHDRAW\r\n");
			   break;
	case DMY_UPDATE:   strcat(prline, "UPDATE\r\n");
			   break;
	case DMY_PLAYBACK: strcat(prline, "PLAYBACK\r\n");
			   break;
	default:           strcat(prline, "Unknown Mode\r\n");
	}
	bvs_write(clno, prline);

	if ((dmyroute.type == DMY_WITHDRAW) || (dmyroute.type == DMY_UPDATE)) {
		sin.s_addr = dmyroute.start;
		sprintf(prline, "  Start Prefix Address : %s\r\n",
							inet_ntoa(sin));
		bvs_write(clno, prline);
		sprintf(prline, 
			"  Advertised Prefix    : %ld/%ld(%4.1f%c) prefixes\r\n",
			dmyroute.current, dmyroute.cnt,
			(((float)dmyroute.current/(float)dmyroute.cnt)*100.0), 
			'%');
		bvs_write(clno, prline);
		sprintf(prline,
			"  Prefixes per Packet  : %d\r\n", dmyroute.pack);
		bvs_write(clno, prline);
		sprintf(prline,
			"  Advertising AS Path  : %s\r\n", dmyroute.aspath);
		bvs_write(clno, prline);
	}
	bvs_write(clno, "\r\n");
	return(BVS_CNT_END);
}

int bvc_dorefleshsystemlog(BVS_CMD_PARAM *prm)
{
	int		clno;

	clno = (prm->noparam).clno;

	restart_log();
	bvs_write(clno, "BGPView System Log File Restarted\r\n");
	return(BVS_CNT_END);
}

int bvc_dorefleshprefixlogall(BVS_CMD_PARAM *prm)
{
	int		clno;

	clno = (prm->noparam).clno;

	init_prefixlog();
	bvs_write(clno, "All BGPView Prefix Log File Restarted\r\n");
	return(BVS_CNT_END);
}

int bvc_doroutertt(BVS_CMD_PARAM *prm)
{
	int		clno;
	u_long		neighbor;
	u_long		prefix;
	int		prefixlen;
	int		timeout;
	int		ret;

	/**** My be bugged **** from here 
	clno 	  = (prm->routertt).clno;
	neighbor  = (prm->routertt).neighbor;
	prefix    = (prm->routertt).prefix;
	prefixlen = (prm->routertt).prefixlen;
	timeout   = (prm->routertt).timeout;
	**** to here ***/

	ret = newcheckrtt(clno, neighbor, prefix, prefixlen, timeout);
	switch(ret) {
	case -1: bvs_write(clno, "!! Many Route Test Process is Running\r\n");
		 break;
	}
	if (ret >= 0) return(BVS_CNT_NOPROMPT);
	return(BVS_CNT_END);
}

int bvc_doroutertt_notimeout(BVS_CMD_PARAM *prm)
{
	int		clno;
	u_long		neighbor;
	u_long		prefix;
	int		prefixlen;
	int		timeout;
	int		ret;

	clno 	  = (prm->routertt).clno;
	neighbor  = (prm->routertt).neighbor;
	prefix    = (prm->routertt).prefix;
	prefixlen = (prm->routertt).prefixlen;
	timeout   = ROUTERTT_TIMEOUT;

	ret = newcheckrtt(clno, neighbor, prefix, prefixlen, timeout);
	switch(ret) {
	case -1: bvs_write(clno, "!! Many Route Test Process is Running\r\n");
		 break;
	}
	if (ret >= 0) return(BVS_CNT_NOPROMPT);
	return(BVS_CNT_END);
}

int bvc_set_vtyrowsize(BVS_CMD_PARAM *prm)
{
	int	clno;
	int	num;
	char	prline[128];

	clno = (prm->one_num).clno;
	num  = (prm->one_num).num;

	(bvs_clients+clno)->rowsize = num;
	sprintf(prline, "BVS Row Size was set %d lines\r\n", num);
	bvs_write(clno, prline);
	return(BVS_CNT_END);
}

#ifdef RADIX
#ifdef RTHISTORY
int pr_routesum(int clno, R_LIST *rlist, net_ulong neighbor, boolean detail,
		int type)
#endif
#ifndef RTHISTORY
int pr_routesum(int clno, R_LIST *rlist, net_ulong neighbor)
#endif
{
	R_LIST		*cptr;
	ROUTE_INFO	*rinfo;
	struct in_addr	sin;
	char		prline[1024], prline2[1024];
	struct timeval	now;
	long		timediff;
	char		tmstr[10];
#ifdef RTHISTORY
	RT_HISTORY	*histptr;
	time_t		pcl;
	struct tm	*tm;
#endif
	
	/* if neighbor is Zero, then show all routes */

	cptr = rlist;
	while(cptr != NULL) {
		rinfo = cptr->route_data;

		if ((neighbor != 0) && (rinfo->source != neighbor)) {
			cptr = cptr->next;
			continue;
		}

#ifdef RTHISTORY
		if (type != RTV_ALL) {
		  if ( ((type == RTV_ACTIVE) && (rinfo->active == false)) ||
		       ((type == RTV_INACTIVE) && (rinfo->active == true)) ) {
			cptr = cptr->next;
			continue;
		  }
		}
#endif

		gettimeofday(&now, (struct timezone *)0);
		timediff = now.tv_sec - rinfo->lastupdate.tv_sec;

		sin.s_addr = rinfo->prefix;
		sprintf(prline2, "%s/%d ", inet_ntoa(sin), rinfo->length);
#ifdef RTHISTORY
		if (rinfo->active == false) {
			sprintf(prline, "! %-17s", prline2);
		} else {
			sprintf(prline, "  %-17s", prline2);
		}
#endif
#ifndef RTHISTORY
		sprintf(prline, "  %-17s", prline2);
#endif

		if (ATRFL_ISSET(rinfo->attribute_flag, SET_NEXTHOP)) {
			sin.s_addr = rinfo->next_hop;
			sprintf(prline2, "%-17s", inet_ntoa(sin));
		} else {
			strcpy(prline2, "                 ");
		}
		strcat(prline, prline2);
		if (ATRFL_ISSET(rinfo->attribute_flag, SET_METRIC)) {
			sprintf(prline2, "%7d", rinfo->metric);
		} else {
			strcpy(prline2, "       ");
		}
		strcat(prline, prline2);
		if (ATRFL_ISSET(rinfo->attribute_flag, SET_LOCALPREF)) {
			sprintf(prline2, "%7d ", rinfo->local_perf);
		} else {
			strcpy(prline2, "        ");
		}
		strcat(prline, prline2);
		sprintf(prline2, "%-8s ", sectoptm(timediff, tmstr));
		strcat(prline, prline2);
		/* strcat(prline, rinfo->path_set); */
		if (rinfo->pathset_ptr != NULL) {
			strcat(prline, rinfo->pathset_ptr);
		}
		if (ATRFL_ISSET(rinfo->attribute_flag, SET_ORIGIN)) {
			switch(rinfo->origin_type) {
			case ORIGIN_IGP:
				strcat(prline, "i\r\n");
				break;
			case ORIGIN_EGP:
				strcat(prline, "e\r\n");
				break;
			case ORIGIN_INCOMPLETE:
				strcat(prline, "?\r\n");
				break;
			}
		}
		bvs_write(clno, prline);
#ifdef RTHISTORY
		/*** Display Update History ***/
		if ((rinfo->history != NULL) && (detail == true)){
			pcl = rinfo->firstupdate.tv_sec;
			tm = localtime(&pcl);
			strftime(prline, 128,
	 "    == Prefix Update History (First %Y/%m/%d %H:%M:%S) ==\r\n", tm);
			bvs_write(clno, prline);
			histptr = rinfo->history;
			while(histptr != NULL) {
			  pcl = histptr->update.tv_sec;
			  tm = localtime(&pcl);
			  strftime(prline, 30, "    %Y/%m/%d %H:%M:%S : ", tm);
			  if (histptr->type == UPDATE) {
				strcat(prline, "UPDATE\r\n");
			  } else {
				strcat(prline, "WITHDRAW\r\n");
			  }
			  bvs_write(clno, prline);
			  histptr = histptr->next;
			}
		}
#endif

		cptr = cptr->next;
	}

}

#ifdef RTHISTORY
int show_bgproute(BVS_CMD_PARAM *prm, int dtype, boolean detail)
#endif
#ifndef RTHISTORY
int bvc_show_bgproute_all(BVS_CMD_PARAM *prm)
#endif
{
	int		clno;
	static RADIX_T	**cnode;
	static int	*upcode;
	RADIX_T		*node;

	if (prm == NULL) {
		cnode = (RADIX_T **)malloc(sizeof(RADIX_T *) * bvs_maxcon);
		bzero(cnode, sizeof(RADIX_T *) * bvs_maxcon);
		upcode = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(upcode, sizeof(int) * bvs_maxcon);
		return(0);
	}

	clno = (prm->noparam).clno;

	if (*(cnode+clno) == NULL) {
		bvs_write(clno, "\r\n ! : Inactive Route\r\n\r\n");
		bvs_write(clno, 
"   Prefix           Next Hop         Metric Loc_Pef Time    AS_Path\r\n");
	}

	node = show_route_all(*(cnode+clno), (upcode+clno));
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			bvs_write(clno, "%% No Route\r\n");
		}
/****
		else {
			bvs_write(clno, "%% No More Route\r\n");
		}
***/
	} else {
#ifdef RTHISTORY
		pr_routesum(clno, node->rinfo, 0, detail, dtype);
#endif
#ifndef RTHISTORY
		pr_routesum(clno, node->rinfo, 0);
#endif
	}
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			*(cnode+clno) = node;
			return(BVS_CNT_END);
		} else {
			*(cnode+clno) = node;
			return(BVS_CNT_FINISH);
		}
	} else {
		*(cnode+clno) = node;
		return(BVS_CNT_CONTINUE);
	}
}

#ifdef RTHISTORY
int bvc_show_bgproute_all(BVS_CMD_PARAM *prm)
{
	return(show_bgproute(prm, RTV_ALL, false));
}

int bvc_show_bgproute_active(BVS_CMD_PARAM *prm)
{
	return(show_bgproute(prm, RTV_ACTIVE, false));
}

int bvc_show_bgproute_inactive(BVS_CMD_PARAM *prm)
{
	return(show_bgproute(prm, RTV_INACTIVE, false));
}

int bvc_show_bgproute_all_detail(BVS_CMD_PARAM *prm)
{
	return(show_bgproute(prm, RTV_ALL, true));
}

int bvc_show_bgproute_active_detail(BVS_CMD_PARAM *prm)
{
	return(show_bgproute(prm, RTV_ACTIVE, true));
}

int bvc_show_bgproute_inactive_detail(BVS_CMD_PARAM *prm)
{
	return(show_bgproute(prm, RTV_INACTIVE, true));
}
#endif

#ifdef RTHISTORY
int output_routesum(R_LIST *rlist, net_ulong neighbor, FILE *ofp,
			boolean detail, int type)
#endif
#ifndef RTHISTORY
int output_routesum(R_LIST *rlist, net_ulong neighbor, FILE *ofp)
#endif
{
	R_LIST		*cptr;
	ROUTE_INFO	*rinfo;
	struct in_addr	sin;
	char		prline[1024], prline2[1024];
	struct timeval	now;
	long		timediff;
	char		tmstr[10];

#ifdef RTHISTORY
	RT_HISTORY	*histptr;
	time_t		pcl;
	struct tm	*tm;
#endif
	
	/* if neighbor is Zero, then show all routes */

	cptr = rlist;
	while(cptr != NULL) {
		rinfo = cptr->route_data;

		if ((neighbor != 0) && (rinfo->source != neighbor)) {
			cptr = cptr->next;
			continue;
		}

#ifdef RTHISTORY
		if (type != RTV_ALL) {
		  if ( ((type == RTV_ACTIVE) && (rinfo->active == false)) ||
		       ((type == RTV_INACTIVE) && (rinfo->active == true)) ) {
			cptr = cptr->next;
			continue;
		  }
		}
#endif

		gettimeofday(&now, (struct timezone *)0);
		timediff = now.tv_sec - rinfo->lastupdate.tv_sec;

		sin.s_addr = rinfo->prefix;
		sprintf(prline2, "%s/%d ", inet_ntoa(sin), rinfo->length);
#ifdef RTHISTORY
		if (rinfo->active == false) {
			sprintf(prline, "! %-17s", prline2);
		} else {
			sprintf(prline, "  %-17s", prline2);
		}
#endif
#ifndef RTHISTORY
		sprintf(prline, "  %-17s", prline2);
#endif

		if (ATRFL_ISSET(rinfo->attribute_flag, SET_NEXTHOP)) {
			sin.s_addr = rinfo->next_hop;
			sprintf(prline2, "%-17s", inet_ntoa(sin));
		} else {
			strcpy(prline2, "                 ");
		}
		strcat(prline, prline2);
		if (ATRFL_ISSET(rinfo->attribute_flag, SET_METRIC)) {
			sprintf(prline2, "%7d", rinfo->metric);
		} else {
			strcpy(prline2, "       ");
		}
		strcat(prline, prline2);
		if (ATRFL_ISSET(rinfo->attribute_flag, SET_LOCALPREF)) {
			sprintf(prline2, "%7d ", rinfo->local_perf);
		} else {
			strcpy(prline2, "        ");
		}
		strcat(prline, prline2);
		sprintf(prline2, "%-8s ", sectoptm(timediff, tmstr));
		strcat(prline, prline2);
		/* strcat(prline, rinfo->path_set); */
		if (rinfo->pathset_ptr != NULL) {
			strcat(prline, rinfo->pathset_ptr);
		}
		if (ATRFL_ISSET(rinfo->attribute_flag, SET_ORIGIN)) {
			switch(rinfo->origin_type) {
			case ORIGIN_IGP:
				strcat(prline, "i\n");
				break;
			case ORIGIN_EGP:
				strcat(prline, "e\n");
				break;
			case ORIGIN_INCOMPLETE:
				strcat(prline, "?\n");
				break;
			}
		}
		fprintf(ofp, prline);

#ifdef RTHISTORY
		/*** Output Update History ***/
		if ((rinfo->history != NULL) && (detail == true)){
			pcl = rinfo->firstupdate.tv_sec;
			tm = localtime(&pcl);
			strftime(prline, 128,
	 "    == Prefix Update History (First %Y/%m/%d %H:%M:%S) ==\n", tm);
			fprintf(ofp, prline);
			histptr = rinfo->history;
			while(histptr != NULL) {
			  pcl = histptr->update.tv_sec;
			  tm = localtime(&pcl);
			  strftime(prline, TIME_BUF, "  %Y/%m/%d %H:%M:%S : ", tm);
			  if (histptr->type == UPDATE) {
				strcat(prline, "UPDATE\n");
			  } else {
				strcat(prline, "WITHDRAW\n");
			  }
			  fprintf(ofp, "  %s", prline);
			  histptr = histptr->next;
			}
		}
#endif
		cptr = cptr->next;
	}

}

#ifdef RTHISTORY
int output_bgproute(BVS_CMD_PARAM *prm, boolean detail, int dtype)
#endif
#ifndef RTHISTORY
int bvc_output_bgproute_all(BVS_CMD_PARAM *prm)
#endif
{
	int		clno;
	static RADIX_T	**cnode;
	static int	*upcode;
	RADIX_T		*node;
	char		fname[FNAMESIZE];
	char		fn[FNAMESIZE];
	static FILE	*fp;

	if (prm == NULL) {
		cnode = (RADIX_T **)malloc(sizeof(RADIX_T *) * bvs_maxcon);
		bzero(cnode, sizeof(RADIX_T *) * bvs_maxcon);
		upcode = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(upcode, sizeof(int) * bvs_maxcon);
		return(0);
	}

	clno = (prm->one_str64).clno;
	strcpy(fn, (prm->one_str64).str);
	if (fn[0] == '/') {
		strcpy(fname, fn);
	} else {
		strcpy(fname, spooldir);
		strcat(fname, "/");
		strcat(fname, fn);
	}

	if (*(cnode+clno) == NULL) {
		if ((fp = fopen(fname, "w")) == NULL) {
			bvs_write(clno, "Could not open target file.\r\n");
			return(BVS_CNT_END);
		}

		bvs_write(clno, "Now Outputting Routes to '");
		bvs_write(clno, fname);
		bvs_write(clno, "'\r\n");
		fprintf(fp, 
"   Prefix           Next Hop         Metric Loc_Pef Time    AS_Path\n");
	}

	node = show_route_all(*(cnode+clno), (upcode+clno));
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			fprintf(fp, "%% No Route\n");
		}
	} else {
#ifdef RTHISTORY
		output_routesum(node->rinfo, 0, fp, detail, dtype);
#endif
#ifndef RTHISTORY
		output_routesum(node->rinfo, 0, fp);
#endif
	}
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			*(cnode+clno) = node;
			fclose(fp);
			bvs_write(clno, "Done(No Route)..\r\n");
			return(BVS_CNT_END);
		} else {
			*(cnode+clno) = node;
			fclose(fp);
			bvs_write(clno, "Done..\r\n");
			return(BVS_CNT_FINISH);
		}
	} else {
		*(cnode+clno) = node;
		return(BVS_CNT_CONTINUE);
	}
}

#ifdef RTHISTORY
int bvc_output_bgproute_all(BVS_CMD_PARAM *prm){
	return(output_bgproute(prm, false, RTV_ALL));
}

int bvc_output_bgproute_active(BVS_CMD_PARAM *prm){
	return(output_bgproute(prm, false, RTV_ACTIVE));
}

int bvc_output_bgproute_inactive(BVS_CMD_PARAM *prm){
	return(output_bgproute(prm, false, RTV_INACTIVE));
}

int bvc_output_bgproute_all_detail(BVS_CMD_PARAM *prm){
	return(output_bgproute(prm, true, RTV_ALL));
}

int bvc_output_bgproute_active_detail(BVS_CMD_PARAM *prm){
	return(output_bgproute(prm, true, RTV_ACTIVE));
}

int bvc_output_bgproute_inactive_detail(BVS_CMD_PARAM *prm){
	return(output_bgproute(prm, true, RTV_INACTIVE));
}
#endif

#ifdef RTHISTORY
int output_bgproute_nei(BVS_CMD_PARAM *prm, boolean detail, int dtype)
#endif
#ifndef RTHISTORY
int bvc_output_bgproute_all_nei(BVS_CMD_PARAM *prm)
#endif
{
	int		clno;
	static RADIX_T	**cnode;
	static int	*upcode;
	RADIX_T		*node;
	char		fname[FNAMESIZE];
	char		fn[FNAMESIZE];
	static FILE	*fp;
	u_long		neighbor;

	if (prm == NULL) {
		cnode = (RADIX_T **)malloc(sizeof(RADIX_T *) * bvs_maxcon);
		bzero(cnode, sizeof(RADIX_T *) * bvs_maxcon);
		upcode = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(upcode, sizeof(int) * bvs_maxcon);
		return(0);
	}

	clno = (prm->one_str64_adrs).clno;
	strcpy(fn, (prm->one_str64_adrs).str);
	neighbor = (prm->one_str64_adrs).addr;

	if (fn[0] == '/') {
		strcpy(fname, fn);
	} else {
		strcpy(fname, spooldir);
		strcat(fname, "/");
		strcat(fname, fn);
	}

	if (*(cnode+clno) == NULL) {
		if ((fp = fopen(fname, "w")) == NULL) {
			bvs_write(clno, "Could not open target file.\r\n");
			return(BVS_CNT_END);
		}

		bvs_write(clno, "Now Outputting Routes to '");
		bvs_write(clno, fname);
		bvs_write(clno, "'\r\n");
		fprintf(fp, 
"   Prefix           Next Hop         Metric Loc_Pef Time    AS_Path\n");
	}

	node = show_route_all(*(cnode+clno), (upcode+clno));
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			fprintf(fp, "%% No Route\n");
		}
	} else {
#ifdef RTHISTORY
		output_routesum(node->rinfo, neighbor, fp, detail, dtype);
#endif
#ifndef RTHISTORY
		output_routesum(node->rinfo, neighbor, fp);
#endif
	}
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			*(cnode+clno) = node;
			fclose(fp);
			bvs_write(clno, "Done(No Route)..\r\n");
			return(BVS_CNT_END);
		} else {
			*(cnode+clno) = node;
			fclose(fp);
			bvs_write(clno, "Done..\r\n");
			return(BVS_CNT_FINISH);
		}
	} else {
		*(cnode+clno) = node;
		return(BVS_CNT_CONTINUE);
	}
}

#ifdef RTHISTORY
int bvc_output_bgproute_all_nei(BVS_CMD_PARAM *prm){
	return(output_bgproute_nei(prm, false, RTV_ALL));
}

int bvc_output_bgproute_active_nei(BVS_CMD_PARAM *prm){
	return(output_bgproute_nei(prm, false, RTV_ACTIVE));
}

int bvc_output_bgproute_inactive_nei(BVS_CMD_PARAM *prm){
	return(output_bgproute_nei(prm, false, RTV_INACTIVE));
}

int bvc_output_bgproute_all_detail_nei(BVS_CMD_PARAM *prm){
	return(output_bgproute_nei(prm, true, RTV_ALL));
}

int bvc_output_bgproute_active_detail_nei(BVS_CMD_PARAM *prm){
	return(output_bgproute_nei(prm, true, RTV_ACTIVE));
}

int bvc_output_bgproute_inactive_detail_nei(BVS_CMD_PARAM *prm){
	return(output_bgproute_nei(prm, true, RTV_INACTIVE));
}
#endif

#ifdef RTHISTORY
int show_bgproute_neighbor(BVS_CMD_PARAM *prm, int dtype)
#endif
#ifndef RTHISTORY
int bvc_show_bgproute_neighbor(BVS_CMD_PARAM *prm)
#endif
{
	int		clno;
	static RADIX_T	**cnode;
	static int	*upcode;
	RADIX_T		*node;
	net_ulong	neigh;

	if (prm == NULL) {
		cnode = (RADIX_T **)malloc(sizeof(RADIX_T *) * bvs_maxcon);
		bzero(cnode, sizeof(RADIX_T *) * bvs_maxcon);
		upcode = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(upcode, sizeof(int) * bvs_maxcon);
		return(0);
	}

	/*****
	clno = (prm->adrs).clno;
	neigh = (prm->adrs).addr;
	*****/

	clno = *(int *)prm;
	neigh = *(net_ulong *)((char *)prm + sizeof(int));

#ifdef DEBUG
	printf("DEBUG: SEARCH Neigh(bvc_show_bgproute_neighbor): %08X\n", ntohl(neigh));
#endif

	if (*(cnode+clno) == NULL) {
		bvs_write(clno, "\r\n ! : Inactive Route\r\n\r\n");
		bvs_write(clno, 
"   Prefix           Next Hop         Metric Loc_Pef Time    AS_Path\r\n");
	}

	node = show_route_all(*(cnode+clno), (upcode+clno));
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			bvs_write(clno, "%% No Route\r\n");
		}
	} else {
#ifdef RTHISTORY
		pr_routesum(clno, node->rinfo, neigh, false, dtype);
#endif
#ifndef RTHISTORY
		pr_routesum(clno, node->rinfo, neigh);
#endif
	}
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			*(cnode+clno) = node;
			return(BVS_CNT_END);
		} else {
			*(cnode+clno) = node;
			return(BVS_CNT_FINISH);
		}
	} else {
		*(cnode+clno) = node;
		return(BVS_CNT_CONTINUE);
	}
}

#ifdef RTHISTORY
int bvc_show_bgproute_neighbor(BVS_CMD_PARAM *prm)
{
	return(show_bgproute_neighbor(prm, RTV_ALL));
}

int bvc_show_bgproute_neighbor_active(BVS_CMD_PARAM *prm)
{
	return(show_bgproute_neighbor(prm, RTV_ACTIVE));
}

int bvc_show_bgproute_neighbor_inactive(BVS_CMD_PARAM *prm)
{
	return(show_bgproute_neighbor(prm, RTV_INACTIVE));
}
#endif

int bvc_showipbgproute(BVS_CMD_PARAM *prm)
{
	int		clno;
	net_ulong	prefix;
	RADIX_T		*node;

	/***
	clno = (prm->adrs).clno;
	prefix = (net_ulong)(prm->adrs).addr;
	***/

	clno = *(int *)prm;
	prefix = *(net_ulong *)((char *)prm + sizeof(int));

#ifdef DEBUG
	printf("DEBUG: SEARCH ROUTE: %08X\n", ntohl(prefix));
	printf("DEBIG: prm print: %i / %08X\n", *(int *)prm, *(u_long *)(prm + sizeof(int)));
	printf("DEBUG: Param DUMP: %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X\n",
		*((char *)prm),
		*((char *)prm+1),
		*((char *)prm+2),
		*((char *)prm+3),
		*((char *)prm+4),
		*((char *)prm+5),
		*((char *)prm+6),
		*((char *)prm+7),
		*((char *)prm+8),
		*((char *)prm+9),
		*((char *)prm+10),
		*((char *)prm+11),
		*((char *)prm+12),
		*((char *)prm+13),
		*((char *)prm+14),
		*((char *)prm+15) );
#endif

	bvs_write(clno, "\r\n ! : Inactive Route\r\n\r\n");
	bvs_write(clno, 
"   Prefix           Next Hop         Metric Loc_Pef Time    AS_Path\r\n");

	node = search_route(prefix, 32);
	if (node == NULL) {
		bvs_write(clno, "%% No Route\r\n");
	} else {
#ifdef RTHISTORY
		pr_routesum(clno, node->rinfo, 0, true, RTV_ALL);
#endif
#ifndef RTHISTORY
		pr_routesum(clno, node->rinfo, 0);
#endif
	}
	return(BVS_CNT_END);
}

int pr_routetime(int clno, R_LIST *rlist, long tsec, long range, int mode)
{
	R_LIST		*cptr;
	ROUTE_INFO	*rinfo;
	struct in_addr	sin;
	char		prline[1024], prline2[1024];
	struct timeval	now;
	long		timediff;
	char		tmstr[10];
	
	/* if neighbor is Zero, then show all routes */

	cptr = rlist;
	while(cptr != NULL) {
		rinfo = cptr->route_data;

		gettimeofday(&now, (struct timezone *)0);
		timediff = now.tv_sec - rinfo->lastupdate.tv_sec;

		switch(mode) {
		case 1:	/* EQ */
			if ((timediff >= tsec) && 
			    (timediff < (tsec+range))) break;
			cptr = cptr->next;
			continue;
		case 2:	/* LE */
			if (timediff < (tsec+range)) break;
			cptr = cptr->next;
			continue;
		case 3: /* GE */
			if (timediff >= tsec) break;
			cptr = cptr->next;
			continue;
		defalult:
			return;
		}

		sin.s_addr = rinfo->prefix;
		sprintf(prline2, "%s/%d ", inet_ntoa(sin), rinfo->length);
		sprintf(prline, "  %-17s", prline2);

		if (ATRFL_ISSET(rinfo->attribute_flag, SET_NEXTHOP)) {
			sin.s_addr = rinfo->next_hop;
			sprintf(prline2, "%-17s", inet_ntoa(sin));
		} else {
			strcpy(prline2, "                 ");
		}
		strcat(prline, prline2);
		if (ATRFL_ISSET(rinfo->attribute_flag, SET_METRIC)) {
			sprintf(prline2, "%7d", rinfo->metric);
		} else {
			strcpy(prline2, "       ");
		}
		strcat(prline, prline2);
		if (ATRFL_ISSET(rinfo->attribute_flag, SET_LOCALPREF)) {
			sprintf(prline2, "%7d ", rinfo->local_perf);
		} else {
			strcpy(prline2, "        ");
		}
		strcat(prline, prline2);
		sprintf(prline2, "%-8s ", sectoptm(timediff, tmstr));
		strcat(prline, prline2);
		/* strcat(prline, rinfo->path_set); */
		if (rinfo->pathset_ptr != NULL) {
			strcat(prline, rinfo->pathset_ptr);
		}
		if (ATRFL_ISSET(rinfo->attribute_flag, SET_ORIGIN)) {
			switch(rinfo->origin_type) {
			case ORIGIN_IGP:
				strcat(prline, "i\r\n");
				break;
			case ORIGIN_EGP:
				strcat(prline, "e\r\n");
				break;
			case ORIGIN_INCOMPLETE:
				strcat(prline, "?\r\n");
				break;
			}
		}
		bvs_write(clno, prline);

		cptr = cptr->next;
	}

}

int bvc_showipbgproutetime_eq(BVS_CMD_PARAM *prm)
{
	int		clno;
	char		tmstr[128];
	long		tvsec;
	long		range;
	char		prline[1024];
	static RADIX_T	**cnode;
	static int	*upcode;
	RADIX_T		*node;

	/**** Function Initialize ***/
	if (prm == NULL) {
		cnode = (RADIX_T **)malloc(sizeof(RADIX_T *) * bvs_maxcon);
		bzero(cnode, sizeof(RADIX_T *) * bvs_maxcon);
		upcode = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(upcode, sizeof(int) * bvs_maxcon);
		return(BVS_CNT_END);
	}

	/****** Parameter Get ******/
	clno = (prm->one_str64).clno;
	strcpy(tmstr, (prm->one_str64).str);

	/***** Prameter Check *****/
#ifdef DEBUGL2
	sprintf(prline, "INPUT Param : Time:%s Mode: EQ\r\n", tmstr);
	bvs_write(clno, prline);
#endif
	ptmtosec(tmstr, &tvsec, &range);
#ifdef DEBUGL2
	sprintf(prline, "OUTPUT Param: Time:%ld sec / Range:%ld \r\n", 
					tvsec, range);
	bvs_write(clno, prline);
#endif
	if (range == 0) {
		bvs_write(clno, "Time String Error\r\n");
		return(BVS_CNT_END);
	}


	/***** Print Route *****/
	if (*(cnode+clno) == NULL) {
		bvs_write(clno, "\r\n ! : Inactive Route\r\n\r\n");
		bvs_write(clno, 
"   Prefix           Next Hop         Metric Loc_Pef Time    AS_Path\r\n");
	}

	node = show_route_all(*(cnode+clno), (upcode+clno));
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			bvs_write(clno, "%% No Route\r\n");
		}
	} else {
		pr_routetime(clno, node->rinfo, tvsec, range, 1);
	}
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			*(cnode+clno) = node;
			return(BVS_CNT_END);
		} else {
			*(cnode+clno) = node;
			return(BVS_CNT_FINISH);
		}
	} else {
		*(cnode+clno) = node;
		return(BVS_CNT_CONTINUE);
	}
}

int bvc_showipbgproutetime_le(BVS_CMD_PARAM *prm)
{
	int		clno;
	char		tmstr[128];
	long		tvsec;
	long		range;
	char		prline[1024];
	static RADIX_T	**cnode;
	static int	*upcode;
	RADIX_T		*node;

	/**** Function Initialize ***/
	if (prm == NULL) {
		cnode = (RADIX_T **)malloc(sizeof(RADIX_T *) * bvs_maxcon);
		bzero(cnode, sizeof(RADIX_T *) * bvs_maxcon);
		upcode = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(upcode, sizeof(int) * bvs_maxcon);
		return(BVS_CNT_END);
	}

	/****** Parameter Get ******/
	clno = (prm->one_str64).clno;
	strcpy(tmstr, (prm->one_str64).str);

	/***** Prameter Check *****/
#ifdef DEBUGL2
	sprintf(prline, "INPUT Param : Time:%s Mode: LE\r\n", tmstr);
	bvs_write(clno, prline);
#endif
	ptmtosec(tmstr, &tvsec, &range);
#ifdef DEBUGL2
	sprintf(prline, "OUTPUT Param: Time:%ld sec / Range:%ld \r\n", 
					tvsec, range);
	bvs_write(clno, prline);
#endif
	if (range == 0) {
		bvs_write(clno, "Time String Error\r\n");
		return(BVS_CNT_END);
	}


	/***** Print Route *****/
	if (*(cnode+clno) == NULL) {
		bvs_write(clno, "\r\n ! : Inactive Route\r\n\r\n");
		bvs_write(clno, 
"   Prefix           Next Hop         Metric Loc_Pef Time    AS_Path\r\n");
	}

	node = show_route_all(*(cnode+clno), (upcode+clno));
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			bvs_write(clno, "%% No Route\r\n");
		}
	} else {
		pr_routetime(clno, node->rinfo, tvsec, range, 2);
	}
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			*(cnode+clno) = node;
			return(BVS_CNT_END);
		} else {
			*(cnode+clno) = node;
			return(BVS_CNT_FINISH);
		}
	} else {
		*(cnode+clno) = node;
		return(BVS_CNT_CONTINUE);
	}
}

int bvc_showipbgproutetime_ge(BVS_CMD_PARAM *prm)
{
	int		clno;
	char		tmstr[128];
	long		tvsec;
	long		range;
	char		prline[1024];
	static RADIX_T	**cnode;
	static int	*upcode;
	RADIX_T		*node;

	/**** Function Initialize ***/
	if (prm == NULL) {
		cnode = (RADIX_T **)malloc(sizeof(RADIX_T *) * bvs_maxcon);
		bzero(cnode, sizeof(RADIX_T *) * bvs_maxcon);
		upcode = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(upcode, sizeof(int) * bvs_maxcon);
		return(BVS_CNT_END);
	}

	/****** Parameter Get ******/
	clno = (prm->one_str64).clno;
	strcpy(tmstr, (prm->one_str64).str);

	/***** Prameter Check *****/
#ifdef DEBUGL2
	sprintf(prline, "INPUT Param : Time:%s Mode: GE\r\n", tmstr);
	bvs_write(clno, prline);
#endif
	ptmtosec(tmstr, &tvsec, &range);
#ifdef DEBUGL2
	sprintf(prline, "OUTPUT Param: Time:%ld sec / Range:%ld \r\n", 
					tvsec, range);
	bvs_write(clno, prline);
#endif
	if (range == 0) {
		bvs_write(clno, "Time String Error\r\n");
		return(BVS_CNT_END);
	}


	/***** Print Route *****/
	if (*(cnode+clno) == NULL) {
		bvs_write(clno, "\r\n ! : Inactive Route\r\n\r\n");
		bvs_write(clno, 
"   Prefix           Next Hop         Metric Loc_Pef Time    AS_Path\r\n");
	}

	node = show_route_all(*(cnode+clno), (upcode+clno));
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			bvs_write(clno, "%% No Route\r\n");
		}
	} else {
		pr_routetime(clno, node->rinfo, tvsec, range, 3);
	}
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			*(cnode+clno) = node;
			return(BVS_CNT_END);
		} else {
			*(cnode+clno) = node;
			return(BVS_CNT_FINISH);
		}
	} else {
		*(cnode+clno) = node;
		return(BVS_CNT_CONTINUE);
	}
}

int rtcount(R_LIST *rlist, u_long *aslen, u_long *ascnt,
            u_long *msklen, net_ulong neighbor)
{
	R_LIST		*cptr;
	ROUTE_INFO	*rinfo;
	char		aspathes[256];
	char		*pathp;
	
	/* if neighbor is Zero, then show all routes */

	cptr = rlist;
	while(cptr != NULL) {
		rinfo = cptr->route_data;

		if ((neighbor != 0) && (rinfo->source != neighbor)) {
			cptr = cptr->next;
			continue;
		}

		msklen[rinfo->length-1]++;
		(*ascnt)++;
		/* strcpy(aspathes, rinfo->path_set); */
		if (rinfo->pathset_ptr != NULL) {
			strcpy(aspathes, rinfo->pathset_ptr);
		} else {
			strcpy(aspathes, "");
		}
		pathp = strtok(aspathes, " ");
		while(pathp != NULL) {
#ifdef DEBUG
			printf("DEBUG: BVC: rtcount: C_ASPATH = %s\n",pathp);
#endif
			if ((pathp[0] != '(') && (pathp[0] != ')')) {
				(*aslen)++;
			}
			pathp = strtok(NULL, " ");
		}
		cptr = cptr->next;
	}

}

int bvc_show_bgproute_sum(BVS_CMD_PARAM *prm)
{
	int		clno;
	static RADIX_T	**cnode;
	static int	*upcode;
	RADIX_T		*node;
	static u_long	*mleng;
	static u_long	*asplen, *aspcnt;
	static int	*bcnt;
	static char	bc[4] = "-\\|/";
	char		prline[128];
	int		cntr;

	if (prm == NULL) {
		cnode = (RADIX_T **)malloc(sizeof(RADIX_T *) * bvs_maxcon);
		bzero(cnode, sizeof(RADIX_T *) * bvs_maxcon);
		upcode = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(upcode, sizeof(int) * bvs_maxcon);
		mleng = (u_long *)malloc(sizeof(u_long)* 32 * bvs_maxcon);
		bzero(mleng, sizeof(u_long)* 32 * bvs_maxcon);
		asplen = (u_long *)malloc(sizeof(u_long) * bvs_maxcon);
		bzero(asplen, sizeof(u_long) * bvs_maxcon);
		aspcnt = (u_long *)malloc(sizeof(u_long) * bvs_maxcon);
		bzero(aspcnt, sizeof(u_long) * bvs_maxcon);
		bcnt = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(bcnt, sizeof(int) * bvs_maxcon);
		return(0);
	}

	clno = (prm->noparam).clno;

	if (*(cnode+clno) == NULL) {
		bvs_write(clno, "BGP Route Counting ... -");
		bzero((mleng + (clno*32)), sizeof(u_long)*32);
		*(bcnt+clno) = 0;
		*(asplen+clno) = 0;
		*(aspcnt+clno) = 0;
	}

#ifdef DEBUG
	printf("DEBUG: RADIX: R_SUMS: as pathes = %ld / AS Count = %ld\n",
			*(asplen+clno), *(aspcnt+clno));
#endif
	node = show_route_all(*(cnode+clno), (upcode+clno));
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			bvs_write(clno, "\r\n%% No Route\r\n");
		}
		else {
			bvs_write(clno, "\r\nRoute Summary ------\r\n");
			sprintf(prline, "  Received Prefixes : %ld\r\n",
								radix_prefixes());
			bvs_write(clno, prline);
			sprintf(prline, "  Received Routes   : %ld\r\n",
								radix_routes());
			bvs_write(clno, prline);
			sprintf(prline, "  Received Nodes    : %ld\r\n", 
								radix_nodes());
			bvs_write(clno, prline);
			bvs_write(clno, "  Prefix Length");
			for(cntr=0; cntr<32; cntr++) {
				if ((cntr%4) == 0) bvs_write(clno, "\r\n    ");
				sprintf(prline, "/%-2d:%-5ld  ",
							cntr+1, 
							*(mleng+(clno*32)+cntr));
				bvs_write(clno, prline);
			}
			if (*(aspcnt+clno) == 0) {
				bvs_write(clno, "\r\n  No AS Path\r\n");
			} else {
				sprintf(prline, 
					"\r\n  AS Path length average : %5.1lf\r\n",
				    ((float)*(asplen+clno)/(float)*(aspcnt+clno)));
				bvs_write(clno, prline);
			}
		}
	} else {
		sprintf(prline, "\b%c", bc[*(bcnt+clno)]);
		bvs_write(clno, prline);
		(*(bcnt+clno))++;
		if (*(bcnt+clno) >= 4) *(bcnt+clno) = 0;
		rtcount(node->rinfo, (asplen+clno), (aspcnt+clno),
			 				(mleng+(clno*32)), 0);
	}
	if (node == NULL) {
		*(cnode+clno) = NULL;
		if (*(cnode+clno) == NULL) {
			return(BVS_CNT_END);
		} else {
			return(BVS_CNT_FINISH);
		}
	} else {
		*(cnode+clno) = node;
		return(BVS_CNT_CONTINUE);
	}
}

#ifdef RTHISTORY
int bvc_clear_route_history(BVS_CMD_PARAM *prm)
{
	int		clno;
	static RADIX_T	*cnode;
	static int	upcode;
	static int	curclno;
	RADIX_T		*node;
	static int	status;

	if (prm == NULL) {
		cnode = NULL;
		curclno = -1;
		return(0);
	}

	clno = (prm->noparam).clno;

	if (rthist_clean == RTHIST_CLEANING) {
		if (curclno != clno) {
			bvs_write(clno,
			    "Anothoer Process is cleaning route history.\r\n");
			return(BVS_CNT_END);
		}
	} else {
		cnode = NULL;
	}

	if (cnode == NULL) {
		bvs_write(clno, " Cleaning Route Histories..\r\n");
	}

	node = clear_rt_history(cnode, &upcode);
	if (node == NULL) {
		if (cnode == NULL) {
			bvs_write(clno, "%% No Route\r\n");
		}
	}

	if (node == NULL) {
		if (cnode == NULL) {
			cnode = NULL;
			rthist_clean = RTHIST_NOTCLEANING;
			curclno = -1;
			return(BVS_CNT_END);
		} else {
			time(&cleaned_time);
			cnode = NULL;
			rthist_clean = RTHIST_NOTCLEANING;
			curclno = -1;
			bvs_write(clno, " --- Cleaning Done\r\n");

			/*** Reset Cleaning Timer ***/
			if (rthistcleartime.tv_sec == 0) {
				return(BVS_CNT_FINISH);
			}
			common_off_timer(rtclear_id);
			rtclear_id = common_entry_timer(&rthistcleartime, 
							0, rthisttimerclear);
			if (rtclear_id == -1) {
				pr_log(VLOG_GLOB, LOG_ALL,
				 "Route History Clearing Timer Set Fail\n", ON);
				notify_systemnotify("STOP",
				       "Route History Clearing Timer Set Fail");
				finish(PROC_SHUTDOWN);
				return(1);
			}

			return(BVS_CNT_FINISH);
		}
	} else {
		cnode = node;
		curclno = clno;
		rthist_clean = RTHIST_CLEANING;
		return(BVS_CNT_CONTINUE);
	}
}

int bvc_next_rthistory_clear_set(BVS_CMD_PARAM *prm)
{
	int		clno;
	struct timeval	tval;
	char		prline[1024];

	clno = (prm->one_num).clno;
	tval.tv_sec = (prm->one_num).num;
	tval.tv_usec = 0;

	common_off_timer(rtclear_id);
	rtclear_id = common_entry_timer(&tval, 0, rthisttimerclear);

	if (rtclear_id == -1) {
		sprintf(prline,
		 "\r\nNext route change history clear could not set.\r\n\r\n");
	} else {
		sprintf(prline,
		 "\r\nNext route change history clear set %d second later.\r\n\r\n",
		  tval);
	}
	bvs_write(clno, prline);

	return(BVS_CNT_END);
}

#endif /* RTHISTORY */

#endif /* RADIX */

int bvc_readcronfile(BVS_CMD_PARAM *prm)
{
	int	clno;
	int	ret;

	clno = (prm->noparam).clno;

	ret = read_cron_file();
	if (ret == -1) {
		bvs_write(clno, "Error: Check Your Cron File\r\n");
	} else {
		bvs_write(clno, "Cron Configuration File Read\r\n");
	}

	return(BVS_CNT_END);
}

int bvc_showcronlist(BVS_CMD_PARAM *prm)
{
	int	clno;
	int	cnt;
	char	prline[1024];

	clno = (prm->noparam).clno;

	bvs_write(clno, "\r\n=== Currenet Cron Status ===\r\n");
	cnt = 0;
	while(cron_list[cnt] != NULL) {
		sprintf(prline, "%02d: %s %s %s %s %s %s\r\n",
			cnt,
			cron_list[cnt]->min,
			cron_list[cnt]->hour,
			cron_list[cnt]->day,
			cron_list[cnt]->month,
			cron_list[cnt]->year,
			cron_list[cnt]->command );
		bvs_write(clno, prline);
		cnt++;
	}
	bvs_write(clno, "\r\n");

	return(BVS_CNT_END);
}

int nexthop_count(R_LIST *rlist, NEXTHOP_LIST **hlist, net_ulong nei)
{
	R_LIST		*routelist;
	ROUTE_INFO	*routeinfo;
	NEXTHOP_LIST	*listtmp;
	int		actcnt;

	actcnt = 0;
	routelist = rlist;
	while(routelist != NULL) {
		routeinfo = routelist->route_data;

		if ((nei != 0) && (routeinfo->source != nei)) {
			routelist = routelist->next;
			continue;
		}
#ifdef RTHISTORY
		if (routeinfo->active == false) {
			routelist = routelist->next;
			continue;
		}
#endif
		actcnt++;

		if (*hlist == NULL) {
			*hlist = (NEXTHOP_LIST *)malloc(sizeof(NEXTHOP_LIST));
			bzero((*hlist), sizeof(NEXTHOP_LIST));
			(*hlist)->next_hop = routeinfo->next_hop;
			(*hlist)->count = 1;
		} else {
			listtmp = *hlist;
			while(1) {
				if (listtmp->next_hop == routeinfo->next_hop) {
					listtmp->count++;
					break;
				}
				if (listtmp->next == NULL) {
					listtmp->next = (NEXTHOP_LIST *)malloc(
	                                                  sizeof(NEXTHOP_LIST));
					bzero(listtmp->next, sizeof(NEXTHOP_LIST));
					(listtmp->next)->next_hop = 
								routeinfo->next_hop;
					(listtmp->next)->count = 1;
					break;
				}
				listtmp = listtmp->next;
			}
		}
		routelist = routelist->next;
	}
	return(actcnt);
}

int bvc_show_bgpnexthop_sum(BVS_CMD_PARAM *prm)
{
	int		clno;
	static RADIX_T	**cnode;
	static int	*upcode;
	static NEXTHOP_LIST	**hoplist;
	static int	*actives;
	RADIX_T		*node;
	char		prline[128];
	int		cntr;
	NEXTHOP_LIST	*listtmp, *ftmp;
	R_LIST		*cptr;
	ROUTE_INFO	*routeinfo;
	int		allroutes;
	struct in_addr	inaddr;

	if (prm == NULL) {
		cnode = (RADIX_T **)malloc(sizeof(RADIX_T *) * bvs_maxcon);
		bzero(cnode, sizeof(RADIX_T *) * bvs_maxcon);
		upcode = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(upcode, sizeof(int) * bvs_maxcon);
		hoplist = (NEXTHOP_LIST **)malloc(sizeof(NEXTHOP_LIST *) * 
								bvs_maxcon);
		bzero(hoplist, sizeof(NEXTHOP_LIST *) * bvs_maxcon);
		actives = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(actives, sizeof(int) * bvs_maxcon);
		return(0);
	}

	clno = (prm->noparam).clno;

	if (*(cnode+clno) == NULL) {
		bvs_write(clno, "BGP Next Hop Summarizing ...");
		*(hoplist + clno) = 0;
		*(actives + clno) = 0;
	}

	node = show_route_all(*(cnode+clno), (upcode+clno));
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			bvs_write(clno, "\r\n%% No Route\r\n");
		}
		else {
			bvs_write(clno, "\r\nNext Hop  Summary ------\r\n");
			sprintf(prline, "  Received Prefixes : %ld\r\n",
								radix_prefixes());
			bvs_write(clno, prline);
			allroutes = radix_routes();
			sprintf(prline, "  Received Routes   : %ld\r\n",
								 allroutes);
			bvs_write(clno, prline);
			sprintf(prline, "  Received Nodes    : %ld\r\n", 
								radix_nodes());
			bvs_write(clno, prline);
			sprintf(prline, "  Active Routes     : %ld\r\n",
								*(actives + clno));
			bvs_write(clno, prline);

			bvs_write(clno, "\r\nNext Hop        Routes Rate\r\n");
			listtmp = *(hoplist+clno);
			while(listtmp != NULL) {
				inaddr.s_addr = listtmp->next_hop;
				sprintf(prline, "%15s %6d %5.1f%%\r\n",
					inet_ntoa(inaddr), listtmp->count,
					((float)(listtmp->count)/(float)allroutes)*100);
				bvs_write(clno, prline);
				listtmp = listtmp->next;
			}
			bvs_write(clno, "\r\n");
			listtmp = *(hoplist+clno);
			while(listtmp != NULL) {
				ftmp = listtmp->next;
				free(listtmp);
				listtmp = ftmp;
			}

		}
	} else {
		*(actives + clno) += nexthop_count(node->rinfo, (hoplist+clno), 0);
	}
	if (node == NULL) {
		*(cnode+clno) = NULL;
		if (*(cnode+clno) == NULL) {
			return(BVS_CNT_END);
		} else {
			return(BVS_CNT_FINISH);
		}
	} else {
		*(cnode+clno) = node;
		return(BVS_CNT_CONTINUE);
	}
}

int bvc_show_bgpnexthop_sum_neighbor(BVS_CMD_PARAM *prm)
{
	int		clno;
	static RADIX_T	**cnode;
	static int	*upcode;
	static NEXTHOP_LIST	**hoplist;
	static int	*actives;
	RADIX_T		*node;
	char		prline[128];
	int		cntr;
	NEXTHOP_LIST	*listtmp, *ftmp;
	R_LIST		*cptr;
	ROUTE_INFO	*routeinfo;
	int		allroutes;
	struct in_addr	inaddr;
	net_ulong	neigh;

	if (prm == NULL) {
		cnode = (RADIX_T **)malloc(sizeof(RADIX_T *) * bvs_maxcon);
		bzero(cnode, sizeof(RADIX_T *) * bvs_maxcon);
		upcode = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(upcode, sizeof(int) * bvs_maxcon);
		hoplist = (NEXTHOP_LIST **)malloc(sizeof(NEXTHOP_LIST *) * 
								bvs_maxcon);
		bzero(hoplist, sizeof(NEXTHOP_LIST *) * bvs_maxcon);
		actives = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(actives, sizeof(int) * bvs_maxcon);
		return(0);
	}

	/****
	clno = (prm->adrs).clno;
	neigh = (prm->adrs).addr;
	****/

	clno = *(int *)prm;
	neigh = *(net_ulong *)((char *)prm + sizeof(int));

	if (*(cnode+clno) == NULL) {
		bvs_write(clno, "BGP Next Hop Summarizing ...");
		*(hoplist + clno) = 0;
		*(actives + clno) = 0;
	}

	node = show_route_all(*(cnode+clno), (upcode+clno));
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			bvs_write(clno, "\r\n%% No Route\r\n");
		}
		else {
			bvs_write(clno, "\r\nNext Hop  Summary ------\r\n");
			sprintf(prline, "  Received Prefixes : %ld\r\n",
								radix_prefixes());
			bvs_write(clno, prline);
			allroutes = radix_routes();
			sprintf(prline, "  Received Routes   : %ld\r\n",
								 allroutes);
			bvs_write(clno, prline);
			sprintf(prline, "  Received Nodes    : %ld\r\n", 
								radix_nodes());
			bvs_write(clno, prline);
			sprintf(prline, "  Active Routes     : %ld\r\n",
								*(actives + clno));
			bvs_write(clno, prline);

			bvs_write(clno, "\r\nNext Hop        Routes Rate\r\n");
			listtmp = *(hoplist+clno);
			while(listtmp != NULL) {
				inaddr.s_addr = listtmp->next_hop;
				sprintf(prline, "%15s %6d %5.1f%%\r\n",
					inet_ntoa(inaddr), listtmp->count,
					((float)(listtmp->count)/(float)allroutes)*100);
				bvs_write(clno, prline);
				listtmp = listtmp->next;
			}
			bvs_write(clno, "\r\n");
			listtmp = *(hoplist+clno);
			while(listtmp != NULL) {
				ftmp = listtmp->next;
				free(listtmp);
				listtmp = ftmp;
			}

		}
	} else {
		*(actives+clno) += nexthop_count(node->rinfo,(hoplist+clno),neigh);
	}
	if (node == NULL) {
		*(cnode+clno) = NULL;
		if (*(cnode+clno) == NULL) {
			return(BVS_CNT_END);
		} else {
			return(BVS_CNT_FINISH);
		}
	} else {
		*(cnode+clno) = node;
		return(BVS_CNT_CONTINUE);
	}
}


#ifdef IRRCHK

void print_file_irrchk(FILE *fp, ROUTE_INFO *rinf, int proc_type)
{
	char		prline[1024], prbuf[1024], buf2[30];
	struct in_addr	in;
	IRR_ENTRY	*irrent;
	u_long		oasn;
        char            *bufptr, *wptr, *wptr2;
	int		irrret;

	bzero(prline, 1024);

	/* Print Route Information */
#ifdef RTHISTORY
	if (rinf->active) {
		strcpy(prline, "  ");
	} else {
		if (proc_type == 1) return;
		strcpy(prline, "! ");
	}
#endif
#ifndef RTHISTORY
	strcpy(prline, "  ");
#endif

	if (rinf->pathset_ptr == NULL) {
		oasn = 0;
	} else {
		bufptr = strdup(rinf->pathset_ptr);
		wptr2 = NULL;
		wptr = strtok(bufptr, " \t\r");
		while(wptr!=NULL) {
			wptr2 = wptr;
			wptr = strtok(NULL, " \t\r");
		}
		if (wptr2 == NULL) {
			oasn = 0;
		} else {
			oasn = atoi(wptr2);
		}
		free(bufptr);
	}
		
	in.s_addr = rinf->prefix;

	sprintf(buf2, "%s/%d", inet_ntoa(in), rinf->length);
	sprintf(prbuf, "%-18s %-5lu", buf2, oasn);
	strcat(prline, prbuf);

	/* Print IRR Information */
	irrent = rinf->irr;
	if (irrent == NULL) {
		/* IRR Information Not Search Yet */
		strcat(prline, " : ----------");
	} else {
		irrret = irr_routecmp(&in, rinf->length, oasn, irrent);

		switch(irrret) {
		case IRR_SEARCH_FAIL:
		case IRR_NOTMATCH:
			strcat(prline, " :                          : ");
			break;
		default:
			sprintf(buf2, "%s/%d",
				     inet_ntoa(irrent->prefix), irrent->p_len);
			sprintf(prbuf, " : %-18s %-5lu : ", buf2, 
							irrent->originas);
			strcat(prline, prbuf);
		}


		switch(irrret) {
		case IRR_SEARCH_FAIL:
			strcat(prline, "**Hmm.. IRR Search Fail?");
			break;
		case IRR_EXACT_AS:
			strcat(prline, "1 : Route Exact Match");
			break;
		case IRR_EXACT_NOTAS:
			strcat(prline, "3 : Route OK but, AS is not match");
			break;
		case IRR_MORES_AS:
			strcat(prline, "2 : MorePrefix Mach");
			break;
		case IRR_MORES_NOTAS:
			strcat(prline, "4 : MorePrefix, but AS is not match");
			break;
		case IRR_NOTMATCH:
			strcat(prline, "5 : IRR not Match");
			break;
		}
	}
	fprintf(fp, "%s\n", prline);

}

void print_vty_irrchk(int clno, ROUTE_INFO *rinf, int proc_type)
{
	char		prline[1024], prbuf[1024], buf2[30];
	struct in_addr	in;
	IRR_ENTRY	*irrent;
	u_long		oasn;
        char            *bufptr, *wptr, *wptr2;
	int		irrret;

	bzero(prline, 1024);

	/* Print Route Information */
#ifdef RTHISTORY
	if (rinf->active) {
		strcpy(prline, "  ");
	} else {
		if (proc_type == 1) return;
		strcpy(prline, "! ");
	}
#endif
#ifndef RTHISTORY
	strcpy(prline, "  ");
#endif

	if (rinf->pathset_ptr == NULL) {
		oasn = 0;
	} else {
		bufptr = strdup(rinf->pathset_ptr);
		wptr2 = NULL;
		wptr = strtok(bufptr, " \t\r");
		while(wptr!=NULL) {
			wptr2 = wptr;
			wptr = strtok(NULL, " \t\r");
		}
		if (wptr2 == NULL) {
			oasn = 0;
		} else {
			oasn = atoi(wptr2);
		}
		free(bufptr);
	}
		
	in.s_addr = rinf->prefix;

	sprintf(buf2, "%s/%d", inet_ntoa(in), rinf->length);
	sprintf(prbuf, "%-18s %-5lu", buf2, oasn);
	strcat(prline, prbuf);

	/* Print IRR Information */
	irrent = rinf->irr;
	if (irrent == NULL) {
		/* IRR Information Not Search Yet */
		strcat(prline, " : ----------\r\n");
	} else {
		irrret = irr_routecmp(&in, rinf->length, oasn, irrent);

		switch(irrret) {
		case IRR_SEARCH_FAIL:
		case IRR_NOTMATCH:
			strcat(prline, " :                          : ");
			break;
		default:
			sprintf(buf2, "%s/%d",
				     inet_ntoa(irrent->prefix), irrent->p_len);
			sprintf(prbuf, " : %-18s %-5lu : ", buf2, 
							irrent->originas);
			strcat(prline, prbuf);
		}


		switch(irrret) {
		case IRR_SEARCH_FAIL:
			strcat(prline, "**Hmm.. IRR Search Fail?\r\n");
			break;
		case IRR_EXACT_AS:
			strcat(prline, "1 : Route Exact Match\r\n");
			break;
		case IRR_EXACT_NOTAS:
			strcat(prline, "3 : Route OK but, AS is not match\r\n");
			break;
		case IRR_MORES_AS:
			strcat(prline, "2 : MorePrefix Mach\r\n");
			break;
		case IRR_MORES_NOTAS:
			strcat(prline, "4 : MorePrefix, but AS is not match\r\n");
			break;
		case IRR_NOTMATCH:
			strcat(prline, "5 : IRR not Match\r\n");
			break;
		}
	}
	bvs_write(clno, prline);

}

void print_file_irr_status(FILE *fp, R_LIST *ri, int proc_type)
{
	R_LIST		*rlptr;
	ROUTE_INFO	*rinf;

	rlptr = ri;
	while(rlptr != NULL) {
		rinf = rlptr->route_data;

		print_file_irrchk(fp, rinf, proc_type);

		rlptr = rlptr->next;
	}

	fclose(fp);

}

void print_irr_status(int clno, R_LIST *ri, int proc_type)
{
	R_LIST		*rlptr;
	ROUTE_INFO	*rinf;

	rlptr = ri;
	while(rlptr != NULL) {
		rinf = rlptr->route_data;

		print_vty_irrchk(clno, rinf, proc_type);

		rlptr = rlptr->next;
	}
}

int proc_do_output_irr_status(BVS_CMD_PARAM *prm, int proc_type)
{
	int		clno;
	static RADIX_T	**cnode;
	static int	*upcode;
	RADIX_T		*node;
	char		fname[FNAMESIZE];
	char		fn[FNAMESIZE];
	static FILE	*fp;

	if (prm == NULL) {
		cnode = (RADIX_T **)malloc(sizeof(RADIX_T *) * bvs_maxcon);
		bzero(cnode, sizeof(RADIX_T *) * bvs_maxcon);
		upcode = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(upcode, sizeof(int) * bvs_maxcon);
		return(0);
	}

	if (irr_check_enable == 0) {
		bvs_write(clno, "IRR Check Disabled\r\n");
		return(BVS_CNT_END);
	}

	clno = (prm->noparam).clno;
	strcpy(fn, (prm->one_str64).str);
	if (fn[0] == '/') {
		strcpy(fname, fn);
	} else {
		strcpy(fname, spooldir);
		strcat(fname, "/");
		strcat(fname, fn);
	}

	if (*(cnode+clno) == NULL) {
		if ((fp = fopen(fname, "w")) == NULL) {
			bvs_write(clno, "Could not open target file.\r\n");
			return(BVS_CNT_END);
		}

		bvs_write(clno, "Now Outputting Routes to '");
		bvs_write(clno, fname);
		bvs_write(clno, "'\r\n");

		fprintf(fp, "\r\n ! : Inactive Route\r\n\r\n");
		fprintf(fp, 
"  Prefix             ASN   : IRR Prefix         ASN   : Compare Result\r\n");
	}

	node = show_route_all(*(cnode+clno), (upcode+clno));
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			fprintf(fp, "%% No Route\r\n");
		}
	} else {
		print_file_irr_status(fp, node->rinfo, proc_type);
	}
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			*(cnode+clno) = node;
			fclose(fp);
			bvs_write(clno, "Done(No Route)..\r\n");
			return(BVS_CNT_END);
		} else {
			*(cnode+clno) = node;
			fclose(fp);
			bvs_write(clno, "Done..\r\n");
			return(BVS_CNT_FINISH);
		}
	} else {
		*(cnode+clno) = node;
		return(BVS_CNT_CONTINUE);
	}
}

int bvc_do_output_irr_status(BVS_CMD_PARAM *prm)
{
	proc_do_output_irr_status(prm, 0);
}

int bvc_do_output_irr_status_active(BVS_CMD_PARAM *prm)
{
	proc_do_output_irr_status(prm, 1);
}

/******* Added at 2006/09/07 *******/
int proc_show_irr_status(BVS_CMD_PARAM *prm, int proc_type)
{
	int		clno;
	static RADIX_T	**cnode;
	static int	*upcode;
	RADIX_T		*node;

	if (prm == NULL) {
		cnode = (RADIX_T **)malloc(sizeof(RADIX_T *) * bvs_maxcon);
		bzero(cnode, sizeof(RADIX_T *) * bvs_maxcon);
		upcode = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(upcode, sizeof(int) * bvs_maxcon);
		return(0);
	}

	if (irr_check_enable == 0) {
		bvs_write(clno, "IRR Check Disabled\r\n");
		return(BVS_CNT_END);
	}

	clno = (prm->noparam).clno;

	if (*(cnode+clno) == NULL) {
		bvs_write(clno, "\r\n ! : Inactive Route\r\n\r\n");
		bvs_write(clno, 
"  Prefix             ASN   : IRR Prefix         ASN   : Compare Result\r\n");
	}

	node = show_route_all(*(cnode+clno), (upcode+clno));
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			bvs_write(clno, "%% No Route\r\n");
		}
	} else {
		print_irr_status(clno, node->rinfo, proc_type);
	}
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			*(cnode+clno) = node;
			return(BVS_CNT_END);
		} else {
			*(cnode+clno) = node;
			return(BVS_CNT_FINISH);
		}
	} else {
		*(cnode+clno) = node;
		return(BVS_CNT_CONTINUE);
	}
}

int bvc_show_irr_status(BVS_CMD_PARAM *prm)
{
	proc_show_irr_status(prm, 0);
}

int bvc_show_irr_status_active(BVS_CMD_PARAM *prm)
{
	proc_show_irr_status(prm, 1);
}

int check_irr_stats(ROUTE_INFO *rinf)
{
	int		stret;
        struct in_addr  in;
        IRR_ENTRY       *irrent;
        u_long          oasn;
        char            *bufptr, *wptr, *wptr2;
        int             irrret;

	stret = 0;

#ifdef RTHISTORY 
	/* Only Count Active route */
	if (!(rinf->active)) return(-1);
#endif

        if (rinf->pathset_ptr == NULL) {
                oasn = 0;
        } else {
                bufptr = strdup(rinf->pathset_ptr);
                wptr2 = NULL;
                wptr = strtok(bufptr, " \t\r");
                while(wptr!=NULL) {
                        wptr2 = wptr;
                        wptr = strtok(NULL, " \t\r");
                }
                if (wptr2 == NULL) {
                        oasn = 0;
                } else {
                        oasn = atoi(wptr2);
                }
		free(bufptr);
        }
                
        in.s_addr = rinf->prefix;

        /* Print IRR Information */
        irrent = rinf->irr;
        if (irrent == NULL) {
                /* IRR Information Not Search Yet */
                stret = 0;
        } else {
                irrret = irr_routecmp(&in, rinf->length, oasn, irrent);

                switch(irrret) {
                case IRR_SEARCH_FAIL:
                        stret = 0;
                        break;
                case IRR_EXACT_AS:
                case IRR_EXACT_NOTAS:
                case IRR_MORES_AS:
                case IRR_MORES_NOTAS:
                case IRR_NOTMATCH:
			stret = irrret;
                        break;
                }
        }
	return(stret);
}

int bvc_show_irr_status_summary(BVS_CMD_PARAM *prm)
{
	/* This Function Counts Only Active Routes */

        int             clno;
        static RADIX_T  **cnode;
        static int      *upcode;
        RADIX_T         *node;
        char            prline[FNAMESIZE];
        static FILE     *fp;
	static u_long	*statscnt; /* max type number */
	int		retstats;
	R_LIST		*rlptr;
	ROUTE_INFO	*rinf;
	static int      *bcnt;
	static char     bc[4] = "-\\|/";

        if (prm == NULL) {
                cnode = (RADIX_T **)malloc(sizeof(RADIX_T *) * bvs_maxcon);
                bzero(cnode, sizeof(RADIX_T *) * bvs_maxcon);
                upcode = (int *)malloc(sizeof(int) * bvs_maxcon);
                bzero(upcode, sizeof(int) * bvs_maxcon);
		statscnt = (u_long *)malloc(sizeof(u_long) * 6 * bvs_maxcon);
		bzero(statscnt, sizeof(statscnt) * bvs_maxcon);
		bcnt = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(bcnt, sizeof(int) * bvs_maxcon);
                return(0);
        }

        if (irr_check_enable == 0) {
                bvs_write(clno, "IRR Check Disabled\r\n");
                return(BVS_CNT_END);
        }

        clno = (prm->noparam).clno;

        if (*(cnode+clno) == NULL) {
                bvs_write(clno, "Now Counting IRR Status Summary ... -");
		*(statscnt+(clno*6)+0) = 0;
		*(statscnt+(clno*6)+1) = 0;
		*(statscnt+(clno*6)+2) = 0;
		*(statscnt+(clno*6)+3) = 0;
		*(statscnt+(clno*6)+4) = 0;
		*(statscnt+(clno*6)+5) = 0;
		*(bcnt+clno) = 0;
        }

        node = show_route_all(*(cnode+clno), (upcode+clno));
        if (node == NULL) {
                if (*(cnode+clno) == NULL) {
                        bvs_write(clno, "%% No Route\r\n");
                }
        } else {
		rlptr = node->rinfo;

		sprintf(prline, "\b%c", bc[*(bcnt+clno)]);
		bvs_write(clno, prline);
		(*(bcnt+clno))++;
		if (*(bcnt+clno) >= 4) *(bcnt+clno) = 0;

		while(rlptr != NULL) {
			rinf = rlptr->route_data;
                	retstats = check_irr_stats(rinf /*, proc_type */);
			if (retstats >= 0) (*(statscnt+(clno*6)+retstats))++;
#ifdef DEBUG
			if (retstats >= 0)
			pr_log2(VLOG_GLOB, LOG_ALL, ON,
				"DEBUG: IRR Stats Sum:%d:retstats=%d/cnt=%ld\n",
				 clno, retstats, *(statscnt+(clno*6)+retstats));
#endif
			rlptr = rlptr->next;
		}
        }
        if (node == NULL) {
                if (*(cnode+clno) == NULL) {
                        *(cnode+clno) = node;
			bvs_write(clno, "\bDone(No Route).\r\n");
                        return(BVS_CNT_END);
                } else {
                        *(cnode+clno) = node;
                        bvs_write(clno, "\bDone.\r\n");
	
                        bvs_write(clno, "Status                             : Routes\r\n");
			sprintf(prline, "-: Non-Count Route                 : %-7d routes\r\n", *(statscnt+(clno*6)+0));
                        bvs_write(clno, prline);
			sprintf(prline, "1: Route Exact Match               : %-7d routes\r\n", *(statscnt+(clno*6)+1));
                        bvs_write(clno, prline);
			sprintf(prline, "2: MorePrefix Match                : %-7d routes\r\n", *(statscnt+(clno*6)+2));
                        bvs_write(clno, prline);
			sprintf(prline, "3: Route OK but, AS is not match   : %-7d routes\r\n", *(statscnt+(clno*6)+3));
                        bvs_write(clno, prline);
			sprintf(prline, "4: MorePrefix, but AS is not match : %-7d routes\r\n", *(statscnt+(clno*6)+4));
                        bvs_write(clno, prline);
			sprintf(prline, "5: IRR not Match                   : %-7d routes\r\n", *(statscnt+(clno*6)+5));
                        bvs_write(clno, prline);
                        bvs_write(clno, "\r\n");

                        return(BVS_CNT_FINISH);
                }
        } else {
                *(cnode+clno) = node;
                return(BVS_CNT_CONTINUE);
        }
}


#endif /* IRRCHK */


/************** Add by K.Kondo 2006/11/09 for Flow Inspection Proj ******/

int bvc_output_aslist_nei(BVS_CMD_PARAM *prm)
/***
#ifdef RTHISTORY
int output_bgproute_nei(BVS_CMD_PARAM *prm, boolean detail, int dtype)
#endif
#ifndef RTHISTORY
int bvc_output_bgproute_all_nei(BVS_CMD_PARAM *prm)
#endif
****/
{
	int		clno;
	static RADIX_T	**cnode;
	static int	*upcode;
	RADIX_T		*node;
	char		fname[FNAMESIZE];
	char		fn[FNAMESIZE];
	static FILE	*fp;
	u_long		neighbor;
	char		*bufptr, *wptr, *wptr2;
	R_LIST		*rl;
	ROUTE_INFO	*ri;
	u_long		oasn;
	struct in_addr	in;

	if (prm == NULL) {
		cnode = (RADIX_T **)malloc(sizeof(RADIX_T *) * bvs_maxcon);
		bzero(cnode, sizeof(RADIX_T *) * bvs_maxcon);
		upcode = (int *)malloc(sizeof(int) * bvs_maxcon);
		bzero(upcode, sizeof(int) * bvs_maxcon);
		return(0);
	}

	clno = (prm->one_str64_adrs).clno;
	strcpy(fn, (prm->one_str64_adrs).str);
	neighbor = (prm->one_str64_adrs).addr;

	if (fn[0] == '/') {
		strcpy(fname, fn);
	} else {
		strcpy(fname, spooldir);
		strcat(fname, "/");
		strcat(fname, fn);
	}

	if (*(cnode+clno) == NULL) {
		if ((fp = fopen(fname, "w")) == NULL) {
			bvs_write(clno, "Could not open target file.\r\n");
			return(BVS_CNT_END);
		}

		bvs_write(clno, "Now Outputting Routes to '");
		bvs_write(clno, fname);
		bvs_write(clno, "'\r\n");
		/******
		fprintf(fp, 
"   Prefix           Next Hop         Metric Loc_Pef Time    AS_Path\n");
		*******/
	}

	node = show_route_all(*(cnode+clno), (upcode+clno));
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			fprintf(fp, "%% No Route\n");
		}
	} else {
/******
#ifdef RTHISTORY
		output_routesum(node->rinfo, neighbor, fp, detail, dtype);
#endif
#ifndef RTHISTORY
		output_routesum(node->rinfo, neighbor, fp);
#endif
*******/
		/***** This Part is printing AS List *****/
		rl = node->rinfo;
		while(rl != NULL) {
			ri = rl->route_data;
			if (ri->source != neighbor) continue;

			bufptr = strdup(ri->pathset_ptr);
			wptr2 = NULL;
			wptr = strtok(bufptr, " \t\r");
			while(wptr != NULL) {
				wptr2 = wptr;
				wptr = strtok(NULL, " \t\r");
			}
			if (wptr2 == NULL) {
				oasn = 0;
			} else {
				oasn = atoi(wptr2);
			}

			in.s_addr = ri->prefix;


			fprintf(fp, "%lu:%s/%d\n", 
				oasn, inet_ntoa(in), ri->length);
				
			free(bufptr);

			rl = rl->next;
		}
	}
	if (node == NULL) {
		if (*(cnode+clno) == NULL) {
			*(cnode+clno) = node;
			fclose(fp);
			bvs_write(clno, "Done(No Route)..\r\n");
			return(BVS_CNT_END);
		} else {
			*(cnode+clno) = node;
			fclose(fp);
			bvs_write(clno, "Done..\r\n");
			return(BVS_CNT_FINISH);
		}
	} else {
		*(cnode+clno) = node;
		return(BVS_CNT_CONTINUE);
	}
}

/************************** Add end 2006/11/09 ************************/

#ifdef DEBUG

#ifdef RADIX
int bvc_dodeleteroute(BVS_CMD_PARAM *prm)
{
	int		clno;
	net_ulong	pref;
	int		mask;
	net_ulong	neigh;
	char		prline[128];
	int		pn;
	int		r;

	clno = (prm->routertt).clno;
	neigh = (prm->routertt).neighbor;
	pref = (prm->routertt).prefix;
	mask = (prm->routertt).prefixlen;
	pn = check_id1topn(neigh);

#ifdef RTHISTORY
	r = withdraw_route(pn, pref, mask, DELETE);
#endif /* RTHISTORY */
#ifndef RTHISTORY
	r = withdraw_route(pn, pref, mask);
#endif /* RTHISTORY */
	if (r == 0) {
		bvs_write(clno, "Route Withdraw Successful\r\n");
	} else {
		bvs_write(clno, "Route Withdraw Fail\r\n");
	}
	return(BVS_CNT_END);
}

int bvc_doclearpeerroute(BVS_CMD_PARAM *prm)
{
	int		clno;
	net_ulong	nei;
	char		prline[128];
	int		pn;

	/****
	clno = (prm->adrs).clno;
	nei = (prm->adrs).addr;
	****/

	clno = *(int *)prm;
	nei = *(net_ulong *)((char *)prm + sizeof(int));

	pn = check_id1topn(nei);

	sprintf(prline, "Clearing Peer Route: Peer No = %d\r\n", pn);
	bvs_write(clno, prline);
	withdraw_peer(pn);
	return(BVS_CNT_END);
}

#endif /* RADIX */


int bvc_dotimertest(BVS_CMD_PARAM *prm)
{
	int		clno;
	struct timeval	tmval;

	clno = (prm->noparam).clno;

	tmval.tv_sec = 1;
	tmval.tv_usec = 0;
	common_entry_timer(&tmval, clno, testfunc);
	return(BVS_CNT_END);
}

void testfunc(int id, int prm)
{
	char	prline[128];

	sprintf(prline, "Common Timer Expierd : ID=%d\r\n", id);
	bvs_write(prm, prline);
	bvs_write(prm, bvs_prompt);
}
#endif
