/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 2002/04                         */
/*                                                              */
/* Radix Tree Routine for IPv6                                  */
/****************************************************************/
/* radix_v6.c,v 1.2 2004/07/13 08:58:44 kuniaki Exp */

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

#include "radix_v6.h"

#ifdef DEBUG
#include <errno.h>
#endif

/*** Local Variables ***/
int	rtclear_id_v6;

int init_radix_v6()
{
	printf("Initializing IPv6 RADIX Tree...\n");
	radixv6_top = (RADIX_V6_T *)malloc(sizeof(RADIX_V6_T));
	if (radixv6_top == NULL) return(1);
	bzero(radixv6_top, sizeof(RADIX_V6_T));
	nodes_v6 = 1;
	routes_v6 = 0;
	prefixes_v6 = 0;
	rthist_clean_v6 = RTHIST_NOTCLEANING;
	cleaned_time_v6 = 0;

	return(0);
}

u_long radix_prefixes_v6()
{
	return(prefixes_v6);
}

u_long radix_routes_v6()
{
	return(routes_v6);
}

u_long radix_nodes_v6()
{
	return(nodes_v6);
}

int entry_route_v6(RADIX_V6_T *node, ROUTE_INFO_V6 *rinfo)
{
	ROUTE_INFO_V6	*ribuf;
	R_LIST_V6	*rlist,*brlist,*lbuf;
	struct timeval	tmvtemp;
	char		addrbuf[50];
#ifdef RTHISTORY
	RT_HISTORY	*histbuf, *histendbuf, *hhist;
	boolean		actbuf;
	char            pathsetbuf[4096];
	ROUTE_INFO_V6	*pathbuf;
#endif

#ifdef DEBUG
	bintostrv6(rinfo->prefix, addrbuf);
	printf("DEBUG: RADIXV6: Entry IPv6 Route : %s/%d\n",
			addrbuf, rinfo->length);
#endif

	ribuf = (ROUTE_INFO_V6 *)malloc(sizeof(ROUTE_INFO_V6));
	if (ribuf == NULL) return(0);
	memcpy(ribuf, rinfo, sizeof(ROUTE_INFO_V6));

	rlist = node->rinfo;
	brlist = NULL;
	while(1) {
		if (rlist == NULL) {
			lbuf = (R_LIST_V6 *)malloc(sizeof(R_LIST_V6));
			if (lbuf == NULL) {
				pr_log(VLOG_GLOB, LOG_ALL, 
				   "entry_route_v6(lbuf): malloc error\n", ON);
				free(ribuf);
				return(0);
			}
			bzero(lbuf, sizeof(R_LIST_V6));
			if (node->rinfo == NULL) {
				node->rinfo = lbuf;
			} else {
				brlist->next = lbuf;
			}
			lbuf->route_data = ribuf;
			prefixes_v6++;
#ifdef RTHISTORY
			ribuf->active = true;
#endif
			return(1); /* Add New Route */
		}
		if (v6addrcmp(ribuf->source, (rlist->route_data)->source,
							IPV6_ADDR_LEN*8) ) {
#ifdef RTHISTORY
			histbuf = (rlist->route_data)->history;
			histendbuf = (rlist->route_data)->histend;
			actbuf = (rlist->route_data)->active;
			tmvtemp.tv_sec = (rlist->route_data)->firstupdate.tv_sec;
#endif
			if (((rlist->route_data)->pathset_ptr) != NULL) {
				pathbuf = rlist->route_data;
				free(pathbuf->pathset_ptr);
				pathbuf->pathset_ptr = NULL;
			}
			free(rlist->route_data);
			rlist->route_data = ribuf;
#ifdef RTHISTORY
			(rlist->route_data)->history = histbuf;
			(rlist->route_data)->histend = histendbuf;
			(rlist->route_data)->active = true;
			(rlist->route_data)->firstupdate.tv_sec = tmvtemp.tv_sec;
			(rlist->route_data)->firstupdate.tv_usec = 0;

			hhist = (RT_HISTORY *)malloc(sizeof(RT_HISTORY));
			if (hhist == NULL) {
				return(0);
			}
			bzero(hhist, sizeof(RT_HISTORY));
			hhist->type = UPDATE;
			gettimeofday(&(hhist->update), (struct timezone *)0);
			if ((rlist->route_data)->history == NULL) {
				(rlist->route_data)->history = hhist;
				(rlist->route_data)->histend = hhist;
			} else {
				histendbuf->next = hhist;
				hhist->before = histendbuf;
				(rlist->route_data)->histend = hhist;
			}
			if (actbuf == false) {
				return(1);
			} else {
				return(2);
			}
#endif
#ifndef RTHISTORY
			return(2); /* Update Route */
#endif
		}
		brlist = rlist;
		rlist = rlist->next;
	}
}

