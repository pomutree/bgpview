/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1997/12                         */
/*                                                              */
/* FSM Routine                                                  */
/****************************************************************/
/* fsm.c,v 1.3 2005/04/27 07:43:43 kuniaki Exp */

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

#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include "bvtypes.h"
#include "bgp.h"
#include "bgplog.h"
#include "timer.h"

#ifdef DEBUG
struct bgp_status_event {
	int	status;
	int	event;
};
struct bgp_status_event	st_ev[MAXPEERS];
#endif

extern int sendopen(/*pn*/);			/* in bgpview.c */
extern int sendkeepalive(/*pn*/);		/* in bgpview.c */
extern int sendnotification(/*pn, ec, sc*/);	/* in bgpview.c */
extern int peer_off_timer(/*id1*/);	/* in timer.c */
extern void pr_update(/*pn*/);		/* in update.c */

char capability_check(char *param, char length, int pn)
{
	u_char	paramcode;
	char	paramlen;
	char	paramcnt;
	char	opterr;
	char	*ptr;
	char	pline[LOGSTRSIZE];


#ifdef DEBUG
	char *ptr2,*ptr3;
	int  cnt_t;
	/* Capability DUMP */
	cnt_t = 0;
	printf("DEBUG: CAPDUMP: LEN: %d Octets\n", length);
	printf("DEBUG: CAPDUMP 0000: ");
	ptr2 = param;
	ptr3 = param+length;
	while(ptr2 < ptr3) {
		printf("%08X ", buf2ulong(ptr2));
		ptr2 += 4;
		cnt_t++;
		if ((cnt_t % 4) == 0) {
			printf("\nDEBUG: CAPDUMP %04d: ", cnt_t*4);
		}
	}
	printf("\n");
#endif
	

	paramcnt = 0;
	opterr = 0;
	ptr = param;
	
	while(paramcnt < length) {
		paramcode = *ptr;
		paramlen  = *(ptr+1);

		sprintf(pline, "CAPACHK: Capability Code : %d\n", paramcode);
		pr_log(VLOG_GLOB, pn, pline);

		switch(paramcode) {
		case CAP_RESERVE:
			/* RESERVE */
			pr_log(VLOG_GLOB, pn, "CAPACHK: Unsupported Capability\n", ON);
			opterr = UNSUPPORTED_CAPABILITY;
			break;

		case CAP_MPBGP:
			/* MPBGP */
#ifdef DEBUG
			printf("DEBUG: CAP_MPBGP: AFIDUMP: %02X%02X %02X %02X\n",
				*(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5));
#endif
			sprintf(pline, "CAPACHK: CAP_MPBGP: AFI : %d\n", ntohs(*(u_short *)(ptr+2)));
			pr_log(VLOG_GLOB, pn, pline);

			switch(ntohs(*(u_short *)(ptr+2))) {

			case AF_IPV4:
#ifdef DEBUG
				printf("DEBUG: CAPACHK: Address Family IPv4\n");
#endif
				opterr = 0;
				break;			

			case AF_IPV6:
#ifdef DEBUG
				printf("DEBUG: CAPACHK: Address Family IPv6\n");
#endif
				opterr = 0;
				break;
			default:
				pr_log(VLOG_GLOB, pn, "CAPACHK: AFI_CHK: Unsupported Capability\n", ON);
				opterr = UNSUPPORTED_CAPABILITY;
			}
			if (opterr != 0) break;

			sprintf(pline, "CAPACHK: CAP_MPBGP: SAFI : %d\n", (u_char)*(ptr+5));
			pr_log(VLOG_GLOB, pn, pline);

			switch((u_char)*(ptr+5)) {
			case SAFI_NLRI_UNI:
				opterr = 0;
				break;
			case SAFI_NLRI_MULTI:
				/* Beta0.38 : Multicast is not supported
                                   But, ignore this */
				opterr = 0;
				break;
			case SAFI_NLRI_BOTH:
				/* Beta0.38 : Multicast is not supported
                                   But, ignore this */
				opterr = 0;
				break;
			case SAFI_NLRI_MPLS:
				/* Capability Error */
			default:
				pr_log(VLOG_GLOB, pn, "CAPACHK: Unsupported Capability\n", ON);
				opterr = UNSUPPORTED_CAPABILITY;
			}
			break;

		case CAP_ROUTEREFRESH: /* RFC2918 */
			/* This Capability is Ignored */
			break;

		case CAP_4OCTAS: /* 4Octet AS Capability */
			pr_log(VLOG_GLOB, pn, 
				"CAPACHK: Receive 4Byte AS Capability\n", ON);
			peer[pn].remote_as4 = buf2ulong(ptr+2);
			pr_log(VLOG_GLOB, pn, 
				"CAPACHK: Remote 4Byte AS Num = %ld\n",
				peer[pn].remote_as4);
#ifdef DEBUG
			printf("DEBUG: Received 4Byte AS Number = %ld\n", 
							peer[pn].remote_as4);
#endif
			if (peer[pn].as4_enable) {
				/* AS Check */
				if (peer[pn].remote_as != peer[pn].remote_as4) {
					opterr = BAD_PEER_AS;
				} else {
					peer[pn].as4_remote = true;
				}
			}
			break;

		case 128: /* Capability Private Use 128 */
			pr_log(VLOG_GLOB, pn,
			  "CAPACHK: Private Capability Code(Cisco?) is Ignored\n", ON);
			break;

		default:
			pr_log(VLOG_GLOB, pn, "CAPACHK: Unsupported Capability\n", ON);
			opterr = UNSUPPORTED_CAPABILITY;
		}
		if (opterr != 0) break;
		paramcnt += (2+paramlen);
		ptr += (2+paramlen);

#ifdef DEBUG
		printf("DEBUG: Parameter Length Check = %d / %d\n",
								 paramcnt, length);
#endif
	}

	return(opterr);

}


