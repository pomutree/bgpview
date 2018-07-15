/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1999/01                         */
/*                                                              */
/* Dummy Route Sender                                           */
/****************************************************************/
/* dummyroute.c,v 1.1 2003/03/31 05:51:04 kuniaki Exp */

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

#include "dummyroute.h"

DUMMYINFO 	dmyroute;
char		fixedpacket[BGP_MAX_PACKET_SIZE];
int		prefixpnt;
char		dmyroutef[256];
char		*asincptr1, *asincptr2;
char		*as4incptr1, *as4incptr2;
unsigned short		asinc1, asinc2;

extern int sockwrite(/*soc, buf, len*/);	/* in 'bgpview.c' */

#ifdef TESTDMY
#define DMY_IDENT	0x10101010
#endif
/*=-=-=-=-=-=-=-=-=--=-=-=-=-=-=-=-=-=-=-=-=-=*/

/*------------------------------
 Return Values:
 	0: Normal End
 	1: Not Found Peer
---------------------------------*/
int set_bgp_update_prev()
{
	int	cnt;
	int	attrcnt;
	int	asnums;
	u_long	aspathdiv[1024];
	u_short	astmp;
	char	*asptr;
	char	path[PATHSETSIZE];
	char	*pktptr;
	u_char	needas4path;

#ifndef TESTDMY
	/* Find Peer */
	for(cnt = 0; cnt < MAXPEERS && peer[cnt].neighbor != 0; cnt++) {
		if (peer[cnt].neighbor == dmyroute.neighbor) {
			dmyroute.pn = cnt;
#ifdef DEBUG
			printf("DMYCFG: Peer is [%d].\n",cnt);
#endif
			break;
		}
	}
	if (cnt >= MAXPEERS) {
		printf("Not Found Peer\n");
		return 1;
	}
#endif

	/*-=-=- make Header -=-=-*/
	pktptr = &fixedpacket[0];
	bzero(pktptr, BGP_MAX_PACKET_SIZE);
	memset(pktptr, 0xFF, 16);
	*(pktptr+18) = BGP_UPDATE;
	if (dmyroute.type == DMY_UPDATE) {
		store_int16(pktptr+19, 0);
		/* Set Arrtibute */
		attrcnt = 0;
		/* ORIGIN IGP */
		*(pktptr + 23 + attrcnt + 0) = 0x40;
		*(pktptr + 23 + attrcnt + 1) = 0x01;
		*(pktptr + 23 + attrcnt + 2) = 0x01;
		*(pktptr + 23 + attrcnt + 3) = 0x00;
		attrcnt += 4;
	
		/* AS Path (AS_SEQUENCE) */
		strcpy(path, dmyroute.aspath);
#ifdef DEBUG
		printf("DEBUG: DMYRT: Sending AS Path = %s\n", path);
#endif
		cnt = 0;
		asptr = strtok(path, "_");
		while(asptr != NULL) {
			sscanf(asptr, "%lu", &aspathdiv[cnt]); 
			/* aspathdiv[cnt] = fourbyteastoulong(asptr); */
			/* sscanf(asptr, "%hu", &aspathdiv[cnt]); */
			cnt++;
			asptr = strtok(NULL, "_");
			if (cnt >= 125) break;
		}
		aspathdiv[cnt] = 0;
		*(pktptr + 23 + attrcnt + 0) = 0x40;
		*(pktptr + 23 + attrcnt + 1) = 0x02;
		asnums = cnt;

		/* SET AS_PATH AS Length */
		if ((peer[dmyroute.pn].as4_enable) && 
		    (peer[dmyroute.pn].as4_remote)) {
			astmp = 4;
		} else {
			astmp = 2;
		}

		if (dmyroute.astype == NORMAL) {
			*(pktptr + 23 + attrcnt + 2) = cnt*astmp+2;
		} else {
			*(pktptr + 23 + attrcnt + 2) = (cnt+2)*astmp+2;
		}
		*(pktptr + 23 + attrcnt + 3) = 0x02; /* AS_SEQUENCE */
		if (dmyroute.astype == NORMAL) {
			*(pktptr + 23 + attrcnt + 4) = cnt;
							/* Number of ASes */
		} else {
			*(pktptr + 23 + attrcnt + 4) = cnt + 2;
		}
			
		cnt = 0;
		needas4path = 0;
		attrcnt += 5;
		while(aspathdiv[cnt] != 0) {
			if (peer[dmyroute.pn].as4_enable) {
				if (peer[dmyroute.pn].as4_remote) {
					store_int32(pktptr+23+attrcnt, 
								aspathdiv[cnt]);
					attrcnt += 4;
				} else {
					if (aspathdiv[cnt] > 65535) {
						astmp = AS_TRANS;
					} else {
						astmp = (u_short)aspathdiv[cnt];
					}
					store_int16(pktptr+23+attrcnt, astmp);
					attrcnt += 2;
					/*  Add AS4_PATH Field */
					needas4path = 1;
				}
			} else {
				if (aspathdiv[cnt] > 65535) {
					astmp = AS_TRANS;
				} else {
					astmp = (u_short)aspathdiv[cnt];
				}
				store_int16(pktptr+23+attrcnt, astmp);
				attrcnt += 2;
			}
			cnt++;
		}

		/*** INCREMENTAL AS Mode ***/
		/*** Initial AS Sequence ***/
		if (dmyroute.astype == INCREMENTAL) {
			if (peer[dmyroute.pn].as4_enable) {
				if (peer[dmyroute.pn].as4_remote) {
					store_int32(pktptr + 23 + attrcnt, 1);
					asincptr1 = pktptr + 23 + attrcnt;
					attrcnt += 4;
					store_int32(pktptr + 23 + attrcnt, 1);
					asincptr2 = pktptr + 23 + attrcnt;
					attrcnt += 4;
				} else {
					store_int16(pktptr + 23 + attrcnt, 1);
					asincptr1 = pktptr + 23 + attrcnt;
					attrcnt += 2;
					store_int16(pktptr + 23 + attrcnt, 1);
					asincptr2 = pktptr + 23 + attrcnt;
					attrcnt += 2;
				}
			} else {
				store_int16(pktptr + 23 + attrcnt, 1);
				asincptr1 = pktptr + 23 + attrcnt;
				attrcnt += 2;
				store_int16(pktptr + 23 + attrcnt, 1);
				asincptr2 = pktptr + 23 + attrcnt;
				attrcnt += 2;
			}
			asinc1 = 1;
			asinc2 = 1;
		}

		/* AS4_PATH */
		if (needas4path) {
			*(pktptr + 23 + attrcnt + 0) = 0xE0;
			*(pktptr + 23 + attrcnt + 1) = 0x11;
			
			if (dmyroute.astype == NORMAL) {
				*(pktptr + 23 + attrcnt + 2) = asnums*4+2;
			} else {
				*(pktptr + 23 + attrcnt + 2) = (asnums+2)*4+2;
			}
			*(pktptr + 23 + attrcnt + 3) = 0x02; /* AS_SEQUENCE */
			if (dmyroute.astype == NORMAL) {
				*(pktptr + 23 + attrcnt + 4) = asnums;
							/* Number of ASes */
			} else {
				*(pktptr + 23 + attrcnt + 4) = asnums + 2;
			}
		
			cnt = 0;
			attrcnt += 5;
			while(aspathdiv[cnt] != 0) {
				store_int32(pktptr+23+attrcnt, aspathdiv[cnt]);
				attrcnt += 4;
				cnt++;
			}

			/*** INCREMENTAL AS Mode ***/
			/*** Initial AS Sequence ***/
			if (dmyroute.astype == INCREMENTAL) {
				store_int32(pktptr + 23 + attrcnt, 1);
				as4incptr1 = pktptr + 23 + attrcnt;
				attrcnt += 4;
				store_int32(pktptr + 23 + attrcnt, 1);
				as4incptr2 = pktptr + 23 + attrcnt;
				attrcnt += 4;
			}
		}
	
		/* NEXT_HOP */
		*(pktptr + 23 + attrcnt + 0) = 0x40;
		*(pktptr + 23 + attrcnt + 1) = 0x03;
		*(pktptr + 23 + attrcnt + 2) = 0x04;
		store_int32(pktptr + 23 + attrcnt + 3,
					ntohl(peer[dmyroute.pn].local_id));
		attrcnt += 7;
		
		/* Multi_Exit_Disc */
		*(pktptr + 23 + attrcnt + 0) = 0x80;
		*(pktptr + 23 + attrcnt + 1) = 0x04;
		*(pktptr + 23 + attrcnt + 2) = 0x04;
		store_int32(pktptr + 23 + attrcnt + 3, 0);
		attrcnt += 7;
	
		/* Total Path Attribute Count */
		store_int16(pktptr + 21, attrcnt);
	
		prefixpnt = attrcnt + 23;
	} else {
		prefixpnt = 21;
	}

	/* Make Start Prefix */
	dmyroute.start = dmyroute.start & 0xFFFFFF00;
	
	return 0;
}