int add_route_v6(ROUTE_INFO_V6 *rinfo)
{
	RADIX_V6_T	*ladd;
	RADIX_V6_T	*c_leaf;
	int		cbit,scbit,loopbit;
	int		ret;
	int		toct,lbit;

	c_leaf = radixv6_top;
	scbit = 0;
	while(1) {
		/* Default Route Add */
		if (rinfo->length == 0) {
			ret = entry_route_v6(radixv6_top, rinfo);
			if (ret == 2) prefixes_v6++;
			break;
		}

		if ((rinfo->length)-1 > c_leaf->check_bit) {
			cbit = c_leaf->check_bit;
		} else {
			cbit = (rinfo->length)-1;
		}

		loopbit = scbit;
		while(loopbit < cbit) {
			toct = (int)loopbit/8;
			lbit = loopbit % 8;
			if ( (0x80 & (rinfo->prefix[toct] << lbit)) !=
			     (0x80 & (c_leaf->key[toct] << lbit)) ) {
				break;
			}
			loopbit++;
		}
#ifdef DEBUG
		printf("DEBUG: RADIXV6: Add Route / Node Check Parm\n");
		printf("DEBUG: RADIXV6:  rinfo->length = %d / c_leaf->check_bit = %d\n",
						rinfo->length, c_leaf->check_bit);
		printf("DEBUG: RADIXV6:  cbit = %d / scbit = %d / loopbit = %d\n",
							cbit, scbit, loopbit);
#endif
		
		if (!(cbit <= loopbit) || (loopbit != c_leaf->check_bit)) { 
			/* add node */
#ifdef DEBUG
			printf("DEBUG: RADIX_V6: ADD node\n");
#endif
			ladd = (RADIX_V6_T *)malloc(sizeof(RADIX_V6_T));
			if (ladd == NULL) {
				return(1);
			}
			bzero(ladd, sizeof(RADIX_V6_T));

			toct = (int)loopbit/8;
			lbit = loopbit%8;
			if (0x80 & (c_leaf->key[toct] << lbit)) {
				ladd->r_leaf = c_leaf;
			} else {
				ladd->l_leaf = c_leaf;
			}
			ladd->upleaf = c_leaf->upleaf;
			ladd->check_bit = loopbit;
			store_v6addr(ladd->key, rinfo->prefix);
			v6addrmask(ladd->key, loopbit+1);
			nodes_v6++;
			if (c_leaf->upleaf != NULL) {
				if (c_leaf == (c_leaf->upleaf)->r_leaf) {
					(c_leaf->upleaf)->r_leaf = ladd;
				} else {
					(c_leaf->upleaf)->l_leaf = ladd;
				}
			}
			c_leaf->upleaf = ladd;
			c_leaf = ladd;
		}

		if ( (v6addrcmp(rinfo->prefix, c_leaf->key, IPV6_ADDR_LEN*8)) &&
		     (rinfo->length-1 == c_leaf->check_bit) ) {
#ifdef DEBUG
			{
				char	tmpv6str[50];
				bintostrv6(rinfo->prefix, tmpv6str);
				printf("DEBUG: RADIXV6: Entry Target: rinfo->prefix = %s", tmpv6str);
				bintostrv6(c_leaf->key, tmpv6str);
				printf(" / c_leaf->key = %s\n", tmpv6str);
			}
#endif

			if (c_leaf->masklen == 0) {
				c_leaf->masklen = rinfo->length-1;
				routes_v6++;
			}
			ret = entry_route_v6(c_leaf, rinfo);
			break;
		}

		scbit = c_leaf->check_bit;
		toct = (int)((c_leaf->check_bit)/8);
		lbit = (c_leaf->check_bit)%8;
		if (0x80 & (rinfo->prefix[toct] << lbit)) {
			if (c_leaf->r_leaf == NULL) {
				/* r_leaf add */
#ifdef DEBUG
				printf("DEBUG: RADIXV6: Right Leaf Add\n");
#endif
				ladd = (RADIX_V6_T *)malloc(sizeof(RADIX_V6_T));
				if (ladd == NULL) {
					return(1);
				}
				bzero(ladd, sizeof(RADIX_V6_T));
				ladd->upleaf = c_leaf;
				ladd->check_bit = rinfo->length-1;
				store_v6addr(ladd->key, rinfo->prefix);
				nodes_v6++;
				c_leaf->r_leaf = ladd;
				c_leaf = ladd;
			} else {
				c_leaf = c_leaf->r_leaf;
			}
		} else {
			if (c_leaf->l_leaf == NULL) {
				/* l_leaf add */
#ifdef DEBUG
				printf("DEBUG: RADIXV6: Left Leaf Add\n");
#endif
				ladd = (RADIX_V6_T *)malloc(sizeof(RADIX_V6_T));
				if (ladd == NULL) {
					return(1);
				}
				bzero(ladd, sizeof(RADIX_V6_T));
				ladd->upleaf = c_leaf;
				ladd->check_bit = rinfo->length-1;
				store_v6addr(ladd->key, rinfo->prefix);
				nodes_v6++;
				c_leaf->l_leaf = ladd;
				c_leaf = ladd;
			} else {
				c_leaf = c_leaf->l_leaf;
			}
		}
	}
	switch(ret) {
		case 1: /* Add New Route */
			return(2);
		case 2: /* Update Route */
			return(3);
		default: /* Error */
			return(1);
	}
}