int openerrchk(int pn)
{
	u_short	bv;
	char	*ptr;
	char	optlen;

	char	paramcnt;
	char	paramcode;
	char	paramlen;
	char	*paramval;
	char	opterr;
	u_short	as2;

	ptr = peer[pn].data;
	/* Version Check */
	if ((char)*ptr != 4) {
		/* Versio  Error */
		/* Support Version is only 4. */
		pr_log(VLOG_GLOB, pn, "OPENCHK: Unsupported Version Number\n",ON);
		return UNSUPPORTED_VERSION_NUMBER;
	}
	ptr++;

	/* Autonomous Number Check */
	if (peer[pn].remote_as > 65535) {
		as2 = AS_TRANS;
	} else {
		as2 = peer[pn].remote_as;
	}
	if (buf2ushort(ptr) != as2) {
		pr_log(VLOG_GLOB, pn, "OPENCHK: Bad AS Number\n",ON);
		return BAD_PEER_AS;
	}
	ptr+=2;

	/* Hold Time Error Check */
	bv = buf2ushort(ptr);
	if ((bv == 1) || (bv == 2)) {
		/* Hold Time Error */
		pr_log(VLOG_GLOB, pn, "OPENCHK: Unacceptable Hold Time\n",ON);
		return UNACCEPTABLE_HOLD_TIME;
	}
	ptr+=2;

	/* BGP Identifier Error Check */
	ptr+=4;
	/* NO CHECK */

	/* Option Parameter Error Check */
	optlen = *ptr;
	ptr++;

	/* OPTION PARAMETER CHECK */
	paramcnt = 0;
	opterr = 0;
	while(paramcnt < optlen) {
		paramcode = *ptr;
		paramlen  = *(ptr+1);

		switch(paramcode) {
		case BGPOPT_AUTHENTICATION:
			/* Authentication NOT SUPPORT */
			pr_log(VLOG_GLOB, pn, "OPENCHK: Authentication Failure\n",ON);
			opterr = AUTHENTICATION_FAILURE;
			break;

		case BGPOPT_CAPABILITY:
			/* Capability Check */
			opterr = capability_check((ptr+2), paramlen, pn);
			break;

		default:
			pr_log(VLOG_GLOB, pn, "OPENCHK: Unsupported Optional Parameter\n",ON);
			opterr = UNSUPPORTED_OPTIONAL_PARAMETER;
		}
		if (opterr != 0) break;
		paramcnt += (2+paramlen);
		ptr += (2+paramlen);
	}

	return(opterr);
}

