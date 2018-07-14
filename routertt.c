/****************************************************************/  
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 2000/03                         */
/*                                                              */
/* Route Update RTT Estimation                                  */
/****************************************************************/
/* routertt.c,v 1.1 2003/03/31 05:51:04 kuniaki Exp */

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

#include "routertt.h"

ROUTERTT routertt[RTTCHECK_MAX];

int init_routertt()
{
	int cnt;

	bzero(routertt, sizeof(ROUTERTT)*RTTCHECK_MAX);

	return(0);
}

int send_update(int rttid)
{
	char	sendpacket[BGP_MAX_PACKET_SIZE];
	int	pktptr;
	int	pn;
	int	setp;
	int	cnt;

	/* find peer */
	pn = check_id1topn(routertt[rttid].neighbor);
	if (pn < 0) {
		routertt[rttid].status = RTR_NOTACTIVE;
		return(1); /* Not find Peer */
	}
	if (peer[pn].status != ESTABLISHED) {
		routertt[rttid].status = RTR_NOTACTIVE;
		return(3); /* Peer Not Established */
	}
	memset(&sendpacket[0], 0xFF, 16);
	sendpacket[18] = BGP_UPDATE;
	if (routertt[rttid].status == RTR_READY) {  /* UPDATE */
		store_int16(&sendpacket[19], 0);

		/* ORIGIN IGP */
		sendpacket[23] = 0x40;
		sendpacket[24] = 0x01;
		sendpacket[25] = 0x01;
		sendpacket[26] = 0x00;

		pktptr = 27;

		/* AS Path (AS_SEQUENCE) */
		sendpacket[pktptr + 0] = 0x40;
		sendpacket[pktptr + 1] = 0x02;
		sendpacket[pktptr + 2] = 0x04;
		sendpacket[pktptr + 3] = 0x02;
		sendpacket[pktptr + 4] = 0x01;
		store_int16(&sendpacket[pktptr+5], (u_short)peer[pn].local_as);
		pktptr += 7;

		/* NEXT_HOP */
		sendpacket[pktptr + 0] = 0x40;
		sendpacket[pktptr + 1] = 0x03;
		sendpacket[pktptr + 2] = 0x04;
		store_int32(&sendpacket[pktptr+3], ntohl(peer[pn].local_id));
		pktptr += 7;

		/* Multi_Exit_Disc */
		sendpacket[pktptr + 0] = 0x80;
		sendpacket[pktptr + 1] = 0x04;
		sendpacket[pktptr + 2] = 0x04;
		store_int32(&sendpacket[pktptr+3], 0);
		pktptr += 7;

		/* Total Path Attribute Count */
		store_int16(&sendpacket[21], pktptr-23);

	} else if (routertt[rttid].status == RTR_RECV_UPD) { /* WITHDRAW */
		pktptr = 21;
	}

	if (routertt[rttid].prefixlen == 0) {
		setp = 2;
	} else {
		setp = ((routertt[rttid].prefixlen-1) / 8) +2;
	}

	sendpacket[pktptr] = routertt[rttid].prefixlen;
	for(cnt=1; cnt<setp; cnt++) {
		sendpacket[pktptr+cnt] = 
			((routertt[rttid].prefix>>((cnt-1)*8))&(0x000000FF));
	}

	pktptr += setp;
	if (routertt[rttid].status == RTR_READY) {
		store_int16(&sendpacket[16], pktptr);
	} else if (routertt[rttid].status == RTR_RECV_UPD) {
		store_int16(&sendpacket[pktptr], 0);
		pktptr += 2;
		store_int16(&sendpacket[16], pktptr);
		store_int16(&sendpacket[19], pktptr-23);
	}

	setp = sockwrite(peer[pn].soc, &sendpacket[0], pktptr);
#ifdef DEBUG
	printf("DEBUG: ROUTERTT: Sent Packet Message Size = %d\n", setp);
	for(cnt=0; cnt<pktptr; cnt++) {
		if ((cnt % 16) == 0) {
			printf("\nDEBUG: ROUTERTT: %04d : ", cnt);
		} else if ((cnt % 4) == 0) {
			printf(" ");
		}
		printf("%02X", (u_char)sendpacket[cnt]);
	}
	printf("\n");
#endif

	if (setp != pktptr) {
		routertt[rttid].status = RTR_NOTACTIVE;
		/* timer off? */
		return(2);
	}

	if (routertt[rttid].status == RTR_READY) {
		gettimeofday(&routertt[rttid].upd_injecttime, 
						(struct timezone *)0);
		routertt[rttid].status = RTR_SENT_UPD;
	} else {
		gettimeofday(&routertt[rttid].wdn_injecttime,
						(struct timezone *)0);
		routertt[rttid].status = RTR_SENT_WDN;
	}

	return(0);
}

