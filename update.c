/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1997/12                         */
/*                                                              */
/* Routeing Information Update Routine                          */
/****************************************************************/
/* update.c,v 1.2 2004/07/13 08:58:44 kuniaki Exp */

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

#include "bgp.h"
#include "bgpdata.h"
#include "bgplog.h"
#include "routertt.h"
#include "timer.h"
#include "mpnlri.h"
#include "route_ipv6.h"

#ifdef RADIX
#include "radix.h"
#include "radix_v6.h"
#endif

void message_dump(int pn)
{
	int 		length;
	char 		*ptn, *pt;
	int 		cnt;
	time_t		clock;
	struct tm	*tm;
	char		buf[32];

	length = peer[pn].datalen;
	ptn = peer[pn].data;

	pt = ptn + length;
	cnt = 0;
	sprintf(logstr, "UPDATE DUMP LEN  : %d Byte\n", length);
        pr_log(VLOG_UPDU, pn, logstr, OFF);
	time(&clock);
	tm = localtime(&clock);
	strftime(buf, 30, "%Y/%m/%d %H:%M:%S", tm);
	sprintf(logstr, "UPDATE DUMP DATE : %s\n", buf);
        pr_log(VLOG_UPDU, pn, logstr, OFF);
        pr_log(VLOG_UPDU, pn, "UPDATE DUMP 0000 : ", OFF);
	while(ptn < pt){
		sprintf(logstr, "%08X ", buf2ulong(ptn));
		pr_log(VLOG_UPDU, pn, logstr, OFF);
		ptn += 4;
		cnt ++;
		if ((cnt % 4) == 0) {
			sprintf(logstr, "\nUPDATE DUMP %04d : ", cnt*4);
			pr_log(VLOG_UPDU, pn, logstr, OFF);
		}
	}
	pr_log(VLOG_UPDU, pn, "\n", OFF);
	return;
}

void pr_route(ROUTE_INFO *routep, int pn)
{
	struct in_addr	inaddr;
	char		lbuf[1024], lbuf2[1024];
	struct timeval	now;
	long		timediff;
	int		cnt;

	gettimeofday(&now, (struct timezone *)0);
	timediff = now.tv_sec - routep->lastupdate.tv_sec;

	inaddr.s_addr = routep->prefix;
	sprintf(lbuf2, "%s/%d ", inet_ntoa(inaddr), routep->length);
	sprintf(lbuf, "   %-17s", lbuf2);
	
	/******************
		Route RTT Check for Update Here
					************************/
	for(cnt=0; cnt<RTTCHECK_MAX; cnt++) {
/*
#ifdef DEBUG
		printf("DEBUG: UPDATERTT: Comparing Prefix .....\n");
		printf("DEBUG: UPDATERTT:   %d: %d/%d : %ld/%ld : %d/%d\n",
			cnt,
			routertt[cnt].status, RTR_SENT_UPD,
			routertt[cnt].prefix, inaddr.s_addr,
			routertt[cnt].prefixlen, routep->length);
#endif
*/
		if ((routertt[cnt].status == RTR_SENT_UPD) &&
		    (routertt[cnt].prefix == inaddr.s_addr) &&
		    (routertt[cnt].prefixlen == routep->length)) {
			recvprefix(cnt, RTR_RECV_UPD);
			break;
		}
	}

	if (ATRFL_ISSET(routep->attribute_flag, SET_NEXTHOP)) {
		inaddr.s_addr = routep->next_hop;
		sprintf(lbuf2, "%-17s", inet_ntoa(inaddr));
	} else {
		strcpy(lbuf2, "                 ");
	}
	strcat(lbuf, lbuf2);
	if (ATRFL_ISSET(routep->attribute_flag, SET_METRIC)) {
		sprintf(lbuf2, "%7d", routep->metric);
	} else {
		strcpy(lbuf2, "       ");
	}
	strcat(lbuf, lbuf2);
	if (ATRFL_ISSET(routep->attribute_flag, SET_LOCALPREF)) {
		sprintf(lbuf2, "%7d", routep->local_perf);
	} else {
		strcpy(lbuf2, "       ");
	}
	strcat(lbuf, lbuf2);
	sprintf(lbuf2, "%7d ", timediff);
	strcat(lbuf, lbuf2);
	/* strcat(lbuf, routep->path_set); */
	if (routep->pathset_ptr != NULL) {
		strcat(lbuf, routep->pathset_ptr);
	}
	if (ATRFL_ISSET(routep->attribute_flag, SET_ORIGIN)) {
		switch(routep->origin_type) {
		case ORIGIN_IGP:
			strcat(lbuf, "i\n");
			break;
		case ORIGIN_EGP:
			strcat(lbuf, "e\n");
			break;
		case ORIGIN_INCOMPLETE:
			strcat(lbuf, "?\n");
			break;
		}
	}
	pr_log(VLOG_UPDE, pn, lbuf);
}