#ifdef RTHISTORY
int withdraw_route_v6(int pn, char *prefix, int masklen, int deltype)
#endif
#ifndef RTHISTORY
int withdraw_route_v6(int pn, char *prefix, int masklen)
#endif
{
	RADIX_V6_T	*node, *bnode;
	int		rcode;
	R_LIST_V6	*rlist, *brlist;
	ROUTE_INFO_V6	*pathbuf;
#ifdef RTHISTORY
	RT_HISTORY	*hhist;
#endif
	char		v6addrstr[50];
	int		toct,lbit;

	node = radixv6_top;

	rcode = 0;
	while(1) {
	  if ((node->check_bit > masklen-1) && (masklen != 0)) {
	    rcode = 1; /* no route */
	    break;
	  }
#ifdef DEBUG
	  bintostrv6(node->key, v6addrstr);
	  printf("DEBUG: RADIXV6: DELETE: Checking pref=%s / mask=%d",
					v6addrstr, node->masklen);
	  printf(" / Check_bit=%d\n", node->check_bit);
#endif
	  if ( ((v6addrcmp(node->key, prefix, IPV6_ADDR_LEN*8)) &&
	  	 (node->masklen == masklen-1)) || (masklen == 0)) {
	    if (masklen == 0) {
	      node = radixv6_top;
	    }
	    rlist = node->rinfo;
	    if (rlist == NULL) {
	      rcode = 1;
	      break;
	    }
	    brlist = NULL;
	    rcode = 1;
	    while(rlist != NULL) {
	      if (pn == sourcetopn(rlist->route_data->source,
	      			   rlist->route_data->src_afi)) {
	      /* delete route */
#ifdef DEBUG
		bintostrv6(rlist->route_data->prefix, v6addrstr);
	        printf("DEBUG: RADIXV6: DELETE: pref = %s / Mask = %d",
	  				v6addrstr, rlist->route_data->length);
	  	bintostrv6(rlist->route_data->source, v6addrstr);
	        printf(" / Neighbor = %s\n", v6addrstr);
#endif

#ifdef RTHISTORY
	        if (deltype == DFLAG) {
		  hhist = (RT_HISTORY *)malloc (sizeof(RT_HISTORY));
	  	  if (hhist != NULL) {
		    bzero(hhist, sizeof(RT_HISTORY));
		    (rlist->route_data)->active = false;
		    hhist->type = WITHDRAW;
		    gettimeofday(&(hhist->update), (struct timezone *)0);
		    if ((rlist->route_data)->history == NULL) {
		      (rlist->route_data)->history = hhist;
		      (rlist->route_data)->histend = hhist;
		    } else {
		      hhist->before = (rlist->route_data)->histend;
		      (rlist->route_data)->histend = hhist;
		      (hhist->before)->next = hhist;
		    }
		    rcode = 0;
		    break;
		  } else {
		    if (((rlist->route_data)->pathset_ptr) != NULL) {
		      pathbuf = rlist->route_data;
		      free(pathbuf->pathset_ptr);
		      pathbuf->pathset_ptr = NULL;
		    }
		    free(rlist->route_data);
		    if (brlist == NULL) {
		      node->rinfo = rlist->next;
		      free(rlist);
		      rlist = node->rinfo;
		    } else {
		      brlist->next = rlist->next;
		      free(rlist);
		      rlist = brlist->next;
		    }
		  }
	        } else {
#endif
		  if (((rlist->route_data)->pathset_ptr) != NULL) {
		    pathbuf = rlist->route_data;
		    free(pathbuf->pathset_ptr);
		    pathbuf->pathset_ptr = NULL;
		  }
		  free(rlist->route_data);
		  if (brlist == NULL) {
		    node->rinfo = rlist->next;
		    free(rlist);
		    rlist = node->rinfo;
		  } else {
		    brlist->next = rlist->next;
		    free(rlist);
		    rlist = brlist->next;
		  }
#ifdef RTHISTORY
	        }
#endif
	        prefixes_v6--;
	        rcode = 0;
	      } else {
	        brlist = rlist;
	        rlist = rlist->next;
	      }
	      if (node == radixv6_top) break;
	      /* Back Track for Removing node */
	      if (node->rinfo == NULL) {
	        node->masklen = 0;
	        routes_v6--;
	        while(1) {
		  bnode = node;
		  node = node->upleaf;
		  if ((bnode->r_leaf != NULL) && (bnode->l_leaf != NULL)) {
		    break;
		  }
		  if (bnode == node->r_leaf) {
		    if (bnode->r_leaf == NULL) {
		      node->r_leaf = bnode->l_leaf;
		    } else {
		      node->r_leaf = bnode->r_leaf;
		    }
		    if (node->r_leaf != NULL) {
		      node->r_leaf->upleaf = node;
		    }
		  } else {
		    if (bnode->r_leaf == NULL) {
		      node->l_leaf = bnode->l_leaf;
		    } else {
		      node->l_leaf = bnode->r_leaf;
		    }
		    if (node->l_leaf != NULL) {
		      node->l_leaf->upleaf = node;
		    }
		  }
#ifdef DEBUG
		  errno = 0;
	  	  bintostrv6(bnode->key, v6addrstr);
	          printf("DEBUG: RADIXV6: WITHDRAW: Freeing Node = %s/%d CB=%d\n",
				v6addrstr, bnode->masklen, bnode->check_bit);
#endif
		  free(bnode);
		  nodes_v6--;
#ifdef DEBUG
		  if (errno != 0) perror("DEBUG: RADIXV6: FreeError: ");
#endif
		  if (node->upleaf == NULL) break;
		  if (node->rinfo != NULL)  break;
	        }
	        break;
	      }
	    }
	    break;
	  }
	  toct = (int)(node->check_bit / 8);
	  lbit = node->check_bit % 8;
	  if (0x80 & (*(prefix+toct) << lbit)) {
	    node = node->r_leaf;
	  } else {
	    node = node->l_leaf;
	  }
	  if (node == NULL) {
	    rcode = 1;
	    break;
	  }
        }
	return(rcode);
}

