/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1998/03                         */
/*                                                              */
/* BGP Datainformation Routine                                  */
/****************************************************************/
/* bgpdata.c,v 1.2 2003/09/25 02:04:06 kuniaki Exp */

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

/*
 * Copyright (c) 2003 Intec NetCore, Inc.
 * All rights reserved.
 */

#include "bgpdata.h"

BGP_INFO	bgpinfo[MAXPEERS];

void clear_info(int pn)
{
	bzero(&bgpinfo[pn], sizeof(BGP_INFO));
	bgpinfo[pn].interval		= DEF_INT;
	bgpinfo[pn].infostatus		= true;
}

int fexist(char *fname)
{
	FILE *f;
	int  ret;

	if ((f = fopen(fname, "r")) == NULL) {
		ret = 0;
	} else {
		ret = 1;
		fclose(f);
	}
	return ret;
}

void clear_info_all()
{
	int	cnt;

	for(cnt = 0;cnt < MAXPEERS; cnt++) clear_info(cnt);
}

void recount_info(int pn)
{
	bgpinfo[pn].msgcnt 		= 0;
	bgpinfo[pn].updatecnt 		= 0;
	bgpinfo[pn].notificationcnt	= 0;
	bgpinfo[pn].opencnt 		= 0;
	bgpinfo[pn].keepalivecnt 	= 0;
	bgpinfo[pn].prefixcnt 		= 0;
	bgpinfo[pn].withdrawcnt 	= 0;
	gettimeofday(&bgpinfo[pn].update, (struct timezone *)0);
}

void recount_all()
{
	int	cnt;

	for(cnt = 0;cnt < MAXPEERS; cnt++) recount_info(cnt);
}


int write_info(int pn)
{
	FILE		*fd;
        time_t          clock;
        struct tm       *tm;
        char            buf[TIME_BUF];
        int             ret;
	int		fi;

	if (bgpinfo[pn].infostatus == false) return(0);

	pr_log(VLOG_GLOB, pn, "Writing BGP Message Information\n", ON);

	if (strlen(bgpinfo[pn].fname) == 0) {
		bgpinfo[pn].infostatus = false;
		return 2;
	}

	fi = fexist(bgpinfo[pn].fname);

        time(&clock);
        tm = localtime(&clock);
        ret = strftime(buf, TIME_BUF, "%Y/%m/%d %H:%M:%S", tm);

	if ((fd = fopen(bgpinfo[pn].fname, "a")) == NULL) {
		pr_log(VLOG_GLOB, pn, 
			"BGP information log file could not opend.\n", ON);
		return 1;
	}

	if (fi == 0) fprintf(fd, INFO_HEAD);

	fprintf(fd, INFO_FORM, buf,
		bgpinfo[pn].msgcnt,		bgpinfo[pn].updatecnt,
		bgpinfo[pn].notificationcnt,	bgpinfo[pn].opencnt,
		bgpinfo[pn].keepalivecnt,	bgpinfo[pn].prefixcnt,
		bgpinfo[pn].withdrawcnt,	bgpinfo[pn].totalprefix);

	fclose(fd);
	recount_info(pn);
	return 0;
}
