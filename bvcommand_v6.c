/****************************************************************/
/*    Programmed By K.Kondo IIJ 2002/04                         */
/*                                                              */
/* BGPView Shell Commands for IPv6                              */
/****************************************************************/
/* bvcommand_v6.c,v 1.2 2004/07/13 08:58:44 kuniaki Exp */

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

#include "bvcommand.h"

#ifdef RADIX
#ifdef RTHISTORY
int bvc_clear_route_history_v6(BVS_CMD_PARAM *prm)
{
        int             	clno;
        static RADIX_V6_T	*cnode;
        static int		upcode;
        static int		curclno;
        RADIX_V6_T		*node;
        static int		status;

        if (prm == NULL) {
                cnode = NULL;
                curclno = -1;
                return(0);
        }

        clno = (prm->noparam).clno;

        if (rthist_clean_v6 == RTHIST_CLEANING) {
                if (curclno != clno) {
                        bvs_write(clno,
                            "Anothoer Process is cleaning route history.\r\n");
                        return(BVS_CNT_END);
                }
        } else {
                cnode = NULL;
        }
        if (cnode == NULL) {
                bvs_write(clno, " Cleaning IPv6 Route Histories..\r\n");
        }

        node = clear_rt_history_v6(cnode, &upcode);
        if (node == NULL) {
                if (cnode == NULL) {
                        bvs_write(clno, "%% No Route\r\n");
                }
        }

        if (node == NULL) {
                if (cnode == NULL) {
                        rthist_clean_v6 = RTHIST_NOTCLEANING;
                        curclno = -1;
                        return(BVS_CNT_END);
                } else {
                        time(&cleaned_time_v6);
                        cnode = NULL;
                        rthist_clean_v6 = RTHIST_NOTCLEANING;
                        curclno = -1;
                        bvs_write(clno, " --- Cleaning Done\r\n");

                        /*** Reset Cleaning Timer ***/
                        if (rthistcleartime_v6.tv_sec == 0) {
                                return(BVS_CNT_FINISH);
                        }
                        common_off_timer(rtclear_id_v6);
                        rtclear_id_v6 = common_entry_timer(&rthistcleartime_v6,
                                                        0, rthisttimerclear_v6);
                        if (rtclear_id_v6 == -1) {
                                pr_log(VLOG_GLOB, LOG_ALL,
                                 "IPv6 Route History Clearing Timer Set Fail\n",
					 ON);
                                notify_systemnotify("STOP",
                                       "IPv6 Route History Clearing Timer Set Fail");
                                finish(PROC_SHUTDOWN);
                                return(1);
                        }

                        return(BVS_CNT_FINISH);
                }
        } else {
                cnode = node;
                curclno = clno;
                rthist_clean_v6 = RTHIST_CLEANING;
                return(BVS_CNT_CONTINUE);
        }
}

int bvc_next_rthistory_clear_set_v6(BVS_CMD_PARAM *prm)
{
        int             clno;
        struct timeval  tval;
        char            prline[1024];

        clno = (prm->one_num).clno;
        tval.tv_sec = (prm->one_num).num;
        tval.tv_usec = 0;

        common_off_timer(rtclear_id_v6);
        rtclear_id_v6 = common_entry_timer(&tval, 0, rthisttimerclear_v6);

        if (rtclear_id_v6 == -1) {
                sprintf(prline,
                 "\r\nNext IPv6 route change history clear could not set.\r\n\r\n");
        } else {
                sprintf(prline,
                 "\r\nNext IPv6 route change history clear set %d second later.\r\n\r\n",
                  tval);
        }
        bvs_write(clno, prline);

        return(BVS_CNT_END);
}

#endif /* RTHISTORY */

#ifdef RTHISTORY
int pr_routesum_v6(int clno, R_LIST_V6 *rlist, char *neighbor, char afi,
		 boolean detail, int type)