RADIX_V6_T *show_route_all_v6(RADIX_V6_T *curnode, int *upcode)
{
	RADIX_V6_T		*bnode;
	int		rcode;

	if (curnode == NULL) {
		/* Show route from TOP Node */
		bnode = radixv6_top;
		*upcode = 0;
#ifdef DEBUG
		printf("DEBUG: RADIXV6: Goto Most Left Leaf\n");
#endif
		while(bnode->l_leaf != NULL) {
			bnode = bnode->l_leaf;
#ifdef DEBUG
			printf("DEBUG: RADIXV6:   Move to Node = %08X\n",bnode);
#endif
		}
		if (bnode->rinfo != NULL) {
#ifdef DEBUG
			printf("DEBUG: RADIXV6:   Found First Route Data\n");
#endif
			return(bnode);
		}
	} else {
		/* Show Current Node */
		bnode = curnode;
	}

#ifdef DEBUG
	printf("DEBUG: RADIXV6: Start Tree Pointer = %08X\n", curnode);
#endif

	rcode = 0;
	while(1) {
#ifdef DEBUG
		printf("DEBUG: RADIXV6: ALLROUTE: Checkbit = %d / upcode=%d\n",
						bnode->check_bit,*upcode);
#endif
		if ((bnode->l_leaf != NULL) && (*upcode == 0)) {
			*upcode = 0;
			bnode = bnode->l_leaf;
		} else if ((bnode->r_leaf != NULL) && (*upcode != 1)) {
			*upcode = 0;
			bnode = bnode->r_leaf;
		} else if (bnode->upleaf != NULL) {
			if (bnode == bnode->upleaf->r_leaf) {
				/* From R_leaf to Upleaf */
				*upcode = 1;
			} else {
				/* From L_leaf to Upleaf */
				*upcode = 2;
			}
			bnode = bnode->upleaf;
		} else {
			*upcode = 0;
			rcode = 1;
			break;
		}
		if (bnode->rinfo != NULL) {
			if (*upcode == 1) break;
			if ((*upcode == 2) && (bnode->r_leaf == NULL)) break;
			if ((*upcode == 0) && (bnode->r_leaf == NULL) &&
					(bnode->l_leaf == NULL)) break;
		}
	}

	if (rcode == 1) {
		return(NULL);
	} else {
		return(bnode);
	}
}