void pr_v6route(ROUTE_INFO_V6 *routep, int pn)
{
	struct in_addr	inaddr;
	char		v6addr[IPV6_ADDR_LEN];
	char		lbuf[1024], lbuf2[1024], lbuf3[1024];
	struct timeval	now;
	long		timediff;
	int		cnt;

	gettimeofday(&now, (struct timezone *)0);
	timediff = now.tv_sec - routep->lastupdate.tv_sec;

	bintostrv6(routep->prefix, lbuf3);
	sprintf(lbuf2, "%s/%d ", lbuf3, routep->length);
	sprintf(lbuf, "   %-17s", lbuf2);
	
	if (ATRFL_ISSET(routep->attribute_flag, SET_NEXTHOP)) {
		if (routep->nexthop_afi == AF_IPV4) {
			if ((inaddr.s_addr = *(net_ulong *)routep->next_hop)==0){
				bintostrv6(routep->next_hop, lbuf3);
				sprintf(lbuf2, "%-17s", lbuf3);
			} else {
				sprintf(lbuf2, "%-17s", inet_ntoa(inaddr));
			}
		} else if (routep->nexthop_afi == AF_IPV6) {
			bintostrv6(routep->next_hop, lbuf3);
			sprintf(lbuf2, "%-17s", lbuf3);
		} else {
			strcpy(lbuf2, "                 ");
		}
	} else {
		strcpy(lbuf2, "                 ");
	}
	strcat(lbuf, lbuf2);
	if (ATRFL_ISSET(routep->attribute_flag, SET_METRIC)) {
		sprintf(lbuf2, "%7d", routep->metric);
	} else {
		strcpy(lbuf2, "       ");
	}
	strcat(lbuf, lbuf2);
	if (ATRFL_ISSET(routep->attribute_flag, SET_LOCALPREF)) {
		sprintf(lbuf2, "%7d", routep->local_perf);
	} else {
		strcpy(lbuf2, "       ");
	}
	strcat(lbuf, lbuf2);
	sprintf(lbuf2, "%7d ", timediff);
	strcat(lbuf, lbuf2);
	/* strcat(lbuf, routep->path_set); */
	if (routep->pathset_ptr != NULL) {
		strcat(lbuf, routep->pathset_ptr);
	}
	if (ATRFL_ISSET(routep->attribute_flag, SET_ORIGIN)) {
		switch(routep->origin_type) {
		case ORIGIN_IGP:
			strcat(lbuf, "i\n");
			break;
		case ORIGIN_EGP:
			strcat(lbuf, "e\n");
			break;
		case ORIGIN_INCOMPLETE:
			strcat(lbuf, "?\n");
			break;
		}
	}
	pr_log(VLOG_UPDE, pn, lbuf);
}

