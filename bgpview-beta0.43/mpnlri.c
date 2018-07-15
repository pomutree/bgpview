/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 2002/03                         */
/*                                                              */
/* Multi Protocol Extentions                                    */
/****************************************************************/
/* mpnlri.c,v 1.1 2003/03/31 05:51:04 kuniaki Exp */

/*
 * Copyright (c) 2002 Internet Initiative Japan Inc.
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

#include "mpnlri.h"

int pr_mp_reach_nlri(char *ptr, int len, V6ROUTE_BUF *routebuf, int pn)
{
	char	pline[LOGSTRSIZE];
	char	sbuf[LOGSTRSIZE];
	char 	*cptr;
	short	afi;
	char	safi;
	char	nexthop_len;
	char	*nexthop;
	char	nofsnpa;
	char	snpalen;
	char	*snpa;
	int	cnt,cnt2;
	char	c2;
	u_char	c;
	u_char	nlrilen;
	char	nlri[IPV6_ADDR_LEN];
	int	pcnt;
	V6ROUTE_LIST	*routelist;

	static char	binc[] = {"0123456789ABCDEF"};

#ifdef DEBUG
	printf("DEBUG: MPNLRI_REACH: Receive MP_BGP Capability Attributes\n");
#endif

	cptr = ptr;
	afi = ntohs(*(short *)cptr);
	cptr+=2;
	safi = *cptr;
	cptr++;

#ifdef DEBUG
	printf("DEBUG: MPNLRI_REACH: AFI = %d, SAFI=%d\n", afi, safi);
#endif

	switch(afi) {
	case AF_IPV6:
		break;
	default:
		/* non-support AFI */
		pr_log(VLOG_UPAT, pn, "    Unsupported AFI - Ignored\n", OFF);
		return(0);
	}

	switch(safi) {
	case SAFI_NLRI_UNI:
		break;
	default:
		/* non-support SAFI */
		pr_log(VLOG_UPAT, pn, "    Unsupported SAFI - Ignored\n", OFF);
		return(0);
	}
	sprintf(pline, "    Receive MP: AFI: %d SAFI: %d\n", afi, safi);
	pr_log(VLOG_UPAT, pn, pline, OFF);

	/* Next Hop Field */
	nexthop_len = *cptr;
	sprintf(pline, "    MPext: Next Hop Length = %d\n", nexthop_len);
	pr_log(VLOG_UPAT, pn, pline, OFF);
	if (nexthop_len != IPV6_ADDR_LEN) {
		pr_log(VLOG_UPAT, pn, "   ! Next Hop Length Error\n", OFF);
		return(0);
	}
	store_v6addr(routebuf->nexthop, cptr+1);
	bintostrv6(routebuf->nexthop, sbuf);
	sprintf(pline, "    MPext: Next Hop : %s\n", sbuf);
	pr_log(VLOG_UPAT, pn, pline, OFF);
	cptr += (nexthop_len + 1);

	/* SNPAs */
	nofsnpa = *cptr;
	sprintf(pline, "    MPext: Number of SNPAs = %d\n", nofsnpa);
	pr_log(VLOG_UPAT, pn, pline, OFF);
	cptr++;
	cnt = 0;
	while(cnt < nofsnpa) {
		snpalen = *cptr;
		c = snpalen % 2;
		c += (int)(snpalen/2);
		snpa = (char *)malloc(c);
		memcpy(snpa, cptr+1, c);
		cnt2 = 0;
		bzero(sbuf, LOGSTRSIZE);
		while(cnt2 < snpalen) {
			c = *(snpa+((int)(cnt/2)));
			if ((cnt2%2) == 0) {
				c &= 0xF0;
				c >>= 4;
				sbuf[cnt2] = binc[c];
			} else {
				c &= 0x0F;
				sbuf[cnt2] = binc[c];
			}
			cnt2++;
		}
		sprintf(pline, "    MPext: SNPA[%d]: %s\n", cnt, sbuf);
		pr_log(VLOG_UPAT, pn, pline, OFF);
		free(snpa);
		cptr += c;
		cnt++;
	}

	/* NLRI */
	routelist = NULL;
	cnt = len - (cptr - ptr);
	cnt2 = 0;
	pcnt = 0;
	while(cnt2 < cnt) {
		bzero(nlri, 16);
		nlrilen = *cptr;
		if (nlrilen > (IPV6_ADDR_LEN*8)) {
			pr_log(VLOG_UPAT, pn, 
				"   ! IPv6 Prefix Length Error\n", OFF);
			return(0);
		}
		pcnt++;
		if (nlrilen != 0) {
			c = nlrilen % 8;
			if (c == 0) {
				memcpy(nlri, cptr+1, (int)(nlrilen/8));
				cptr += (int)(nlrilen/8)+1;
				cnt2 += (int)(nlrilen/8)+1;
			} else {
				memcpy(nlri, cptr+1, (int)(nlrilen/8)+1);
				cptr += (int)(nlrilen/8)+2;
				cnt2 += (int)(nlrilen/8)+2;
				c2 = 0x80;
				c2 >>= c;
				nlri[(int)(nlrilen/8)] &= c2;
			}
		} else {
			cptr++;
			cnt2++;
		}
		bintostrv6(nlri, sbuf);
		sprintf(pline, "    MPext: Prefix : %s/%d\n", sbuf, nlrilen);
		pr_log(VLOG_UPAT, pn, pline, OFF);

		/* Store Prefix */
		if (routelist == NULL) {
			routelist = (V6ROUTE_LIST *)malloc(sizeof(V6ROUTE_LIST));
			bzero(routelist, sizeof(V6ROUTE_LIST));
			routebuf->preflist = routelist;
		} else {
			routelist->next = 
				(V6ROUTE_LIST *)malloc(sizeof(V6ROUTE_LIST));
			bzero(routelist->next, sizeof(V6ROUTE_LIST));
			routelist = routelist->next;
		}
		store_v6addr(routelist->v6prefix, nlri);
		routelist->length = nlrilen;
		routelist->next = NULL;
	}
	routebuf->nofprefix = pcnt;

	return(pcnt);
}