void sigexec_getdmyinfo()
{
	int ret;

	ret = getdmyinfo();
	switch(ret) {
	case 1:
		printf("Data File Could Not read\r\n");
		break;
	case 2:
		printf("Target Peer is not Established\r\n");
		break;
	case 3:
		printf("Now Sending\r\n");
		break;
	case 4:
		printf("Dump File Not Configured\r\n");
		break;
	case 5:
		printf("Dump File Could Not Opened\r\n");
		break;
	}
}

/*------------------------------
 Return Values:
	0: Normal End(Read Succeeded)
	1: Data File Could Not read
	2: Target Peer is not Established
	3: Now Actioning
	4: Dump File Not Configured
	5: Dump File Could Not Opened
---------------------------------*/
int getdmyinfo()
{
	FILE 		*dfp;
	char 		getline[256];
	char 		*esp,*val;
	int  		cnt;
	struct in_addr	pin;

	/* Clear Struct */
	if (dmyroute.pn != -1) return(3);
	dmyroute.pn		= -1;
	dmyroute.neighbor 	= 0;
	dmyroute.start		= 0;
	dmyroute.cnt		= 0;
	dmyroute.pack		= 0;
	dmyroute.aspath[0]	= '\0';
	dmyroute.current	= 0;
	dmyroute.type		= DMY_UPDATE; /* update */
	dmyroute.astype		= NORMAL;

	/* Dummy Route Information Read */
	if ((dfp = fopen(dmyroutef, "r")) == NULL) return(1);

	while(!feof(dfp)) {
		bzero(getline, 255);
		esp = fgets(getline, 255, dfp);
#ifdef DEBUG
		printf("DEBUG DMMY: %s\n", getline);
#endif
		if (strlen(getline) >= 255) {
			printf("Statement Too Long.\n");
			continue;
		}
		esp = strtok(getline, " \t\r\n");
		if (esp == NULL) continue;
		if (*esp == '\0') continue;
		if (*esp == '#') continue;
		val = strtok(NULL, " \t\r\n");
		if (val == NULL) continue;

		cnt = 0;
		while(*(esp+cnt) != '\0') {
			*(esp+cnt) = toupper(*(esp+cnt));
			cnt++;
		}
		printf("CHECKING COMMAND = %s / VALUE  = %s\n", esp, val);

		if (strcmp("NEIGHBOR", esp) == 0) {
			if (val == NULL) continue;
			inet_aton(val, &pin);
			dmyroute.neighbor = pin.s_addr;
		} else if (strcmp("STARTPREFIX", esp) == 0) {
			if (val == NULL) continue;
			inet_aton(val, &pin);
			dmyroute.start = ntohl(pin.s_addr);
		} else if (strcmp("NUMBEROF", esp) == 0) {
			if (val == NULL) continue;
			sscanf(val, "%lu", &dmyroute.cnt);
		} else if (strcmp("PACK", esp) == 0) {
			if (val == NULL) continue;
			sscanf(val, "%d", &dmyroute.pack);
		} else if (strcmp("ASPATH", esp) == 0) {
			if (val == NULL) continue;
			strcpy(&dmyroute.aspath[0], val);
		} else if (strcmp("DUMPFILE", esp) == 0) {
			if (val == NULL) continue;
			strcpy(&dmyroute.dump[0], val);
		} else if (strcmp("TYPE", esp) == 0) {
			if (val == NULL) continue;
			cnt = 0;
			while(*(val+cnt) != '\0') {
				*(val+cnt) = toupper(*(val+cnt));
				cnt++;
			}
			if (strcmp(val, "PLAYBACK") == 0) {
				dmyroute.type = DMY_PLAYBACK;
			} else if (strcmp(val, "UPDATE") == 0) {
				dmyroute.type = DMY_UPDATE;
			} else if (strcmp(val, "WITHDRAW") == 0) {
				dmyroute.type = DMY_WITHDRAW;
			}
		} else if (strcmp("ASTYPE", esp) == 0) {
			if (val == NULL) continue;
			cnt = 0;
			while(*(val+cnt) != '\0') {
				*(val+cnt) = toupper(*(val+cnt));
				cnt++;
			}
			if (strcmp(val, "NORMAL") == 0) {
				dmyroute.astype = NORMAL;
			} else if (strcmp(val, "INCREMENTAL") == 0) {
				dmyroute.astype = INCREMENTAL;
			}
		}
	}
	fclose(dfp);
	/* Check Configuring */
	set_bgp_update_prev();
	if (peer[dmyroute.pn].status != ESTABLISHED) {
		dmyroute.pn = -1;
		return(2);
	}

	/* dummyroute playback mode, dump file open */
	if (dmyroute.type == DMY_PLAYBACK) {
		if (dmyroute.dump[0] == '\0') {
			dmyroute.pn = -1;
			return(4);
		}
		if ((dmyroute.dumpfp = fopen(dmyroute.dump, "r")) == NULL) {
			dmyroute.pn = -1;
			return(5);
		}
	}
	return(0);
}