#endif
#ifndef RTHISTORY
int pr_routesum_v6(int clno, R_LIST_V6 *rlist, char *neighbor, char afi)
#endif
{
        R_LIST_V6       *cptr;
        ROUTE_INFO_V6   *rinfo;
        struct in_addr  sin;
        char            prline[1024], prline2[1024];
        struct timeval  now;
        long            timediff;
        char            tmstr[10];
#ifdef RTHISTORY
        RT_HISTORY      *histptr;
        time_t          pcl;
        struct tm       *tm;
#endif
	char		v6addrstr[50];

        /* if neighbor is NULL, then show all routes */

        cptr = rlist;
        while(cptr != NULL) {
                rinfo = cptr->route_data;

                if (neighbor != NULL) {
                    if (afi == rinfo->src_afi) {
                	switch(afi) {
                	case AF_IPV4:
                	    if (*(net_ulong *)neighbor != *(net_ulong *)(rinfo->source)) {
				cptr = cptr->next;
				continue;
			    }
			    break;
                	case AF_IPV6:
			    if (!v6addrcmp(rinfo->source, neighbor, IPV6_ADDR_LEN*8)) {
                        	cptr = cptr->next;
                        	continue;
			    }
			    break;
			}
		    } else {
		    	cptr = cptr->next;
		    	continue;
		    }
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

		bintostrv6(rinfo->prefix, v6addrstr);
                sprintf(prline2, "%s/%d ", v6addrstr, rinfo->length);
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
			if (rinfo->nexthop_afi == AF_IPV4) {
                        	sin.s_addr = *((net_ulong *)rinfo->next_hop);
                        	sprintf(prline2, "%-17s", inet_ntoa(sin));
			} else {
				bintostrv6(rinfo->next_hop, v6addrstr);
                        	sprintf(prline2, "%-17s", v6addrstr);
			}
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
int show_bgproute_v6(BVS_CMD_PARAM *prm, int dtype, boolean detail)
#endif
#ifndef RTHISTORY
int bvc_show_bgproute_all_v6(BVS_CMD_PARAM *prm)
#endif
{
        int             	clno;
        static RADIX_V6_T	**cnode;
        static int      	*upcode;
        RADIX_V6_T         	*node;

        if (prm == NULL) {
                cnode = (RADIX_V6_T **)malloc(sizeof(RADIX_V6_T *) * bvs_maxcon);
                bzero(cnode, sizeof(RADIX_V6_T *) * bvs_maxcon);
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
        node = show_route_all_v6(*(cnode+clno), (upcode+clno));
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
                pr_routesum_v6(clno, node->rinfo, 0, 0, detail, dtype);
#endif
#ifndef RTHISTORY
                pr_routesum_v6(clno, node->rinfo, 0, 0);
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
int bvc_show_bgproute_all_v6(BVS_CMD_PARAM *prm)
{
        return(show_bgproute_v6(prm, RTV_ALL, false));
}

int bvc_show_bgproute_active_v6(BVS_CMD_PARAM *prm)
{
        return(show_bgproute_v6(prm, RTV_ACTIVE, false));
}

int bvc_show_bgproute_inactive_v6(BVS_CMD_PARAM *prm)
{
        return(show_bgproute_v6(prm, RTV_INACTIVE, false));
}

int bvc_show_bgproute_all_detail_v6(BVS_CMD_PARAM *prm)
{
        return(show_bgproute_v6(prm, RTV_ALL, true));
}

int bvc_show_bgproute_active_detail_v6(BVS_CMD_PARAM *prm)
{
        return(show_bgproute_v6(prm, RTV_ACTIVE, true));
}

int bvc_show_bgproute_inactive_detail_v6(BVS_CMD_PARAM *prm)
{
        return(show_bgproute_v6(prm, RTV_INACTIVE, true));
}
#endif /* RTHISTORY */

#ifdef RTHISTORY
int output_routesum_v6(R_LIST_V6 *rlist, char *neighbor, char afi, FILE *ofp,
                        boolean detail, int type)
#endif
#ifndef RTHISTORY
int output_routesum_v6(R_LIST_V6 *rlist, char *neighbor, char afi, FILE *ofp)
#endif
{
        R_LIST_V6	*cptr;
        ROUTE_INFO_V6	*rinfo;
        struct in_addr  sin;
        char            prline[1024], prline2[1024];
        struct timeval  now;
        long            timediff;
        char            tmstr[10];

#ifdef RTHISTORY
        RT_HISTORY      *histptr;
        time_t          pcl;
        struct tm       *tm;
#endif
	char		v6addrstr[50];

        /* if neighbor is NULL, then show all routes */

        cptr = rlist;
        while(cptr != NULL) {
                rinfo = cptr->route_data;

                if (neighbor != NULL) {
		    if (afi == rinfo->src_afi) {
			switch(afi) {
			case AF_IPV4:
			    if (*(net_ulong *)neighbor != *(net_ulong *)(rinfo->source)) {
                        	cptr = cptr->next;
                        	continue;
			    }
			    break;
			case AF_IPV6:
			    if (!v6addrcmp(rinfo->source, neighbor, IPV6_ADDR_LEN*8)) {
                        	cptr = cptr->next;
                        	continue;
			    }
			    break;
			}
		    } else {
			cptr = cptr->next;
			continue;
		    }
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

		bintostrv6(rinfo->prefix, v6addrstr);
                sprintf(prline2, "%s/%d ", v6addrstr, rinfo->length);
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
			if (rinfo->nexthop_afi == AF_IPV4) {
                        	sin.s_addr = *((net_ulong *)rinfo->next_hop);
                        	sprintf(prline2, "%-17s", inet_ntoa(sin));
			} else {
				bintostrv6(rinfo->next_hop, v6addrstr);
				sprintf(prline2, "%-17s", v6addrstr);
			}
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
int output_bgproute_v6(BVS_CMD_PARAM *prm, boolean detail, int dtype)
#endif
#ifndef RTHISTORY
int bvc_output_bgproute_all_v6(BVS_CMD_PARAM *prm)
#endif
{
        int             	clno;
        static RADIX_V6_T	**cnode;
        static int      	*upcode;
        RADIX_V6_T	         *node;
        char           		fname[FNAMESIZE];
        char            	fn[FNAMESIZE];
        static FILE     	*fp;

        if (prm == NULL) {
                cnode = (RADIX_V6_T **)malloc(sizeof(RADIX_V6_T *) * bvs_maxcon);
                bzero(cnode, sizeof(RADIX_V6_T *) * bvs_maxcon);
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

        node = show_route_all_v6(*(cnode+clno), (upcode+clno));
        if (node == NULL) {
                if (*(cnode+clno) == NULL) {
                        fprintf(fp, "%% No Route\n");
                }
        } else {
#ifdef RTHISTORY
                output_routesum_v6(node->rinfo, 0, 0, fp, detail, dtype);
#endif
#ifndef RTHISTORY
                output_routesum_v6(node->rinfo, 0, 0, fp);
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
int bvc_output_bgproute_all_v6(BVS_CMD_PARAM *prm){
        return(output_bgproute_v6(prm, false, RTV_ALL));
}

int bvc_output_bgproute_active_v6(BVS_CMD_PARAM *prm){
        return(output_bgproute_v6(prm, false, RTV_ACTIVE));
}

int bvc_output_bgproute_inactive_v6(BVS_CMD_PARAM *prm){
        return(output_bgproute_v6(prm, false, RTV_INACTIVE));
}

int bvc_output_bgproute_all_detail_v6(BVS_CMD_PARAM *prm){
        return(output_bgproute_v6(prm, true, RTV_ALL));
}

int bvc_output_bgproute_active_detail_v6(BVS_CMD_PARAM *prm){
        return(output_bgproute_v6(prm, true, RTV_ACTIVE));
}

int bvc_output_bgproute_inactive_detail_v6(BVS_CMD_PARAM *prm){
        return(output_bgproute_v6(prm, true, RTV_INACTIVE));
}
#endif

int bvc_showipbgproute_v6(BVS_CMD_PARAM *prm)
{
        int             clno;
        char       	prefix[IPV6_ADDR_LEN];
        RADIX_V6_T      *node;

        clno = (prm->ipv6addr).clno;
        store_v6addr(prefix, (prm->ipv6addr).v6addr);

        bvs_write(clno, "\r\n ! : Inactive Route\r\n\r\n");
        bvs_write(clno, 
"   Prefix           Next Hop         Metric Loc_Pef Time    AS_Path\r\n");

        node = search_route_v6(prefix, IPV6_ADDR_LEN*8);
        if (node == NULL) {
                bvs_write(clno, "%% No Route\r\n");
        } else {
#ifdef RTHISTORY
                pr_routesum_v6(clno, node->rinfo, 0, 0, true, RTV_ALL);
#endif
#ifndef RTHISTORY
                pr_routesum_v6(clno, node->rinfo, 0, 0);
#endif
        }
        return(BVS_CNT_END);
}

#ifdef RTHISTORY
int show_bgproute_neighbor_v6(BVS_CMD_PARAM *prm, int dtype)
#endif
#ifndef RTHISTORY
int bvc_show_bgproute_neighbor_v6(BVS_CMD_PARAM *prm)
#endif
{
        int             	clno;
        static RADIX_V6_T	**cnode;
        static int		*upcode;
        RADIX_V6_T		*node;
	char			neiafi;
	char			neigh[IPV6_ADDR_LEN];

        if (prm == NULL) {
                cnode = (RADIX_V6_T **)malloc(sizeof(RADIX_V6_T *) * bvs_maxcon);
                bzero(cnode, sizeof(RADIX_V6_T *) * bvs_maxcon);
                upcode = (int *)malloc(sizeof(int) * bvs_maxcon);
                bzero(upcode, sizeof(int) * bvs_maxcon);
                return(0);
        }

        clno = (prm->ipv46addr).clno;
	neiafi = (prm->ipv46addr).afi;
	if (neiafi == AF_IPV4) {
		*(net_ulong *)neigh = (prm->ipv46addr).address.v4addr;
	} else {
		store_v6addr(neigh, (prm->ipv46addr).address.v6addr);
	}

        if (*(cnode+clno) == NULL) {
                bvs_write(clno, "\r\n ! : Inactive Route\r\n\r\n");
                bvs_write(clno, 
"   Prefix           Next Hop         Metric Loc_Pef Time    AS_Path\r\n");
        }

        node = show_route_all_v6(*(cnode+clno), (upcode+clno));
        if (node == NULL) {
                if (*(cnode+clno) == NULL) {
                        bvs_write(clno, "%% No Route\r\n");
                }
        } else {
#ifdef RTHISTORY
                pr_routesum_v6(clno, node->rinfo, neigh, neiafi, false, dtype);
#endif
#ifndef RTHISTORY
                pr_routesum_v6(clno, node->rinfo, neigh, neiafi);
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
int bvc_show_bgproute_neighbor_v6(BVS_CMD_PARAM *prm)
{
        return(show_bgproute_neighbor_v6(prm, RTV_ALL));
}

int bvc_show_bgproute_neighbor_active_v6(BVS_CMD_PARAM *prm)
{
        return(show_bgproute_neighbor_v6(prm, RTV_ACTIVE));
}

int bvc_show_bgproute_neighbor_inactive_v6(BVS_CMD_PARAM *prm)
{
        return(show_bgproute_neighbor_v6(prm, RTV_INACTIVE));
}
#endif

#ifdef RTHISTORY
int output_bgproute_nei_v6(BVS_CMD_PARAM *prm, boolean detail, int dtype)
#endif
#ifndef RTHISTORY
int bvc_output_bgproute_all_nei_v6(BVS_CMD_PARAM *prm)
#endif
{
        int             	clno;
        static RADIX_V6_T  	**cnode;
        static int      	*upcode;
        RADIX_V6_T         	*node;
        char            	fname[FNAMESIZE];
        char            	fn[FNAMESIZE];
        static FILE     	*fp;
        char          		neighbor[IPV6_ADDR_LEN];
	char			neiafi;

        if (prm == NULL) {
                cnode = (RADIX_V6_T **)malloc(sizeof(RADIX_V6_T *) * bvs_maxcon);
                bzero(cnode, sizeof(RADIX_V6_T *) * bvs_maxcon);
                upcode = (int *)malloc(sizeof(int) * bvs_maxcon);
                bzero(upcode, sizeof(int) * bvs_maxcon);
                return(0);
        }

        clno = (prm->onestr_ipv46addr).clno;
        strcpy(fn, (prm->onestr_ipv46addr).str);
	neiafi = (prm->onestr_ipv46addr).afi;
	if (neiafi == AF_IPV4) {
		*(net_ulong *)neighbor = (prm->onestr_ipv46addr).address.v4addr;
	} else {
		store_v6addr(neighbor, (prm->onestr_ipv46addr).address.v6addr);
	}

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

        node = show_route_all_v6(*(cnode+clno), (upcode+clno));
        if (node == NULL) {
                if (*(cnode+clno) == NULL) {
                        fprintf(fp, "%% No Route\n");
                }
        } else {
#ifdef RTHISTORY
                output_routesum_v6(node->rinfo, neighbor, neiafi, fp, detail, dtype);
#endif
#ifndef RTHISTORY
                output_routesum_v6(node->rinfo, neighbor, neiafi, fp);
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
int bvc_output_bgproute_all_nei_v6(BVS_CMD_PARAM *prm){
        return(output_bgproute_nei_v6(prm, false, RTV_ALL));
}

int bvc_output_bgproute_active_nei_v6(BVS_CMD_PARAM *prm){
        return(output_bgproute_nei_v6(prm, false, RTV_ACTIVE));
}

int bvc_output_bgproute_inactive_nei_v6(BVS_CMD_PARAM *prm){
        return(output_bgproute_nei_v6(prm, false, RTV_INACTIVE));
}

int bvc_output_bgproute_all_detail_nei_v6(BVS_CMD_PARAM *prm){
        return(output_bgproute_nei_v6(prm, true, RTV_ALL));
}

int bvc_output_bgproute_active_detail_nei_v6(BVS_CMD_PARAM *prm){
        return(output_bgproute_nei_v6(prm, true, RTV_ACTIVE));
}

int bvc_output_bgproute_inactive_detail_nei_v6(BVS_CMD_PARAM *prm){
        return(output_bgproute_nei_v6(prm, true, RTV_INACTIVE));
}
#endif

#endif /* RADIX */
