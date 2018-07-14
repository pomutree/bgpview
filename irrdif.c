/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo, Atelier Mahoroba 2006/08           */
/*                                                              */
/* IRRd Interface                                               */
/****************************************************************/
/* irrdif.c,v 1.12 2010/11/14 09:33:57 kuniaki Exp */

/*
 * Copyright (c) 2006 Atelier Mahoroba
 * All rights Reserved.
 */

#ifdef IRRCHK

#include "irrdif.h"

#include "bgplog.h"

char	irrhost[FNAMESIZE];
int	irrport;
int	irrsock;

int	irr_checking_status;
int	irr_chk_timer_id;

int      irr_check_enable;	/* 1: ON, 0:OFF */

void init_irrif()
{
	struct timeval tmval;

	strcpy(irrhost, DEF_IRR_HOST);
	irrport = DEF_IRR_PORT;

	/* Start Walk Timer */
	tmval.tv_sec  = IRR_WALK_TIMER;
	tmval.tv_usec = 0;
	irr_chk_timer_id = 
	      common_entry_timer(&tmval, IRRCHK_ACTIVE, activate_bgp_chk_irr);
	irr_checking_status = IRRCHK_TIMERWAIT;
	irr_check_enable = 1;
}

int set_irrparam(int type, char *val)
{
	int	ret;

	ret = 0;
#ifdef DEBUG
	printf("DEBUG: Set IRR Param  Type = %d / Val = %s\n", type, val);
#endif
	switch(type) {
	case SET_IRR_DISABLE:
		if (strncmp(val, "OFF", 5) == 0) {
			irr_check_enable = 0;
			common_off_timer(irr_chk_timer_id);
			irr_chk_timer_id = 0;
		}
		break;

	case SET_IRR_HOST:
		bzero(irrhost, FNAMESIZE);
		strncpy(irrhost, val, FNAMESIZE-1);
		ret = 1;
		break;
	
	case SET_IRR_PORT:
		irrport = atoi(val);
		ret = 1;
		break;
	}
#ifdef DEBUG
	printf("DEBUG: Set Data : irrhost = %s / irrport = %d\n",
			irrhost, irrport);
#endif

	return(ret);
}

int set_irr_peerparam(int type, char *val, int pn)
{
	int	ret;

	ret = 0;
#ifdef DEBUG
	printf("DEBUG: Set IRR Param for Peer : Type = %d / Val = %s\n",
						type, val);
#endif

	switch(type) {
	case SET_P_IRRCOMPARE:
		if (strcmp("OFF", val) == 0) {
			peer[pn].irr_compare = false;
		} else {
			peer[pn].irr_compare = true;
		}
		break;
			
	case SET_P_IRRNOTIFY:
		strncpy(peer[pn].irr_notify, val, FNAMESIZE-1);
		break;

	case SET_P_IRRNOTELEVEL:
		peer[pn].irr_notelevel = atoi(val);
		break;

	default:
		peer[pn].irr_notelevel = 0;
		peer[pn].irr_compare = false;
	}

	return(ret);
}

int open_irr(char *host, int port)
{
	struct hostent		*he;
	struct sockaddr_in	sin;
	unsigned int		**adrptr;

	static char	multicmd[] = "!!\n";

#ifdef DEBUG
	printf("DEBUG: Connect IRR = %s(%d)\n", host, port);
#endif
	
	irrsock = socket(AF_INET, SOCK_STREAM, 0);
	sin.sin_family = AF_INET;
	sin.sin_port   = htons(port);

	sin.sin_addr.s_addr = inet_addr(host);
	if (sin.sin_addr.s_addr == 0xffffffff) {

		if ((he = gethostbyname(host)) == NULL) {
			close(irrsock);
			pr_log2(VLOG_GLOB, LOG_ALL, ON,
				"Unknown IRR host : %s\n", host);
			return(1);
		}

		adrptr = (unsigned int **)he->h_addr_list;
		while(*adrptr != NULL) {
			sin.sin_addr.s_addr = *(*adrptr);
			if (connect(irrsock,
				(struct sockaddr *)&sin, sizeof(sin)) == 0) {
				break;
			}
		}
	} else {
		if (connect(irrsock,(struct sockaddr *)&sin,sizeof(sin)) != 0) {
			perror("connect");
			close(irrsock);
			return(1);
		}
	}

	/* Send Multi Command */
	if (sockwrite(irrsock, multicmd, 3) == 3) {
		return(0);
	} else {
		printf("IRR Socket Open Error\n");
		close(irrsock);
		return(1);
	}
}

