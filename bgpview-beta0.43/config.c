/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1998/1                          */
/*                                                              */
/* Start Configuration Routine                                  */
/****************************************************************/
/* config.c,v 1.3 2005/04/27 07:43:43 kuniaki Exp */

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

#include "bgp.h"
#include <syslog.h>
#include <ctype.h>
#include "bgplog.h"
#include "bgpdata.h"
#include "bvshell.h"
#include "dummyroute.h"
#include "mailannounce.h"
#include "timer.h"
#include "mpnlri.h"

int check_nlritype(char *nlristr)
{
	int	cnt;
	int	ret;
	char	nlstr[LOGSTRSIZE];

	strcpy(nlstr, nlristr);
	cnt = 0;
	while(nlstr[cnt] != '\0') {
		nlstr[cnt] = toupper(nlstr[cnt]);
		cnt++;
	}
	
	if (strcmp(nlstr, "IPV4_UNICAST") == 0) {
		ret = IPv4_UNICAST;
	} else if (strcmp(nlstr, "IPV6_UNICAST") == 0) {
		ret = IPv6_UNICAST;
	} else {
		ret = 0;
	}
	return(ret);
}

int read_conf(fname)
char *fname;
{
	FILE 		*fp;
	char 		readbuf[256];
	char 		*cmd, *val;
	char		strbuf[32];
	int  		cnt;
	int  		pn;
	struct in_addr	pin;

	IDENT = 0;
	LOCAL_AS = 0;

	if (fname == NULL) {
		printf("Configuration file Not Specified\n");
		return 1;
	}
	if ((fp = fopen(fname, "r")) == NULL) {
		printf("Configuration file Not Found: %s\n", fname);
		return 1;
	} else {
		printf("Reading Startup Configuration\n");
	}

	pn = -1;
	while(!feof(fp)) {
		bzero(readbuf, 255);
		cmd = fgets(readbuf, 255, fp);
#ifdef DEBUG
		printf("DEBUG CONF: %s",readbuf);
#endif
		if (strlen(readbuf) >= 255) {
			printf("Statement Too Long.\n");
			continue;
		}
		cmd = strtok(readbuf, " \t\r\n");
		if (cmd == NULL) continue;
		if (*cmd == '#') continue;
		val = strtok(NULL, " \t\r\n");

		cnt = 0;
		while(*(cmd+cnt) != '\0') {
			*(cmd+cnt) = toupper(*(cmd+cnt));
			cnt++;
		}
#ifdef DEBUG
		if (val == NULL) {
			printf("CHECKING COMMAND = %s / VALUABLE = (NULL)\n", cmd);
		} else {
			printf("CHECKING COMMAND = %s / VALUABLE = %s\n", cmd, val);
		}
#endif
		if (strcmp("LOCAL_AS", cmd) == 0) {
			if (val == NULL) continue;
			/* sscanf(val, "%hu", &LOCAL_AS); */
			/* LOCAL_AS = fourbyteastoulong(val); */
			sscanf(val, "%lu", &LOCAL_AS);
		} else if (strcmp("IDENTIFIER", cmd) == 0) {
			if (val == NULL) continue;
			inet_aton(val, &pin);
			IDENT = pin.s_addr;
		} else if (strcmp("SHELL_PORT", cmd) == 0) {
			if (val == NULL) continue;
			bvs_init_set_values(SET_BVS_PORT, val);
		} else if (strcmp("SHELL_MAXCON", cmd) == 0) {
			if (val == NULL) continue;
			bvs_init_set_values(SET_BVS_MAXCON, val);
		} else if (strcmp("SHELL_PASSWD", cmd) == 0) {
			if (val == NULL) continue;
			bvs_init_set_values(SET_BVS_PASSWD, val);
		} else if (strcmp("SHELL_MORE", cmd) == 0) {
			if (val == NULL) continue;
			bvs_init_set_values(SET_BVS_ROWSIZE, val);
#ifdef RTHISTORY
		} else if (strcmp("RTHISTORYCLR", cmd) == 0) {
			if (val == NULL) continue;
			sscanf(val, "%ld", &rthistcleartime.tv_sec);
			rthistcleartime.tv_usec = 0;
		} else if (strcmp("V6RTHISTORYCLR", cmd) == 0) {
			if (val == NULL) continue;
			sscanf(val, "%ld", &rthistcleartime_v6.tv_sec);
			rthistcleartime_v6.tv_usec = 0;
#endif
#ifdef IRRCHK
		} else if (strcmp("IRR_CHECK", cmd) == 0) {
			if (val == NULL) continue;
			set_irrparam(SET_IRR_DISABLE, val);
		} else if (strcmp("IRR_HOST", cmd) == 0) {
			if (val == NULL) continue;
			set_irrparam(SET_IRR_HOST, val);
		} else if (strcmp("IRR_PORT", cmd) == 0) {
			if (val == NULL) continue;
			set_irrparam(SET_IRR_PORT, val);
#endif
		} else if (strcmp("CRON", cmd) == 0) {
			if (val == NULL) continue;
			set_cronfile(val);
		} else if (strcmp("SENDMAIL_CMD", cmd) == 0) {
			if (val == NULL) continue;
			strcpy(sendmail_command, val);
		} else if (strcmp("ADMIN_ADDR", cmd) == 0) {
			if (val == NULL) continue;
			strcpy(admin_addr, val);
		} else if (strcmp("NOTIFY_ADDR", cmd) == 0) {
			if (val == NULL) continue;
			strcpy(mail_addr, val);
		} else if (strcmp("SPOOL_DIR", cmd) == 0) {
			if (val == NULL) continue;
			strcpy(spooldir, val);
		} else if (strcmp("SYSNAME", cmd) == 0) {
			if (val == NULL) continue;
			strcpy(sysname, val);
		} else if (strcmp("DUMMY_ROUTE_CFG", cmd) == 0) {
			if (val == NULL) continue;
			strcpy(dmyroutef, val);
		} else if (strcmp("SYSTEM_LOG", cmd) == 0) {
			if (val == NULL) {
				facility = -1;
				continue;
			}
			strcpy(syslogname, val);
			if (syslogname[0] == '@') {
				bzero(strbuf, 32);
				cnt = 0;
				while(syslogname[cnt+1] != '\0') {
					strbuf[cnt] =
						tolower(syslogname[cnt+1]);
					cnt++;
				}
				facility = LOG_LOCAL0;
				if (strcmp(strbuf, "local1") == 0) {
					facility = LOG_LOCAL1;
				} else if (strcmp(strbuf, "local2") == 0) {
					facility = LOG_LOCAL2;
				} else if (strcmp(strbuf, "local3") == 0) {
					facility = LOG_LOCAL3;
				} else if (strcmp(strbuf, "local4") == 0) {
					facility = LOG_LOCAL4;
				} else if (strcmp(strbuf, "local5") == 0) {
					facility = LOG_LOCAL5;
				} else if (strcmp(strbuf, "local6") == 0) {
					facility = LOG_LOCAL6;
				} else if (strcmp(strbuf, "local7") == 0) {
					facility = LOG_LOCAL7;
				} else {
					facility = -1;
				}
#ifdef DEBUG
				printf("DEBUG CONF: Syslog Facility : %04X\n",
								facility);
#endif
			} else facility = -1;
			open_syslog(syslogname);
		} else if (strcmp("NEIGHBOR", cmd) == 0) {
			if(++pn >= MAXPEERS) {
				printf(
				"Configuration file is in invalid format.\n");
				return 1;
			}
			bzero(&peer[pn], sizeof(PEERS));
			peer[pn].peerstatus= true;
			peer[pn].as4_enable= false;
			peer[pn].as4_remote= false;
			peer[pn].data	   = NULL;
			peer[pn].neighbor  = 0;
			peer[pn].remote_as = 0;
			peer[pn].remote_as4= 0;
			peer[pn].datalen   = 0;
			peer[pn].soc       = -1;
			peer[pn].flag	   = 0;
			peer[pn].nlri      = 0;
			peer[pn].status    = IDLE;
			peer[pn].event     = BGP_START;
			peer[pn].holdtimer = HOLD_TIMER;
			peer[pn].keepalive = KEEPALIVE_TIMER;
			peer[pn].holdtimer_conf = HOLD_TIMER;
			peer[pn].keepalive_conf = KEEPALIVE_TIMER;
			peer[pn].resume    = OFF;
			peer[pn].logname[0]= '\0';
			peer[pn].logfd	   = NULL;
			peer[pn].prefixsum = false;
			peer[pn].port	   = BGP_PORT;
#ifdef TCP_MD5SIG
			peer[pn].md5sig	   = false;
			peer[pn].md5pass[0]= '\0';
#endif
			peer[pn].session_re_init_timer = RE_INITIAL_TIMER;
			LG_ZERO(peer[pn].logtype);
			peer[pn].logtype   = 0x00;
			clear_info(pn);
		} else if (strcmp("DESCRIPTION", cmd) == 0) {
			if (val == NULL) continue;
			strncpy(peer[pn].description, val, DESCSIZE-1);
		} else if (strcmp("PEER_ADMIN", cmd) == 0) {
			if (val == NULL) continue;
			strncpy(peer[pn].admin_addr, val, FNAMESIZE-1);
		} else if (strcmp("4OCTETS_AS", cmd) == 0) {
			if (val == NULL) continue;
			if (strcmp("ON", val) == 0) {
				peer[pn].as4_enable = true;
			}
		} else if (strcmp("ADDRESS", cmd) == 0) {
			if (val == NULL) continue;
			inet_aton(val, &pin);
			peer[pn].neighbor = pin.s_addr;
		} else if (strcmp("REMOTE_AS", cmd) == 0) {
			if (val == NULL) continue;
			/* sscanf(val, "%hu", &peer[pn].remote_as); */
			/* peer[pn].remote_as = fourbyteastoulong(val); */
			sscanf(val, "%lu", &peer[pn].remote_as);
		} else if (strcmp("IND_IDENT", cmd) == 0) {
			if (val == NULL) continue;
			inet_aton(val, &pin);
			peer[pn].local_id = pin.s_addr;
		} else if (strcmp("IND_AS", cmd) == 0) {
			if (val == NULL) continue;
			/* sscanf(val, "%hu", &peer[pn].local_as); */
			/* peer[pn].local_as = fourbyteastoulong(val); */
			sscanf(val, "%lu", &peer[pn].local_as);
		} else if (strcmp("HOLD_TIMER", cmd) == 0) {
			if (val == NULL) continue;
			sscanf(val, "%d", &cnt);
			if ((cnt == 1) && (cnt == 2)) {
				printf("Illegal Hold Timer Value - Set Software Default\n");
			} else {
				peer[pn].holdtimer = cnt;
				peer[pn].holdtimer_conf = cnt;
				/*** Keepalive Timer is calculated by hold timer */
				if (cnt == 0) {
					peer[pn].keepalive = 0;
					peer[pn].keepalive_conf = 0;
				} else {
					peer[pn].keepalive = (int)(cnt/3);
					peer[pn].keepalive_conf = (int)(cnt/3);
				}
			}
		/****** Keepalive Timer Configuration Deleted 2005/11/16 ****
		} else if (strcmp("KEEPALIVE", cmd) == 0) {
			if (val == NULL) continue;
			sscanf(val, "%d", &peer[pn].keepalive);
		**************************************************************/
		} else if (strcmp("NLRI", cmd) == 0) {
			peer[pn].nlri = check_nlritype(val);
		} else if (strcmp("LOGNAME", cmd) == 0) {
			if (val == NULL) continue;
			strcpy(peer[pn].logname, val);
			open_log(val, pn);
		} else if (strcmp("VIEW", cmd) == 0) {
			if (val == NULL) continue;
			cnt = 0;
			while(*(val+cnt) != '\0') {
				*(val+cnt) = toupper(*(val+cnt));
				cnt++;
			}
			if (strcmp("OPEN", val) == 0) {
				LG_SET(VLOG_OPEN, peer[pn].logtype);
			} else if (strcmp("UPDATE", val) == 0) {
				LG_SET(VLOG_UPDE, peer[pn].logtype);
			} else if (strcmp("NOTIFICATION", val) == 0) {
				LG_SET(VLOG_NOTI, peer[pn].logtype);
			} else if (strcmp("KEEPALIVE", val) == 0) {
				LG_SET(VLOG_KEEP, peer[pn].logtype);
			} else if (strcmp("SYSTEM", val) == 0) {
				LG_SET(VLOG_GLOB, peer[pn].logtype);
			} else if (strcmp("UPDATTR", val) == 0) {
				LG_SET(VLOG_UPAT, peer[pn].logtype);
			} else if (strcmp("UPDATEDUMP", val) == 0) {
				LG_SET(VLOG_UPDU, peer[pn].logtype);
			}
		} else if (strcmp("ANNOUNCE", cmd) == 0) {
			if (val == NULL) continue;
			cnt = 0;
			while(*(val+cnt) != '\0') {
				*(val+cnt) = toupper(*(val+cnt));
				cnt++;
			}
			if (strcmp("STATUS_CHANGE", val) == 0) {
				peer[pn].status_notify = true;
			}
		} else if (strcmp("INFO", cmd) == 0) {
			if (val == NULL) continue;
			strcpy(bgpinfo[pn].fname ,val);
			if (strlen(bgpinfo[pn].fname) == 0) {
				bgpinfo[pn].infostatus = false;
			}
		} else if (strcmp("INTERVAL_INFO", cmd) == 0) {
			if (val == NULL) continue;
			sscanf(val, "%d", &bgpinfo[pn].interval);
			if (bgpinfo[pn].interval == 0) {
				bgpinfo[pn].infostatus = false;
			}
		} else if (strcmp("PREFIXOUT", cmd) == 0) {
			if (val == NULL) continue;
			strcpy(peer[pn].prefixout ,val);
		} else if (strcmp("PREFIXOUTSUM", cmd) == 0) {
			if (val == NULL) continue;
			cnt = 0;
			while(*(val+cnt) != '\0') {
				*(val+cnt) = toupper(*(val+cnt));
				cnt++;
			}
			if (strcmp("ON", val) == 0) {
				peer[pn].prefixsum = true;
			} else if (strcmp("OFF", val) == 0) {
				peer[pn].prefixsum = false;
			}
		}
#ifdef TCP_MD5SIG
		else if (strcmp("MD5PASS", cmd) == 0) {
			if (val == NULL) continue;
			peer[pn].md5sig = true;
			bzero(peer[pn].md5pass, MD5SIG_PASWORD_MAXLEN);
			strncpy(peer[pn].md5pass, val, MD5SIG_PASWORD_MAXLEN);
		}
#endif
		else if (strcmp("RECONNECTWAIT", cmd) == 0) {
			if (val == NULL) continue;
			sscanf(val, "%d", &peer[pn].session_re_init_timer);
		}
#ifdef IRRCHK
		else if (strcmp("IRRCOMPARE", cmd) == 0) {
			if (val == NULL) continue;
			set_irr_peerparam(SET_P_IRRCOMPARE, val, pn);
		}
		else if (strcmp("IRRNOTIFY", cmd) == 0) {
			if (val == NULL) continue;
			set_irr_peerparam(SET_P_IRRNOTIFY, val, pn);
		}
		else if (strcmp("IRRNOTELEVEL", cmd) == 0) {
			if (val == NULL) continue;
			set_irr_peerparam(SET_P_IRRNOTELEVEL, val, pn);
		}
#endif /* IRRCHK */
	}
	fclose(fp);
	if (pn == -1) {
		printf("Peer is not configured\n");
		return 1;
	} else {
		return 0;
	}
}