int newcheckrtt(int vtyno, net_ulong neighbor, net_ulong prefix,
		int prefixlen, int timeout)
{
	int		cnt;
	struct in_addr  sin;
	char		prline[128];
	struct timeval	tv;
	int		sret;

	cnt = 0;
	while(cnt < RTTCHECK_MAX) {
		if (routertt[cnt].status == RTR_NOTACTIVE) break;
		cnt++;
	}

	bzero(&routertt[cnt], sizeof(ROUTERTT));
	if (cnt < RTTCHECK_MAX) {
		routertt[cnt].vtyno = vtyno;
		routertt[cnt].neighbor = neighbor;
		routertt[cnt].prefix = prefix;
		routertt[cnt].prefixlen = prefixlen;
		routertt[cnt].timeout = timeout;
		routertt[cnt].timer_id = -1;
		routertt[cnt].status = RTR_READY;

		
		bvs_write(vtyno, "Route RTT Check\r\n");
		sin.s_addr = neighbor;
		sprintf(prline, "  Sender      : %s\r\n", inet_ntoa(sin));
		bvs_write(vtyno, prline);
		sin.s_addr = prefix;
		sprintf(prline, "  Send Prefix : %s/%d\r\n", inet_ntoa(sin),
							    prefixlen);
		sret = send_update(cnt);
		switch(sret) {
		case 1:	bvs_write(routertt[cnt].vtyno,
					"  !! Peer Not Found\r\n");
			routertt[cnt].status = RTR_NOTACTIVE;
			return(-2);
			break;
		case 2: bvs_write(routertt[cnt].vtyno,
					"  !! Update packet could not send\r\n");
			routertt[cnt].status = RTR_NOTACTIVE;
			return(-3);
			break;
		case 3: bvs_write(routertt[cnt].vtyno,
					"  !! Peer Not Established\r\n");
			routertt[cnt].status = RTR_NOTACTIVE;
			return(-4);
			break;
		default:
			tv.tv_sec = timeout;
			tv.tv_usec = 0;
			routertt[cnt].timer_id =
				common_entry_timer(&tv, cnt, routerttexpier);
			if (routertt[cnt].timer_id == -1) {
				routertt[cnt].status = RTR_NOTACTIVE;
				bvs_write(routertt[cnt].vtyno,
					"  !! Timer Could Not Set\r\n");
				return(-4);
			}
		}
	} else {
		return(-1);
	}
	return(cnt);
}