int close_irr()
{
	static char fincmd[] = "!q\n";

	if (irrsock != 0) {
		sockwrite(irrsock, fincmd, 3);
		close(irrsock);
		irrsock = 0;
	}
}

int irr_search_route(struct in_addr *in, u_short plen)
{
	char	s_str[FNAMESIZE];

	sprintf(s_str, "!r%s/%d,l\n", inet_ntoa(*in), plen);

	if (sockwrite(irrsock, s_str, strlen(s_str)) != strlen(s_str)) {
		pr_log2(VLOG_GLOB, LOG_ALL, ON,
				"IRR Search Command Send Error\n");
		return(1);
	} else {
		return(0);
	}
}

int sread(char *buf, int len)
{
        int     	cnt;
        int    		rel;
	struct timeval	tval;
	fd_set		irrset;
	int		readzerocnt;

	tval.tv_sec  = IRR_SEARCH_TIMEOUT;
	tval.tv_usec = 0;
        cnt = 0;
	readzerocnt = 0;
	if (irrsock == 0) return(0);
        bzero(buf, len);
        while(cnt < len) {
		FD_ZERO(&irrset);
		FD_SET(irrsock, &irrset);
		if (select(FD_SETSIZE, &irrset, NULL, NULL, &tval) < 0) {
			if (errno == EINTR) {
				printf("Select Return Code EINTR on irrdata\n");
				continue;
			}
			perror("select on irrdata");
			pr_log2(VLOG_GLOB, LOG_ALL, ON,
				"Select Error on IRR Check\n");
			return(0);
		}
		if (FD_ISSET(irrsock, &irrset)) {
                	rel = read(irrsock, (char *)(buf + cnt), (len-cnt));
			if (rel == 0) {
				if (readzerocnt > 100) {
					/* read fail */
					return(0);
				} else {
					readzerocnt++;
				}
			} else {
				readzerocnt = 0;
			}
                	cnt += rel;
#ifdef DEBUG
                	printf("DEBUG: Reading %d/%d/%d\n",rel,cnt,len);
                	fflush(stdout);
#endif
		} else {
			/* Data Read Receive Timeout */
			return(0);
		}
        }
        *(buf+len) = '\0';
#ifdef DEBUG
        printf("%d:%s", strlen(buf), buf);
#endif
	return(cnt);
}

int getcontrol(char *type)
{
        char    	line[STRBUFLEN];
        char    	c;
        int     	cnt;
        int     	ret;
	struct timeval	tval;
	fd_set		irrset;

	tval.tv_sec  = IRR_SEARCH_TIMEOUT;
	tval.tv_usec = 0;
	if (irrsock == 0) return(0);
        bzero(line, STRBUFLEN);
        cnt = 0;
        while(1) {
		FD_ZERO(&irrset);
		FD_SET(irrsock, &irrset);
		if (select(FD_SETSIZE, &irrset, NULL, NULL, &tval) < 0) {
			if (errno == EINTR) {
				printf("Select Return Code EINTR on irr\n");
				continue;
			}
			perror("select on irr");
			pr_log2(VLOG_GLOB, LOG_ALL, ON, 
				"Select Error on IRR Check\n");
			return(0);
		}
#ifdef DEBUG
		printf("DEBUG: irrif.c: getcontrol: select return = %d\n",ret);
#endif
		if (FD_ISSET(irrsock, &irrset)) {
	                if (read(irrsock, &c, 1) == 0) {
				/* Socket Broken? */
				close(irrsock);
				irrsock = 0;
				return(0);
			}
                	if (c == '\n') break;
                	if (c == '\r') break;
                	if (cnt == -1) break;
                	if (cnt >= STRBUFLEN) continue;
                	line[cnt] = c;
                	cnt++;
		} else {
			/* Control Receive Timeout */
			return(0);
		}
        }

        ret = 0;
        switch(line[0]) {
        case 'A':
                *type = 'A';
                ret = atoi(&line[1]);
                break;
        case 'B':
                *type = 'B';
                break;
        case 'C':
                *type = 'C';
                ret = -1;
                break;
        case 'D':
                *type = 'D';
                ret = -1;
                break;
        case 'E':
                *type = 'E';
                break;
        case 'F':
                *type = 'F';
                break;
        default:
                *type = 'Z';
                ret = 0;
        }
        return(ret);
}