u_long delete_peer_route_v6(RADIX_V6_T **cnode, int pn)
{
	RADIX_V6_T	*node, *bnode;
	R_LIST_V6	*rlist, *brlist;
	u_long		bufnodes;
	ROUTE_INFO_V6	*pathbuf;
	char		v6addrstr[50];

	node = *cnode;
	rlist = node->rinfo;
	if (rlist == NULL) return(0);
	brlist = NULL;
	bufnodes = nodes_v6;
	while(rlist != NULL) {
		if (pn == sourcetopn(rlist->route_data->source,
					rlist->route_data->src_afi)) {
			/* delete route */
#ifdef DEBUG
	bintostrv6(rlist->route_data->prefix, v6addrstr);
	printf("DEBUG: RADIXV6: DELETE: pref = %s / Mask = %d\n",
		v6addrstr, rlist->route_data->length);
	bintostrv6(rlist->route_data->source, v6addrstr);
	printf("                      Neighbor = %s\n", v6addrstr);
#endif
			if (((rlist->route_data)->pathset_ptr) != NULL) {
				pathbuf = rlist->route_data;
				free(pathbuf->pathset_ptr);
				pathbuf->pathset_ptr = NULL;
			}
			free(rlist->route_data);
			if (brlist == NULL) {
				node->rinfo = rlist->next;
				free(rlist);
				rlist = node->rinfo;
			} else {
				brlist->next = rlist->next;
				free(rlist);
				rlist = brlist->next;
			}
			prefixes_v6--;
		} else {
			brlist = rlist;
			rlist = rlist->next;
		}
		/* Back Track for Removing node */
		if (node->rinfo == NULL) {
			node->masklen = 0;
			routes_v6--;
			while(1) {
				bnode = node;
				node = node->upleaf;
				if ((bnode->r_leaf != NULL) && 
				    (bnode->l_leaf != NULL)) {
					break;
				}
				if (bnode == node->r_leaf) {
					if (bnode->r_leaf == NULL) {
						node->r_leaf = bnode->l_leaf;
					} else {
						node->r_leaf = bnode->r_leaf;
					}
					if (node->r_leaf != NULL) {
						node->r_leaf->upleaf = node;
					}
				} else {
					if (bnode->r_leaf == NULL) {
						node->l_leaf = bnode->l_leaf;
					} else {
						node->l_leaf = bnode->r_leaf;
					}
					if (node->l_leaf != NULL) {
						node->l_leaf->upleaf = node;
					}
				}
#ifdef DEBUG
				errno = 0;
	bintostrv6(bnode->key, v6addrstr);
	printf("DEBUG: RADIXV6: WITHDRAW: Freeing Node = %s/%d CB=%d\n",
			v6addrstr, bnode->masklen, bnode->check_bit);
#endif
				free(bnode);
				nodes_v6--;
#ifdef DEBUG
	if (errno != 0) perror("DEBUG: RADIXV6: FreeError: ");
#endif
				if (node->upleaf == NULL) break;
				if (node->rinfo != NULL)  break;
			}
			break;
		}
	}

	*cnode = node;
	return(bufnodes-nodes_v6);
}

