/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 2000/01                         */
/*                                                              */
/* BGPView Shell 			                        */
/****************************************************************/
/* bvshell.c,v 1.2 2003/08/29 08:53:26 kuniaki Exp */

/*
 * Copyright (c) 2000-2002 Internet Initiative Japan Inc.
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

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "bgp.h"
#include "bvshell.h"
#include "bvcommand.h"
#include "bgplog.h"

/***** Valiables *****/
int		bvs_port;
int		bvs_maxcon;
char		bvs_pass[FNAMESIZE];
int		bvs_rowsize;

int		bvs_svsoc;
BVS_CLIENT	*bvs_clients;

BVS_CMDS	*cmds;

char		bvs_prompt[32];
char 		bvs_promptmark[] = BVSPROMPTMKLIST;

/***** Subroutins *****/

int bvs_cmd_exit(BVS_CMD_PARAM *prm)
{
	int clno;

	clno = (prm->noparam).clno;
	close((bvs_clients+clno)->clsock);
	/* (bvs_clients+clno)->clsock = (int)NULL; */
	(bvs_clients+clno)->clsock = 0;
}

void bvs_entry_command(int (*function)(BVS_CMD_PARAM *), 
			const char *cmdline, const char *help, boolean more)
{
	BVS_CMDS	*curptr, *lvptr;
	char		tgline[MAXIMUM_CMD_LENGTH];
	char		*tgptr;
	int		lv;

#ifdef DEBUG
	printf("DEBUG: BVS_CMD: Entry Command-> %s\n",cmdline);
#endif

	if (cmds == NULL) {
#ifdef DEBUG
		printf("DEBUG: BVS_CMD: Entry Top Level(exit)\n");
#endif
		cmds = (BVS_CMDS *)malloc(sizeof(BVS_CMDS));
		bzero(cmds, sizeof(BVS_CMDS));
		strcpy(cmds->word, "exit");
		cmds->function = bvs_cmd_exit;
		cmds->next = NULL;
		cmds->lower = NULL;
		strcpy(cmds->help, "Logout BGPView Shell");
	}
	curptr = cmds;
	strcpy(tgline, cmdline);
	lv = 0;
	tgptr = strtok(tgline, " ");
	while(tgptr != NULL) {
		if (strcmp(curptr->word, tgptr) == 0) {
			tgptr = strtok(NULL, " ");
			if (tgptr == NULL) {
				if (curptr->function == NULL) {
					curptr->function = function;
					strcpy(curptr->help, help);
					curptr->more = more;
				} else {
					printf("Same Command Already Entried\n");
					exit(1);
				}
			} else {
				if (curptr->lower == NULL) {
#ifdef DEBUG
					printf("DEBUG: BVS_CMD: Make Lower Level:%s\n",tgptr);
#endif
					curptr->lower = 
					  (BVS_CMDS *)malloc(sizeof(BVS_CMDS));
					curptr = curptr->lower;
					lv++;
					strcpy(curptr->word, tgptr);
					curptr->function = NULL;
					curptr->next = NULL;
					curptr->lower = NULL;
					strcpy(curptr->help, "");
					curptr->more = false;
				} else {
					curptr = curptr->lower;
					lv++;
				}
				if (lv >= BVSC_WORDMAX) {
					printf("Command Maximum Word Over\n");
					exit(1);
				}
			}
		} else if (curptr->next == NULL) {
#ifdef DEBUG
			printf("DEBUG: BVS_CMD: Make Next:%s\n",tgptr);
#endif
			curptr->next = (BVS_CMDS *)malloc(sizeof(BVS_CMDS));
			curptr = curptr->next;
			strcpy(curptr->word, tgptr);
			curptr->function = NULL;
			curptr->next = NULL;
			curptr->lower = NULL;
			strcpy(curptr->help, "");
			curptr->more = false;
		} else {
			curptr = curptr->next;
		}
	}
}