int make_sendpacket() {

	int	lcnt;
	short	ptr;
	u_long	setp;
	char	*pktptr;

	pktptr = &fixedpacket[0];
	ptr = prefixpnt;
	lcnt = dmyroute.pack;
	while(lcnt > 0) {
		setp = dmyroute.start + (dmyroute.current * 256);
		setp >>= 8;
		setp |= 0x18000000;
		store_int32(pktptr+ptr, setp);
		ptr += 4;
		lcnt--;
		dmyroute.current++;
		if (dmyroute.current >= dmyroute.cnt) break;
		if (ptr > 4090) break;
	}

	/*** AS INCREMENTAL ***/
	if (dmyroute.astype == INCREMENTAL) {
		if (asinc1 >= 65535) {
			asinc1 = 1;
			if (asinc2 >= 65535) {
				asinc2 = 1;
			} else {
				asinc2 += 1;
			}
		} else {
			asinc1 += 1;
		}
	
		if (peer[dmyroute.pn].as4_enable) {
			if (peer[dmyroute.pn].as4_remote) {
				store_int32(asincptr1, asinc1);
				store_int32(asincptr2, asinc2);
			} else {
				store_int16(asincptr1, asinc1);
				store_int16(asincptr2, asinc2);
				store_int32(as4incptr1, asinc1);
				store_int32(as4incptr2, asinc2);
			}
		} else {
			store_int16(asincptr1, asinc1);
			store_int16(asincptr2, asinc2);
		}	
	}

	if (dmyroute.type == DMY_UPDATE) {
#ifdef DEBUG
		printf("DMYSEND: Packet Fixed Length = %d(UPDATE)\n",prefixpnt);
		printf("Packed Prefix = %d\n", dmyroute.pack-lcnt);
#endif
		store_int16(pktptr+16, ptr);
	} else {
#ifdef DEBUG
		printf("DMYSEND: Packet Fixed Length = Head21Byte + Tail2Byte\n");
		printf("Packed Prefix = %d\n", dmyroute.pack-lcnt);
#endif
		store_int16(pktptr+ptr, 0);
		ptr += 2;
		store_int16(pktptr+16, ptr);
		store_int16(pktptr+19, ptr-23);
	}
	return ptr;
}