char *readdata()
{
        char    type;
        int     result;
        char    *data;
        int     stop;
        int     getdata;

        data = NULL;
        stop = 0;
        getdata = 0;
        while(stop == 0){
                result = getcontrol(&type);

#ifdef DEBUG
                printf("DATA Type %c\n",type);
#endif

                switch(type) {
                case 'A':
#ifdef DEBUG
                        printf("reading data = %d byte\n",result);
#endif
                        data = (char *)malloc(result+1);
#ifdef DEBUG
                        printf("MALLOCED ADDRESS:0x%08X\n", data);
#endif
                        if (sread(data, result) < result) {
				/* Receive In-Complete */
				getdata = 0;
			} else {
                        	getdata = 1;
			}
                        break;

                case 'B':
                        stop = 1;
                        break;
                case 'C':
#ifdef DEBUG
                        printf("Complete\n");
#endif
                        stop = 1;
                        break;
                case 'D':
                        stop = 1;
                        break;
                case 'E':
                        stop = 1;
                        break;
                case 'F':
                        stop = 1;
                        break;
                default:
                        stop = 1;
                        break;
                }
        }

        return(data);
}

void sepprefix(struct in_addr *prefix, u_short *plen,char *descs)
{
	char	*strptr;
	char	strbuf[STRBUFLEN];

	bzero(strbuf, STRBUFLEN);
	strncpy(strbuf, descs, STRBUFLEN-1);
	if ((strptr = strchr(strbuf, '/')) != NULL) {
		*strptr = '\0';
		inet_aton(strbuf, prefix);
		*plen = atoi(strptr+1);
#ifdef DEBUG2
		printf("DEBUG: Prefix Separation : %s / %s\n", strbuf,strptr+1);
#endif
	} else {
		inet_aton(strbuf, prefix);
		if ((ntohl(prefix->s_addr) & 0x80000000) == 0) {
			/* Class A */
			*plen = 8;
#ifdef DEBUG2
		printf("DEBUG: Prefix Separation Class A : %s\n", strbuf);
#endif
		} else if ((ntohl(prefix->s_addr) & 0xC0000000) == 0x80000000) {
			/* Class B */
			*plen = 16;
#ifdef DEBUG2
		printf("DEBUG: Prefix Separation Class B : %s\n", strbuf);
#endif
		} else if ((ntohl(prefix->s_addr) & 0xE0000000) == 0xC0000000) {
			/* Class C */
			*plen = 24;
#ifdef DEBUG2
		printf("DEBUG: Prefix Separation Class C : %s\n", strbuf);
#endif
		} else {
			/* Class D or E */
			*plen = 28;
		}
	}
#ifdef DEBUG2
	printf("DEBUG: sepprefix: %s/%d\n", inet_ntoa(*prefix), *plen);
	pr_log2(VLOG_GLOB, LOG_ALL, ON,
		"DEBUG: sepprefix: %s/%d\n", inet_ntoa(*prefix), *plen);
#endif
}

IRR_ENTRY *get_items(char *data)
{
	IRR_ENTRY	*irrentry;
	char		*descs, *desce;
	int		flg;
	

	irrentry = (IRR_ENTRY *)malloc(sizeof(IRR_ENTRY));
	if (irrentry == NULL) {
		pr_log2(VLOG_GLOB, LOG_ALL, ON,
				"IRR Data Entry Malloc Error\n");
		return(NULL);
	}
	bzero(irrentry, sizeof(IRR_ENTRY));

        if (data != NULL) {
                descs = data;
                while(*descs != '\0') {
                        flg = 0;
                        if (strstr(descs, "origin:") == descs) {
                                descs += 7;
                                while(1) {
                                        if ((*descs != ' ') &&
                                            (*descs != '\t')) {
                                                break;
                                        }
                                        descs++;
                                }
                                descs += 2;
                                desce = strchr(descs, '\n');
                                flg = 1;
                        } else if (strstr(descs, "route:") == descs) {
                                descs += 6;
                                while(1) {
                                        if ((*descs != ' ') &&
                                            (*descs != '\t')) {
                                                break;
                                        }
                                        descs++;
                                }
                                desce = strchr(descs, '\n');
                                flg = 2;
                        } else if (strstr(descs, "*or:") == descs) {
                                descs += 4;
                                while(1) {
                                        if ((*descs != ' ') &&
                                            (*descs != '\t')) {
                                                break;
                                        }
                                        descs++;
                                }
                                descs += 2;
                                desce = strchr(descs, '\n');
                                flg = 1;
                        } else if (strstr(descs, "*rt:") == descs) {
                                descs += 4;
                                while(1) {
                                        if ((*descs != ' ') &&
                                            (*descs != '\t')) {
                                                break;
                                        }
                                        descs++;
                                }
                                desce = strchr(descs, '\n');
                                flg = 2;
                        }
                        if (desce != NULL) {
                                *desce = '\0';
                        }
                        if (flg > 0) {
				/* In this section, 
				   add items to chace database 
				   2006/08/04 */
                                /* strcat(retas, descs); */
                                switch(flg) {
                                case 2:
#ifdef DEBUG
					printf("DEBUG: Get IRR Prefix : %s\n",
								descs);
					pr_log2(VLOG_GLOB, LOG_ALL, ON,
						"DEBUG: Get IRR Prefix : %s\n",
								descs);
#endif
					sepprefix(&irrentry->prefix, 
						&irrentry->p_len, descs);
                                        break;
                                case 1:
#ifdef DEBUG
					printf("DEBUG: Get IRR OAS : %s\n",
								descs);
					pr_log2(VLOG_GLOB, LOG_ALL, ON,
						"DEBUG: Get IRR OAS : %s\n",
								descs);
#endif
					irrentry->originas = atol(descs);
                                        break;
                                }
                                descs = desce + 1;
                        } else {
                                descs++;
                        }
                        if (descs == NULL) break;
                }
#ifdef DEBUG
                printf("Searched Data:%s\n", descs);
#endif
                free(data);
		irrentry->status = CA_ENTRIED;
        } else {
		irrentry->status = CA_UNENTRIED;
	}
	gettimeofday(&irrentry->entry_time, (struct timezone *)0);
	return(irrentry);
}