int withdraw_peer_v6(int pn)
{
	RADIX_V6_T	*bnode;
	int		rcode;
	static int	upcode;
	u_long		wdcode;

	bnode = radixv6_top;
	upcode = 0;
	while(bnode->l_leaf != NULL) {
		bnode = bnode->l_leaf;
	}
	if (bnode == NULL) {
#ifdef DEBUG
		printf("DEBUG: RADIXV6: DELPEER: No Route\n");
#endif
		return(0);
	}

	if (peer[pn].neighbor == 0) {
#ifdef DEBUG
		printf("DEBUG: RADIXV6: DELPEER: No Peer\n");
#endif
		return(0);
	}

#ifdef DEBUG
	printf("DEBUG: RADIXV6: Start Tree Pointer = %08X\n", bnode);
#endif
	wdcode = delete_peer_route_v6(&bnode, pn);

	while(1) {
		rcode = 0;
		while(1) {
#ifdef DEBUG
			printf("DEBUG: RADIXV6: DELPEER: Checkbit = %d / upcode=%d\n",
							bnode->check_bit, upcode);
#endif
			if ((bnode->l_leaf != NULL) && (upcode == 0)) {
				upcode = 0;
				bnode = bnode->l_leaf;
			} else if ((bnode->r_leaf != NULL) && (upcode != 1)) {
				upcode = 0;
				bnode = bnode->r_leaf;
			} else if (bnode->upleaf != NULL) {
				if (bnode == bnode->upleaf->r_leaf) {
					/* From R_leaf to Upleaf */
					upcode = 1;
				} else {
					/* From L_leaf to Upleaf */
					upcode = 2;
				}
				bnode = bnode->upleaf;
			} else {
				upcode = 0;
				rcode = 1;
				break;
			}
			if (bnode->rinfo != NULL) {
				if (upcode == 1) break;
				if ((upcode == 2) && 
				    		(bnode->r_leaf == NULL)) break;
				if ((upcode == 0) && (bnode->r_leaf == NULL) &&
						(bnode->l_leaf == NULL)) break;
			}
		}
	
		if (rcode == 1) {
			break;
		}
		/* delete route */
		wdcode = delete_peer_route_v6(&bnode, pn);
		if (wdcode != 0) upcode = 0;
	}

#ifdef RTHISTORY
	withdraw_route_v6(pn, 0, 0, DELETE);
#endif
#ifndef RTHISTORY
	withdraw_route_v6(pn, 0, 0);
#endif
	return(0);
}