make_dumppacket()
{
	char	linebuf[256];
	char	*strptn;
	int	size;
	int	sizecount;
	char	*pktptr;
	u_long	getdump;
	int	cnt;
	int	b1,b2,b3;

	if (dmyroute.dumpfp == NULL) {
		dmyroute.pn = -1;
		return 0;
	}
#ifdef DEBUG
	printf("Makeing Playback Packet...\n");
#endif

	pktptr = &fixedpacket[0];
	sizecount = 0;
	size = 0;
	while(1) {
		fgets(linebuf, 255, dmyroute.dumpfp);
		if (feof(dmyroute.dumpfp)) {
#ifdef DEBUG
			printf("Dump File Close\n");
#endif
			fclose(dmyroute.dumpfp);
			dmyroute.dumpfp = NULL;
			break;
		}
#ifdef DEBUG
		printf("Read Line : %s", linebuf);
#endif
		if (strstr(linebuf, "UPDATE DUMP LEN") == &linebuf[0]) {
			strptn = strtok((char *)(linebuf+19), " \t\r\n");
			size = atoi(strptn);
#ifdef DEBUG
			printf("PlayBackData Found : Data Length = %d Byte\n",
					size);
#endif
		} else if (strstr(linebuf, "UPDATE DUMP DATE") == &linebuf[0]) {
#ifdef DEBUG
			printf("GET Original Update Time: %s\n", linebuf);
#endif
		} else if (strstr(linebuf, "UPDATE DUMP ") == &linebuf[0]) {
			cnt = 0;
			strptn = strtok((char *)(linebuf+19), " \t\r\n");
			while(cnt < 4) {
				if (strptn != NULL) {
				  switch(size - sizecount) {
				  case 1:
				    sscanf(strptn, "%02X", &b1);
				    *(pktptr + sizecount + 19) = b1;
				    sizecount += 1;
				    break;
				  case 2:
				    sscanf(strptn, "%02X%02X", &b1, &b2);
				    *(pktptr + sizecount + 19) = b1;
				    *(pktptr + sizecount + 20) = b2;
				    sizecount += 2;
				    break;
				  case 3:
				    sscanf(strptn, "%02X%02X%02X", &b1,&b2,&b3);
				    *(pktptr + sizecount + 19) = b1;
				    *(pktptr + sizecount + 20) = b2;
				    *(pktptr + sizecount + 21) = b3;
				    sizecount += 3;
				    break;
				  default:
				    sscanf(strptn,"%08X", &getdump);
				    store_int32(pktptr+sizecount+19, getdump);
				    sizecount += 4;
				  }
				  strptn = strtok(NULL, " \t\r\n");
				} else break;
				cnt++;
			}
			if (sizecount >= size) break;
		}
	}
	if (sizecount == 0) return 0;
	store_int16(pktptr+16, sizecount+19);
	*(pktptr+18) = BGP_UPDATE;
	return sizecount + 19;
}