int pr_mp_unreach_nlri(char *ptr, int len, V6ROUTE_BUF *routebuf, int pn)
{
	char	pline[LOGSTRSIZE];
	char	sbuf[LOGSTRSIZE];
	char 	*cptr;
	short	afi;
	char	safi;
	int	cnt,cnt2;
	char	c2;
	u_char	c;
	u_char	nlrilen;
	char	nlri[IPV6_ADDR_LEN];
	int	pcnt;
	V6ROUTE_LIST	*routelist;

#ifdef DEBUG
	printf("DEBUG: MPNLRI_UNREACH: Receive MP_BGP Capability Attributes\n");
#endif

	cptr = ptr;
	afi = ntohs(*(short *)cptr);
	cptr+=2;
	safi = *cptr;
	cptr++;

#ifdef DEBUG
	printf("DEBUG: MPNLRI_UNREACH: AFI = %d, SAFI=%d\n", afi, safi);
#endif

	switch(afi) {
	case AF_IPV6:
		break;
	default:
		/* non-support AFI */
		pr_log(VLOG_UPAT, pn, "    Unsupported AFI - Ignored\n", OFF);
		return(1);
	}

	switch(safi) {
	case SAFI_NLRI_UNI:
		break;
	default:
		/* non-support SAFI */
		pr_log(VLOG_UPAT, pn, "    Unsupported SAFI - Ignored\n", OFF);
		return(1);
	}
	sprintf(pline, "    Receive MP: UNREACH: AFI: %d SAFI: %d\n", afi, safi);
	pr_log(VLOG_UPAT, pn, pline, OFF);

	/* NLRI */
	routelist = NULL;
	cnt = len - (cptr - ptr);
	cnt2 = 0;
	pcnt = 0;
	while(cnt2 < cnt) {
		bzero(nlri, 16);
		nlrilen = *cptr;
		if (nlrilen > (IPV6_ADDR_LEN*8)) {
			pr_log(VLOG_UPAT, pn, 
				"   ! IPv6 Prefix Length Error\n", OFF);
			return(0);
		}
		pcnt++;
		if (nlrilen != 0) {
			c = nlrilen % 8;
			if (c == 0) {
				memcpy(nlri, cptr+1, (int)(nlrilen/8));
				cptr += (int)(nlrilen/8)+1;
				cnt2 += (int)(nlrilen/8)+1;
			} else {
				memcpy(nlri, cptr+1, (int)(nlrilen/8)+1);
				cptr += (int)(nlrilen/8)+2;
				cnt2 += (int)(nlrilen/8)+2;
				c2 = 0x80;
				c2 >>= c;
				nlri[(int)(nlrilen/8)] &= c2;
			}
		} else {
			cptr++;
			cnt2++;
		}
		bintostrv6(nlri, sbuf);
		sprintf(pline, "    MPext: Withdrawn : %s/%d\n", sbuf, nlrilen);
		pr_log(VLOG_UPAT, pn, pline, OFF);

		/* Store Prefix */
		if (routelist == NULL) {
			routelist = (V6ROUTE_LIST *)malloc(sizeof(V6ROUTE_LIST));
			bzero(routelist, sizeof(V6ROUTE_LIST));
			routebuf->preflist = routelist;
		} else {
			routelist->next = 
				(V6ROUTE_LIST *)malloc(sizeof(V6ROUTE_LIST));
			bzero(routelist->next, sizeof(V6ROUTE_LIST));
			routelist = routelist->next;
		}
		store_v6addr(routelist->v6prefix, nlri);
		routelist->length = nlrilen;
		routelist->next = NULL;
	}
	routebuf->nofprefix = pcnt;

	return(pcnt);
}

void free_v6routebuf(V6ROUTE_BUF *rbuf)
{
	V6ROUTE_LIST	*lbuf,*nxt;

	lbuf = rbuf->preflist;
	if (lbuf == NULL) return;
	nxt = lbuf->next;
	if (lbuf != NULL) {
		free(lbuf);
	}
	while(nxt != NULL) {
		lbuf = nxt;
		nxt = lbuf->next;
		free(lbuf);
	}
}