void clear_peer(pn)
int pn;
{
	struct timeval	timer_value;
	char		logstr[120];

	if (peer[pn].soc == -1) {
		/* Peer was already cleared */
		return;
	}
	if (peer[pn].peerstatus == true) peer[pn].resume = ON;
	close(peer[pn].soc);
	FD_CLR(peer[pn].soc, &readfds);
	peer[pn].soc 	 = -1;
	peer[pn].holdtimer = peer[pn].holdtimer_conf;
	peer[pn].keepalive = peer[pn].keepalive_conf;
	if (peer[pn].peerstatus == true) {
	    /* timer_value.tv_sec = RE_INITIAL_TIMER; */
	    timer_value.tv_sec = peer[pn].session_re_init_timer;
	    timer_value.tv_usec = 0;
	    peer_off_timer(peer[pn].neighbor);
	    if (!entry_timer(&timer_value, peer[pn].neighbor,
						ID_RE_INITIAL_TIMER)) {
		pr_log(VLOG_GLOB, LOG_ALL, "failed to allocate memory.\n", ON);
		return;
	    }
	}

	peer[pn].event = BGP_NONE;
	peer[pn].status = IDLE;
	notify_peer_status_change(pn);

	sprintf(logstr, "Clearing Peer %d\n", pn);
	pr_log(VLOG_GLOB, pn, logstr, ON);

#ifdef RADIX
	sprintf(logstr, "Clearing Radix Tree for Peer\n");
	pr_log(VLOG_GLOB, pn, logstr, ON);
	withdraw_peer(pn);
#endif
}