RADIX_V6_T *search_route_v6(char *prefix, int masklen)
{
	RADIX_V6_T	*node, *bnode;
	int		rcode;
	char		v6addrstr[50];
	int		toct, lbit;
	char		v6addr[IPV6_ADDR_LEN];

	node = radixv6_top;

	if ((masklen == 0) || (prefix == NULL)) {
		return(radixv6_top);
	}

	while(1) {
		if (node->check_bit > masklen-1) {
			break;
		}
#ifdef DEBUG
		bintostrv6(node->key, v6addrstr);
		printf("DEBUG: RADIXV6: SEARCH: Checking pref=%s / mask=%d\n",
					v6addrstr, node->masklen);
		printf("                               Check_bit=%d\n",
						node->check_bit);
#endif

		toct = (int)(node->check_bit / 8);
		lbit = node->check_bit % 8;
		if (0x80 & (*(prefix + toct) << lbit)) {
			if (node->r_leaf == NULL) break;
			node = node->r_leaf;
#ifdef DEBUG
			printf("DEBUG: RADIXV6: SEARCH: Goto right Route\n");
#endif
		} else {
			if (node->l_leaf == NULL) break;
			node = node->l_leaf;
#ifdef DEBUG
			printf("DEBUG: RADIXV6: SEARCH: Goto left Route\n");
#endif
		}
		if (node == NULL) {
			break;
		}
	}
	if (node == NULL) {
		return(NULL);
	}

	rcode = 0;
	while(1) {
		if (node->masklen != 0) {
			store_v6addr(v6addr, prefix);
			v6addrmask(v6addr, node->masklen);
			if (v6addrcmp(v6addr, node->key, IPV6_ADDR_LEN*8)) break;
		}
		if (node->upleaf == NULL) {
			rcode = 1;
			break;
		}
		node = node->upleaf;
	}

	if (rcode == 1) return(NULL);
	return(node);
}