void bvs_init_set_default()
{
	bvs_port = BVS_PORT_DEFAULT;
	bvs_maxcon = BVS_MAXCON_DEFAULT+1;
	bvs_rowsize = DEF_ROWSIZE;
	bzero(bvs_pass, FNAMESIZE);
	cmds = NULL;

	/* Static Value initialize */
	bvc_show_bgproute_all((BVS_CMD_PARAM *)0);
	bvc_show_bgproute_all_v6((BVS_CMD_PARAM *)0);
	bvc_output_bgproute_all((BVS_CMD_PARAM *)0);
	bvc_output_bgproute_all_v6((BVS_CMD_PARAM *)0);
	bvc_output_bgproute_all_nei((BVS_CMD_PARAM *)0);
	bvc_output_bgproute_all_nei_v6((BVS_CMD_PARAM *)0);
	bvc_show_bgproute_neighbor((BVS_CMD_PARAM *)0);
	bvc_show_bgproute_neighbor_v6((BVS_CMD_PARAM *)0);
	bvc_show_bgproute_sum((BVS_CMD_PARAM *)0);
	bvc_showipbgproutetime_eq((BVS_CMD_PARAM *)0);
	bvc_showipbgproutetime_le((BVS_CMD_PARAM *)0);
	bvc_showipbgproutetime_ge((BVS_CMD_PARAM *)0);
	bvc_show_bgpnexthop_sum((BVS_CMD_PARAM *)0);
	bvc_show_bgpnexthop_sum_neighbor((BVS_CMD_PARAM *)0);
#ifdef RTHISTORY
	bvc_clear_route_history((BVS_CMD_PARAM *)0);
	bvc_clear_route_history_v6((BVS_CMD_PARAM *)0);
#endif

#ifdef IRRCHK
	bvc_show_irr_status((BVS_CMD_PARAM *)0);
	bvc_do_output_irr_status((BVS_CMD_PARAM *)0);
	bvc_show_irr_status_summary((BVS_CMD_PARAM *)0);
#endif

	bvc_output_aslist_nei((BVS_CMD_PARAM *)0);



	/* Entry Commands */
	bvs_entry_command(bvs_cmd_exit, "quit", "Logout BGPView Shell", false);
	bvs_entry_command(bvs_clearbgp, "clear ip bgp %ADR", 
						"Clear BGP Session", false);
	bvs_entry_command(bvc_shutdown, "shutdown", "shutdown BGPView", false);
	bvs_entry_command(bvc_reboot, "reboot", "Reboot BGPView", false);
	bvs_entry_command(bvc_setannouncestatus, 
				"set neighbor %ADR announce status %SWC",
				"Enabling/Disabling status change announcement",
				false);
	bvs_entry_command(bvc_setpeerdisable, "set neighbor %ADR disable",
				"Disable peer", false);
	bvs_entry_command(bvc_setpeerenable, "set neighbor %ADR enable",
				"Enable peer", false);
	bvs_entry_command(bvc_setinfologstatus, 
				"set neighbor %ADR infolog %SWC",
				"Enabling/Disabling Infomation Log Status",
				false);
	bvs_entry_command(bvc_bgpsum, "show ip bgp summary", 
					"Show BGP Peer Summary", false);
	bvs_entry_command(bvc_shousers, "show users", 
					"Show Logged-in users", false);
	bvs_entry_command(bvc_shoneighbor, "show ip bgp neighbor %ADR", 
					"Show BGP Peer Information", false);
#ifdef HAVE_TOP
	bvs_entry_command(bvc_shoproc, "show proccess",
					"Show System Process", false);
#endif /* HAVE_TOP */
	bvs_entry_command(bvc_shostatics, "show ip bgp statics %ADR",
						"Show BGP Statics", false);
	bvs_entry_command(bvc_clrstatics, "clear ip bgp statics %ADR",
						"Clear BGP Statics", false);
	bvs_entry_command(bvc_clrstaticsall, "clear ip bgp statics all",
						"Clear All BGP Statics", false);
	bvs_entry_command(bvc_shoversion, "show version", 
						"Show BGPView Version", false);
	bvs_entry_command(bvc_shoconfig, "show configuration", 
					"Show BGPView Configuration File", true);
	bvs_entry_command(bvc_doannouncedummy, "do announce dummyroute", 
					"Start dummyroute announce", false);
	bvs_entry_command(bvc_shoannouncedummy, "show announce dummyroute", 
					"Show dummyroute announce status", false);
	bvs_entry_command(bvc_dorefleshsystemlog, "do reflesh systemlog", 
					"System log file restart", false);
	bvs_entry_command(bvc_dorefleshprefixlogall, "do reflesh prefixlog all", 
					"Prefix log file restart", false);
	bvs_entry_command(bvc_doroutertt,
		      "do test route rtt neighbor %ADR inject %PRF timeout %INT", 
		      "Route RTT Test", false);
	bvs_entry_command(bvc_doroutertt_notimeout,
		      "do test route rtt neighbor %ADR inject %PRF", 
		      "Route RTT Test", false);
	bvs_entry_command(bvc_set_vtyrowsize,
		      "set vty rowsieze %INT", "Set VTY row size", false);

#ifdef RADIX
	bvs_entry_command(bvc_show_bgproute_all,
			"show ip bgp", "Show BGP Route All", true);
	bvs_entry_command(bvc_show_bgproute_all_v6,
			"show v6 bgp", "Show IPv6 BGP Route All", true);
	bvs_entry_command(bvc_output_bgproute_all,
			"do output route filename %STR", 
			"BGP ALL Route output to file", true);
	bvs_entry_command(bvc_output_bgproute_all_v6,
			"do output v6 route filename %STR", 
			"BGP ALL IPv6 Route output to file", true);
	bvs_entry_command(bvc_output_bgproute_all_nei,
			"do output route filename %STR neighbor %ADR", 
			"BGP ALL Route output to file from neighbor", true);
	bvs_entry_command(bvc_output_bgproute_all_nei_v6,
			"do output v6 route filename %STR neighbor %A46", 
			"BGP ALL IPv6 Route output to file from neighbor", true);
	bvs_entry_command(bvc_show_bgproute_neighbor,
			"show ip bgp neighbor %ADR routes",
			"Show BGP Route From Neighbor", true);
	bvs_entry_command(bvc_show_bgproute_neighbor_v6,
			"show v6 bgp neighbor %A46 routes",
			"Show BGP Route From Neighbor", true);
	bvs_entry_command(bvc_showipbgproute,
			"show ip bgp %ADR", "Show BGP Route", true);
	bvs_entry_command(bvc_showipbgproute_v6,
			"show v6 bgp %V6A", "Show BGP IPV6 Route", true);
	bvs_entry_command(bvc_show_bgproute_sum,
			"show ip route summary",
			"Show BGP Routes Summary", false);
	bvs_entry_command(bvc_showipbgproutetime_eq,
			"show ip bgp route time %STR",
			"Show BGP Route with Announced time", true);
	bvs_entry_command(bvc_showipbgproutetime_le,
			"show ip bgp route time %STR le",
			"Show BGP Route with Announced time", true);
	bvs_entry_command(bvc_showipbgproutetime_ge,
			"show ip bgp route time %STR ge",
			"Show BGP Route with Announced time", true);
#ifdef RTHISTORY
	bvs_entry_command(bvc_clear_route_history,
			"clear route history",
			"Cleaning Route Update/Withdraw History", false);
	bvs_entry_command(bvc_clear_route_history_v6,
			"clear v6 route history",
			"Cleaning IPv6 Route Update/Withdraw History", false);
	bvs_entry_command(bvc_next_rthistory_clear_set,
			"set route history clear time %INT",
			"Set Next Cleaning Route Update/Withdraw History Time", false);
	bvs_entry_command(bvc_next_rthistory_clear_set_v6,
			"set v6 route history clear time %INT",
			"Set Next Cleaning IPv6 Route Update/Withdraw History Time", false);
	bvs_entry_command(bvc_show_bgproute_active,
			"show ip bgp active",
			"Show BGP active routes", false);
	bvs_entry_command(bvc_show_bgproute_active_v6,
			"show v6 bgp active",
			"Show IPv6 BGP active routes", false);
	bvs_entry_command(bvc_show_bgproute_inactive,
			"show ip bgp inactive",
			"Show BGP inactive routes", false);
	bvs_entry_command(bvc_show_bgproute_inactive_v6,
			"show v6 bgp inactive",
			"Show IPv6 BGP inactive routes", false);
	bvs_entry_command(bvc_show_bgproute_all_detail,
			"show ip bgp detail",
			"Show BGP all routes with history", false);
	bvs_entry_command(bvc_show_bgproute_all_detail_v6,
			"show v6 bgp detail",
			"Show IPv6 BGP all routes with history", false);
	bvs_entry_command(bvc_show_bgproute_active_detail,
			"show ip bgp active detail",
			"Show BGP active routes with history", false);
	bvs_entry_command(bvc_show_bgproute_active_detail_v6,
			"show v6 bgp active detail",
			"Show IPv6 BGP active routes with history", false);
	bvs_entry_command(bvc_show_bgproute_inactive_detail,
			"show ip bgp inactive detail",
			"Show BGP inactive routes with history", false);
	bvs_entry_command(bvc_show_bgproute_inactive_detail_v6,
			"show v6 bgp inactive detail",
			"Show IPv6 BGP inactive routes with history", false);
	bvs_entry_command(bvc_show_bgproute_neighbor_active,
			"show ip bgp neighbor %ADR active routes",
			"Show BGP Active Route From Neighbor", true);
	bvs_entry_command(bvc_show_bgproute_neighbor_active_v6,
			"show v6 bgp neighbor %A46 active routes",
			"Show BGP Active IPv6 Route From Neighbor", true);
	bvs_entry_command(bvc_show_bgproute_neighbor_inactive,
			"show ip bgp neighbor %ADR inactive routes",
			"Show BGP Active Route From Neighbor", true);
	bvs_entry_command(bvc_show_bgproute_neighbor_inactive_v6,
			"show v6 bgp neighbor %A46 inactive routes",
			"Show BGP Active IPv6 Route From Neighbor", true);
	bvs_entry_command(bvc_output_bgproute_active,
			"do output route filename %STR active", 
			"BGP ALL Active Route output to file", true);
	bvs_entry_command(bvc_output_bgproute_active_v6,
			"do output v6 route filename %STR active", 
			"BGP ALL Active IPv6 Route output to file", true);
	bvs_entry_command(bvc_output_bgproute_active_nei,
			"do output route filename %STR neighbor %ADR active", 
			"BGP ALL Active Route output to file from neighbor", true);
	bvs_entry_command(bvc_output_bgproute_active_nei_v6,
			"do output v6 route filename %STR neighbor %A46 active", 
			"BGP ALL Active IPv6 Route output to file from neighbor", true);
	bvs_entry_command(bvc_output_bgproute_inactive,
			"do output route filename %STR inactive", 
			"BGP ALL In-Active Route output to file", true);
	bvs_entry_command(bvc_output_bgproute_inactive_v6,
			"do output v6 route filename %STR inactive", 
			"BGP ALL In-Active IPv6 Route output to file", true);
	bvs_entry_command(bvc_output_bgproute_inactive_nei,
			"do output route filename %STR neighbor %ADR inactive", 
			"BGP ALL In-Active Route output to file from neighbor", true);
	bvs_entry_command(bvc_output_bgproute_inactive_nei_v6,
			"do output v6 route filename %STR neighbor %A46 inactive", 
			"BGP ALL In-Active IPv6 Route output to file from neighbor", true);
	bvs_entry_command(bvc_output_bgproute_all_detail,
			"do output route filename %STR detail", 
			"BGP ALL Detail Route output to file", true);
	bvs_entry_command(bvc_output_bgproute_all_detail_v6,
			"do output v6 route filename %STR detail", 
			"BGP ALL Detail IPv6 Route output to file", true);
	bvs_entry_command(bvc_output_bgproute_all_detail_nei,
			"do output route filename %STR neighbor %ADR detail", 
			"BGP ALL Detail Route output to file from neighbor", true);
	bvs_entry_command(bvc_output_bgproute_all_detail_nei_v6,
			"do output v6 route filename %STR neighbor %A46 detail", 
			"BGP ALL Detail IPv6 Route output to file from neighbor", true);
	bvs_entry_command(bvc_output_bgproute_active_detail,
			"do output route filename %STR active detail", 
			"BGP ALL Detail Active Route output to file", true);
	bvs_entry_command(bvc_output_bgproute_active_detail_v6,
			"do output v6 route filename %STR active detail", 
			"BGP ALL Detail Active IPv6 Route output to file", true);
	bvs_entry_command(bvc_output_bgproute_active_detail_nei,
			"do output route filename %STR neighbor %ADR active detail", 
			"BGP ALL Detail Active Route output to file from neighbor", true);
	bvs_entry_command(bvc_output_bgproute_active_detail_nei_v6,
			"do output v6 route filename %STR neighbor %A46 active detail", 
			"BGP ALL Detail Active IPv6 Route output to file from neighbor", true);
	bvs_entry_command(bvc_output_bgproute_inactive_detail,
			"do output route filename %STR inactive detail", 
			"BGP ALL Detail In-Active Route output to file", true);
	bvs_entry_command(bvc_output_bgproute_inactive_detail_v6,
			"do output v6 route filename %STR inactive detail", 
			"BGP ALL Detail In-Active IPv6 Route output to file", true);
	bvs_entry_command(bvc_output_bgproute_inactive_detail_nei,
			"do output route filename %STR neighbor %ADR inactive detail", 
			"BGP ALL Detail In-Active Route output to file from neighbor", true);
	bvs_entry_command(bvc_output_bgproute_inactive_detail_nei_v6,
			"do output v6 route filename %STR neighbor %A46 inactive detail", 
			"BGP ALL Detail In-Active IPv6 Route output to file from neighbor", true);
#endif /* RTHISTORY */
#endif
	bvs_entry_command(bvc_readcronfile,
			"do read cron file", 
			"Read Cron Configuration File", false);
	bvs_entry_command(bvc_showcronlist,
			"show cron list", 
			"Show Configurated Cron List", false);
	bvs_entry_command(bvc_show_bgpnexthop_sum,
			"show ip nexthop summary", 
			"Show BGP Next Hop Summary", false);
	bvs_entry_command(bvc_show_bgpnexthop_sum_neighbor,
			"show ip nexthop summary neighbor %ADR", 
			"Show BGP Next Hop Summary with neighbor", false);

#ifdef DEBUG
#ifdef RADIX
	bvs_entry_command(bvc_dodeleteroute, 
			"do withdraw neighbor %ADR route %PRF",
			"DEBUG: Prefix Withdraw", false);
	bvs_entry_command(bvc_doclearpeerroute, 
			"do clear peer route neighbor %ADR",
			"DEBUG: Clear Route for Peer", false);
#endif /* RADIX */
	bvs_entry_command(bvc_dotimertest, "do timer test",
					"DEBUG: Common Timer Test", false);
#endif /* DEBUG */

#ifdef IRRCHK
	bvs_entry_command(bvc_show_irr_status, "show irr status",
			"Show IRR route check", true);
	bvs_entry_command(bvc_show_irr_status_active, "show irr status active",
			"Show IRR route check", true);
	/*
	bvs_entry_command(bvc_show_irr_status_nei, 
			"show irr status active neighbor %ADR",
			"Show IRR route check for a neighbor", true);
	bvs_entry_command(bvc_show_irr_status_active_nei, 
			"show irr status active neighbor %ADR active",
			"Show IRR route check for a neighbor", true);
	*/
	
	bvs_entry_command(bvc_do_output_irr_status, 
			"do output irrstats filename %STR all",
			"Output IRR Status to File", true);
	bvs_entry_command(bvc_do_output_irr_status_active, 
			"do output irrstats filename %STR active",
			"Output IRR Status to File", true);
	/*
	bvs_entry_command(bvc_do_output_irr_status_nei, 
			"do output irrstats filename %STR neighbor %ADR all",
			"Output IRR Status to File for a neighbor", true);
	bvs_entry_command(bvc_do_output_irr_status_active_nei, 
			"do output irrstats filename %STR neighbor %ADR active",
			"Output IRR Status to File for a neighbor", true);
	*/

	bvs_entry_command(bvc_show_irr_status_summary,
			"show irr status summary",
			"Show IRR Statys Summary", true);
	/*
	bvs_entry_command(bvc_show_irr_status_summary_nei,
			"show irr status summary neighbor %ADR",
			"Show IRR Statys Summary for a neighbor", true);
	*/
#endif

	bvs_entry_command(bvc_output_aslist_nei, 
			"do output aslist filename %STR neighbor %ADR",
			"Output AS-List to File for a neighbor", true);
	
#ifdef DEBUG
	/* View Entried Command */
	{
		BVS_CMDS	*curptr, *stkptr[BVSC_WORDMAX];
		int		lv;

		curptr = cmds;
		lv = 0;
		stkptr[0] = cmds;
		while(1) {
			printf("%s:%d ", curptr->word, lv);
			fflush(stdout);
			if (curptr->lower == NULL) {
				printf("<CR>\n");
				if (curptr->next == NULL) {
					while(1) {
						curptr = stkptr[lv];
						lv--;
						printf(".");
						fflush(stdout);
						if (curptr->next != NULL) {
							curptr= curptr->next;
							break;
						}
						if (lv <= 0) break;
					}
					if (lv==0) break;
				} else {
					curptr = curptr->next;
				}
			} else {
				lv++;
				printf(",");
				fflush(stdout);
				stkptr[lv] = curptr;
				curptr = curptr->lower;
			}

		}
	}
#endif
}