IRR_ENTRY *irr_check(struct in_addr *prefix, u_short plen)
{
	IRR_ENTRY *dbret;
	char	 *irrdata;

	/* IRR Search */
#ifdef DEBUG
	printf("DEBUG: Search Command Send\n");
#endif
	if (irr_search_route(prefix, plen) == 1) {
		pr_log2(VLOG_GLOB, LOG_ALL, ON,
			"IRR Search Command Error\n");
		return(0);
	}
#ifdef DEBUG
	printf("DEBUG: Receiving Search Data\n");
#endif
	irrdata = readdata();
	dbret = get_items(irrdata);
	if (dbret == NULL) {
		/* Search Fail */
		return(NULL);
	}
	return(dbret);
}

int irrchk_r_list(R_LIST *rlist)
{
	R_LIST		*rptr;
	ROUTE_INFO	*ri;
	int		flag;
	struct in_addr	in;
	char		*bufptr, *wptr, *wptr2;
	struct timeval	tmv;
	IRR_ENTRY	*irrent;
	int		pn;

	
	gettimeofday(&tmv, (struct timezone *)0);

	rptr = rlist;
	while(rptr != NULL) {
		ri = rptr->route_data;

		/* Neighbor Check */
		pn = check_id1topn(ri->source);
#ifdef DEBUG
		in.s_addr = htonl(ri->source);
		printf("DEBUG: IRR Checking Peer = %d (%s)\n",
				 pn, inet_ntoa(in));
#endif
		if (pn < 0) {
#ifdef DEBUG
			printf("DEBUG: Illegal Peer\n");
#endif
			/* Illegal Peer */
			rptr = rptr->next;
			continue;
		}
		if (peer[pn].irr_compare == false) {
#ifdef DEBUG
			printf("DEBUG: Disabled Peer\n");
#endif
			/* Disabled Peer */
			rptr = rptr->next;
			continue;
		}

		flag = 0;
		irrent = ri->irr;
		if (irrent != NULL) {
			if (irrent->entry_time.tv_sec != 0) {
				if ((tmv.tv_sec - irrent->entry_time.tv_sec)
							> IRR_CACHE_EXPTIME) {
					/* Expiration Check */
					flag = 1;
#ifdef DEBUG
				printf("DEBUG: IRR Cash Expire -> %08X/%d\n",
					ri->prefix, ri->length);
#endif
				}
			}
		} else {
			flag = 1;
		}

		if (flag == 1) {
#ifdef DEBUG
			printf("DEBUG: IRR Search : %08X/%d\n",
						ri->prefix, ri->length);
#endif
			/* Search New */
			if (ri->irr != NULL) free(ri->irr);
			in.s_addr = ri->prefix;
			ri->irr = irr_check(&in, ri->length);
			free(bufptr);
			bufptr = NULL;
		}
#ifdef DEBUG
		else {
			printf("DEBUG: IRR Cached : %08X/%d\n",
						ri->prefix, ri->length);
		}
#endif
			
			
		rptr = rptr->next;
	}

	return(0);
}