#ifdef RTHISTORY
RADIX_V6_T *clear_rt_history_v6(RADIX_V6_T *curnode, int *upcode)
{
	RADIX_V6_T	*bnode;
	int		rcode;
	R_LIST_V6	*rlist;
	ROUTE_INFO_V6	*rinfo;
	RT_HISTORY	*histnext;

	if (curnode == NULL) {
		/* Show route from TOP Node */
		bnode = radixv6_top;
		*upcode = 0;
#ifdef DEBUG
		printf("DEBUG: RTHISTCLEAN_V6: Goto Most Left Leaf\n");
#endif
		while(bnode->l_leaf != NULL) {
		    bnode = bnode->l_leaf;
#ifdef DEBUG
		    printf("DEBUG: RTHISTCLEAN_V6:   Move to Node = %08X\n",bnode);
#endif
		}
		if (bnode->rinfo != NULL) {
#ifdef DEBUG
		    printf("DEBUG: RTHISTCLEAN_V6:   Found First Route Data\n");
#endif
		    /** Cleaning Node **/
		    rlist = bnode->rinfo;
		    while(rlist != NULL) {
			rinfo = rlist->route_data;
			if (rinfo->active == false) {
			    withdraw_route_v6(sourcetopn(rinfo->source, rinfo->src_afi),
						rinfo->prefix, 
						rinfo->length,
						DELETE);
			    rlist = bnode->rinfo;
		    	} else {
			    while(rinfo->history != NULL) {
			        histnext = rinfo->history;
			        rinfo->history = histnext->next;
			        histnext->before = NULL;
			        free(histnext);
			    }
			    rinfo->histend = NULL;
			    rlist = rlist->next;
		        }
		    }
		    return(bnode);
		}
	} else {
		/* Show Current Node */
		bnode = curnode;
	}

#ifdef DEBUG
	printf("DEBUG: RTHISTCLEAN: Start Tree Pointer = %08X\n", curnode);
#endif

	rcode = 0;
	while(1) {
#ifdef DEBUG
		printf("DEBUG: RTHISTCLEAN: ALLROUTE: Checkbit = %d / upcode=%d\n",
						bnode->check_bit,*upcode);
#endif
		if ((bnode->l_leaf != NULL) && (*upcode == 0)) {
			*upcode = 0;
			bnode = bnode->l_leaf;
		} else if ((bnode->r_leaf != NULL) && (*upcode != 1)) {
			*upcode = 0;
			bnode = bnode->r_leaf;
		} else if (bnode->upleaf != NULL) {
			if (bnode == bnode->upleaf->r_leaf) {
				/* From R_leaf to Upleaf */
				*upcode = 1;
			} else {
				/* From L_leaf to Upleaf */
				*upcode = 2;
			}
			bnode = bnode->upleaf;
		} else {
			*upcode = 0;
			rcode = 1;
			break;
		}
		if (bnode->rinfo != NULL) {
			if (*upcode == 1) break;
			if ((*upcode == 2) && (bnode->r_leaf == NULL)) break;
			if ((*upcode == 0) && (bnode->r_leaf == NULL) &&
					(bnode->l_leaf == NULL)) break;
		}
	}

	if (rcode == 1) {
		return(NULL);
	} else {
		/** Cleaning Node **/
		rlist = bnode->rinfo;
		while(rlist != NULL) {
			rinfo = rlist->route_data;
			if (rinfo->active == false) {
				withdraw_route_v6(sourcetopn(rinfo->source, rinfo->src_afi),
							 rinfo->prefix, 
							rinfo->length, DELETE);
				rlist = bnode->rinfo;
			} else {
				while(rinfo->history != NULL) {
					histnext = rinfo->history;
					rinfo->history = histnext->next;
					histnext->before = NULL;
					free(histnext);
				}
				rinfo->histend = NULL;
				rlist = rlist->next;
			}
		}

		return(bnode);
	}
}

void rthisttimerclear_v6()
{
        pr_log(VLOG_GLOB, LOG_ALL,
                         "IPv6 Route Change History Clearing by timer\n", ON);
        if ( (bvs_clients+bvs_maxcon-1)->cont_code == -1) {
                /*** Another job is working ***/
                pr_log(VLOG_GLOB, LOG_ALL,
                         " ==> Another job is working...\n", OFF);
                rtclear_id_v6 = common_entry_timer(&rthistcleartime_v6, 0,
                                                         rthisttimerclear_v6);
                if (rtclear_id_v6 == -1) {
                        pr_log(VLOG_GLOB, LOG_ALL,
                             "Timer Set Error : rthisttimerclear_v6 - 1\n", ON);
                        notify_systemnotify("STOP",
                                "Timer Set Error : rthisttimerclear_v6 - 1");
                        finish(PROC_SHUTDOWN);
                        return;
                }
                return;
        }

        strcpy((bvs_clients+bvs_maxcon-1)->cmdline, "clear v6 route history");
        bvs_exec_command(bvs_maxcon-1);

        if (rthistcleartime_v6.tv_sec != 0) {
                rtclear_id_v6 = common_entry_timer(&rthistcleartime_v6, 0,
                                                         rthisttimerclear_v6);
                if (rtclear_id_v6 == -1) {
                        pr_log(VLOG_GLOB, LOG_ALL,
                                "Timer Set Error : rthisttimerclear_v6 - 1\n",
				ON);
                        notify_systemnotify("STOP",
                                "Timer Set Error : rthisttimerclear_v6 - 1");
                        finish(PROC_SHUTDOWN);
                        return;
                }
        }
}

int start_rthistclear_timer_v6()
{

        if (rthistcleartime_v6.tv_sec == 0) return(0);

        rtclear_id_v6 = common_entry_timer(&rthistcleartime_v6, 0,
						rthisttimerclear_v6);
        if (rtclear_id_v6 == -1) return(1);

        return(0);
}

#endif /** RTHISTORY **/
