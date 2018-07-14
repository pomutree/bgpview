/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1997/12                         */
/*                                                              */
/* Main Routine                                                 */
/****************************************************************/
/* bgpview.c,v 1.7 2005/08/18 06:17:32 kuniaki Exp */

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
 * Copyright (c) 2003 Intec NetCore, Inc.
 * All rights reserved.
 */

#include "bgp.h"

#ifdef sun
#define PID	"/var/adm/bgpview.pid"
#endif
#ifndef sun
#define PID	"/var/run/bgpview.pid"
#endif

extern int read_conf(/*fname*/);	/* in config.c */
extern int check_fsm(/*peer_num*/);	/* in fsm.c */
extern void clear_peer(/*pn*/);		/* in fsm.c */

/* Externed Global Valiables */
char *bgptypestring[BGPTYPES] = {
        "Undefine type",
        "Open",
        "Update",
        "Notification",
        "Keepalive"
};

PEERS		peer[MAXPEERS];
fd_set		readfds;
int		sv_sock;
net_ulong	IDENT;
/* u_short		LOCAL_AS; */
u_long		LOCAL_AS;
char		syslogname[FNAMESIZE];
int		facility;
char		configfile[FNAMESIZE];
struct timeval	uptime;
char		spooldir[FNAMESIZE];
char		sysname[16];

#ifdef RTHISTORY
struct timeval	rthistcleartime;
struct timeval	rthistcleartime_v6;
#endif

int		exec_argc;
char		**exec_argv;

struct eventview {
	int	event;
	int	status;
};

struct eventview ev[MAXPEERS];

void finish(int mode)
{
	int pn;

#ifdef DEBUG
	switch(mode) {
	case PROC_SHUTDOWN:
		printf("DEBUG: finish: Process Shutdown Mode\n");
		break;
	case PROC_REBOOT:
		printf("DEBUG: finish: Process Reboot Mode\n");
		break;
	default:
		printf("DEBUG: finish: Unknown Finish Mode = %d\n", mode);
	}
#endif
	if (mode != PROC_CHILDEND) {
		pr_log(VLOG_GLOB, LOG_ALL, "Finish processing\n", ON);
	}
	for(pn = 0; pn < MAXPEERS && peer[pn].neighbor != 0; pn++)
		close(peer[pn].soc);
	if (sv_sock != -1) close(sv_sock);
	clear_timer();
	close_log();
	close_prefixlog();
	bvs_finish(mode);

	if (mode != PROC_CHILDEND) {
		unlink(PID);
	}

	if (mode == PROC_REBOOT) {
		printf("BGPView process rebooting....\n");
		execvp(exec_argv[0], exec_argv);
	}
	if (mode != PROC_CHILDEND) {
		exit(0);
	}
}

void finish_sig()
{
	notify_systemnotify("SHUTDOWN", "BGPView received shutdown signal");
	finish(PROC_SHUTDOWN);
}

void start_bgpinfo_timer()
{
	int		cnt;
	struct timeval 	tm;

	for(cnt = 0; cnt < MAXPEERS && peer[cnt].neighbor != 0; cnt++) {
		tm.tv_sec = bgpinfo[cnt].interval;
		tm.tv_usec = 0;
		if (!entry_timer(&tm, peer[cnt].neighbor, ID_BGPINFO_TIMER)) {
			pr_log(VLOG_GLOB, LOG_ALL,
				"failed to allocate memory.\n", ON);
			return;
		}
	}
}

int init()
{
	struct sockaddr_in	sin;
	struct hostent		*hp;
	char			hostname[MAXHOSTNAMELEN];
	int			cnt;
	FILE			*fp;
	pid_t			pid;
	char			strbuf[MAXHOSTNAMELEN+16];
	int			sopt = 1;

	sv_sock = -1;
	bzero(&peer[0], (sizeof(PEERS)));
	facility = -1;
	sfd = NULL;

	if ((fp = fopen(PID, "r")) == NULL) {
		if ((fp = fopen(PID, "w")) == NULL) {
			printf("PID File could not create.\n");
			return 1;
		} else {
			pid = getpid();
			fprintf(fp, "%d", pid);
			fclose(fp);
		}
	} else {
		printf("PID File already exist\n");
		fclose(fp);
		return 1;
	}

	signal(SIGINT, finish_sig);
	signal(SIGTERM, finish_sig);
	signal(SIGHUP, restart_log);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, wait_sigchld);

	bzero(peer,(sizeof(PEERS) * MAXPEERS));

	for(cnt=0; cnt<MAXPEERS; cnt++){
		ev[cnt].status = 0;
		ev[cnt].event  = 0;
	}

	init_dummy();
	clear_timer();
	bvs_init_set_default();
	init_routertt();
	init_mailannounce();

	strcpy(sysname, DEFSYSNAME);

#ifdef RADIX
	init_radix();
	init_radix_v6();
#endif
	strcpy(spooldir, SPOOL_DEFAULT);
#ifdef RTHISTORY
	rthistcleartime.tv_sec = 0;
	rthistcleartime.tv_usec = 0;
	rthistcleartime_v6.tv_sec = 0;
	rthistcleartime_v6.tv_usec = 0;
#endif
	init_cron();

	/* save Start time */
	gettimeofday(&uptime, (struct timezone *)0);

#ifdef IRRCHK
	/* IRR Check Initializing */
	init_irrif();