void walk_bgp_chk_irr()
{
	int	clno;
	static RADIX_T	*cnode=NULL;
	static int	upcode=0;
	RADIX_T		*node=NULL;
	struct timeval	tmval;

	if (irr_check_enable == 0) return; /* IRR Check Disable */

	if (irr_checking_status == IRRCHK_TIMERWAIT) return;
	if (cnode == NULL) {
		if (open_irr(irrhost, irrport) == 1) {
			node = NULL;
			cnode = NULL;
			upcode = 0;
			/* Next Walk Timer Set */
			tmval.tv_sec  = IRR_WALK_TIMER;
			tmval.tv_usec = 0;
			irr_chk_timer_id = common_entry_timer(&tmval,
					 IRRCHK_ACTIVE, activate_bgp_chk_irr);
			irr_checking_status = IRRCHK_TIMERWAIT;
			pr_log2(VLOG_GLOB, LOG_ALL, ON, "IRR Walker Stop\n");
			return;
		} 
	}
	node = show_route_all(cnode, &upcode);
	if (node != NULL) {
		irrchk_r_list(node->rinfo);
	}

	/* Socket Check */
	if (irrsock == 0) {
		pr_log2(VLOG_GLOB, LOG_ALL, ON, "IRR Connection Fail\n");
		node  = NULL;
	}

	if (node == NULL) {
		if (cnode == NULL) {
			cnode = node;
			/* Next Walk Timer Set */
			tmval.tv_sec  = IRR_WALK_TIMER;
			tmval.tv_usec = 0;
			irr_chk_timer_id = common_entry_timer(&tmval,
					 IRRCHK_ACTIVE, activate_bgp_chk_irr);
			irr_checking_status = IRRCHK_TIMERWAIT;
			pr_log2(VLOG_GLOB, LOG_ALL, ON, "IRR Walker Stop\n");
			/* return(BVS_CNT_END); */
			cnode = NULL;
			upcode = 0;
			close_irr();
			return;
		} else {
			cnode = node;
			/* Next Walk Timer Set */
			tmval.tv_sec  = IRR_WALK_TIMER;
			tmval.tv_usec = 0;
			irr_chk_timer_id = common_entry_timer(&tmval,
					 IRRCHK_ACTIVE, activate_bgp_chk_irr);
			irr_checking_status = IRRCHK_TIMERWAIT;
			pr_log2(VLOG_GLOB, LOG_ALL, ON, "IRR Walker Done\n");
			/* return(BVS_CNT_FINISH); */
			cnode = NULL;
			upcode = 0;
			close_irr();
			return;
		}
	} else {
		cnode = node;
		irr_checking_status = IRRCHK_ACTIVE;
#ifdef DEBUG_VERBOSE
		pr_log2(VLOG_GLOB, LOG_ALL, ON, "DEBUG: IRR Walker GoNext\n");
#endif
		/* return(BVS_CNT_CONTINUE); */
		return;
	}
}

void activate_bgp_chk_irr(int start)
{
	if (irr_check_enable == 0) return; /* IRR Check Disable */

#ifdef DEBUG
	printf("DEBUG: IRR Check Flag change to IRRCHK_ACTIVE\n");
#endif
	pr_log2(VLOG_GLOB, LOG_ALL, ON, "IRR Walker Start\n");
	irr_checking_status = IRRCHK_ACTIVE;
}

int irr_walker_check()
{
	return(irr_checking_status);
}

int irr_routecmp(struct in_addr *in, int plen, u_long oasn, IRR_ENTRY *irrent)
{
	u_long	mask;

	if (irrent == NULL) {
		return(IRR_SEARCH_FAIL);
	}

	if (irrent->status == CA_UNENTRIED) {
		return(IRR_NOTMATCH);
	}

	if ((plen <= 0) || (irrent->p_len <= 0)) {
		return(IRR_SEARCH_FAIL);
	}

	/* Exact Check */
	if (irrent->p_len > plen) {
		return(IRR_NOTMATCH);
	}

	mask = 0xFFFFFFFF << (32 - irrent->p_len);
	if ((ntohl(in->s_addr) && mask) == 
				    (ntohl(irrent->prefix.s_addr) && mask)) {
		if (plen == irrent->p_len) {
			/* Exact */
			if (oasn == irrent->originas) {
				return(IRR_EXACT_AS);
			} else {
				return(IRR_EXACT_NOTAS);
			}
		} else {
			/* More Prefix */
			if (oasn == irrent->originas) {
				return(IRR_MORES_AS);
			} else {
				return(IRR_MORES_NOTAS);
			}
		}
	} else {
		return(IRR_NOTMATCH);
	}
}

#endif /* IRRCHK */