void recvprefix(int rttid, int type)
{
	struct timeval	tv;
	char		prline[128];
	int		sret;

	if (type == RTR_RECV_UPD) {
		routertt[rttid].status = RTR_RECV_UPD;
		gettimeofday(&routertt[rttid].upd_receivetime,
						(struct timezone *)0);
		timer_diff(&routertt[rttid].upd_receivetime, 
			   &routertt[rttid].upd_injecttime,
			   &tv);
		sprintf(prline, "  Update RTT  : %ld.%ld sec\r\n",
					tv.tv_sec, (long)(tv.tv_usec/10000));
		bvs_write(routertt[rttid].vtyno, prline);
		common_off_timer(routertt[rttid].timer_id);
		sret = send_update(rttid);
		switch(sret) {
		case 1:	bvs_write(routertt[rttid].vtyno,
					"  !! Peer Not Found\r\n");
			routertt[rttid].status = RTR_NOTACTIVE;
			bvs_write(routertt[rttid].vtyno, bvs_prompt);
			break;
		case 2: bvs_write(routertt[rttid].vtyno,
					"  !! Update packet could not send\r\n");
			routertt[rttid].status = RTR_NOTACTIVE;
			bvs_write(routertt[rttid].vtyno, bvs_prompt);
			break;
		case 3: bvs_write(routertt[rttid].vtyno,
					"  !! Peer Not Established\r\n");
			routertt[rttid].status = RTR_NOTACTIVE;
			bvs_write(routertt[rttid].vtyno, bvs_prompt);
			break;
		default:
			tv.tv_sec = routertt[rttid].timeout;
			tv.tv_usec = 0;
			routertt[rttid].timer_id =
				common_entry_timer(&tv, rttid, routerttexpier);
			if (routertt[rttid].timer_id == -1) {
				routertt[rttid].status = RTR_NOTACTIVE;
				bvs_write(routertt[rttid].vtyno, bvs_prompt);
			}
		}
	} else if (type == RTR_RECV_WDN) {
		routertt[rttid].status = RTR_RECV_WDN;
		gettimeofday(&routertt[rttid].wdn_receivetime,
						(struct timezone *)0);
		timer_diff(&routertt[rttid].wdn_receivetime, 
			   &routertt[rttid].wdn_injecttime,
			   &tv);
		sprintf(prline, "  Withdraw RTT: %ld.%ld sec\r\n",
					tv.tv_sec, (long)(tv.tv_usec/10000));
		bvs_write(routertt[rttid].vtyno, prline);
		common_off_timer(routertt[rttid].timer_id);
		routertt[rttid].status = RTR_NOTACTIVE;
		bvs_write(routertt[rttid].vtyno, bvs_prompt);
	}
}

void routerttexpier(int id, int prm)
{
	int		sret;
	struct timeval	tv;

	if (routertt[prm].status == RTR_SENT_UPD) {
		bvs_write(routertt[prm].vtyno, "  !! Update Expired\r\n");
		routertt[prm].status = RTR_RECV_UPD;
		sret = send_update(prm);
		switch(sret) {
		case 1:	bvs_write(routertt[prm].vtyno,
					"  !! Peer Not Found\r\n");
			routertt[prm].status = RTR_NOTACTIVE;
			bvs_write(routertt[prm].vtyno, bvs_prompt);
			return;
			break;
		case 2: bvs_write(routertt[prm].vtyno,
					"  !! Update packet could not send\r\n");
			routertt[prm].status = RTR_NOTACTIVE;
			bvs_write(routertt[prm].vtyno, bvs_prompt);
			return;
			break;
		case 3: bvs_write(routertt[prm].vtyno,
					"  !! Peer Not Established\r\n");
			routertt[prm].status = RTR_NOTACTIVE;
			bvs_write(routertt[prm].vtyno, bvs_prompt);
			return;
			break;
		default:
			tv.tv_sec = routertt[prm].timeout;
			tv.tv_usec = 0;
			routertt[prm].timer_id =
				common_entry_timer(&tv, prm, routerttexpier);
			if (routertt[prm].timer_id == -1) {
				routertt[prm].status = RTR_NOTACTIVE;
				bvs_write(routertt[prm].vtyno, bvs_prompt);
				return;
			}
		}
	} else if (routertt[prm].status == RTR_SENT_WDN) {
		bvs_write(routertt[prm].vtyno, "  !! Withdraw Expired\r\n");
		routertt[prm].status = RTR_NOTACTIVE;
		bvs_write(routertt[prm].vtyno, bvs_prompt);
	} else {
		bvs_write(routertt[prm].vtyno, "  !! Illegal Expire\r\n");
		routertt[prm].status = RTR_NOTACTIVE;
		bvs_write(routertt[prm].vtyno, bvs_prompt);
	}
}