#endif

	read_conf(configfile);
	pr_log(VLOG_GLOB, LOG_ALL, "Initializing Process\n", ON);

	init_prefixlog();
	signal(SIGUSR1, init_prefixlog);
	signal(SIGUSR2, sigexec_getdmyinfo);

	/* initiate resouces */
	FD_ZERO(&readfds);

	/****** Server Socket Open ********/
	/* get host name */
	if (gethostname(hostname, sizeof(hostname)) != 0) {
		pr_log(VLOG_GLOB, LOG_ALL, "failed to get hostname.\n", ON);
		return 1;
	}

	/* lookup network number */
	if ((hp = gethostbyname(hostname)) == NULL) {
		sprintf(strbuf, "%s: host unknown.\n", hostname);
		pr_log(VLOG_GLOB, LOG_ALL, strbuf, ON);
		/* return 1; */
	}
#ifdef DEBUG
	printf("My Hostname : %s\n", hostname);
	if (hp != NULL) printf("GET hostname: %s\n", hp->h_name);
#endif

	/* get socket */
#ifdef DEBUG
	printf("DEBUG: Open Socket\n");
#endif
	if ((sv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		pr_log(VLOG_GLOB, LOG_ALL, "socket() Error\n", ON);
		perror("server: socket");
		return 1;
	}

#ifdef DEBUG
	printf("DEBUG: Set Socket Option: SO_REUSEADDR\n");
#endif
	if (setsockopt(sv_sock, SOL_SOCKET, SO_REUSEADDR,
						&sopt, sizeof(sopt)) < 0) {
		pr_log(VLOG_GLOB, LOG_ALL,
			"Socketoption(SO_REUSEADDR) set error (BGP)\n", ON);
		perror("socket option(SO_REUSEADDR) set error (BGP)");
		return(1);
	}

#ifndef LINUX
#ifdef SO_REUSEPORT
#ifdef DEBUG
	printf("DEBUG: Set Socket Option: SO_REUSEPORT\n");
#endif
	if (setsockopt(sv_sock, SOL_SOCKET, SO_REUSEPORT,
						&sopt, sizeof(sopt)) < 0) {
		pr_log(VLOG_GLOB, LOG_ALL,
			"Socketoption(SO_REUSEPORT) set error (BGP)\n", ON);
		perror("socket option(SO_REUSEPORT) set error (BGP)");
		return(1);
	}
#endif /* SO_REUSEPORT */
#endif /* LINUX */

	/* cleate address & port BGP(179) */
	sin.sin_family 		= AF_INET;
	sin.sin_addr.s_addr 	= htonl(INADDR_ANY);
	sin.sin_port 		= htons(BGP_PORT);
	
	/* try bind */
#ifdef DEBUG
	printf("DEBUG: Binding\n");
#endif
	if (bind(sv_sock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		pr_log(VLOG_GLOB, LOG_ALL, "bind() Error\n", ON);
		perror("server: bind");
		return 1;
	}
	
	/* listen on the socket */
#ifdef DEBUG
	printf("DEBUG: Listening\n");
#endif
	if (listen(sv_sock, 5) < 0) {
		pr_log(VLOG_GLOB, LOG_ALL, "listen() Error\n", ON);
		perror("server: listen");
		return 1;
	}
	FD_SET(sv_sock, &readfds);

	printf("BGPView Shell Initializing\n");
	if (bvs_init_start() == 1) {
		return 1;
	}

	/* BGP INFO Timer Start */
	start_bgpinfo_timer();

#ifdef RTHISTORY
	/* Route Change Clearing Timer Start */
	if (start_rthistclear_timer() == 1) {
		return 1;
	}
	if (start_rthistclear_timer_v6() == 1) {
		return 1;
	}
#endif

	/* Cron read conf */
	read_cron_file();

	printf("BGPView cron Started\n");

	printf("BGPView Initialize Done..\n");
	return 0;
}

int check_id1topn(net_ulong id1)
{
	int	ret;
	int	pn;
#ifdef DEBUG
	struct in_addr adr1, adr2;
	char	adrstr[128];
#endif

	ret = -1;
	for(pn = 0; pn < MAXPEERS && peer[pn].neighbor != 0; pn++) {
#ifdef DEBUG
		adr1.s_addr = peer[pn].neighbor;
		sprintf(adrstr, "%s", inet_ntoa(adr1));
		adr2.s_addr = id1;
		printf("DEBUG: Check Neighbor[%d]: %s : %s\n", pn,
			adrstr, inet_ntoa(adr2));
#endif
		if (peer[pn].neighbor == id1) {
			ret = pn;
			break;
		}
	}
	return ret;
}

int sourcetopn(char *v6adrs, char afi)
{
        int     ret;

        switch(afi) {
        case AF_IPV4:
                ret = check_id1topn(*((net_ulong *)v6adrs));
                break;
        case AF_IPV6:
                /* source address IPv6 not supported */
                ret = -1;
                break;
        }
#ifdef DEBUG
	{
		char tmpv6str[50];
		bintostrv6(v6adrs, tmpv6str);
		printf("DEBUG: sourcetopn: v6adrs = %s, afi = %d, ret = %d\n",
			tmpv6str, afi, ret);
	}
#endif
        return(ret);
}

int check_soctopn(int soc)
{
	int	ret;
	int	pn;

	ret = -1;
	for(pn = 0; pn < MAXPEERS && peer[pn].neighbor != 0; pn++) {
		if (peer[pn].soc == soc) {
			ret = pn;
			break;
		}
	}
	return ret;
}

void pr_notification(pn)
int pn;
{
	int	ptr;
	char	*submsg, code, idx;

	static char *note[] = { "",
				"Message Header Error",
				"Open Message Error",
				"UPDATE Message Error",
				"Hold Timer Expired",
				"Finite State Machine Error",
				"Cease"	};

	static char *mhe[] = {	"",
				"Connection Not Synchronized",
				"Bad Message Length",
				"Bad Message Type"	};

	static char *ome[] = {	"",
				"Unsupported Version Number",
				"Bad Peer AS",
				"Bad BGP Identifier",
				"Unsupported Optional Parameter",
				"Authentication Failure",
				"Unacceptable Hold Time",
				"Unsupported Capability" /* RFC2842 */ };

	static char *ume[] = {	"",
				"Malformed Attribute List",
				"Unrecognized Well-known Attribute",
				"Missing Well-known Attribute",
				"Attribute Flags Error",
				"Attribute Length Error",
				"Invalid ORIGIN Attribute",
				"AS Routing Loop",
				"Invalid NEXT_HOP Attribute",
				"Optional Attribute Error",
				"Invalid Network Field",
				"Malformed AS_PATH" };

	code = *peer[pn].data;
	pr_log(VLOG_NOTI, pn, "NOTIFICATION Message:\n", ON);
	sprintf(logstr, "  Error Code    = %d (%s)\n", (int)code, note[code]);
	pr_log(VLOG_NOTI, pn, logstr, OFF);

#ifdef DEBUG
	printf("DEBUG: NOTIFICATION Message:\n");
	printf("DEBUG:  Error Code    = %d (%s)\n", (int)code, note[code]);
#endif
	idx = *(peer[pn].data + 1);
	switch(code) {
	case 1:
		submsg = mhe[idx];
		break;
	case 2:
		submsg = ome[idx];
		break;
	case 3:
		submsg = ume[idx];
		break;
	default:
		submsg = "SubMessage Un-Defined";
	}
	sprintf(logstr, "  Error Subcode = %d (%s)\n", (int)idx, submsg);
	pr_log(VLOG_NOTI, pn, logstr, OFF);
	pr_log(VLOG_NOTI, pn, "  Data:\n", OFF);
#ifdef DEBUG
	printf("DEBUG:  Error Subcode = %d (%s)\n", (int)idx, submsg);
	printf("DEBUG:  Data:\n");
#endif
	for(ptr = 2;ptr < peer[pn].datalen; ptr++){
		sprintf(logstr, "%02X",*(peer[pn].data + ptr));
		pr_log(VLOG_NOTI, pn, logstr, OFF);
#ifdef DEBUG
		printf("%s", logstr);
#endif
	}
	pr_log(VLOG_NOTI, pn, "\n", OFF);
#ifdef DEBUG
	printf("\n", logstr);
#endif
}

int sockwrite(soc, buf, len)
int soc;
char *buf;
int len;
{
	char *ptr;
	int  left,written;
	fd_set	wset;
	struct timeval tval;
	int	rtcnt;

	tval.tv_sec  = 1;
	tval.tv_usec = 0;
	ptr = buf;
	left = len;
	rtcnt = 0;
	while(left > 0) {
		FD_ZERO(&wset);
		FD_SET(soc, &wset);

		if (select(FD_SETSIZE, NULL, &wset, NULL, &tval) < 0) {
			if (errno == EINTR) {
				printf("Select Return Code EINTR\n");
				continue;
			}
			perror("select");
			notify_systemnotify("REBOOT",
				"Select Error : sockwrite");
			finish(PROC_REBOOT);
			return(0);
		}
		if (FD_ISSET(soc, &wset)) {
			written = write(soc, ptr, left);
			if (written <= 0) {
				perror("write");
				pr_log2(VLOG_GLOB, LOG_ALL, ON,
						"Write Error : %d\n",errno);
				return written;
			}
			left -= written;
			ptr += written;
		} else {
			rtcnt++;
			if (rtcnt > 3) return -1;
#ifdef DEBUG
			printf("Socket Write Retry\n");
#endif
		}
	}
	return len - left;
}

int sendopen(int pn)
{
	char	buf[1024];
	int	sz;
	int	bt;
#ifdef DEBUG
	struct in_addr adr;
#endif

	if (peer[pn].local_id == 0) {
		peer[pn].local_id = IDENT;
	}
	if (peer[pn].local_as == 0) {
		peer[pn].local_as = LOCAL_AS;
	}

	memset(buf, 0xFF, 29);
	/* store_int16(buf+16, 29); */
	*(buf+18) = BGP_OPEN;
	*(buf+19) = 4;
	if (peer[pn].local_as > 65535) {
		/* Non-Mappable Local AS */
		store_int16(buf+20, AS_TRANS);
	} else {
		/* Mappable locla AS */
		store_int16(buf+20, (u_short)peer[pn].local_as);
	}
	store_int16(buf+22, peer[pn].holdtimer);
	store_int32(buf+24, ntohl(peer[pn].local_id));
	*(buf+28) = '\0';

	/*** Insert MPBGP Option for IPv6 ***/
	bt = 0;
	if (peer[pn].nlri == IPv6_UNICAST) {
		*(buf+bt+29) = BGPOPT_CAPABILITY;
		*(buf+bt+30) = 6;
		*(buf+bt+31) = CAP_MPBGP;
		*(buf+bt+32) = 4;
		store_int16(buf+bt+33, AF_IPV6);
		*(buf+bt+35) = 0;
		*(buf+bt+36) = SAFI_NLRI_UNI;
		bt += 8;
	} if (peer[pn].nlri == IPv4_UNICAST) {
		*(buf+bt+29) = BGPOPT_CAPABILITY;
		*(buf+bt+30) = 6;
		*(buf+bt+31) = CAP_MPBGP;
		*(buf+bt+32) = 4;
		store_int16(buf+bt+33, AF_IPV4);
		*(buf+bt+35) = 0;
		*(buf+bt+36) = SAFI_NLRI_UNI;
		bt += 8;
	}

	/*** 4 Octet Capability Enabling ***/
	if (peer[pn].as4_enable) {
		*(buf+bt+29) = BGPOPT_CAPABILITY;
		*(buf+bt+30) = 6;
		*(buf+bt+31) = CAP_4OCTAS;
		*(buf+bt+32) = 4;
		store_int32(buf+bt+33, peer[pn].local_as);
		bt += 8;
	}

	*(buf+28) = bt;
	store_int16(buf+16, 29+bt);


#ifdef DEBUG
	printf("Sending Open Message :\n");
	printf("  Socket Number       : %d\n", peer[pn].soc);
	printf("  Data Length	      : %d\n", (int)buf2ushort(buf+16));
	printf("  Header Type         : %d\n", *(buf+18));
	printf("  Version             : %d\n", *(buf+19));
	printf("  My Autonomous Number: %d\n", (int)buf2ushort(buf+20));
	printf("  Hold Time           : %d\n", (int)buf2ushort(buf+22));
	(void)memcpy((char *)&adr.s_addr, buf+24, 4);
	printf("  BGP Identifier      : %s\n", inet_ntoa(adr));
	printf("  Option Length       : %d\n", *(buf+28));
	if (*(buf+28) != 0) {
		int optcnt;
		optcnt = 0;
		printf("  Option DUMP         : ");
		while(optcnt < *(buf+28)) {
			printf("%02X", (u_char)*(buf+29+optcnt));
			optcnt++;
		}
		printf("\n");
	}
#endif

	if ((sz = sockwrite(peer[pn].soc, buf, 29+bt)) > 0) {
		sprintf(logstr,
			"Open Message Send Succeed: wrote %d byte\n", sz);
		pr_log(VLOG_OPEN, pn, logstr, ON);

	} else {
		pr_log(VLOG_OPEN, pn, "Open Message Send Fail : \n", ON);
	}
	return sz;
}

int sendkeepalive(pn)
int pn;
{
	char	buf[256];
	int	sz;

	memset(buf, 0xFF, 19);
	store_int16(buf+16, 19);
	*(buf+18) = BGP_KEEPALIVE;

	if ((sz = sockwrite(peer[pn].soc, buf, 19)) > 0) {
		sprintf(logstr,
			"KEEPALIVE Message Send Succeed: wrote %d byte\n", sz);
		pr_log(VLOG_KEEP, pn, logstr, ON);
	} else {
		sprintf(logstr, "KEEPALIVE Message Send Fail: %d\n", sz);
		pr_log(VLOG_KEEP, pn, logstr, ON);
	}
	return sz;
}

/********************************************************
 * Send Notification Message                            *
 *------------------------------------------------------*
 * pn : Peer Number                                     *
 * ec : Error Code                                      *
 * sc : Sub Error Code                                  *
 *------------------------------------------------------*
 * Return:                                              *
 *  Send Status. It equal with sockwrite function       *
 ********************************************************/
int sendnotification(int pn, char ec, char sc)
{
	char	buf[256];
	int	sz;

	memset(buf, 0xFF, 21);
	store_int16(buf+16, 21);
	*(buf+18) = BGP_NOTIFICATION;
	*(buf+19) = ec;
	*(buf+20) = sc;

	if ((sz = sockwrite(peer[pn].soc, buf, 21)) > 0) {
		sprintf(logstr,
		    "Notification Message Send Succeed: wrote %d byte\n", sz);
		pr_log(VLOG_NOTI, pn, logstr, ON);
	} else {
		pr_log(VLOG_NOTI, pn, "Notification Message Send Fail\n", ON);
	}
	return sz;
}

#ifdef DVERV
void recv_dump(char *data, int len, int pn)
{
	char	buf[32];
	int 	length;
	char	*ptn, *pt;
	int	cnt;
	struct tm	*tm;
	time_t		clock;

	length = len;
	ptn = data;
	pt = ptn+length;
	cnt = 0;
	sprintf(logstr, "RECV DUMP LEN  : %d Octets\n", length);
	pr_log(VLOG_GLOB, pn, logstr, OFF);
	time(&clock);
	tm = localtime(&clock);
	strftime(buf, 30, "%Y/%m/%d %H:%M:%S", tm);
	sprintf(logstr, "RECV DUMP DATE : %s\n", buf);
	pr_log(VLOG_GLOB, pn, logstr, OFF);
	pr_log(VLOG_GLOB, pn, "RECV DUMP 0000 : ", OFF);
	while(ptn < pt) {
		sprintf(logstr, "%08X ", buf2ulong(ptn));
		pr_log(VLOG_GLOB, pn, logstr, OFF);
		ptn += 4;
		cnt++;
		if ((cnt % 4) == 0) {
			sprintf(logstr, "\nRECV DUMP %04d : ", cnt*4);
			pr_log(VLOG_GLOB, pn, logstr, OFF);
		}
	}
	pr_log(VLOG_GLOB, pn, "\n", OFF);
	return;
}
#endif


int recvdata(int pn,int len)
{
	int	ret;
	int	left;
	int	rcnt;
	int	readed;
	char	readbuf[BGP_MAX_PACKET_SIZE];

#ifdef DEBUG
	printf("Receiving Data : %d Octets\n",len);
#endif
	if (len <= 0) return 0;

	left = len;
	if (peer[pn].data != NULL) free(peer[pn].data);
	peer[pn].data = (char *)malloc(len);
	bzero(peer[pn].data, len);
#ifdef DEBUG
	printf("Malloc OK  : %d Octets\n",len);
#endif
	rcnt = 0;
	readed = 0;
	while(1) {
		bzero(&readbuf[0], BGP_MAX_PACKET_SIZE);
		ret = read(peer[pn].soc, &readbuf[0], left); 
                if (ret <= 0) {
                        pr_log2(VLOG_GLOB, pn, ON,
                                "Socket Read Error pn=%d DataLen=%d\n",
                                pn, len);
			rcnt++;
			if (rcnt >= 6) break;
                        continue;
                }
		memcpy(((char *)peer[pn].data + readed), readbuf, ret);
#ifdef DEBUG
		printf("Received Data  : %d Octets\n",ret);
#endif
		left -= ret;
		readed += ret;
		if (left <= 0) break;
		rcnt++;
		if (rcnt >= 6) break;
	}
		
	if ((left > 0) && (ret != 0)) {
		sprintf(logstr, "Data Read Fail : %d/%d : peer = %d\n",
			readed, len, pn);
		pr_log(VLOG_GLOB, pn, logstr, ON);
		ret = -1;
	}
	peer[pn].datalen = readed;
#ifdef DVERV
	recv_dump(peer[pn].data, peer[pn].datalen, pn);
#endif
	return readed;
}

int check_header(int pn)
{
	char			headbuf[20];
	BGPV4_HEADER		bgp_head;
	char			*ptr;
	int			ret;
	char			m1;
	int			mcount;
	struct in_addr		adr;

	mcount = 0;
	while(mcount < MARKERLENGTH) {
		ret = read(peer[pn].soc, (char *)&m1, 1);
		if (ret < 0) return -1;
		if (ret == 0) {
			/* Connection Closed */
			peer[pn].event = BGP_STOP;
#ifdef DEBUG
			printf("DEBUG: Header Error\n");
#endif
			return 0;
		}

		if ((char)m1 == (char)0xFF) { 
			mcount++;
		} else {
			return 0;
		}
	}


#if 0
	sprintf(logstr, "Marker[Peer no. %d]: %08X %08X %08X %08X\n",
		pn, 	buf2ulong(headbuf),   buf2ulong(headbuf+4),
 			buf2ulong(headbuf+8), buf2ulong(headbuf+12));
	pr_log(VLOG_GLOB, pn, logstr, ON);
#endif

#if 0
	if ( (0xFFFFFFFF != buf2ulong(headbuf)) ||
	     (0xFFFFFFFF != buf2ulong(headbuf+4)) ||
	     (0xFFFFFFFF != buf2ulong(headbuf+8)) ||
	     (0xFFFFFFFF != buf2ulong(headbuf+12)) ) {
		pr_log(VLOG_GLOB, pn, "Marker Could Not Recognized\n", ON);
		peer[pn].event = BGP_NONE;
		return -1;
	}
#endif

	ret = read(peer[pn].soc, (char *)&headbuf[0],HEADERLENGTH-MARKERLENGTH);
	bgp_head.type = headbuf[2];
	bgp_head.length = buf2ushort(headbuf);

#ifdef DEBUG
	pr_log2(VLOG_GLOB, pn, ON, "BGP Header Type = %d, Length = %d\n",
					bgp_head.type, bgp_head.length);
#endif

	if ((bgp_head.type >= BGPTYPES) || (bgp_head.type <= 0)) {
		pr_log2(VLOG_GLOB, pn, ON,
			"GET Message Type = %d (Unknown) / Data Length  = %d\n",
			bgp_head.type, bgp_head.length);
		return(0);
	}

	pr_log2(VLOG_GLOB, pn, ON,
		"GET Message Type = %d (%s) / Data Length  = %d\n",
		bgp_head.type, bgptypestring[bgp_head.type], bgp_head.length);

	if (bgp_head.length > HEADERLENGTH) {
#ifdef DEBUG
		pr_log2(VLOG_GLOB, pn, ON,
			"DEBUG: Receive Data Size = %d / Peer = %d\n",
			(bgp_head.length - HEADERLENGTH), pn);
#endif
		ret = recvdata(pn, (bgp_head.length - HEADERLENGTH));
		if (ret < 0) return -1;
		if (ret == 0) {
			/* Connection Closed */
			peer[pn].event = BGP_STOP;
			return 0;
		}
	}

	bgpinfo[pn].msgcnt++;

	switch(bgp_head.type){
	case BGP_UNDEFINE:
		pr_log(VLOG_OPEN, pn, "Receive Type 0 Message(UNDEFINE)\n", ON);
		peer[pn].event = BGP_NONE;
		break;
	case BGP_OPEN:
		pr_log(VLOG_OPEN, pn, "Receive OPEN Message\n", ON);

		ptr = peer[pn].data;

		sprintf(logstr, "  Version             : %d\n", *ptr);
		pr_log(VLOG_OPEN, pn, logstr,OFF);
		sprintf(logstr,
			"  My Autonomous Number: %d\n", buf2ushort(ptr+1));
		pr_log(VLOG_OPEN, pn, logstr, OFF);
		sprintf(logstr,
			"  Hold Time           : %d\n", buf2ushort(ptr+3));
		pr_log(VLOG_OPEN, pn, logstr, OFF);
		(void)memcpy((char *)&adr.s_addr, ptr+5, 4);
		sprintf(logstr, "  BGP Identifier      : %s\n", inet_ntoa(adr));
		pr_log(VLOG_OPEN, pn, logstr, OFF);
		sprintf(logstr, "  Option Length       : %d\n", *(ptr+9));
		pr_log(VLOG_OPEN, pn, logstr, OFF);
		peer[pn].event = RECV_OPEN;
		bgpinfo[pn].opencnt++;
		break;
	case BGP_UPDATE:
		pr_log(VLOG_UPDE, pn, "Receive UPDATE Message\n", ON);
		peer[pn].event = RECV_UPDATE;
		bgpinfo[pn].updatecnt++;
		break;
	case BGP_NOTIFICATION:
		pr_log(VLOG_NOTI, pn, "Receive NOTIFICATION Message\n", ON);
		pr_notification(pn);
		peer[pn].event = RECV_NOTIFICATION;
		bgpinfo[pn].notificationcnt++;
		break;
	case BGP_KEEPALIVE:
		pr_log(VLOG_KEEP, pn, "Receive KEEPALIVE Message\n", ON);
		peer[pn].event = RECV_KEEPALIVE;
		bgpinfo[pn].keepalivecnt++;
		break;
	default:
		sprintf(logstr,
			"Receive Unknown Message Type %d then BGP_STOP\n",
			bgp_head.type);
		pr_log(VLOG_GLOB, pn, logstr, ON);
		peer[pn].event = BGP_STOP;
	}
	return 0;
}

void resetfds()
{
	int pn;

	FD_ZERO(&readfds);
	FD_SET(sv_sock, &readfds);

	for(pn = 0; pn < MAXPEERS && peer[pn].neighbor != 0; pn++) {
		if (peer[pn].soc != -1) {
			FD_SET(peer[pn].soc, &readfds);
		}
	}
	bvs_readsocset(&readfds);
}

void usage()
{
	printf("BGPView %s\n", BV_VERSION);
	printf("  %s\n\n", BV_COPYRIGHT);
	printf("  Usage:\n");
	printf("    bgpview [-v] [-f filename] [-d]\n");
	printf("       -v          : Version\n");
	printf("       -f filename : Config File Name\n");
	printf("		     (Defalut : %s)\n", CONF_FILE);
	printf("       -d          : Daemon\n");
	printf("\n");
}

int execoption(int opcn, char *val[])
{
	int cnt;
	int ret;

	cnt = 1;
	ret = 0;
	strcpy(configfile, "");
	while(val[cnt] != NULL) {
		if (strcmp("-v", val[cnt]) == 0) {
			usage();
			return(1);
		} else if (strcmp("-f", val[cnt]) == 0) {
			if (val[cnt+1] == NULL) {
				usage();
				return(1);
			}
			strcpy(configfile, val[cnt+1]);
			cnt++;
		} else if (strcmp("-d", val[cnt]) == 0) {
			ret = 2;
		} else {
			usage();
			printf("Illegal Option\n");
			return(1);
		}
		cnt++;
	}
	if (strlen(configfile) == 0) {
		strcpy(configfile, CONF_FILE);
	}
	return(ret);
}

int connect_waiting()
{
        int     ret;
        int     cnt;

        ret = 0;
        for(cnt = 0; cnt < MAXPEERS && peer[cnt].neighbor != 0; cnt++) {
                if (peer[cnt].status == CONNECT) {
#ifdef DEBUG
        printf("DEBUG: NON-BLOCKING FOR CONNECT STATUS\n");
#endif
                        ret = 1;
                        break;
                }
        }
        return(ret);
}


main(int argc, char *argv[])
{
	int			fromlen;
	register int 		ns;
	int			quit;
	struct sockaddr_in	sin;
	struct timeval		timer_value;
	net_ulong		event_id1;
	int			event_id2;
	int			pn, lf, nfd;
	int			execret;

	exec_argc = argc;
	exec_argv = argv;

	/* Process Initialize */
	execret = execoption(exec_argc, exec_argv);
	switch(execret) {
		case 0:	/* Normal Execute */
			break;

		case 2:	/* Daemon Mode */
			if (daemon(1, 1) < 0) {
				printf("failed to become a daemon..\n");
				exit(1);
			}
			break;

		default:/* fail to execute */
			finish(PROC_SHUTDOWN);
			exit(1);
	}

	if (init() != 0) {
		printf("Process Initialize Error\n");
		finish(PROC_SHUTDOWN);
		exit(1);
	}

	/* state initialize */
	quit = 0;
	while(!quit) {

		lf = 0;
#ifdef DEBUG
		printf("Processing FSM\n");
#endif
		for(pn = 0; pn < MAXPEERS && peer[pn].neighbor != 0; pn++) {
#ifdef DEBUG
			printf("Checking FSM Peer = %d\n",pn);
#endif
			quit = check_fsm(pn);
			if (quit != 0) {
#ifdef DEBUG
				printf("QUIT FSM Code = %d\n",quit);
#endif
				peer[pn].soc = -1;
				if (peer[pn].peerstatus == false) {
					peer[pn].event = BGP_NONE;
				} else {
					peer[pn].event = BGP_START;
				}
				peer[pn].status = IDLE;
				quit = 0;
			}
		}

#ifdef DEBUG
		printf("Checking LOOP\n");
#endif
		for(pn = 0; pn < MAXPEERS && peer[pn].neighbor != 0; pn++) {
			if ((ev[pn].status != peer[pn].status) ||
			    (ev[pn].event  != peer[pn].event)) {
			pr_log(VLOG_GLOB, pn, "EVENT VIEW: ", ON);
			sprintf(logstr, "  Peer No = %d ",pn);
			pr_log(VLOG_GLOB, pn, logstr, OFF);
			sprintf(logstr, "  Event   = %d ",peer[pn].event);
			pr_log(VLOG_GLOB, pn, logstr, OFF);
			sprintf(logstr, "  Status  = %d\n",peer[pn].status);
			pr_log(VLOG_GLOB, pn, logstr, OFF);
			}
			if (peer[pn].event != BGP_NONE) lf = 1;
			ev[pn].status = peer[pn].status;
			ev[pn].event  = peer[pn].event;
		}

		if (lf == 1) {
#ifdef DEBUG
			printf("LOOP Enable\n");
#endif
			continue;
		}

		/* Socket Selecting */
		check_timer(&event_id1, &event_id2, &timer_value);

		if ((event_id1 == 0) && (event_id2 == 0)) {
			if (bvs_cont_proc() == 1) {
				timer_value.tv_sec  = 0;
				timer_value.tv_usec = 0;
			}
			if (send_dummy() == 1) {
				timer_value.tv_sec  = 0;
				timer_value.tv_usec = 0;
			}
			if (connect_waiting() == 1) {
				timer_value.tv_sec  = 0;
				timer_value.tv_usec = 0;
			}
#ifdef IRRCHK
			if (irr_check_enable == 1) {
			  if (irr_walker_check() == IRRCHK_ACTIVE) {
				timer_value.tv_sec  = 0;
				timer_value.tv_usec = 0;
			  }
			}
#endif	
			resetfds();
			nfd = select(FD_SETSIZE, &readfds, 
						NULL, NULL, &timer_value);

			if (nfd < 0) {
				if (errno == EINTR) {
					printf("Select Return Code EINTR\n");
					continue;
				}
				perror("select");
				notify_systemnotify("REBOOT",
						"Select Error : main() - 1");
				finish(PROC_REBOOT);
				return(1);
			}

			/* Time Out Check */
			check_timer(&event_id1, &event_id2, &timer_value);
		}

		/* Common timer Time out */
		if (event_id1 == COMMON_ID1) {
			common_timer_expire(event_id2);
			continue;
		}

		if ((event_id1 != 0) && (event_id2 != 0)) {
			sprintf(logstr, "Cause Time Out : ID1=0x%08X ID2=%d\n",
						ntohl(event_id1), event_id2);
			pr_log(VLOG_GLOB, LOG_ALL, logstr, ON);

			/* Check Peer Number */
			pn = check_id1topn(event_id1);
			if (pn == -1) {
				quit = 1;
				break;
			}

			switch(event_id2){
			case ID_CNT_RETRY_TIMER:
				pr_log(VLOG_GLOB, pn, 
					"Connect Retry Timer Expired\n", ON);
				peer[pn].event = CNT_RETRY_TIME_EXP;
				break;
			case ID_HOLD_TIMER:
				pr_log(VLOG_GLOB, pn,
					"Hold Timer Expired\n", ON);
				peer[pn].event = HOLD_TIME_EXP;
				break;
			case ID_KEEPALIVE_TIMER:
				pr_log(VLOG_GLOB, pn, 
					"Keepalive Timer Expired\n", ON);
				peer[pn].event = KEEPALIVE_TIME_EXP;
				break;
			case ID_MINORIGIN_INTERVAL:
				pr_log(VLOG_GLOB, pn, 
				"Minimum Origin Interval Timer Expired\n", ON);
				break;
			case ID_MINROUTEADV_INTERVAL:
				pr_log(VLOG_GLOB, pn, "Minimum Route Advertise"
					" Interval Timer Expired\n", ON);
				break;
			case ID_RE_INITIAL_TIMER:
				pr_log(VLOG_GLOB, pn, 
					"Re-Initial Timer Expired\n", ON);
				peer[pn].resume = OFF;
				if (peer[pn].peerstatus == false) {
					peer[pn].event = BGP_NONE;
				} else {
					peer[pn].event = BGP_START;
				}
				peer[pn].status = IDLE;
				break;
			case ID_BGPINFO_TIMER:
				pr_log(VLOG_GLOB, pn, 
					"BGP_INFO Timer Expired\n", ON);
				write_info(pn);
				if (bgpinfo[pn].interval == 0) {
					bgpinfo[pn].infostatus == false;
				}
				if (bgpinfo[pn].infostatus == false) break;
				timer_value.tv_sec = bgpinfo[pn].interval;
				timer_value.tv_usec = 0;
				if (!entry_timer(&timer_value,
						peer[pn].neighbor,
						ID_BGPINFO_TIMER)) {
					pr_log(VLOG_GLOB, LOG_ALL,
                                	"failed to allocate memory.\n", ON);
					notify_systemnotify("STOP",
						"Memory Allocation Error : main() - 2");
					finish(PROC_SHUTDOWN);
					return(1);
				}
				break;
			default:
				peer[pn].event = BGP_NONE;
				break;
			}
			continue;
		}
#ifdef DEBUG
/*		printf("Checking Server Socket\n");*/
#endif
		if (FD_ISSET(sv_sock, &readfds)) {
			/* accept connection */
			pr_log(VLOG_GLOB, LOG_ALL, 
				"Server Socket Connect Request Received\n", ON);
#ifdef DEBUG
			printf("DEBUG: Accepting\n");
			printf("DEBUG: Before Socket : %d\n", sv_sock);
#endif
			fromlen = sizeof(sin);
			if ((ns = accept(sv_sock, (struct sockaddr *)&sin,
					&fromlen)) < 0) {
				perror("server: accept");
				break;
			}
			sprintf(logstr, "  Requesting Neighbor = %s\n",
					inet_ntoa(sin.sin_addr));
			pr_log(VLOG_GLOB, LOG_ALL, logstr, OFF);
			pn = check_id1topn(sin.sin_addr.s_addr);
			if (pn < 0) {
				pr_log(VLOG_GLOB, LOG_ALL,
				"  This Request Could Not Accept\n",OFF);
				close(ns);
			} else {
				sprintf(logstr, "  Accepting Socket : %d\n",ns);
				pr_log(VLOG_GLOB, LOG_ALL, logstr, OFF);
				sprintf(logstr, "  Accepting Peer   : %d\n",pn);
				pr_log(VLOG_GLOB, LOG_ALL, logstr, OFF);
				if (pn < 0) {
					pr_log(VLOG_GLOB, LOG_ALL,
						"  Socket Clear\n", OFF);
					close(ns);
				} else {
					if (peer[pn].peerstatus == false) {
						pr_log(VLOG_GLOB, LOG_ALL,
						  "  Disabled Peer\n", OFF);
						close(ns);
					/***** Templary Comment Out 
						2006/08/26 By K.Kondo
					} else if (peer[pn].resume == ON) {
						pr_log(VLOG_GLOB, LOG_ALL,
						  "  Now Resuming Peer\n", OFF);
						close(ns);
					*************************************/
					} else if (peer[pn].soc != ns) {
					  if (peer[pn].status <= ACTIVE) {
					    /* Server Socket Connect */
					    pr_log(VLOG_GLOB, LOG_ALL,
						  "  Old Peer Clear\n", OFF);
					    clear_peer(pn);
					    peer[pn].resume = OFF;
					    off_timer(peer[pn].neighbor,
					    		ID_RE_INITIAL_TIMER);
					    peer[pn].soc = ns;
					    FD_SET(peer[pn].soc, &readfds); 
					    peer[pn].status = ACTIVE;
					    peer[pn].event = BGP_TRANS_CNT_OPEN;
					    peer[pn].port = 
					     ((struct sockaddr_in)sin).sin_port;
#ifdef TCP_MD5SIG
					    md5passwd_set(pn);
#endif
					  } else {
					    /* Server Socket Reject */
					    pr_log(VLOG_GLOB, LOG_ALL,
					        "  This neighbor"
					        " already connected\n", OFF);
					    close(ns);
					  }
					} else {
						peer[pn].soc = ns;
						FD_SET(peer[pn].soc, &readfds); 
						peer[pn].status = ACTIVE;
						peer[pn].event = 
							BGP_TRANS_CNT_OPEN;
					        peer[pn].port = 
					     ((struct sockaddr_in)sin).sin_port;
#ifdef TCP_MD5SIG
					        md5passwd_set(pn);
#endif
					}
				}
			}
			continue;
		}

		/* Check BVS Server Socket */
		if (bvs_open_new_shell(&readfds)) continue;
#ifdef DEBUG
/*		printf("Checking Client Sockets\n"); */
#endif
		for(pn = 0; pn < MAXPEERS && peer[pn].neighbor != 0; pn++) {
#ifdef DEBUG
			printf("Checking Peer %d\n",pn);
#endif
			if (peer[pn].soc == -1) {
				continue;
			}

			if (peer[pn].status == CONNECT) {
			    sin.sin_family      = AF_INET;
			    sin.sin_port        = htons(BGP_PORT);
			    sin.sin_addr.s_addr = peer[pn].neighbor;
			    if (connect(peer[pn].soc, (struct sockaddr *)&sin,
				    sizeof(struct sockaddr)) < 0) {
				if (errno == EALREADY) {
       				    /* waiting the completion of the
				     * last call of connect().
				     */
       				    continue;
				} else if (errno != EISCONN) {
       				    /* the connection to the peer seems
				     * to be failed.
				     */
       				    peer[pn].event = BGP_TRANS_CNT_OPEN_FAIL;
       				    continue;
				}
			    }
			}

			if (peer[pn].status == CONNECT) {
				fcntl(peer[pn].soc, F_SETFL, peer[pn].flag);
				peer[pn].event = BGP_TRANS_CNT_OPEN;
			} else {
				if (FD_ISSET(peer[pn].soc, &readfds)) {
					check_header(pn);
				}
			}
		}
#ifdef DEBUG
		printf("Socket Check Done.\n");
#endif
		/* BVS Client Socket Check */
		bvs_input(&readfds);

#ifdef IRRCHK
		/* IRR Check */
		walk_bgp_chk_irr();
#endif
	}

	close(sv_sock);
	exit(0);
}