/* Return Value ---
	0: No More Route
	1: More Route
	2: Error
--------------------*/
int send_dummy() {
	int	sendcnt;
	char 	*ptr;
	int	writed;
#ifdef DEBUG
	int	cnt;
	printf("DMYSEND: Current End CHK = %ld / Peer = %d\n", 
				dmyroute.current, dmyroute.pn);
#endif

	if (dmyroute.pn == -1) return 0;

	if (peer[dmyroute.pn].status != ESTABLISHED) {
		printf("Peer[%d] is not ESTABLISHED (but %d)."
			" Could Not Send Update.\n",
			dmyroute.pn, peer[dmyroute.pn].status);
		dmyroute.pn = -1;
		return 2;
	}

	if (dmyroute.type == DMY_PLAYBACK) {
		sendcnt = make_dumppacket();
	} else {
		sendcnt = make_sendpacket();
	}
#ifdef DEBUG
	printf("DMYSEND: Packet Made.. %d Byte\n", sendcnt);
#endif
	if (sendcnt <= 0) {
		if ((dmyroute.type == DMY_PLAYBACK) && 
		    (dmyroute.dumpfp == NULL)) {
			dmyroute.pn = -1;
			return 0;
		}
		return 1;
	}
	ptr = &fixedpacket[0];
#ifdef DEBUG
	printf("DMYSEND: Packet Head Address %08X\n", ptr);
#endif
	store_int16(ptr+16, sendcnt);
#ifdef DEBUG
	printf("DMYSEND: Packet Length Set\n");
#endif

	writed = sockwrite(peer[dmyroute.pn].soc, ptr, sendcnt);

#ifdef DEBUG
	printf("DMYSEND: Sent Packet Message Size = %d\n", writed);
	for(cnt=0; cnt<sendcnt; cnt++) {
		if ((cnt % 16) == 0) {
			printf("\nDMYDUMP: %04d : ", cnt);
		} else if ((cnt % 4) == 0) {
			printf(" ");
		}
		printf("%02X", (u_char)fixedpacket[cnt]);
	}
	printf("\n");
	printf("DMYSEND: Current End = %ld\n", dmyroute.current);
#endif
	if (sendcnt != writed) {
		printf("Update Message Send Fail\n");
		dmyroute.pn = -1;
		return 0;
	}
	if ((dmyroute.current >= dmyroute.cnt) && (dmyroute.type != DMY_PLAYBACK)) {
		dmyroute.pn = -1;
		return 0;
	}
	return 1;
}