int bvs_init_set_values(int type, char *datas)
{
	int ret;
	int intval;
	char *charval;

	ret = 0;

	switch(type) {
	case SET_BVS_PORT:
		intval = atoi(datas);
		if ((intval < 1024) || (intval > 65535)) {
			printf("Illegal BVS Port\n");
			ret = 1;
		} else {
			bvs_port = intval;
		}
		break;

	case SET_BVS_MAXCON:
		intval = atoi(datas);
		if ((intval < 0) || (intval > BVS_MAXIMUM)) {
			printf("Illegal Maximum BVS Connection\n");
			ret = 1;
		} else {
			bvs_maxcon = intval+1;
		}
		break;

	case SET_BVS_PASSWD:
		charval = (char *)datas;
		strncpy(bvs_pass, charval, FNAMESIZE-1);
		break;

	case SET_BVS_ROWSIZE:
		intval = atoi(datas);
		if (intval < 0) {
			printf("Illegal Row Size\n");
			ret = 1;
		} else {
			bvs_rowsize = intval;
		}
		break;

	default:
		ret = 1;
	}
	return(ret);
}

int bvs_init_start()
{
	struct sockaddr_in	sin;
	struct hostent		*hp;
	char			hostname[MAXHOSTNAMELEN];
	char			strbuf[MAXHOSTNAMELEN+16];
	int			sopt = 1;

	/* Malloc Clients Socket Area */
	bvs_clients = (BVS_CLIENT *)malloc(sizeof(BVS_CLIENT) * bvs_maxcon);
	if (bvs_clients == NULL) {
		pr_log(VLOG_GLOB, LOG_ALL, "failed to malloc (BVS)\n", ON);
		return(1);
	}
	bzero(bvs_clients, (sizeof(BVS_CLIENT)*bvs_maxcon));

	/* BVS Socket Open */
	if (gethostname(hostname, sizeof(hostname)) != 0) {
		pr_log(VLOG_GLOB, LOG_ALL, "failed to get hostname(bvs).\n", ON);
		return(1);
	}

	if ((hp = gethostbyname(hostname)) == NULL) {
		sprintf(strbuf, "%s: host unknown.\n", hostname);
		pr_log(VLOG_GLOB, LOG_ALL, strbuf, ON);
		/* return(1); */
	}

#ifdef DEBUG
	printf("DEBUG: BVS Hostname : %s\n", hostname);
	if (hp != NULL) printf("DEBUG: GET Hostname : %s\n", hp->h_name);
#endif

#ifdef DEBUG
	printf("DEBUG: BVS Open Socket\n");
#endif
	if ((bvs_svsoc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		pr_log(VLOG_GLOB, LOG_ALL, "socket() error (BVS)\n", ON);
		perror("server: socket(bvs)");
		return(1);
	}

#ifdef DEBUG
	printf("DEBUG: BVS Set Socket Option: SO_REUSEADDR\n");
#endif
	if (setsockopt(bvs_svsoc, SOL_SOCKET, SO_REUSEADDR,
					&sopt, sizeof(sopt)) < 0) {
		pr_log(VLOG_GLOB, LOG_ALL,
			 "Socketoption(SO_REUSEADDR) set error (BVS)\n", ON);
		perror("socket option(SO_REUSEADDR) set error (BVS)");
		return(1);
	}

#ifndef LINUX
#ifdef SO_REUSEPORT
#ifdef DEBUG
	printf("DEBUG: BVS Set Socket Option: SO_REUSEPORT\n");
#endif
	if (setsockopt(bvs_svsoc, SOL_SOCKET, SO_REUSEPORT,
					&sopt, sizeof(sopt)) < 0) {
		pr_log(VLOG_GLOB, LOG_ALL, 
			"Socketoption(SO_REUSEPORT) set error (BVS)\n", ON);
		perror("socket option(SO_REUSEPORT) set error (BVS)");
		return(1);
	}
#endif /* SO_REUSEPORT */
#endif /* LINUX */

	sin.sin_family		= AF_INET;
	sin.sin_addr.s_addr	= htonl(INADDR_ANY);
	sin.sin_port		= htons(bvs_port);

#ifdef DEBUG
	printf("DEBUG: BVS Scoket Binding\n");
#endif
	if (bind(bvs_svsoc, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		pr_log(VLOG_GLOB, LOG_ALL, "bind() Error (BVS)\n", ON);
		perror("server: bind(bvs)");
		return(1);
	}

#ifdef DEBUG
	printf("DEBUG: BVS Socket Listening\n");
#endif
	if (listen(bvs_svsoc, 5) < 0) {
		pr_log(VLOG_GLOB, LOG_ALL, "listen() Error (BVS)\n", ON);
		perror("server: listen(bvs)");
		return(1);
	}

	/**** Inline Virtual Terminal Open ****/
#ifdef DEBUG
	printf("DEBUG: BVS Inline VTY1 Open -");
#endif
	/* if (((bvs_clients+bvs_maxcon-1)->clsock = open("/dev/null", O_WRONLY)) == (int)NULL){ */
	if (((bvs_clients+bvs_maxcon-1)->clsock = open("/dev/null", O_WRONLY)) == 0){
		printf("Inline Virtual Terminal Could not open..\n");
		return(1); 
	}
	(bvs_clients+bvs_maxcon-1)->status = BVSC_INPUT_CMD;
#ifdef DEBUG
	printf(" Done\n");
#endif

	return(0);
}

void bvs_finish(int type)
{
	int	cnt;

	cnt = 0;
	while(cnt < bvs_maxcon) {
		if (bvs_clients == NULL) break;
		/* if ((bvs_clients + cnt)->clsock != (int)NULL) { */
		if ((bvs_clients + cnt)->clsock != 0) {
			if (type != PROC_CHILDEND) {
			  bvs_write(cnt, "BVS Close Request Received...\r\n");
			}
			close((bvs_clients+cnt)->clsock);
		}
		cnt++;
	}

	if (bvs_clients != NULL) free(bvs_clients);
	if (type != PROC_CHILDEND) {
		printf("Closing BVS Server Socket..\n");
	}
	close(bvs_svsoc);
}

void bvs_readsocset(fd_set *fds)
{
	int cnt;

	FD_SET(bvs_svsoc, fds);
	cnt = 0;
	while(cnt < bvs_maxcon-1) {
		/* if ((bvs_clients + cnt)->clsock != (int)NULL) { */
		if ((bvs_clients + cnt)->clsock != 0) {
			FD_SET((bvs_clients + cnt)->clsock, fds);
		}
		cnt++;
	}
}

void *bvs_search_function(const char *cmdline, BVS_CMD_PARAM *param, boolean *more)
{
	char		cmd[MAXIMUM_CMD_LENGTH+1];
	char		*cmdptr;
	BVS_CMDS	*curptr, *stack, *substack;
	char		words[BVSC_WORDMAX][BVSC_WRCMAX+1];
	int		lv;
	char		*prmptr;
	struct in_addr  sin;
	char		*bufp;
	char		strbuf[128];
	int		intbuf;
	char		charbuf;

	strcpy(cmd, cmdline);
	cmdptr = strtok(cmd, " ");
	lv = 0;
	while(cmdptr != NULL) {
		bzero(words[lv], BVSC_WRCMAX+1);
		strncpy(words[lv], cmdptr, BVSC_WRCMAX);
		cmdptr = strtok(NULL, " ");
		lv++;
	}
	strcpy(words[lv], "");

	bzero(param, sizeof(BVS_CMD_PARAM));
	prmptr = (char *)param + sizeof(int);
	curptr = cmds;
	lv = 0;
	while(1) {
		stack = NULL;
		substack = curptr;
		while(curptr != NULL) {
#ifdef DEBUG
			printf("DEBUG: BVS: Parameter %s/%s\n",
						curptr->word, words[lv]);
#endif
			if (strstr(curptr->word, words[lv]) == curptr->word) {
				if (stack == NULL) {
					stack = curptr;
				} else {
					/* command incomplete */
					return(NULL);
				}
			}
			curptr = curptr->next;
		}
		if (stack == NULL) {
		    curptr = substack;
		    while(curptr != NULL) {
			if (*(curptr->word) == '%') {
#ifdef DEBUG			
			    printf("DEBUG: BVS: Checking Valiables\n");
#endif
			    if (stack == NULL) {
#ifdef DEBUG			
				printf("DEBUG: BVS: Stacking Valiables: %s\n", words[lv]);
#endif
				stack = curptr;
				if (strcmp((curptr->word)+1,"ADR")==0) {
				    inet_aton(words[lv], &sin);
				    *(u_long *)prmptr = sin.s_addr;
#ifdef DEBUG
				    printf("DEBUG: BVS: Address Parameter = %08X\n", ntohl(*(u_long *)prmptr));
#endif
				    prmptr += sizeof(u_long);
				} else if (strcmp((curptr->word)+1,"INT")==0) {
				    *(int *)prmptr = atoi(words[lv]);
				    prmptr += sizeof(int);
				} else if (strcmp((curptr->word)+1,"PRF")==0) {
				    bufp = strchr(words[lv], '/');
				    if (bufp == NULL) {
					/* Prefix Format Error */
					return(NULL);
				    } else {
					*bufp = '\0';
					inet_aton(words[lv], &sin);
					*(u_long *)prmptr = sin.s_addr;
					prmptr += sizeof(u_long);
					*(int *)prmptr = atoi(bufp+1);
					prmptr += sizeof(int);
				    }
				} else if (strcmp((curptr->word)+1,"STR")==0) {
				    if (strlen(words[lv]) > 63) {
				    	/* Longer String */
				    	return(NULL);
				    }
				    strcpy((char *)prmptr, words[lv]);
				    prmptr += 64;
				} else if (strcmp((curptr->word)+1,"LSR")==0) {
				    intbuf = lv;
				    strcpy(strbuf, words[intbuf]);
				    while(strlen(words[intbuf+1]) != 0) {
				      strcat(strbuf, " ");
				      strcat(strbuf, words[intbuf+1]);
				      intbuf++;
				    }
				    strcpy((char *)prmptr, strbuf);
				    prmptr += 128;
				    lv = intbuf;
				} else if (strcmp((curptr->word)+1,"SWC")==0) {
				    strcpy(strbuf, words[lv]);
				    intbuf = 0;
				    while(strbuf[intbuf] != '\0') {
				    	strbuf[intbuf] = toupper(strbuf[intbuf]);
				    	intbuf++;
				    }
				    if (strcmp(strbuf, "ON") == 0) {
				    	*(char *)prmptr = (char)1;
				    } else if (strcmp(strbuf, "OFF") == 0) {
				    	*(char *)prmptr = (char)0;
				    } else {
				    	/* Parameter Error */
				    	return(NULL);
				    }
				    prmptr += 1;
				} else if (strcmp((curptr->word)+1,"V6A")==0) {
				    if (chkfmt(CHKFMT_V6A, words[lv]) == false) {
				    	/* Format Error */
				    	return(NULL);
				    }
				    strtobinv6(prmptr, words[lv]);
				    prmptr += IPV6_ADDR_LEN;
				} else if (strcmp((curptr->word)+1,"V6P")==0) {
				    if (chkfmt(CHKFMT_V6P, words[lv]) == false) {
				    	/* Format Error */
				    	return(NULL);
				    }
				    bufp = strchr(words[lv], '/');
				    if (bufp == NULL) {
					/* Prefix Format Error */
					return(NULL);
				    } else {
					*bufp = '\0';
					strtobinv6(prmptr, words[lv]);
					prmptr += IPV6_ADDR_LEN;
					*(int *)prmptr = atoi(bufp+1);
					prmptr += sizeof(int);
				    }
				} else if (strcmp((curptr->word)+1,"A46")==0) {
				    charbuf = checkafi(words[lv]);
#ifdef DEBUG
				    printf("DEBUG: A46: cmd afi = %d\n", charbuf);
#endif
				    switch(charbuf) {
				    case AF_IPV4:
				    	inet_aton(words[lv], &sin);
				    	*(net_ulong *)prmptr = sin.s_addr;
					break;
				    case AF_IPV6:
				    	strtobinv6(prmptr, words[lv]);
				    	break;
				    default:
				    	/* Format Error */
				    	return(NULL);
				    }
				    prmptr += IPV6_ADDR_LEN;
				    *prmptr = charbuf;
				    prmptr++;
				}
		  	    } else {
				return(NULL);
			    }
			}
			curptr = curptr->next;
		    }
		}
		if (stack == NULL) return(NULL); /* command imcomplete */
		lv++;
		curptr = stack;
		if (strlen(words[lv]) == 0) {
			if (curptr->function == NULL) {
				/* command imcomplete */
				return(NULL);
			} else {
				/* command complete */
				*more = curptr->more;
				return(curptr->function);
			}
		}
		if (curptr->lower == NULL) {
			/* command imcomplete */
			return(NULL);
		}
		curptr = curptr->lower;
	}
}

int bvs_exec_command(int clno)
{
	char 		cs[MAXIMUM_CMD_LENGTH+32];
	char 		vty_cr[] = "\r\n";
	int		(*function)(BVS_CMD_PARAM *);
	BVS_CMD_PARAM	prm;
	int		cnt;
	int		ret;
	boolean		more;
	struct in_addr	sin;

	bvs_write(clno, vty_cr);

	ret = 0;
	if ((bvs_clients+clno)->status == BVSC_LOGIN) {
		if (checkpw(bvs_pass, (bvs_clients+clno)->cmdline) == 1) {
		/* if (strcmp((bvs_clients+clno)->cmdline, bvs_pass) == 0) { */
			(bvs_clients+clno)->status = BVSC_INPUT_CMD;
			sin.s_addr = htonl((bvs_clients+clno)->addr);
			sprintf(logstr, "BVS Logged in from %s\n",
						inet_ntoa(sin));
			pr_log(VLOG_GLOB, LOG_ALL, logstr, ON);
		} else {
			bvs_write(clno, "\r\nPassword Incorrect!\r\n");
			(bvs_clients+clno)->status = BVSC_LOGIN;
			sin.s_addr = htonl((bvs_clients+clno)->addr);
			sprintf(logstr, "BVS Login Password Fail : %s\n",
						inet_ntoa(sin));
			pr_log(VLOG_GLOB, LOG_ALL, logstr, ON);
		}
	} else {
#ifdef DEBUG
		sprintf(cs, "Input Command: %s\r\n", (bvs_clients+clno)->cmdline);
		bvs_write(clno, cs);
#endif
		/* Search Function */
		function = bvs_search_function((bvs_clients+clno)->cmdline,
						 &prm, &more);
#ifdef DEBUG
		printf("DEBUG: BVS: bvs_search_function OK\n");
#endif

		if (strlen((bvs_clients+clno)->cmdline) > 0) {
			/* Entry History */
			cnt = BVSC_MAXHISTORY-1;
			while(cnt > 1) {
				strcpy((bvs_clients+clno)->history[cnt],
					(bvs_clients+clno)->history[cnt-1]);
				cnt--;
			}
			strcpy((bvs_clients+clno)->history[1],
			     			 (bvs_clients+clno)->cmdline);
			if ((bvs_clients+clno)->histnum < BVSC_MAXHISTORY) {
				(bvs_clients+clno)->histnum++;
			}
			(bvs_clients+clno)->curhitory = 0;
			(bvs_clients+clno)->history[0][0] = '\0';

			if (function == NULL) {
				bvs_write(clno, "! Command Incomplete\r\n");
			}
		}
#ifdef DEBUG
		printf("DEBUG: BVS: Command Line History Record done\n");
#endif

		if (function != NULL) {
			sprintf(logstr, "BVS: Command input : VTY%d : %s : exec\n", 
						clno, (bvs_clients+clno)->cmdline);
			pr_log(VLOG_GLOB, LOG_ALL, logstr, ON);

			/* Execute Command */
			prm.noparam.clno = clno;
			/* Retcode is continue code */
			ret = (*function)(&prm);
#ifdef DEBUG
			printf("DEBUG: Command Exec Return Code = %d\n", ret);
#endif
			(bvs_clients+clno)->enable_more = false;
			switch(ret) {
			case 0:
				/* Not Continue */
				(bvs_clients+clno)->cont_code = 0;
				break;
			case 1:
				/* Continue */
				(bvs_clients+clno)->enable_more = more;
				if (more) {
					(bvs_clients+clno)->nextmore =
						(bvs_clients+clno)->rowsize;
				}
				(bvs_clients+clno)->cont_code = 1;
				(bvs_clients+clno)->cont_func = function;
				memcpy(&((bvs_clients+clno)->cont_param), &prm,
						sizeof(BVS_CMD_PARAM));
				break;
			case 2:
				/* Finish Continue */
				(bvs_clients+clno)->cont_code = 0;
				break;
			case 3:
				/* No Prompt */
				(bvs_clients+clno)->cont_code = 0;
				break;
			}
		} else {
			sprintf(logstr, "BVS: Command input : VTY%d : %s : error\n", 
						clno, (bvs_clients+clno)->cmdline);
			pr_log(VLOG_GLOB, LOG_ALL, logstr, ON);
		}
	}

	if ((bvs_clients+clno)->status == BVSC_LOGIN) {
		bvs_write(clno, "\r\nPassword:");
	} else {
		if ((ret == 0)||(ret == 2)) {
			bvs_write(clno, bvs_prompt);
		}
	}

	bzero((bvs_clients+clno)->cmdline, MAXIMUM_CMD_LENGTH+1);
	(bvs_clients+clno)->col = 0;
}

int crcheck(char *str, int len, int clno)
{
	char	strbuf[1024];
	int	cnt;
	int	ptr;

	bzero(strbuf, 1024);
	strncpy(strbuf, str, len);
	ptr = 0;
	cnt = 0;
	while(strbuf[ptr] != '\0') {
		if (strbuf[ptr] == '\n') {
			cnt++;
		}
		ptr++;
	}
	if ((bvs_clients+clno)->enable_more) {
		(bvs_clients+clno)->nextmore -= cnt;
	}
	return(cnt);
}

boolean morecheck(int clno)
{
	if ((bvs_clients+clno)->enable_more) {
		if ((bvs_clients+clno)->nextmore <= 0) {
			return(true);
		}
	}
	return(false);
}

int bvs_write(int clno, char *str)
{
	int 		length;
	char 		*ptr;
	fd_set 		wset;
	struct timeval	tval;
	int		rtcnt;
	int		left, written;

	tval.tv_sec  = 1;
	tval.tv_usec = 0;
	length = strlen(str);
	ptr = str;
	left = length;
	rtcnt = 0;
	/* if ((bvs_clients+clno)->clsock == (int)NULL) { */
	if ((bvs_clients+clno)->clsock == 0) {
		return(-1);
	}
	while(left > 0) {
		FD_ZERO(&wset);
		FD_SET((bvs_clients+clno)->clsock, &wset);

		if (select(FD_SETSIZE, NULL, &wset, NULL, &tval) < 0) {
			if (errno == EINTR) {
				printf("BVS: Select Return Code EINTR\n");
				continue;
			}
			perror("bvs select");
			return(-1);
		}
		if (FD_ISSET((bvs_clients+clno)->clsock, &wset)) {
			written = write((bvs_clients+clno)->clsock, ptr, left);
			if (written <= 0) {
				perror("bvs write");
				sprintf(logstr, "BVS: Write Error : %d\n", errno);
				pr_log(VLOG_GLOB, LOG_ALL, logstr, ON);
				close((bvs_clients+clno)->clsock);
				/* (bvs_clients+clno)->clsock = (int)NULL; */
				(bvs_clients+clno)->clsock = 0;
				return written;
			}
			left -= written;
			ptr += written;
		} else {
			rtcnt++;
			if (rtcnt > 3) return -1;
#ifdef DEBUG
			printf("DEBUG: BVS: Socket Write Retry\n");
#endif
		}
	}
	/* crcheck(str, length-left, clno); */
	return length - left;
}

int bvs_cmd_auto_complete(const char *cmdline, char *addchrs)
{
	char		cmd[MAXIMUM_CMD_LENGTH];
	char		*cmdptr;
	BVS_CMDS	*curptr, *stack;
	char		words[BVSC_WORDMAX][BVSC_WRCMAX+1];
	int		lv;
	int		cptn;
	int		nextw;
	boolean		ftchk;
	char		charbuf;

#ifdef DEBUG
	printf("DEBUG: BVS: complete line : '%s':%d\n",cmdline,strlen(cmdline));
#endif
	if (*(cmdline + strlen(cmdline) - 1) == ' ') {
		/* Next Word Search */
		nextw = 1;
	} else {
		nextw = 0;
	}
#ifdef DEBUG
	printf("DEBUG: BVS: Next Word Search Param = %d\n", nextw);
#endif
	bzero(words, BVSC_WORDMAX*BVSC_WRCMAX);
	strcpy(cmd, cmdline);
	cmdptr = strtok(cmd, " ");
	lv = 0;
	while(cmdptr != NULL) {
		bzero(words[lv], BVSC_WRCMAX+1);
		strncpy(words[lv], cmdptr, BVSC_WRCMAX);
		cmdptr = strtok(NULL, " ");
		lv++;
	}

	curptr = cmds;
	lv = 0;
	while(1) {
		stack = NULL;
		while(curptr != NULL) {
#ifdef DEBUG
			printf("DEBUG: BVS: Compareing: %s/%s\n",
						curptr->word, words[lv]);

#endif			
			if ((strlen(words[lv]) == 0) && 
			    (*(curptr->word) == '%')) {
				/* Parameter Detection */
				return(0);
			} else 
			if (strstr(curptr->word, words[lv]) == curptr->word) {
				if (stack == NULL) {
					stack = curptr;
#ifdef DEBUG
					printf("DEBUG: BVS: Match Command\n");
#endif
				} else {
					/* command incomplete */
#ifdef DEBUG
					printf("DEBUG: BVS: Not Complete 1\n");
#endif
					return(0);
				}
			} else if (*(curptr->word) == '%') {
				if (strlen(words[lv]) != 0) {
				  /* format check */
				  if (strcmp(curptr->word, "%ADR") == 0) {
				    ftchk = chkfmt(CHKFMT_ADR, words[lv]);
				  } else if (strcmp(curptr->word, "%PRF") == 0) {
				    ftchk = chkfmt(CHKFMT_PRF, words[lv]);
				  } else if (strcmp(curptr->word, "%INT") == 0) {
				    ftchk = chkfmt(CHKFMT_INT, words[lv]);
				  } else if (strcmp(curptr->word, "%V6A") == 0) {
				    ftchk = chkfmt(CHKFMT_V6A, words[lv]);
				  } else if (strcmp(curptr->word, "%V6P") == 0) {
				    ftchk = chkfmt(CHKFMT_V6P, words[lv]);
				  } else if (strcmp(curptr->word, "%A46") == 0) {
				    charbuf = checkafi(words[lv]);
				    if (charbuf == 0) {
				      ftchk = false;
				    } else {
				      ftchk = true;
				    }
				  } else {
				    ftchk = true;
				  }
				  if (ftchk == true) stack = curptr;
				}
			}
			curptr = curptr->next;
		}
		if (stack == NULL) return(0); /* command imcomplete */
		lv++;
		curptr = stack;
		if (strlen(words[lv]) == 0) {
			if (nextw == 0) {
#ifdef DEBUG
			printf("DEBUG: BVS: auto complete target : %s/%s\n",
					words[lv-1], curptr->word);
#endif
				if (*(curptr->word) == '%') {
					return(0);
				}
				cptn = strlen(words[lv-1]);
				strcpy(addchrs, (curptr->word + cptn));
				strcat(addchrs, " ");
				return(strlen(addchrs));
			} else {
				nextw--;
			}
		}
		if (curptr->lower == NULL) {
			/* command imcomplete */
			return(0);
#ifdef DEUBG
			printf("DEBUG: BVS: Not Complete 2\n");
#endif
		}
		curptr = curptr->lower;
	}
}

int bvs_cmd_pr_help(int clno, const char *cmdline)
{
	char		cmd[MAXIMUM_CMD_LENGTH];
	char		*cmdptr;
	BVS_CMDS	*curptr, *stack;
	BVS_CMDS	*ptrlist[BVSC_WORDMAX];
	int		ptrcnt;
	char		words[BVSC_WORDMAX][BVSC_WRCMAX+1];
	int		lv;
	int		cptn;
	int		nextw, prcnt;
	char		prline[BVSC_HELPMAXC+BVSC_WRCMAX+3];
	char		swword[BVSC_WRCMAX+1];
	boolean		ftchk;
	char		charbuf;

#ifdef DEBUG
	printf("DEBUG: BVS: complete line : '%s':%d\n",cmdline,strlen(cmdline));
#endif
	nextw = 0;
	if (*(cmdline + strlen(cmdline) - 1) == ' ')  nextw = 1;
#ifdef DEBUG
	printf("DEBUG: BVS: Next Word Search Param = %d\n", nextw);
#endif
	bzero(words, BVSC_WORDMAX*BVSC_WRCMAX);
	strcpy(cmd, cmdline);
	cmdptr = strtok(cmd, " ");
	lv = 0;
	prcnt = 0;
	while(cmdptr != NULL) {
		bzero(words[lv], BVSC_WRCMAX+1);
		strncpy(words[lv], cmdptr, BVSC_WRCMAX);
		cmdptr = strtok(NULL, " ");
		lv++;
	}

	curptr = cmds;
	lv = 0;
	ptrcnt = 0;
	while(1) {
		stack = NULL;
		while(curptr != NULL) {
#ifdef DEBUG
			printf("DEBUG: BVS: Comparing: %s/%s\n",
						curptr->word, words[lv]);
#endif
			if ((strstr(curptr->word, words[lv]) == curptr->word) ||
			    (*(curptr->word) == '%')) {
			    if ((*(curptr->word)=='%') && (strlen(words[lv])==0)) {
			    	ftchk = true;
			    } else if (strcmp(curptr->word, "%ADR") == 0) {
#ifdef DEBUG			
				printf("DEBUG: BVS: ADR Check -> %s/%s\n",
					curptr->word, words[lv]);
#endif
			        ftchk = chkfmt(CHKFMT_ADR, words[lv]);
#ifdef DEBUG			
				if (ftchk == true) 
				printf("DEBUG: BVS: ADR Check -> TRUE\n");
#endif
			    } else if (strcmp(curptr->word, "%PRF") == 0) {
			        ftchk = chkfmt(CHKFMT_PRF, words[lv]);
			    } else if (strcmp(curptr->word, "%INT") == 0) {
			        ftchk = chkfmt(CHKFMT_INT, words[lv]);
			    } else if (strcmp(curptr->word, "%V6A") == 0) {
			        ftchk = chkfmt(CHKFMT_V6A, words[lv]);
			    } else if (strcmp(curptr->word, "%V6P") == 0) {
			        ftchk = chkfmt(CHKFMT_V6P, words[lv]);
			    } else if (strcmp(curptr->word, "%A46") == 0) {
			        charbuf = checkafi(words[lv]);
			        if (charbuf == 0) {
			      	    ftchk = false;
				} else {
				    ftchk = true;
				}
			    } else {
			     	ftchk = true;
			    }
			    if (ftchk == true) {
				ptrlist[ptrcnt] = curptr;
				ptrcnt++;
#ifdef DEBUG
				printf("DEBUG: BVS: Match Command: %d\n",
							ptrcnt);
#endif
				if (strlen(words[lv+1]) != 0) {
					if (stack == NULL) {
						stack = curptr;
					} else {
						/* command incomplete */
#ifdef DEBUG
						printf("DEBUG: BVS: Not Complete 1\n");
#endif
						return(0);
					}
				}
				stack = curptr;
			    }
			}
			curptr = curptr->next;
		}
		if (ptrcnt == 0) return(0); /* command imcomplete */
		lv++;
#ifdef DEBUG
		printf("DEBUG: BVS: Total Match = %d items\n", ptrcnt);
#endif
		curptr = stack;
		if (strlen(words[lv]) == 0) {
			if (nextw == 0) {
				if (prcnt == 0) bvs_write(clno, "\r\n");
#ifdef DEBUG
			printf("DEBUG: BVS: Help Target : %d items\n", ptrcnt);
#endif
				nextw = 0;
				while(nextw < ptrcnt) {
					if (*(ptrlist[nextw]->word) == '%') {
					    if (strcmp(ptrlist[nextw]->word,
								"%ADR") == 0) {
						strcpy(swword, "<Address>");
					    } else
					    if (strcmp(ptrlist[nextw]->word,
								"%INT") == 0) {
						strcpy(swword, "<Number>");
					    } else
					    if (strcmp(ptrlist[nextw]->word,
								"%PRF") == 0) {
						strcpy(swword, "<Prefix/len>");
					    } else
					    if (strcmp(ptrlist[nextw]->word,
					    			"%STR") == 0) {
					    	strcpy(swword, "<String>");
					    } else
					    if (strcmp(ptrlist[nextw]->word,
					    			"%LSR") == 0) {
					    	strcpy(swword, "<Words>");
					    } else
					    if (strcmp(ptrlist[nextw]->word,
					    			"%SWC") == 0) {
					    	strcpy(swword, "<ON/OFF>");
					    } else
					    if (strcmp(ptrlist[nextw]->word,
					    			"%V6A") == 0) {
					    	strcpy(swword, "<IPv6 Address>");
					    } else
					    if (strcmp(ptrlist[nextw]->word,
					    			"%V6P") == 0) {
					    	strcpy(swword, "<IPv6 Address/Len>");
					    } else
					    if (strcmp(ptrlist[nextw]->word,
					    			"%A46") == 0) {
					        strcpy(swword, "<IPv4/6 Address>");
					    }
					} else {
						strcpy(swword, ptrlist[nextw]->word);
					}
					sprintf(prline, "%-16s %s\r\n",
						swword, ptrlist[nextw]->help);
					bvs_write(clno, prline);
					prcnt++;
					nextw++;
				}
				return(prcnt);
			} else {
				if (ptrcnt > 1) return(0);
				if (ptrcnt == 1) {
					if (prcnt == 0) bvs_write(clno, "\r\n");
					if (ptrlist[0]->function != NULL) {
						strcpy(swword, "<CR>");
						sprintf(prline, "%-16s %s\r\n",
							swword, ptrlist[0]->help);
						bvs_write(clno, prline);
						prcnt++;
					}
				}
				nextw--;
			}
		}
		if (curptr->lower == NULL) {
			return(prcnt);
		}
#ifdef DEBUG
		printf("DEBUG: BVS: Go Lower Leverl: %d\n", lv);
#endif
		curptr = curptr->lower;
		ptrcnt = 0;
	}
}

int bvs_cmd_history(int clno, int mode, char *now)
{
	/* Mode:1 before / 2: After */

	if ((bvs_clients+clno)->curhitory == 0) {
		/* Push Now Command */
		strcpy((bvs_clients+clno)->history[0], now);
#ifdef DEBUG
		printf("DEBUG: BVS: Push History 0 : '%s'\n",now);
#endif
	}
	if (mode == 1){
		if (((bvs_clients+clno)->histnum) == 
			(bvs_clients+clno)->curhitory) {
			return(0);
		}
		(bvs_clients+clno)->curhitory++;
		strcpy((bvs_clients+clno)->cmdline, 
		    (bvs_clients+clno)->history[(bvs_clients+clno)->curhitory]);
	} else if (mode == 2) {
		if ((bvs_clients+clno)->curhitory == 0) {
			return(0);
		}
		(bvs_clients+clno)->curhitory--;
		strcpy((bvs_clients+clno)->cmdline,
		    (bvs_clients+clno)->history[(bvs_clients+clno)->curhitory]);
	}
#ifdef DEBUG
	printf("DEBUG: BVS: Current History Number = %d/%d\n",
		(bvs_clients+clno)->curhitory,
		(bvs_clients+clno)->histnum);
#endif
	return(1);
}


int bvs_iac_proc(int cnt, int *pnt, char *str)
{
	*pnt+=3;
	return(0);
}

int bvs_cmd_wordback(int clno)
{
	int	retc;
	int	flags;
	int	curptr;

	flags = 0;
	retc = 0;
	curptr = (bvs_clients+clno)->col;
	while(flags != 2) {
		if (curptr == 0) break;
		if (*(((bvs_clients+clno)->cmdline)+curptr-1) == ' ') {
			if (flags == 0) {
				retc++;
				curptr--;
			} else if (flags == 1) {
				flags = 2;
			}
		} else {
			flags = 1;
			retc++;
			curptr--;
		}
	}
#ifdef DEBUG
	printf("DEBUG: BVS: BackWrod : %d char\n", retc);
#endif
	return(retc);
}

int ins_char(int clno, const char *ch, int cnt, int mode)
{
	int	length;
	int	curcol;
	int	bcnt;
	char	buf[MAXIMUM_CMD_LENGTH];
	char	buf2[MAXIMUM_CMD_LENGTH];

	length = strlen((bvs_clients+clno)->cmdline);
	curcol = (bvs_clients+clno)->col;
#ifdef DEBUG
	printf("DEUBG: INS_CHAR: Begin chars = %d / len = %d / Col = %d\n",
					 cnt, length, curcol);
#endif
	switch(mode) {
	case BVSE_INS:
		if ((length + cnt) > (MAXIMUM_CMD_LENGTH)) {
			bvs_write(clno, "\a");
			cnt = MAXIMUM_CMD_LENGTH - length;
			if (cnt == 0) break;
		}
		memmove((((bvs_clients+clno)->cmdline)+curcol+cnt),
				(((bvs_clients+clno)->cmdline)+curcol),
				length - curcol + 1);
		strncpy((((bvs_clients+clno)->cmdline)+curcol), ch, cnt);

		if ((bvs_clients+clno)->status == BVSC_LOGIN) {
			curcol += cnt;
			break;
		}
		strcpy(buf, (((bvs_clients+clno)->cmdline)+curcol));
		bcnt = strlen((bvs_clients+clno)->cmdline) - curcol - cnt;
		if (bcnt > 0) {
			bzero(buf2, MAXIMUM_CMD_LENGTH);
			memset(buf2, '\b', bcnt);
			strcat(buf, buf2);
		}
		bvs_write(clno, buf);
		curcol += cnt;
		break;

	case BVSE_DEL:
		if (length < (curcol + cnt)) {
			cnt = cnt - (length - curcol);
			bvs_write(clno, "\a");
			if (cnt <= 0) break;
		}
		memmove((((bvs_clients+clno)->cmdline)+curcol),
				(((bvs_clients+clno)->cmdline)+curcol+cnt),
				(length - (curcol + cnt))+1);

		if ((bvs_clients+clno)->status == BVSC_LOGIN) break;
		strcpy(buf, (((bvs_clients+clno)->cmdline)+curcol));
		bzero(buf2, MAXIMUM_CMD_LENGTH);
		memset(buf2, ' ', cnt);
		strcat(buf, buf2);
		bzero(buf2, MAXIMUM_CMD_LENGTH);
		memset(buf2, '\b', 
			strlen((((bvs_clients+clno)->cmdline)+curcol))+cnt);
		strcat(buf, buf2);
		bvs_write(clno, buf);
		break;

	case BVSE_BS:
		if (curcol < cnt) {
			cnt = curcol;
			bvs_write(clno, "\a");
			if (cnt <= 0) break;
		}
		memmove((((bvs_clients+clno)->cmdline)+curcol-cnt),
				(((bvs_clients+clno)->cmdline)+curcol),
				length - curcol + 1);

		if ((bvs_clients+clno)->status == BVSC_LOGIN) {
			curcol -= cnt;
			break;
		}

		bzero(buf, MAXIMUM_CMD_LENGTH);
		memset(buf, '\b', cnt);
		strcat(buf, (((bvs_clients+clno)->cmdline)+curcol)-cnt);
		bzero(buf2, MAXIMUM_CMD_LENGTH);
		memset(buf2, ' ', cnt);
		strcat(buf, buf2);
		bzero(buf2, MAXIMUM_CMD_LENGTH);
		memset(buf2, '\b', 
			strlen((bvs_clients+clno)->cmdline)+(cnt*2)-curcol);
		strcat(buf, buf2);
		bvs_write(clno, buf);

		curcol -= cnt;
		break;
	}
	(bvs_clients+clno)->col = curcol;
#ifdef DEBUG
	printf("DEUBG: INS_CHAR: End chars = %d / len = %d / Col = %d\n",
			 cnt, strlen((bvs_clients+clno)->cmdline), curcol);
#endif
}

int mov_cur(int clno, int vec, int c)
{
	/* vec = 1: Right / 2: left */

	int	curcol;
	int	length;
	char	buf[MAXIMUM_CMD_LENGTH];

#ifdef DEBUG
	printf("DEBUG: Move CUR : Vec=%d / Char=%d\n", vec, c);
#endif
	length = strlen((bvs_clients+clno)->cmdline);
	curcol = (bvs_clients+clno)->col;
	if (vec == 1) {
		if ((curcol+c) > length) {
			c = length - curcol;
		}
		if (c > 0) {
			bzero(buf, MAXIMUM_CMD_LENGTH);
			strncpy(buf, (((bvs_clients+clno)->cmdline)+curcol), c);
			bvs_write(clno, buf);
			curcol += c;
		}
	} else {
		if (curcol < c) {
			c = curcol;
		}
		if (c > 0) {
			bzero(buf, MAXIMUM_CMD_LENGTH);
			memset(buf, '\b', c);
			bvs_write(clno, buf);
			curcol -= c;
		}
	}
	(bvs_clients+clno)->col = curcol;
}

int clearscreen(int clno)
{
	char cls[MAXIMUM_CMD_LENGTH+30];

	cls[0] = 0x1B;
	cls[1] = '[';
	cls[2] = '2';
	cls[3] = 'J';
	cls[4] = 0x1B;
	cls[5] = '[';
	cls[6] = '0';
	cls[7] = ';';
	cls[8] = '0';
	cls[9] = 'H';
	cls[10] = '\0';
	strcat(cls, bvs_prompt);
	strcat(cls, (bvs_clients+clno)->cmdline);

	bvs_write(clno, cls);
	(bvs_clients+clno)->col = strlen((bvs_clients+clno)->cmdline);
}

int bvs_cont_proc()
{
	int ret, retc;
	int clno;

	retc = 0;
	clno = 0;
	while(clno < bvs_maxcon) {

		/* Continue Command Check */
		ret = 0;
		if ((bvs_clients+clno)->cont_code == 1) {
				ret = 
	(*((bvs_clients+clno)->cont_func))(&((bvs_clients+clno)->cont_param));
			if ((ret == 0)||(ret == 2)) {
				(bvs_clients+clno)->cont_code = 0;
				bvs_write(clno, bvs_prompt);
			}
		}
		if (ret == 1) retc = 1;
		clno++;
	}
	return(retc);

}

int bvs_input(fd_set *fds)
{
	int	cnt,cpnt,tgpnt,bcnt;
	char	instr[MAXIMUM_CMD_LENGTH+1];
	char	tgstr[MAXIMUM_CMD_LENGTH+1];
	char	compstr[BVSC_WRCMAX+1];
	char	compline[MAXIMUM_CMD_LENGTH+1];
	int	complen;
	int	readc;
	char	*spp;
	int	complete;
	char	buf[MAXIMUM_CMD_LENGTH*4];
	int	ret;
	
	cnt = 0;
	while(cnt < (bvs_maxcon-1)) {
		/* if ((bvs_clients+cnt)->clsock == (int)NULL) { */
		if ((bvs_clients+cnt)->clsock == 0) {
			cnt++;
			continue;
		}

		/* Continue Command Check */
		if ((bvs_clients+cnt)->cont_code == 1) {
			cnt++;
			continue;
		}

		if (FD_ISSET((bvs_clients+cnt)->clsock, fds)) {
			complete = 0;
			bzero(instr, MAXIMUM_CMD_LENGTH+1);
			readc = read((bvs_clients+cnt)->clsock, 
						&instr, MAXIMUM_CMD_LENGTH);
			if (readc == 0) {
				close((bvs_clients+cnt)->clsock);
				/* (bvs_clients+cnt)->clsock = (int)NULL; */
				(bvs_clients+cnt)->clsock = 0;
				cnt++;
				continue;
			}

			if ((bvs_clients+cnt)->cmd_lock == true) {
				bvs_write(cnt, "\a");
				cnt++;
				continue;
			}

			cpnt = 0;
			tgpnt = 0;

			/*********************
			if ((strlen(instr) + strlen((bvs_clients+cnt)->cmdline))
							> MAXIMUM_CMD_LENGTH) {
				instr[MAXIMUM_CMD_LENGTH -
				    strlen((bvs_clients+cnt)->cmdline)] = '\0';
				bvs_write(cnt, "\a");
			}
			************************/

#ifdef DEBUG
			printf("DEBUG: BVS: Received:");
			fflush(stdout);
#endif
			bzero(tgstr, MAXIMUM_CMD_LENGTH+1);
			/* while(instr[cpnt] != (int)NULL) { */
			while(instr[cpnt] != 0) {
#ifdef DEBUG
				printf("%02X", instr[cpnt]);
				fflush(stdout);
#endif
				if ((bvs_clients+cnt)->esc == 1) {
					switch(instr[cpnt]) {
					case '[':
						(bvs_clients+cnt)->esc = 2;
						break;
					default:
						(bvs_clients+cnt)->esc = 0;
					}
				} else if ((bvs_clients+cnt)->esc == 2) {
					switch(instr[cpnt]) {
					case 'A':
						instr[cpnt] = 0x10;
						break;
					case 'B':
						instr[cpnt] = 0x0E;
						break;
					case 'C':
						instr[cpnt] = 0x06;
						break;
					case 'D':
						instr[cpnt] = 0x02;
						break;
					}
					(bvs_clients+cnt)->esc = 0;
				}
				if ((bvs_clients+cnt)->esc != 0) {
					cpnt++;
					continue;
				}
				switch(instr[cpnt]) {
				case -1:
					/* TELNET IAC Detection */
					bvs_iac_proc(cnt, &cpnt, instr);
					break;
				case 0x1B:
					/* Escape */
					(bvs_clients+cnt)->esc = 1;
					cpnt++;
					break;
				case '?':
					if ((bvs_clients+cnt)->status ==
								BVSC_LOGIN) {
						cpnt++;
						break;
					}
					/* auto complete */
					complen = bvs_cmd_pr_help(cnt,
						(bvs_clients+cnt)->cmdline);
					if (complen > 0) {
						strcpy(buf, bvs_prompt);
						strcat(buf, 
						    (bvs_clients+cnt)->cmdline);
						bvs_write(cnt, buf);
						(bvs_clients+cnt)->col = 
						strlen((bvs_clients+cnt)->cmdline);
					} else {
						bvs_write(cnt, "\a");
					}
					cpnt++;
					break;
				case '\r':
				case '\n':
					/* CR Detection */
					complete = 1;
					instr[cpnt] = '\0';
					break;
				case '\t':
					/* Auto Complete */
					if ((bvs_clients+cnt)->status ==
								BVSC_LOGIN) {
						cpnt++;
						break;
					}
					/* auto complete */
					if (strlen((bvs_clients+cnt)->cmdline) >
							(bvs_clients+cnt)->col) {
						strcpy(buf, "\r\n");
						strcat(buf, bvs_prompt);
						strcat(buf, 
						    (bvs_clients+cnt)->cmdline);
						bvs_write(cnt, buf);
						(bvs_clients+cnt)->col = 
						strlen((bvs_clients+cnt)->cmdline);
						bvs_write(cnt, "\a");
						break;
					}

					complen = bvs_cmd_auto_complete(
					  	(bvs_clients+cnt)->cmdline,
					  	compstr);
					if (complen == 0) {
						/* Beep Send */
						bvs_write(cnt, "\a");
					} else {
						ins_char(cnt, compstr, 
							strlen(compstr), BVSE_INS);
					}
					cpnt++;
					break;
				case 0x7F:
				case '\b':
					/* Back Space Detection */
					ins_char(cnt, NULL, 1, BVSE_BS);
					cpnt++;
					break;
				case 0x15:
					/* Line Clear (Ctrl-u) */
					bzero(buf, MAXIMUM_CMD_LENGTH*3);
					complen=strlen((bvs_clients+cnt)->cmdline);
					memset(buf, '\b', 
					      ((bvs_clients+cnt)->col)+complen*2);
					memset((buf+((bvs_clients+cnt)->col)), 
								' ', complen);
					bvs_write(cnt, buf);
					bzero((bvs_clients+cnt)->cmdline,
							MAXIMUM_CMD_LENGTH);
					(bvs_clients+cnt)->col = 0;
					cpnt++;
					break;
				case 0x17:
					/* Word Clear (Ctrl-w) */
					complen = bvs_cmd_wordback(cnt);
					ins_char(cnt, NULL, complen, BVSE_BS);
					cpnt++;
					break;
				case 0x10:
					/* History Before (Ctrl-p) */
					readc = strlen((bvs_clients+cnt)->cmdline);
#ifdef DEBUG
					printf("DEBUG: BVS: Delete C = %d\n", readc);
#endif
					complen = bvs_cmd_history(cnt, 1,
						 (bvs_clients+cnt)->cmdline);
					if (complen == 0) {
						bvs_write(cnt, "\a");
					} else {
						bzero(buf, MAXIMUM_CMD_LENGTH*4);
						memset(buf, '\b', readc*3);
						memset((buf+readc),' ',readc);
						strcat(buf,
						      (bvs_clients+cnt)->cmdline);
						bvs_write(cnt, buf);
						(bvs_clients+cnt)->col =
						strlen((bvs_clients+cnt)->cmdline);
					}
					cpnt++;
					break;
				case 0x0E:
					/* History After (Ctrl-n) */
					readc = strlen((bvs_clients+cnt)->cmdline);
					complen = bvs_cmd_history(cnt, 2,
						 (bvs_clients+cnt)->cmdline);
					if (complen == 0) {
						bvs_write(cnt, "\a");
					} else {
						bzero(buf, MAXIMUM_CMD_LENGTH*4);
						memset(buf, '\b', readc*3);
						memset((buf+readc),' ',readc);
						strcat(buf,
						      (bvs_clients+cnt)->cmdline);
						bvs_write(cnt, buf);
						(bvs_clients+cnt)->col =
						strlen((bvs_clients+cnt)->cmdline);
					}
					cpnt++;
					break;
				case 0x02:
					/* Move cur left */
					mov_cur(cnt, 2, 1);
					cpnt++;
					break;
				case 0x06:
					/* Move cur right */
					mov_cur(cnt, 1, 1);
					cpnt++;
					break;
				case 0x01:
					/* Move Cur Top (Ctrl-a) */
					mov_cur(cnt, 2, (bvs_clients+cnt)->col);
					cpnt++;
					break;
				case 0x04:
					/* Delete Char (Ctrl-d) */
					ins_char(cnt, NULL, 1, BVSE_DEL);
					cpnt++;
					break;
				case 0x05:
					/* Move Cur End (Ctrl-e) */
					mov_cur(cnt, 1, 
					  (strlen((bvs_clients+cnt)->cmdline))
					  -((bvs_clients+cnt)->col));
					cpnt++;
					break;
				case 0x0B:
					/* Clear from cursor (Ctrl-k) */
					ins_char(cnt, NULL,
					strlen((bvs_clients+cnt)->cmdline) -
					(bvs_clients+cnt)->col, BVSE_DEL);
					cpnt++;
					break;
				case 0x0C:
					/* Clear Screen (Ctrl-l) */
					clearscreen(cnt);
					cpnt++;
					break;
				default:
					if ((instr[cpnt] >= 0x20) &&
					    		(instr[cpnt] <= 0x7e)) {
					    tgstr[0] = instr[cpnt];
					    tgstr[1] = '\0';
					    ins_char(cnt, tgstr, 1, BVSE_INS);
					}
					cpnt++;
				}
			}
#ifdef DEBUG
			printf("\n");
			fflush(stdout);
#endif

			if (complete == 1) bvs_exec_command(cnt);
			break;
		}

		cnt++;
	}
	return(0);
}

void bvs_vtymodeset(int clno)
{
	char cmd1[] = { IAC, WILL, TELOPT_ECHO, '\0' };
	char cmd2[] = { IAC, WILL, TELOPT_SGA, '\0' };
	char cmd3[] = { IAC, DONT, TELOPT_LINEMODE, '\0' };
	char cmd4[] = { IAC, DO, TELOPT_NAWS, '\0' };
	char cmd5[] = { IAC, DONT, TELOPT_LFLOW, '\0' };

	bvs_write(clno, cmd1);
	bvs_write(clno, cmd2);
	bvs_write(clno, cmd3);
	bvs_write(clno, cmd4);
/* 	bvs_write(clno, cmd5); */
}

int bvs_open_new_shell(fd_set *fds)
{
	struct sockaddr_in	sin;
	int			fromlen;
	register int		ns;
	int 			cnt;

	if (FD_ISSET(bvs_svsoc, fds)) {
		/* accept connection */
		pr_log(VLOG_GLOB, LOG_ALL,
			"BVS: Server Socket Connection Request Received\n", ON);
#ifdef DEBUG
		printf("DEBUG: BVS: Accepting\n");
		printf("DEBUG: BVS: Before Socket: %d\n", bvs_svsoc);
#endif
		fromlen = sizeof(sin);
		if ((ns = accept(bvs_svsoc, (struct sockaddr *)&sin,
							&fromlen)) < 0) {
			perror("bvs server: accept");
			return(0);
		}
		sprintf(logstr, "  BVS Requesting Neighbor = %s\n",
						inet_ntoa(sin.sin_addr));
		pr_log(VLOG_GLOB, LOG_ALL,  logstr, OFF);
		
		/* Initialize New Client */
		cnt = 0;
		while(cnt < (bvs_maxcon-1)) {
			/* if ((bvs_clients+cnt)->clsock == (int)NULL) break;*/
			if ((bvs_clients+cnt)->clsock == 0) break;
			cnt++;
		}
		if (cnt < (bvs_maxcon-1)) {
			bzero((bvs_clients + cnt), sizeof(BVS_CLIENT));
			(bvs_clients + cnt)->clsock = ns;
			(bvs_clients + cnt)->addr = 
					ntohl((u_long)sin.sin_addr.s_addr);
			(bvs_clients + cnt)->status = BVSC_LOGIN;
			(bvs_clients + cnt)->cmd_lock = false;
			gettimeofday(&((bvs_clients + cnt)->lgtime),
						(struct timezone *)0);
			(bvs_clients + cnt)->col = 0;
			(bvs_clients + cnt)->rowsize = bvs_rowsize;
			(bvs_clients + cnt)->userlevel = DEFUSERLEVEL;
			sprintf(bvs_prompt, "%s%c ", sysname, 
			     bvs_promptmark[(bvs_clients + cnt)->userlevel]);
		} else {
#ifdef DEBUG
			printf("DEBUG: BVS: Client Socket Maximum\n");
#endif
			pr_log(VLOG_GLOB, LOG_ALL, "BVS Client Over\n", ON);
			write(ns, "BGPView: No More Sockets\r\n", 25);
			close(ns);
			return(0);
		}
	} else {
		return(0);
	}

	/* VTY Mode Change */
	bvs_vtymodeset(cnt);

	/* BGPView Version Print */
	bvs_write(cnt, "\r\nBGPView Version ");
	bvs_write(cnt, BV_VERSION);
	bvs_write(cnt, "\r\n");
	bvs_write(cnt, BV_COPYRIGHT);
	bvs_write(cnt, "\r\n\r\n");

	/* Password Prompt */
	bvs_write(cnt, "Password:");
	return(1);
}