int check_fsm(int peer_num)
{
	int			ret;
	struct sockaddr_in	sin;
	struct timeval		timer_value;
	int			ec;
	char			*ptr;
	int			n;

	ret = 0;

#ifdef DEBUG
	if ((st_ev[peer_num].status != peer[peer_num].status) ||
	    (st_ev[peer_num].event  != peer[peer_num].event)) {
	printf("EVENT VIEW: Peer No = %d / BGP STATE = %d / BGP EVENT = %d\n",
			peer_num, peer[peer_num].status, peer[peer_num].event);
	}
	st_ev[peer_num].status = peer[peer_num].status;
	st_ev[peer_num].event  = peer[peer_num].event;
#endif

	switch(peer[peer_num].status) {
	case IDLE:
#ifdef DEBUG
		printf("Processing IDLE\n");
#endif
		switch(peer[peer_num].event) {
		case BGP_NONE:
			break;
		case BGP_START:
			/**** Initialize resouces ****/

			if (peer[peer_num].resume == OFF) {
				/**** Start Connect Retry Timer ****/
#ifdef DEBUG
				printf("DEBUG: Set a Connect Retry Timer\n");
#endif
				timer_value.tv_sec = CNT_RETRY_TIMER;
				timer_value.tv_usec = 0;
				(void)entry_timer(&timer_value,
					peer[peer_num].neighbor,
					ID_CNT_RETRY_TIMER);	/* XXX */

				/**** Initiate a transport connection ****/
				/***** Client Socket Open ***/
#ifdef DEBUG
				printf("DEBUG: Open Client Socket\n");
#endif
				if ((peer[peer_num].soc =
				    socket(AF_INET, SOCK_STREAM, 0)) < 0) {
					perror("client: socket");
					peer[peer_num].soc = -1;
					ret = 3; /* Socket Error */
					break;
				}
				sin.sin_family      = AF_INET;
				sin.sin_addr.s_addr = peer[peer_num].neighbor;
				sin.sin_port	    = htons(BGP_PORT);
#ifdef DEBUG
				printf("DEBUG: Connecting Socket\n");
#endif

#ifdef TCP_MD5SIG
				/* Set TCP Signature */
				md5passwd_set(peer_num);
#endif
				/* Socket is set non-blocking */
				peer[peer_num].flag = 
					fcntl(peer[peer_num].soc, F_GETFL, 0);
				fcntl(peer[peer_num].soc, F_SETFL, 
				      peer[peer_num].flag | O_NONBLOCK );
	
				errno = 0;
				if ((n = connect(peer[peer_num].soc, 
				  (struct sockaddr *)&sin, sizeof(sin))) < 0) {
					  if (errno != EINPROGRESS) {
						peer[peer_num].event = 
							BGP_TRANS_CNT_OPEN_FAIL;
						peer[peer_num].status = CONNECT;
					} else {
						peer[peer_num].event = BGP_NONE;
						peer[peer_num].status = CONNECT;
					}
				} else {
				FD_SET(peer[peer_num].soc, &readfds);
					peer[peer_num].event = BGP_NONE;
					peer[peer_num].status = CONNECT;
				}
#ifdef DEBUG
				printf("DEBUG: Connect Result : %d\n",n);
				printf("DEBUG: Connecting Done.\n");
#endif
			}
			break;
		default:
			if (peer[peer_num].peerstatus == false) {
				peer[peer_num].event = BGP_NONE;
				peer[peer_num].status = IDLE;
			} else {
				peer[peer_num].event = BGP_START;
				peer[peer_num].status = IDLE;
				notify_peer_status_change(peer_num);
			}
		}
		break;
	case CONNECT:
#ifdef DEBUG
		printf("Processing CONNECT\n");
#endif
		switch(peer[peer_num].event) {
		case BGP_NONE:
			break;
		case BGP_START:
			/* NON PROCESS */
			break;
		case BGP_TRANS_CNT_OPEN:
			/* Complete initialization */

			/* Clear Connect Retry Timer */
			off_timer(peer[peer_num].neighbor, ID_CNT_RETRY_TIMER);

			/* OPEN Message sent */
			sendopen(peer_num);
			peer[peer_num].event = BGP_NONE;
			peer[peer_num].status = OPENSENT;
			break;
		case BGP_TRANS_CNT_OPEN_FAIL:
			/* Restart Connect Retry Timer */
			off_timer(peer[peer_num].neighbor, ID_CNT_RETRY_TIMER);
			timer_value.tv_sec = CNT_RETRY_TIMER;
			timer_value.tv_usec = 0;
			(void)entry_timer(&timer_value, peer[peer_num].neighbor,
					ID_CNT_RETRY_TIMER);	/* XXX */
			clear_peer(peer_num);
			peer[peer_num].status = ACTIVE;
			peer[peer_num].event = BGP_NONE;
			break;
		case CNT_RETRY_TIME_EXP:
			/* Restart Connect Retry Timer */
			off_timer(peer[peer_num].neighbor, ID_CNT_RETRY_TIMER);
			timer_value.tv_sec = CNT_RETRY_TIMER;
			timer_value.tv_usec = 0;
			(void)entry_timer(&timer_value, peer[peer_num].neighbor,
					ID_CNT_RETRY_TIMER);	/* XXX */

			/* Initiate a transport connection */
			/***** Client Socket Open ***/
#ifdef DEBUG
			printf("DEBUG: Open Client Socket\n");
#endif
			if ((peer[peer_num].soc = 
					socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				peer[peer_num].soc = -1;
				perror("client: socket");
				ret = 3;
				break;
			}
			sin.sin_family	    = AF_INET;
			sin.sin_addr.s_addr = peer[peer_num].neighbor;
			sin.sin_port	    = htons(BGP_PORT);
			if (connect(peer[peer_num].soc, (struct sockaddr *)&sin,
				    sizeof(sin)) < 0) {
				peer[peer_num].event = BGP_TRANS_CNT_OPEN_FAIL;
			} else {
				FD_SET(peer[peer_num].soc, &readfds);
				peer[peer_num].event = BGP_TRANS_CNT_OPEN;
			}
			break;
		default:
			clear_peer(peer_num);
			break;
		}
		break;
	case ACTIVE:
#ifdef DEBUG
		printf("Processing ACTIVE\n");
#endif
		switch(peer[peer_num].event) {
		case BGP_NONE:
			break;
		case BGP_START:
			/* NON PROCESS */
			break;
		case BGP_TRANS_CNT_OPEN:
			/* Complete Initialization */
			/* Clear Connect Retry Timer */
			off_timer(peer[peer_num].neighbor, ID_CNT_RETRY_TIMER);

			/* OPEN Message sent */
			sendopen(peer_num);

			/* Start Hole Timer */
			timer_value.tv_sec = 240;
			timer_value.tv_usec = 0;
			(void)entry_timer(&timer_value, peer[peer_num].neighbor,
						ID_HOLD_TIMER);	/* XXX */

			peer[peer_num].event = BGP_NONE;
			peer[peer_num].status = OPENSENT;
			break;
		case BGP_TRANS_CNT_OPEN_FAIL:
			/* Close Connection */
			clear_peer(peer_num);

			/* Restart Connect Retry Timer */
			off_timer(peer[peer_num].neighbor, ID_CNT_RETRY_TIMER);
			timer_value.tv_sec = CNT_RETRY_TIMER;
			timer_value.tv_usec = 0;
			(void)entry_timer(&timer_value, peer[peer_num].neighbor,
					ID_CNT_RETRY_TIMER);	/* XXX */
			peer[peer_num].event = BGP_NONE;
			peer[peer_num].status = ACTIVE;
			break;
		case CNT_RETRY_TIME_EXP:
			/* Restart Connect Retry Timer */
			off_timer(peer[peer_num].neighbor, ID_CNT_RETRY_TIMER);
			timer_value.tv_sec = CNT_RETRY_TIMER;
			timer_value.tv_usec = 0;
			(void)entry_timer(&timer_value, peer[peer_num].neighbor,
					ID_CNT_RETRY_TIMER);	/* XXX */

			/* Initiate a transport connection */
			/***** Client Socket Open ***/
#ifdef DEBUG
			printf("DEBUG: Open Client Socket\n");
#endif
			if ((peer[peer_num].soc =
					socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				perror("clinet: socket");
				peer[peer_num].soc = -1;
				ret = 3;
				break;
			}
			sin.sin_family	    = AF_INET;
			sin.sin_addr.s_addr = peer[peer_num].neighbor;
			sin.sin_port	    = htons(BGP_PORT);
			if (connect(peer[peer_num].soc,
				(struct sockaddr *)&sin, sizeof(sin)) < 0) {

				peer[peer_num].event = BGP_TRANS_CNT_OPEN_FAIL;
			} else {
				FD_SET(peer[peer_num].soc, &readfds);
				peer[peer_num].event = BGP_TRANS_CNT_OPEN;
			}
			peer[peer_num].status = CONNECT;
			break;
		default:
			clear_peer(peer_num);
			break;
		}
		break;
	case OPENSENT:
#ifdef DEBUG
		printf("Processing OPENSENT\n");
#endif
		switch(peer[peer_num].event) {
		case BGP_NONE:
			break;
		case BGP_START:
			/* NON PROCESS */
			break;
		case BGP_TRANS_CNT_CLOSE:
			/* Close transport connection */
			clear_peer(peer_num);

			/* Restart Connect Retry Timer */
			off_timer(peer[peer_num].neighbor, ID_CNT_RETRY_TIMER);
			timer_value.tv_sec = CNT_RETRY_TIMER;
			timer_value.tv_usec = 0;
			(void)entry_timer(&timer_value, peer[peer_num].neighbor,
					ID_CNT_RETRY_TIMER);	/* XXX */
			peer[peer_num].event = BGP_NONE;
			peer[peer_num].status = ACTIVE;
			break;
		case BGP_TRANS_FATAL_ERR:
			/* Release Resouces */
			clear_peer(peer_num);
			break;
		case RECV_OPEN:
			/* Process OPEN */
			ec = openerrchk(peer_num);
			if (ec == 0) {
				/* Process OPEN is OK */

				/* KEEPALIVE Message sent */
				sendkeepalive(peer_num);

				ptr = peer[peer_num].data;
				/* Get Hold Timer Value */
				peer[peer_num].holdtimer = buf2ushort(ptr+3);
				if (peer[peer_num].holdtimer != 0) {
					peer[peer_num].keepalive = 
						(int)(peer[peer_num].holdtimer/3);
				}

				/* Timer Set */
				if (peer[peer_num].holdtimer > 0) {
#ifdef DEBUG
					printf("TIMER Setting:\n");
#endif
					/* Hold Timer Set */
					timer_value.tv_sec = 
						peer[peer_num].holdtimer;
					timer_value.tv_usec = 0;
					(void)entry_timer(&timer_value,
						peer[peer_num].neighbor,
						ID_HOLD_TIMER);	/* XXX */

					/* Keep Alive Timer Set */
					off_timer(peer[peer_num].neighbor,
							ID_KEEPALIVE_TIMER);
					timer_value.tv_sec = 
						peer[peer_num].keepalive;
					timer_value.tv_usec = 0;
					(void)entry_timer(&timer_value,
						peer[peer_num].neighbor,
						ID_KEEPALIVE_TIMER); /* XXX */
				} 
#ifdef DEBUG
				else {
					printf("TIMER Not Set(Hold Time 0):\n");
				}
#endif
				/* Status Config */
				peer[peer_num].event = BGP_NONE;
				peer[peer_num].status = OPENCONFIRM;
			} else {
				/* Process OPEN failed */

				/* NOTIFICATION Message sent */
				sendnotification(peer_num, 
							OPEN_MESSAGE_ERROR,ec);

				clear_peer(peer_num);
			}
			break;
		default:
			clear_peer(peer_num);
			break;
		}
		break;
	case OPENCONFIRM:
#ifdef DEBUG
		printf("Processing OPENCONFIRM\n");
#endif
		switch(peer[peer_num].event) {
		case BGP_NONE:
			break;
		case BGP_START:
			/* NO PROCESS */
			break;
		case BGP_TRANS_CNT_CLOSE:
			/* Release resouces */
			clear_peer(peer_num);
			break;
		case BGP_TRANS_FATAL_ERR:
			/* Release resouces */
			clear_peer(peer_num);
			break;
		case KEEPALIVE_TIME_EXP:
			/* Restart Keep Alive Timer */
			off_timer(peer[peer_num].neighbor, ID_KEEPALIVE_TIMER);
			timer_value.tv_sec = peer[peer_num].keepalive;
			timer_value.tv_usec = 0;
			(void)entry_timer(&timer_value, peer[peer_num].neighbor,
					ID_KEEPALIVE_TIMER);	/* XXX */

			/* KEEPALIVE Message sent */
			sendkeepalive(peer_num);

			peer[peer_num].event = BGP_NONE;
			break;
		case RECV_KEEPALIVE:
			/* Complete initialization */
			/* Restart Hold Timer */
			off_timer(peer[peer_num].neighbor, ID_HOLD_TIMER);
			timer_value.tv_sec = peer[peer_num].holdtimer;
			timer_value.tv_usec = 0;
			(void)entry_timer(&timer_value, peer[peer_num].neighbor,
					ID_HOLD_TIMER);		/* XXX */

			peer[peer_num].status = ESTABLISHED;
			peer[peer_num].event = BGP_NONE;
			notify_peer_status_change(peer_num);
			break;
		case RECV_NOTIFICATION:
			/* Close transport connection */
			/* Release resouces */
			clear_peer(peer_num);
			break;
		default:
			/* Close Transport connection */
			/* Release resouces */
			/* NOTIFICATION Message sent */
			clear_peer(peer_num);
			break;
		}
		break;
	case ESTABLISHED:
#ifdef DEBUG
		printf("Processing ESTABLISHED\n");
#endif
		switch(peer[peer_num].event) {
		case BGP_NONE:
			break;
		case BGP_START:
			/* NO PROCESS */
			break;
		case BGP_TRANS_CNT_CLOSE:
			/* Release Resouces */
			clear_peer(peer_num);
			break;
		case BGP_TRANS_FATAL_ERR:
			/* Release resouces */
			clear_peer(peer_num);
			break;
		case KEEPALIVE_TIME_EXP:
			/* Restart Keep Alive Timer */
			off_timer(peer[peer_num].neighbor, ID_KEEPALIVE_TIMER);
			timer_value.tv_sec = peer[peer_num].keepalive;
			timer_value.tv_usec = 0;
			(void)entry_timer(&timer_value, peer[peer_num].neighbor,
					ID_KEEPALIVE_TIMER);	/* XXX */

			/* KEEPALIVE Message sent */
			sendkeepalive(peer_num);

			peer[peer_num].event = BGP_NONE;
			break;
		case HOLD_TIME_EXP:
			/**** Hold Timer Expired then Clear Peer ***/
			/* NOTIFICATION Message sent */
			sendnotification(peer_num, HOLD_TIME_EXPIERD, 0);
			clear_peer(peer_num);

			/************* BUG!! Delete *************
			timer_value.tv_sec = peer[peer_num].holdtimer - 2;
			timer_value.tv_usec = 0;
			(void)entry_timer(&timer_value, peer[peer_num].neighbor,
					ID_HOLD_TIMER);		/* XXX

			/* KEEPALIVE Message sent 
			sendkeepalive(peer_num);
			peer[peer_num].event = BGP_NONE;
			*********************************************/
			break;
		case RECV_KEEPALIVE:
			/* Restart Hold timer */
			off_timer(peer[peer_num].neighbor, ID_HOLD_TIMER);
			timer_value.tv_sec = peer[peer_num].holdtimer - 2;
			timer_value.tv_usec = 0;
			(void)entry_timer(&timer_value, peer[peer_num].neighbor,
					ID_HOLD_TIMER);		/* XXX */

			/* KEEPALIVE Message sent */
			sendkeepalive(peer_num);
			peer[peer_num].event = BGP_NONE;
			break;
		case RECV_UPDATE:
			/* Restart Hold timer */
			off_timer(peer[peer_num].neighbor, ID_HOLD_TIMER);
			timer_value.tv_sec = peer[peer_num].holdtimer - 2;
			timer_value.tv_usec = 0;
			(void)entry_timer(&timer_value, peer[peer_num].neighbor,
					ID_HOLD_TIMER);		/* XXX */

			/* Process UPDATE */
			if (1) {
				/* Process UPDATE is OK */
				pr_update(peer_num);

				/* UPDATE Message sent */
				peer[peer_num].event = BGP_NONE;
			} else {
				/* Process UPDATE fail */

				/* NOTIFICATION Message sent */
				peer[peer_num].event = BGP_START;
				peer[peer_num].status = IDLE;
				notify_peer_status_change(peer_num);
			}
			break;
		case RECV_NOTIFICATION:
			/* Close transport connection */
			clear_peer(peer_num);

			/* Release resouces */
			break;
		default:
#ifdef DEBUG
			printf("ESTABLISH State event default\n");
#endif
			/* Close Transport connection */
			clear_peer(peer_num);
#ifdef DEBUG
			printf("Peer Cleared\n");
#endif
			/* Release resouces */
			/* NOTIFICATION Message sent */
			break;
		}
		break;
	default:
		clear_peer(peer_num);
		break;
	}
#ifdef DEBUG
	printf("FSM Return Value = %d\n", ret);
#endif
	return ret;
}