void init_dummy() {
	dmyroute.pn		= -1;
	dmyroute.neighbor 	= 0;
	dmyroute.start		= 0;
	dmyroute.cnt		= 0;
	dmyroute.pack		= 0;
	dmyroute.aspath[0]	= '\0';
	dmyroute.current	= 0;
	dmyroute.type		= DMY_UPDATE;
	dmyroute.dump[0]	= '\0';
	dmyroute.dumpfp		= NULL;
	dmyroute.astype		= NORMAL;

	strcpy(dmyroutef, "/usr/local/etc/dummyroute.cfg");
}

#ifdef TESTDMY
net_ulong IDENT;

main()
{
	int a;

	IDENT = htonl(DMY_IDENT);

	dmyroute.pn		= -1;
	dmyroute.neighbor 	= 0;
	dmyroute.start		= 0;
	dmyroute.cnt		= 0;
	dmyroute.pack		= 0;
	dmyroute.aspath[0]	= '\0';
	dmyroute.current	= 0;
	dmyrotue.type		= DMY_UPDATE;
	dmyroute.dump[0]	= '\0';
	dmyroute.dumpfp		= NULL;
	dmyroute.astype		= NORMAL;

	getdmyinfo();

	while(1) {
		a = make_sendpacket();
		printf("Pack Size = %d\n",a);
		if (dmyroute.current >= dmyroute.cnt) {
			dmyroute.pn = -1;
			break;
		}
	}
}
#endif