void pr_update(int pn)
{
	unsigned char 	*ptr, *ptr2;
	char		cbuf[128], *cp;
	int		pt2;
	struct in_addr 	inaddr;
	int		length, le1, le2, cnt, ptmp;
	int		attr_len, plen, ret;
	u_int		attr_octs, type_code;
	ROUTE_INFO	getroute, prroute;
	ROUTE_INFO	as4_getroute;
	char		logstr[128];
	prefix_t	outpre;
	char		pathbuf[4096], as4_pathbuf[4096];
	int		pathbuflen, as4_pathbuflen;
	int		ipv6updcnt, ipv6wdrcnt;
	V6ROUTE_BUF	v6update, v6withdraw;
	ROUTE_INFO_V6	v6getroute, v6prroute;
	ROUTE_INFO_V6	as4_v6getroute;
	V6ROUTE_LIST	*listbuf;

	static char	*path_attr[] = {
			/* 0 */ "UNKNOWN",
			/* 1 */ "Origin",
			/* 2 */ "AS_PATH",
			/* 3 */ "NEXT_HOP",
			/* 4 */ "MULTI_EXIT_DISC",
			/* 5 */ "LOCAL_PREF",
			/* 6 */ "ATOMIC_AGGREGATE",
			/* 7 */ "AGGREGATOR",
			/* 8 */ "COMMUNITIES",
			/* 9 */ "ORIGINATOR_ID",
			/* 10 */ "CLUSTER_LIST",
			/* 11 */ "DPA",
			/* 12 */ "ADVERTISER",
			/* 13 */ "RCIDPATH_CLUSTERID",
			/* 14 */ "MP_REACH_NLRI",
			/* 15 */ "MP_UNREACH_NLRI",
			/* 16 */ "EXTENDED_COMMUNITIES",
			/* 17 */ "NEW_AS_PATH",
			/* 18 */ "NEW_AGGREGATOR" };

	ptr = peer[pn].data;

	bzero(&v6update, sizeof(V6ROUTE_BUF));
	bzero(&v6withdraw, sizeof(V6ROUTE_BUF));
	ipv6updcnt = 0;
	ipv6wdrcnt = 0;

	pr_log(VLOG_UPDE, pn, "====== UPDATE Information =====\n", ON);

	message_dump(pn);
	bzero(&getroute, sizeof(ROUTE_INFO));
	bzero(&v6getroute, sizeof(ROUTE_INFO_V6));
	
	length = (int)buf2ushort(ptr);
	le1 = length;
	sprintf(logstr, "Unfeasible Route Length = %d octets\n", length);
	pr_log(VLOG_UPDE, pn, logstr, OFF);

	/* Length Check */
	if ((peer[pn].datalen - 4) < length) {
		/* Unfeasible Route Length Error */
		pr_log(VLOG_UPDE, pn, "!!! Unfeasible Route Length Error.\n", OFF);
		return;
	}

	ptr += 2;
	pt2 = 0;
	if (length == 0) {
		pr_log(VLOG_UPDE, pn,
			"  WITHDRAWN Route is not presented.\n",OFF);
	} else {
		while(pt2 < length) {
			strcpy(logstr, "  WITHDRAWN : ");

			plen = (int)*(ptr+pt2);
			if (plen > 32) {
				pt2++;
				pr_log(VLOG_UPDE, pn, 
					"Prefix too long.(WITHDRAW)\n", OFF);
				continue;
			}

			if (plen <= 0) {
				cnt = 0;
			} else {
				cnt = (int)(((plen-1)/8) + 1);
			}

			inaddr.s_addr = 0;
			memmove((u_char *)&inaddr.s_addr, 
					(u_char *)(ptr+pt2+1), cnt);
			sprintf(logstr, "%s/%d\n", inet_ntoa(inaddr), plen);
			pr_log(VLOG_UPDE, pn, logstr, OFF);
			bgpinfo[pn].withdrawcnt++;
			pt2 += cnt+1;

#ifdef RADIX
			/* Withdraw route from radix tree */
#ifdef RTHISTORY
			withdraw_route(pn, inaddr.s_addr, plen, DFLAG);
#endif
#ifndef RTHISTORY
			withdraw_route(pn, inaddr.s_addr, plen);
#endif
#endif

			/**********
				Route RTT Check for Withdraw Here
							***************/
			for(cnt=0; cnt<RTTCHECK_MAX; cnt++) {
				if ((routertt[cnt].status == RTR_SENT_WDN) &&
		    		(routertt[cnt].prefix == inaddr.s_addr) &&
		    		(routertt[cnt].prefixlen == plen)) {
					recvprefix(cnt, RTR_RECV_WDN);
					break;
				}
			}


			/* for prefix log */
			outpre.prefix = inaddr.s_addr;
			outpre.length = plen;
			write_prefixlog(1, &outpre, pn);
		}
	}
	ptr += length;
	length = (int)buf2ushort(ptr);
	le2 = length;
	sprintf(logstr, "  Total Path Attribute Length = %d octets\n", length);
	pr_log(VLOG_UPDE, pn, logstr, OFF);
	ptr += 2;
	if (length == 0) {
		pr_log(VLOG_UPDE, pn,
			"    Path Attribute is not presented\n",OFF);
	} else {
		ptr2 = ptr + length;
		while(ptr < ptr2){
			type_code = *(char *)(ptr + 1);
			sprintf(logstr, 
		"    Attr. Type: Attr. Flag = %02X / Attr. Type Code = %d\n",
						*(u_char *)ptr, type_code);
			pr_log(VLOG_UPAT, pn, logstr, OFF);

			if (*ptr & 0x80) {
				strcpy(logstr, "    Attribute : OPTIONAL ");
				getroute.attr_type = OPTIONAL;
				v6getroute.attr_type = OPTIONAL;
			} else {
				getroute.attr_type = WELL_KNOWN;
				v6getroute.attr_type = WELL_KNOWN;
				strcpy(logstr, "    Attribute : WELL-KNOWN ");
			}

			if (*ptr & 0x40) {
				getroute.transitive = TRANSITIVE;
				v6getroute.transitive = TRANSITIVE;
				strcat(logstr, "TRANSITIVE ");
			} else {
				getroute.transitive = NON_TRANSITIVE;
				v6getroute.transitive = NON_TRANSITIVE;
				strcat(logstr, "NON-TRANSITIVE ");
			}

			if (*ptr & 0x20) {
				getroute.partial = PARTIAL;
				v6getroute.partial = PARTIAL;
				strcat(logstr, "PARTIAL ");
			} else {
				getroute.partial = COMPLETE;
				v6getroute.partial = COMPLETE;
				strcat(logstr, "COMPLETE ");
			}

			strcat(logstr, "\n");
			pr_log(VLOG_UPAT, pn, logstr, OFF);

			if (*ptr & 0x10) {
				attr_len = 2;
				pr_log(VLOG_UPAT, pn, 
					"    Attribute Length Enhanced\n", OFF);
			} else {
				attr_len = 1;
				pr_log(VLOG_UPAT, pn, 
					"    Attribute Length Normal\n", OFF);
			}

			if (type_code > ATTR_MAX) {
				type_code = ATTR_UNKNOWN;
			}

			/* Show Path Attribute */
			sprintf(logstr, "    Path Attribute = %s\n",
						path_attr[type_code]);
			pr_log(VLOG_UPAT, pn, logstr, OFF);
			ptr += 2;
	
			if (attr_len == 1) {
				attr_octs = (u_char)*ptr;
				ptr++;
			} else {
				attr_octs = (u_short)buf2ushort(ptr);
				ptr += 2;
			}
			sprintf(logstr,
				"    Attribute Length = %d\n", attr_octs);
			pr_log(VLOG_UPAT, pn, logstr, OFF);

			switch(type_code) {
			case ATTR_UNKNOWN:
				ptr += attr_octs;
				break;
			case ATTR_ORIGIN:
				ATRFL_SET(SET_ORIGIN, getroute.attribute_flag);
				ATRFL_SET(SET_ORIGIN, v6getroute.attribute_flag);
				strcpy(logstr, "    ORIGIN Type = ");
				getroute.origin_type = *ptr;
				v6getroute.origin_type = *ptr;
				switch (getroute.origin_type) {
				case ORIGIN_IGP:
					strcat(logstr, "IGP\n");
					break;
				case ORIGIN_EGP:
					strcat(logstr, "EGP\n");
					break;
				case ORIGIN_INCOMPLETE:
					strcat(logstr, "INCOMPLETE\n");
					break;
				default:
					strcat(logstr, "ABNORMAL\n");
				}
				pr_log(VLOG_UPAT, pn, logstr, OFF);
				ptr += attr_octs;
				break;
			case ATTR_AS_PATH:
				if (attr_octs == 0) {
					pr_log(VLOG_UPAT, pn, 
					"    AS_PATH Not Presented\n", OFF);
					break;
				}
				ATRFL_SET(SET_PATHTYPE, getroute.attribute_flag);
				ATRFL_SET(SET_PATHTYPE, v6getroute.attribute_flag);
				ptmp = 0;
				pathbuf[0] = '\0';
				while(ptmp < attr_octs) {
				  strcpy(logstr, "    PATH Segment Type = ");
				  getroute.path_type = *(ptr+ptmp);
				  v6getroute.path_type = *(ptr+ptmp);
				  ptmp++;
				  switch(getroute.path_type){
			  	  case AS_SET:
				  	strcat(logstr, "AS_SET : ");
					break;
				  case AS_SEQUENCE:
					strcat(logstr, "AS_SEQUENCE : ");
					break;
				  case AS_CONFED_SET:
					strcat(logstr, "AS_CONFED_SET : ");
					break;
				  case AS_CONFED_SEQUENCE:
					strcat(logstr, "AS_CONFED_SEQUENCE : ");
					break;
				  default:
					sprintf(cbuf, "ABNORMAL(%d) : ",
							getroute.path_type);
					strcat(logstr, cbuf);
				  }
				  pr_log(VLOG_UPAT, pn, logstr, OFF);
#ifdef DEBUG
				  printf("LOGSTR:%s\n", logstr);
#endif
				  length = *(unsigned char *)(ptr+ptmp);
				  ptmp++;
				  sprintf(logstr, "%d\n", length);
				  pr_log(VLOG_UPAT, pn, logstr, OFF);
				  switch(getroute.path_type) {
				    case AS_CONFED_SET:
				    case AS_CONFED_SEQUENCE:
					strcat(pathbuf, "(");
					break;
				    case AS_SET:
					strcat(pathbuf, "{");
					break;
				  }
				  cp = ptr + ptmp;
				  if (peer[pn].as4_remote && peer[pn].as4_enable) {
				    for(cnt = 0; cnt < length; cnt++, cp += 4){
				      /***
				      strcpy(cbuf, 
					ulongtofourbyteas(buf2ulong(cp)));
				      **/
				      sprintf(cbuf, "%lu", (u_long)buf2ulong(cp));
				      switch(getroute.path_type) {
					case AS_SET:
				          strcat(cbuf, ",");
				          strcat(pathbuf, cbuf);
					  break;
					default:
				          strcat(cbuf, " ");
				          strcat(pathbuf, cbuf);
					  break;
				      }
				    }
				  } else {
				    for(cnt = 0; cnt < length; cnt++, cp += 2){
				      switch(getroute.path_type) {
					case AS_SET:
					  sprintf(cbuf, "%d,",
						(int)buf2ushort(cp));
					  strcat(pathbuf, cbuf);
					  break;
					default:
					  sprintf(cbuf, "%d ",
						(int)buf2ushort(cp));
					  strcat(pathbuf, cbuf);
					  break;
				      }
				    }
				  }
				  switch(getroute.path_type) {
				    case AS_CONFED_SET:
				    case AS_CONFED_SEQUENCE:
					cnt = strlen(pathbuf);
					if (cnt > 0) {
					  pathbuf[cnt-1] = '\0';
					}
					strcat(pathbuf, ") ");
					break;
				    case AS_SET:
					cnt = strlen(pathbuf);
					if (cnt > 0) {
					  pathbuf[cnt-1] = '\0';
					}
					strcat(pathbuf, "} ");
					break;
				  }
				  if (peer[pn].as4_remote && peer[pn].as4_enable) {
				    ptmp += length*4;
				  } else {
				    ptmp += length*2;
				  }
				}
				
				/* Add 16 Sep 2001 Dynamic Path set */
				if (strlen(pathbuf) == 0) {
					getroute.pathset_ptr = NULL;
					v6getroute.pathset_ptr = NULL;
					sprintf(logstr, "    AS_Path = N/A\n");
				} else {
					getroute.pathset_ptr = &pathbuf[0];
					v6getroute.pathset_ptr = &pathbuf[0];
					strcpy(getroute.pathset_ptr, pathbuf);
					strcpy(v6getroute.pathset_ptr, pathbuf);
					sprintf(logstr, "    AS_Path = %s\n",
								getroute.pathset_ptr);
#ifdef DEBUG
					printf(logstr, "    AS_Path = %s\n",
								getroute.pathset_ptr);
#endif
					
				}
				pr_log(VLOG_UPAT, pn, logstr, OFF);
				ptr += attr_octs;
				break;
			case ATTR_NEXT_HOP:
				ATRFL_SET(SET_NEXTHOP, getroute.attribute_flag);
				ATRFL_SET(SET_NEXTHOP, v6getroute.attribute_flag);
				(void)memcpy((char *)&getroute.next_hop,
					ptr, 4);
				(void)memcpy((char *)&v6getroute.next_hop,
					ptr, 4);
				inaddr.s_addr = getroute.next_hop;
				sprintf(logstr, "    NEXT_HOP Address = %s\n",
							inet_ntoa(inaddr));
				pr_log(VLOG_UPAT, pn, logstr, OFF);
				ptr += attr_octs;
				break;
			case ATTR_MULTI_EXIT_DISC:
				ATRFL_SET(SET_METRIC, getroute.attribute_flag);
				ATRFL_SET(SET_METRIC, v6getroute.attribute_flag);
				getroute.metric = buf2ulong(ptr);
				sprintf(logstr, "    MULTI_EXIT_DISC = %lu\n",
							getroute.metric);
				pr_log(VLOG_UPAT, pn, logstr, OFF);
				ptr += attr_octs;
				break;
			case ATTR_LOCAL_PREF:
				ATRFL_SET(SET_LOCALPREF, getroute.attribute_flag);
				ATRFL_SET(SET_LOCALPREF, v6getroute.attribute_flag);
				getroute.local_perf = buf2ulong(ptr);
				v6getroute.local_perf = buf2ulong(ptr);
				sprintf(logstr, "    Local Preferance = %lu\n",
							getroute.local_perf);
				pr_log(VLOG_UPAT, pn, logstr, OFF);
				ptr += attr_octs;
				break;
			case ATTR_ATOMIC_AGGREGATE:
				ATRFL_SET(SET_ATOMICAGGREGATE, getroute.attribute_flag);
				ATRFL_SET(SET_ATOMICAGGREGATE, v6getroute.attribute_flag);
				pr_log(VLOG_UPAT, pn, 
					"    Atomic Aggregate ON\n",OFF);
				/* length 0 Data */
				break;
			case ATTR_AGGREGATOR:
				ATRFL_SET(SET_AGGREGATOR, getroute.attribute_flag);
				ATRFL_SET(SET_AGGREGATOR, v6getroute.attribute_flag);
				getroute.aggregate_as = buf2ushort(ptr);
				v6getroute.aggregate_as = buf2ushort(ptr);
				(void)memcpy(
					(char *)&getroute.aggregate_address,
					ptr+2, 4);
				(void)memcpy(
					(char *)&v6getroute.aggregate_address,
					ptr+2, 4);
				sprintf(logstr, "    Aggregate AS = %d : ",
						getroute.aggregate_as);
				pr_log(VLOG_UPAT, pn, logstr, OFF);
				inaddr.s_addr = getroute.aggregate_address;
				sprintf(logstr, "Address = %s\n",
							inet_ntoa(inaddr));
				pr_log(VLOG_UPAT, pn, logstr, OFF);
				ptr += attr_octs;
				break;
			case ATTR_COMMUNITIES:
				ATRFL_SET(SET_COMMUNITIES, getroute.attribute_flag);
				ATRFL_SET(SET_COMMUNITIES, v6getroute.attribute_flag);
				cnt = 0;
				getroute.comlist_cnt = 0;
				v6getroute.comlist_cnt = 0;
				pr_log(VLOG_UPAT, pn,
					"    Community List = ", OFF);
				while((cnt*4) < attr_octs) {
					getroute.communities[cnt] = 
					    buf2ulong(ptr + cnt * 4);
					v6getroute.communities[cnt] = 
					    buf2ulong(ptr + cnt * 4);
					sprintf(logstr, "0x%08X(%d:%d) ",
						getroute.communities[cnt],
				(u_short)(getroute.communities[cnt] >> 16),
				(u_short)(getroute.communities[cnt] & 0xFFFF));
					pr_log(VLOG_UPAT, pn, logstr, OFF);
					cnt++;
					if (cnt >= MAXCOMMUNITYLIST) {
						pr_log(VLOG_UPAT, pn,
						"Community List Overflow", OFF);
						break;
					}
					getroute.comlist_cnt++;
					v6getroute.comlist_cnt++;
				}
				pr_log(VLOG_UPAT, pn, "\n", OFF);
				ptr += attr_octs;
				break;
			case ATTR_ORIGINATOR_ID:
				ATRFL_SET(SET_ORIGINATOR, getroute.attribute_flag);
				ATRFL_SET(SET_ORIGINATOR, v6getroute.attribute_flag);
				(void)memcpy((char *)&getroute.originator_id,
					ptr, 4);
				(void)memcpy((char *)&v6getroute.originator_id,
					ptr, 4);
				inaddr.s_addr = getroute.originator_id;
				sprintf(logstr, "    Originator ID = %s\n",
							inet_ntoa(inaddr));
				pr_log(VLOG_UPAT, pn, logstr, OFF);
				ptr += attr_octs;
				break;
			case ATTR_CLUSTER_LIST:
				ATRFL_SET(SET_CLUSTER_LIST, getroute.attribute_flag);
				ATRFL_SET(SET_CLUSTER_LIST, v6getroute.attribute_flag);
				cnt = 0;
				pr_log(VLOG_UPAT, pn, 
					"    Cluster List = ", OFF);
				while((cnt*4) < attr_octs) {
					(void)memcpy(
						(char *)&getroute.cluster_list[cnt],
						ptr + cnt * 4, 4);
					(void)memcpy(
						(char *)&v6getroute.cluster_list[cnt],
						ptr + cnt * 4, 4);
					inaddr.s_addr = 
						getroute.cluster_list[cnt];
					sprintf(logstr, "%s ", 
							inet_ntoa(inaddr));
					pr_log(VLOG_UPAT, pn, logstr, OFF);
					cnt++;
					if (cnt >= MAXCLUSTERLIST) {
						pr_log(VLOG_UPAT, pn,
						"Cluster List Overflow", OFF);
						break;
					}
				}
				pr_log(VLOG_UPAT, pn, "\n", OFF);
				ptr += attr_octs;
				break;

			case ATTR_DPA: /* Chen */
			case ATTR_ADVERTISER: /* RFC1863 */
			case ATTR_RCIDPATH_CLUSTERID: /* RFC1863 */
				/* Unsupport */
				ptr += attr_octs;
				break;

			case ATTR_MP_REACH_NLRI: /* RFC2283 */
				ipv6updcnt = pr_mp_reach_nlri(ptr, attr_octs, &v6update, pn);
				sprintf(logstr, "    Received IPv6 Update Routes = %d\n", ipv6updcnt);
				pr_log(VLOG_UPAT, pn, logstr, OFF);
				ptr += attr_octs;
				break;

			case ATTR_MP_UNREACH_NLRI: /* RFC2283 */
				ipv6wdrcnt = pr_mp_unreach_nlri(ptr, attr_octs, &v6withdraw, pn);
				sprintf(logstr, "    Received IPv6 Withdrawn Routes = %d\n", ipv6wdrcnt);
				pr_log(VLOG_UPAT, pn, logstr, OFF);
				ptr += attr_octs;
				break;

			case ATTR_NEW_AS_PATH: /* E.Chen */
				if (attr_octs == 0) {
					pr_log(VLOG_UPAT, pn, 
					"    NEW_AS_PATH Not Presented\n", OFF);
					break;
				}
				/*******************
				ATRFL_SET(SET_PATHTYPE, as4_getroute.attribute_flag);
				ATRFL_SET(SET_PATHTYPE, as4_v6getroute.attribute_flag);
				********************/
				ptmp = 0;
				as4_pathbuf[0] = '\0';
				while(ptmp < attr_octs) {
				  strcpy(logstr, "    AS4_PATH Segment Type = ");
				  as4_getroute.path_type = *(ptr+ptmp);
				  as4_v6getroute.path_type = *(ptr+ptmp);
				  ptmp++;
				  switch(as4_getroute.path_type){
			  	  case AS_SET:
				  	strcat(logstr, "AS_SET : ");
					break;
				  case AS_SEQUENCE:
					strcat(logstr, "AS_SEQUENCE : ");
					break;
				  case AS_CONFED_SET:
					strcat(logstr, "AS_CONFED_SET : ");
					break;
				  case AS_CONFED_SEQUENCE:
					strcat(logstr, "AS_CONFED_SEQUENCE : ");
					break;
				  default:
					sprintf(cbuf, "ABNORMAL(%d) : ",
							as4_getroute.path_type);
					strcat(logstr, cbuf);
				  }
				  pr_log(VLOG_UPAT, pn, logstr, OFF);
#ifdef DEBUG
				  printf("LOGSTR:%s\n", logstr);
#endif
				  length = *(unsigned char *)(ptr+ptmp);
				  ptmp++;
				  sprintf(logstr, "%d\n", length);
				  pr_log(VLOG_UPAT, pn, logstr, OFF);
				  switch(as4_getroute.path_type) {
				    case AS_CONFED_SET:
				    case AS_CONFED_SEQUENCE:
					strcat(as4_pathbuf, "(");
					break;
				    case AS_SET:
					strcat(as4_pathbuf, "{");
					break;
				  }
				  cp = ptr + ptmp;
				  for(cnt = 0; cnt < length; cnt++, cp += 4){
				    switch(as4_getroute.path_type) {
				      case AS_SET:
					sprintf(cbuf, "%lu,", 
						(u_long)buf2ulong(cp));
					strcat(as4_pathbuf, cbuf);
					break;
				      default:
					/***
					sprintf(cbuf, "%d.%d ",
						(int)buf2ushort(cp),
						(int)buf2ushort(cp+2));
					****/
					sprintf(cbuf, "%lu,", 
						(u_long)buf2ulong(cp));
					strcat(as4_pathbuf, cbuf);
					break;
				    }
				  }
				  switch(as4_getroute.path_type) {
				    case AS_CONFED_SET:
				    case AS_CONFED_SEQUENCE:
					cnt = strlen(as4_pathbuf);
					if (cnt > 0) {
					  as4_pathbuf[cnt-1] = '\0';
					}
					strcat(as4_pathbuf, ") ");
					break;
				    case AS_SET:
					cnt = strlen(as4_pathbuf);
					if (cnt > 0) {
					  as4_pathbuf[cnt-1] = '\0';
					}
					strcat(as4_pathbuf, "} ");
					break;
				  }
				  ptmp += length*4;
				}
				
				/* Add 16 Sep 2001 Dynamic Path set */
				if (strlen(as4_pathbuf) == 0) {
					as4_getroute.pathset_ptr = NULL;
					as4_v6getroute.pathset_ptr = NULL;
					sprintf(logstr, "    AS_Path = N/A\n");
				} else {
					as4_getroute.pathset_ptr = &as4_pathbuf[0];
					as4_v6getroute.pathset_ptr = &as4_pathbuf[0];
					strcpy(as4_getroute.pathset_ptr, as4_pathbuf);
					strcpy(as4_v6getroute.pathset_ptr, as4_pathbuf);
					sprintf(logstr, "    AS4_Path = %s\n",
					   	     as4_getroute.pathset_ptr);
				}
				pr_log(VLOG_UPAT, pn, logstr, OFF);
				ptr += attr_octs;
				break;
			case ATTR_NEW_AGGREGATOR: /* E.Chen */
			case ATTR_EXTENDED_COMMUNITIES: /* Rosen */

			default:
				/* Unknown Attribute Error */
				ptr += attr_octs;
			}
		}
	}

	/* Network Layer Reachability Information Print */
	/* length = peer[pn].datalen - 23 - le1 - le2; */
	length = peer[pn].datalen - 4 - le1 - le2;
	sprintf(logstr,
	"    Network Layer Reachability Information Length = %d\n",length);
	pr_log(VLOG_UPDE, pn, logstr, OFF);
	pr_log(VLOG_UPDE, pn,
"   Prefix           Next Hop         Metric Loc_Pef Time    AS_Path\n", OFF);
	pt2 = 0;

	/* Fixed Information Copy to getroute value */
	gettimeofday(&getroute.lastupdate, (struct timezone *)0);
	gettimeofday(&getroute.firstupdate, (struct timezone *)0);
	getroute.source = peer[pn].neighbor;

	while(pt2 < length) {
		memmove(&prroute, &getroute, sizeof(ROUTE_INFO)); 
		prroute.length = *(char *)(ptr+pt2);

		if (prroute.length > 32) {
			pt2++;
			pr_log(VLOG_UPDE, pn, "Prefix too long.\n", OFF);
			continue;
		}

		prroute.prefix = 0;
		if (prroute.length <= 0) {
			cnt = 0;
		} else {
			cnt = (int)(((prroute.length-1)/8) + 1);
		}

		memmove((u_char *)&prroute.prefix, (u_char *)(ptr+pt2+1), cnt);

		if (prroute.pathset_ptr != NULL) {
		    pathbuflen = strlen(getroute.pathset_ptr);
		    if (pathbuflen != 0) {
			prroute.pathset_ptr = (char *)malloc(pathbuflen+1);
			if (prroute.pathset_ptr == NULL) {
			    pr_log(VLOG_GLOB, LOG_ALL, 
			        "pr_update: pathset_ptr: Malloc error\n", ON);
			    notify_systemnotify("STOP",
			    			"Malloc Error : pr_update");
			    finish(PROC_SHUTDOWN);
			    return;
			}
		    }
		    strcpy(prroute.pathset_ptr, getroute.pathset_ptr);
		}

		pr_route(&prroute, pn);
		bgpinfo[pn].prefixcnt++;
		pt2 += cnt+1;

		write_prefixlog(0, &prroute, pn);
#ifdef DEBUG
		printf("DEBUG: Entering add_route\n");
#endif

#ifdef RADIX
		/* Add Route to RADIX Tree */
		ret = add_route(&prroute);
		if (ret == 1) {
			pr_log(VLOG_GLOB, pn, "Memory Allocation Error\n", ON);
#ifdef DEBUG
			printf("Update Message -> Memory Allocation Error\n");
#endif
		}
#ifdef DEBUG
		if (ret == 2) {
			printf("Update Message -> New Route\n");
		} else if (ret == 3) {
			printf("Update Message -> Update Route\n");
		}
#endif
#endif
	}

	if (ipv6updcnt != 0) {
		gettimeofday(&v6getroute.lastupdate, (struct timezone *)0);
		gettimeofday(&v6getroute.firstupdate, (struct timezone *)0);
		bzero(v6getroute.source, IPV6_ADDR_LEN);
		memcpy(v6getroute.source, &peer[pn].neighbor, 4);
		v6getroute.src_afi = AF_IPV4;
		pr_log(VLOG_UPDE, pn, "IPv6 Routes\n", OFF);
		pr_log(VLOG_UPDE, pn,
"   Prefix           Next Hop         Metric Loc_Pef Time    AS_Path\n", OFF);

		cnt = 0;
		if (v6update.preflist != NULL) {
			listbuf = v6update.preflist;

			/* Next Hop Length Detection */
			if (*(u_long *)(&v6getroute.next_hop[0]) == 0) {
				v6getroute.nexthop_afi = AF_IPV6;
				store_v6addr(v6getroute.next_hop, v6update.nexthop);
				ATRFL_SET(SET_NEXTHOP, v6getroute.attribute_flag);
			} else {
				v6getroute.nexthop_afi = AF_IPV4;
			}

		} else {
			ipv6updcnt = 0;
		}
		while(cnt < ipv6updcnt) {
			bzero(&v6prroute, sizeof(ROUTE_INFO_V6));
			memmove(&v6prroute, &v6getroute, sizeof(ROUTE_INFO_V6)); 

			v6prroute.length = listbuf->length;

			if (v6prroute.length > (IPV6_ADDR_LEN*8)) {
				pt2++;
				pr_log(VLOG_UPDE, pn, "IPv6 Prefix too long.\n", OFF);
				continue;
			}
	
			bzero(v6prroute.prefix, IPV6_ADDR_LEN);
			store_v6addr(v6prroute.prefix, listbuf->v6prefix);
	
			if (v6prroute.pathset_ptr != NULL) {
		    	pathbuflen = strlen(v6getroute.pathset_ptr);
		    	if (pathbuflen != 0) {
				v6prroute.pathset_ptr = (char *)malloc(pathbuflen+1);
				if (v6prroute.pathset_ptr == NULL) {
			    	pr_log(VLOG_GLOB, LOG_ALL, 
			        	"pr_update: pathset_ptr: Malloc error\n", ON);
			    	notify_systemnotify("STOP",
			    				"Malloc Error : pr_update");
			    	finish(PROC_SHUTDOWN);
			    	return;
				}
		    	}
		    	strcpy(v6prroute.pathset_ptr, v6getroute.pathset_ptr);
			}

			pr_v6route(&v6prroute, pn);
			write_v6prefixlog(0, &v6prroute, pn);

#ifdef RADIX
			/* Add IPv6 Route to RADIX Tree */
			ret = add_route_v6(&v6prroute);
			if (ret == 1) {
				pr_log(VLOG_GLOB, pn, 
				    "Memory Allocation Error(IPv6Add)\n", ON);
#ifdef DEBUG
				printf("Update Message(IPv6) -> Memory Allocation Error\n");
#endif
			}
#ifdef DEBUG
			if (ret == 2) {
				printf("Update Message(IPv6) -> New Route\n");
			} else if (ret == 3) {
				printf("Update Message(IPv6) -> Update Route\n");
			}
#endif
#endif

			listbuf = listbuf->next;
			cnt++;
	

		}
		bgpinfo[pn].prefixcnt += ipv6updcnt;
	}
	if (ipv6wdrcnt != 0) {
		listbuf = v6withdraw.preflist;
		while(listbuf != NULL) {
#ifdef DEBUG
			{
				char	tmpv6str[50];
				bintostrv6(listbuf->v6prefix, tmpv6str);
				printf("DEBUG: V6DELETE: Prefix %s/%d\n",
						tmpv6str, listbuf->length);
			}
#endif
#ifdef RADIX
			/* Withdraw route from radixv6 tree */
#ifdef RTHISTORY
			withdraw_route_v6(pn, listbuf->v6prefix, 
					  listbuf->length, DFLAG);
#endif
#ifndef RTHISTORY
			withdraw_route_v6(pn, listbuf->v6prefix,
					  listbuf->length);
#endif
#endif
			listbuf = listbuf->next;
		}
		write_v6prefixlog(1, &v6withdraw, pn);
		bgpinfo[pn].withdrawcnt += ipv6wdrcnt;
	}
	free_v6routebuf(&v6update);
	free_v6routebuf(&v6withdraw);

	pr_log(VLOG_UPDE, pn, "===== UPDATE Information Done =====\n", ON);
}
