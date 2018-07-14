/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1997/12                         */
/*                                                              */
/* Radix Tree Routine                                           */
/****************************************************************/
/* radix.c,v 1.4 2005/06/24 01:28:54 kuniaki Exp */

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

#include "radix.h"

#ifdef DEBUG
#include <errno.h>
#endif

RADIX_T *radix_top;
u_long  nodes;
u_long  routes;
u_long  prefixes;


/*** Local Variables ***/
int	rtclear_id;

int init_radix()
{
	printf("Initializing RADIX Tree...\n");
	radix_top = (RADIX_T *)malloc(sizeof(RADIX_T));
	if (radix_top == NULL) return(1);
	bzero(radix_top, sizeof(RADIX_T));
	nodes = 1;
	routes = 0;
	prefixes = 0;
#ifdef RTHISTORY
	rthist_clean = RTHIST_NOTCLEANING;
	cleaned_time = 0;
#endif

	return(0);
}

u_long radix_prefixes()
{
	return(prefixes);
}

u_long radix_routes()
{
	return(routes);
}

u_long radix_nodes()
{
	return(nodes);
}

int entry_route(RADIX_T *node, ROUTE_INFO *rinfo)
{
	ROUTE_INFO	*ribuf;
	R_LIST		*rlist,*brlist,*lbuf;
	struct timeval	tmvtemp;
#ifdef RTHISTORY
	RT_HISTORY	*histbuf, *histendbuf, *hhist;
	boolean		actbuf;
	char            pathsetbuf[4096];
	ROUTE_INFO	*pathbuf;
#endif

#ifdef RADIXDEBUG
	printf("DEBUG: RADIX: Entry Route : %08X/%d\n",
			ntohl(rinfo->prefix), rinfo->length);
#endif

	ribuf = (ROUTE_INFO *)malloc(sizeof(ROUTE_INFO));
	if (ribuf == NULL) return(0);
	memcpy(ribuf, rinfo, sizeof(ROUTE_INFO));

	rlist = node->rinfo;
	brlist = NULL;
	while(1) {
		if (rlist == NULL) {
			lbuf = (R_LIST *)malloc(sizeof(R_LIST));
			if (lbuf == NULL) {
				pr_log(VLOG_GLOB, LOG_ALL, 
				   "entry_route(lbuf): malloc error\n", ON);
				free(ribuf);
				return(0);
			}
			bzero(lbuf, sizeof(R_LIST));
			if (node->rinfo == NULL) {
				node->rinfo = lbuf;
			} else {
				brlist->next = lbuf;
			}
			lbuf->route_data = ribuf;
			prefixes++;
#ifdef RTHISTORY
			ribuf->active = true;
#endif
			return(1); /* Add New Route */
		}
		if (ribuf->source == (rlist->route_data)->source) {
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
#ifdef IRRCHK
			if ((rlist->route_data)->irr != NULL) {
				free((rlist->route_data)->irr);
				(rlist->route_data)->irr = NULL;
			}
#endif
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

int add_route(ROUTE_INFO *rinfo)
{
	RADIX_T	*ladd;
	RADIX_T	*c_leaf;
	int	cbit,scbit,loopbit;
	int	ret;

	c_leaf = radix_top;
	scbit = 0;
	while(1) {
		/* Default Route Add */
		if (rinfo->length == 0) {
			ret = entry_route(radix_top, rinfo);
			if (ret == 2) prefixes++;
			break;
		}

		if ((rinfo->length)-1 > c_leaf->check_bit) {
			cbit = c_leaf->check_bit;
		} else {
			cbit = (rinfo->length)-1;
		}
		loopbit = scbit;
		while(loopbit < cbit) {
			if ( (0x80000000 & (ntohl(rinfo->prefix) << loopbit)) !=
			     (0x80000000 & (ntohl(c_leaf->key) << loopbit)) ) {
				break;
			}
			loopbit++;
		}
#ifdef RADIXDEBUG
		printf("DEBUG: RADIX: Add Route / Node Check Parm\n");
		printf("DEBUG: RADIX:  rinfo->length = %d / c_leaf->check_bit = %d / key = %08X\n",
			rinfo->length, c_leaf->check_bit, ntohl(c_leaf->key));
		printf("DEBUG: RADIX:  cbit = %d / scbit = %d / loopbit = %d\n",
							cbit, scbit, loopbit);
		printf("DEBUG: RADIX: Entry Route : %08X/%d (add_route)\n",
			ntohl(rinfo->prefix), rinfo->length);
#endif
		
		if (!(cbit <= loopbit) || (loopbit != c_leaf->check_bit)) { 
			/* add node */
#ifdef RADIXDEBUG
			printf("DEBUG: RADIX: ADD node\n");
#endif
			ladd = (RADIX_T *)malloc(sizeof(RADIX_T));
			if (ladd == NULL) {
				return(1);
			}
			bzero(ladd, sizeof(RADIX_T));

			if (0x80000000 & (ntohl(c_leaf->key) << loopbit)) {
				ladd->r_leaf = c_leaf;
			} else {
				ladd->l_leaf = c_leaf;
			}
			ladd->upleaf = c_leaf->upleaf;
			ladd->check_bit = loopbit;
			ladd->key = 
			  htonl((net_ulong)(((long)0xFFFFFFFF << (31-loopbit)) &
							 ntohl(rinfo->prefix)));
/**
			ladd->key = 
				htonl((net_ulong)(((long)0x80000000 >> loopbit) &
							 ntohl(rinfo->prefix)));
**/
#ifdef DEBUG
printf("DEBUG: %08X\n", (long)0xFFFFFFFF << (31-loopbit));
#endif
			nodes++;
			if (c_leaf->upleaf != NULL) {
				if (c_leaf == (c_leaf->upleaf)->r_leaf) {
					(c_leaf->upleaf)->r_leaf = ladd;
				} else {
					(c_leaf->upleaf)->l_leaf = ladd;
				}
			}
			c_leaf->upleaf = ladd;
			c_leaf = ladd;
#ifdef RADIXDEBUG2
		printf("DEBUG: RADIX: Added Node Infomation\n");
		printf("DEBUG: RADIX:  rinfo->length = %d / c_leaf->check_bit = %d / key = %08X\n",
			rinfo->length, c_leaf->check_bit, ntohl(c_leaf->key));
		printf("DEBUG: RADIX:  cbit = %d / scbit = %d / loopbit = %d\n",
							cbit, scbit, loopbit);
#endif
		}

		if ((rinfo->prefix == c_leaf->key) &&
		    (rinfo->length-1 == c_leaf->check_bit)) {
			if (c_leaf->masklen == 0) {
				c_leaf->masklen = rinfo->length-1;
				routes++;
			}
			ret = entry_route(c_leaf, rinfo);
			break;
		}

		scbit = c_leaf->check_bit;
		if (0x80000000 & (ntohl(rinfo->prefix) << c_leaf->check_bit)) {
			if (c_leaf->r_leaf == NULL) {
				/* r_leaf add */
				ladd = (RADIX_T *)malloc(sizeof(RADIX_T));
				if (ladd == NULL) {
					return(1);
				}
				bzero(ladd, sizeof(RADIX_T));
				ladd->upleaf = c_leaf;
				ladd->check_bit = rinfo->length-1;
				ladd->key = rinfo->prefix;
				nodes++;
				c_leaf->r_leaf = ladd;
				c_leaf = ladd;
			} else {
				c_leaf = c_leaf->r_leaf;
			}
		} else {
			if (c_leaf->l_leaf == NULL) {
				/* l_leaf add */
				ladd = (RADIX_T *)malloc(sizeof(RADIX_T));
				if (ladd == NULL) {
					return(1);
				}
				bzero(ladd, sizeof(RADIX_T));
				ladd->upleaf = c_leaf;
				ladd->check_bit = rinfo->length-1;
				ladd->key = rinfo->prefix;
				nodes++;
				c_leaf->l_leaf = ladd;
				c_leaf = ladd;
			} else {
				c_leaf = c_leaf->l_leaf;
			}
		}
#ifdef RADIXDEBUG2
		printf("DEBUG: RADIX: Next Node Infomation\n");
		printf("DEBUG: RADIX:  rinfo->length = %d / c_leaf->check_bit = %d\n",
						rinfo->length, c_leaf->check_bit);
		printf("DEBUG: RADIX:  cbit = %d / scbit = %d / loopbit = %d\n",
							cbit, scbit, loopbit);
#endif
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
int withdraw_route(int pn, net_ulong prefix, int masklen, int deltype)
#endif
#ifndef RTHISTORY
int withdraw_route(int pn, net_ulong prefix, int masklen)
#endif
{
	RADIX_T		*node, *bnode;
	int		rcode;
	R_LIST		*rlist, *brlist;
	ROUTE_INFO	*pathbuf;
#ifdef RTHISTORY
	RT_HISTORY	*hhist;
#endif

	node = radix_top;

	rcode = 0;
	while(1) {
	  if ((node->check_bit > masklen-1) && (masklen != 0)) {
	    rcode = 1; /* no route */
	    break;
	  }
#ifdef RADIXDEBUG
	  printf("DEBUG: RADIX: DELETE: Checking pref=%08X / mask=%d",
					ntohl(node->key), node->masklen);
	  printf(" / Check_bit=%d\n", node->check_bit);
#endif
	  if ( ((node->key == prefix) && (node->masklen == masklen-1)) ||
	       							(masklen == 0)) {
	    if (masklen == 0) {
	      node = radix_top;
	    }
	    rlist = node->rinfo;
	    if (rlist == NULL) {
	      rcode = 1;
	      break;
	    }
	    brlist = NULL;
	    rcode = 1;
	    while(rlist != NULL) {
	      if (pn == check_id1topn(rlist->route_data->source)) {
	      /* delete route */
#ifdef RADIXDEBUG
	        printf("DEBUG: RADIX: DELETE: pref = %08X / Mask = %d",
	  	  ntohl(rlist->route_data->prefix), rlist->route_data->length);
	        printf(" / Neighbor = %08X\n", ntohl(rlist->route_data->source));
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
#ifdef IRRCHK
		    if ((rlist->route_data)->irr != NULL) {
		      free((rlist->route_data)->irr);
		      (rlist->route_data)->irr = NULL;
		    }
#endif
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
#ifdef IRRCHK
		  if ((rlist->route_data)->irr != NULL) {
		    free((rlist->route_data)->irr);
		    (rlist->route_data)->irr = NULL;
		  }
#endif
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
	        prefixes--;
	        rcode = 0;
	      } else {
	        brlist = rlist;
	        rlist = rlist->next;
	      }
	      if (node == radix_top) break;
	      /* Back Track for Removing node */
	      if (node->rinfo == NULL) {
	        node->masklen = 0;
	        routes--;
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
#ifdef RADIXDEBUG
		  errno = 0;
	          printf("DEBUG: RADIX: WITHDRAW: Freeing Node = %08X/%d CB=%d\n",
			ntohl(bnode->key), bnode->masklen, bnode->check_bit);
#endif
		  free(bnode);
		  nodes--;
#ifdef RADIXDEBUG
		  if (errno != 0) perror("DEBUG: RADIX: FreeError: ");
#endif
		  if (node->upleaf == NULL) break;
		  if (node->rinfo != NULL)  break;
	        }
	        break;
	      }
	    }
	    break;
	  }
	  if (0x80000000 & (ntohl(prefix) << (node->check_bit))) {
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

RADIX_T *show_route_all(RADIX_T *curnode, int *upcode)
{
	RADIX_T		*bnode;
	int		rcode;

	if (curnode == NULL) {
		/* Show route from TOP Node */
		bnode = radix_top;
		*upcode = 0;
#ifdef RADIXDEBUG
		printf("DEBUG: RADIX: Goto Most Left Leaf\n");
#endif
		while(bnode->l_leaf != NULL) {
			bnode = bnode->l_leaf;
#ifdef RADIXDEBUG
			printf("DEBUG: RADIX:   Move to Node = %08X\n",bnode);
#endif
		}
		if (bnode->rinfo != NULL) {
#ifdef RADIXDEBUG
			printf("DEBUG: RADIX:   Found First Route Data\n");
#endif
			return(bnode);
		}
	} else {
		/* Show Current Node */
		bnode = curnode;
	}

#ifdef RADIXDEBUG
	printf("DEBUG: RADIX: Start Tree Pointer = %08X\n", curnode);
#endif

	rcode = 0;
	while(1) {
#ifdef RADIXDEBUG
		printf("DEBUG: RADIX: ALLROUTE: Checkbit = %d / upcode=%d\n",
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

u_long delete_peer_route(RADIX_T **cnode, int pn)
{
	RADIX_T		*node, *bnode;
	R_LIST		*rlist, *brlist;
	u_long		bufnodes;
	ROUTE_INFO	*pathbuf;

	node = *cnode;
	rlist = node->rinfo;
	if (rlist == NULL) return(0);
	brlist = NULL;
	bufnodes = nodes;
	while(rlist != NULL) {
		if (pn == check_id1topn(rlist->route_data->source)) {
			/* delete route */
#ifdef RADIXDEBUG
	printf("DEBUG: RADIX: DELETE: pref = %08X / Mask = %d\n",
		ntohl(rlist->route_data->prefix), rlist->route_data->length);
	printf("                      Neighbor = %08X\n",
		ntohl(rlist->route_data->source));
#endif
			if (((rlist->route_data)->pathset_ptr) != NULL) {
				pathbuf = rlist->route_data;
				free(pathbuf->pathset_ptr);
				pathbuf->pathset_ptr = NULL;
			}
#ifdef IRRCHK
			if ((rlist->route_data)->irr != NULL) {
				free((rlist->route_data)->irr);
				(rlist->route_data)->irr = NULL;
			}
#endif
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
			prefixes--;
		} else {
			brlist = rlist;
			rlist = rlist->next;
		}
		/* Back Track for Removing node */
		if (node->rinfo == NULL) {
			node->masklen = 0;
			routes--;
			while(1) {
				bnode = node;
				node = node->upleaf;
				/*** Debug 2008/06/18 ***/
				if (node == NULL) break;

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
#ifdef RADIXDEBUG
				errno = 0;
	printf("DEBUG: RADIX: WITHDRAW: Freeing Node = %08X/%d CB=%d\n",
			ntohl(bnode->key), bnode->masklen, bnode->check_bit);
#endif
				free(bnode);
				nodes--;
#ifdef RADIXDEBUG
	if (errno != 0) perror("DEBUG: RADIX: FreeError: ");
#endif
				if (node->upleaf == NULL) break;
				if (node->rinfo != NULL)  break;
			}
			break;
		}
	}

	*cnode = node;
	return(bufnodes-nodes);
}

int withdraw_peer(int pn)
{
	RADIX_T		*bnode;
	int		rcode;
	static int	upcode;
	u_long		wdcode;

	bnode = radix_top;
	upcode = 0;
	while(bnode->l_leaf != NULL) {
		bnode = bnode->l_leaf;
	}
	if (bnode == NULL) {
#ifdef DEBUG
		printf("DEBUG: RADIX: DELPEER: No Route\n");
#endif
		return(0);
	}

	if (peer[pn].neighbor == 0) {
#ifdef DEBUG
		printf("DEBUG: RADIX: DELPEER: No Peer\n");
#endif
		return(0);
	}

#ifdef DEBUG
	printf("DEBUG: RADIX: Start Tree Pointer = %08X\n", bnode);
	pr_log2(VLOG_GLOB, LOG_ALL, ON,
			"RADIX: Start Bnode IN  Pointer = %08X\n", bnode);
#endif
	wdcode = delete_peer_route(&bnode, pn);
#ifdef DEBUG
	pr_log2(VLOG_GLOB, LOG_ALL, ON,
			"RADIX: Start Bnode OUT Pointer = %08X\n", bnode);
#endif
	

	while(1) {
		rcode = 0;
		while(1) {
#ifdef DEBUG
			printf("DEBUG: RADIX: DELPEER: Checkbit = %d / upcode=%d\n",
							bnode->check_bit, upcode);
#endif
			if (bnode == NULL) {
				pr_log2(VLOG_GLOB, LOG_ALL, ON,
					"BNODE == NULL!!! BUG? (upcode = %d)\n",
					upcode);
				rcode = 1;
				break;
			}

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
#ifdef DEBUG
				pr_log2(VLOG_GLOB, LOG_ALL, ON,
				"Set bnode->upleaf to bnode(upcode=%d): %08X\n",
							upcode, bnode->upleaf);
#endif
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
#ifdef DEBUG2
	pr_log2(VLOG_GLOB, LOG_ALL, ON,
			"RADIX: Bnode IN  Pointer = %08X\n", bnode);
#endif
		wdcode = delete_peer_route(&bnode, pn);
#ifdef DEBUG2
	pr_log2(VLOG_GLOB, LOG_ALL, ON,
			"RADIX: Bnode OUT Pointer = %08X\n", bnode);
#endif
		if (wdcode != 0) upcode = 0;
	}

#ifdef RTHISTORY
	withdraw_route(pn, 0, 0, DELETE);
#endif
#ifndef RTHISTORY
	withdraw_route(pn, 0, 0);
#endif
	return(0);
}

RADIX_T *search_route(net_ulong prefix, int masklen)
{
	RADIX_T		*node, *bnode;
	int		rcode;
	net_ulong	cmask;

	node = radix_top;

	if ((masklen == 0) || (prefix == 0)) {
		return(radix_top);
	}

	while(1) {
		if (node->check_bit > masklen-1) {
			break;
		}
#ifdef DEBUG
		printf("DEBUG: RADIX: SEARCH: Checking pref=%08X / mask=%d\n",
					ntohl(node->key), node->masklen);
		printf("                               Check_bit=%d\n",
						node->check_bit);
#endif

		if (0x80000000 & (ntohl(prefix) << (node->check_bit))) {
			if (node->r_leaf == NULL) break;
			node = node->r_leaf;
#ifdef DEBUG
			printf("DEBUG: RADIX: SEARCH: Goto right Route\n");
#endif
		} else {
			if (node->l_leaf == NULL) break;
			node = node->l_leaf;
#ifdef DEBUG
			printf("DEBUG: RADIX: SEARCH: Goto left Route\n");
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
			cmask = ((long)0xFFFFFFFF << (31-node->masklen) &
				 (long)0xFFFFFFFF << (32-masklen));
			/***
			cmask = ((long)0x80000000 >> (node->masklen)) & 
				       ((long)0x80000000 >> (masklen-1));
			****/
#ifdef DEBUG
			/****
			printf("DEBUG: RADIX: SEARCH: Check MASK p1 = %08X\n",
					((long)0x80000000 >> (node->masklen)));
			printf("DEBUG: RADIX: SEARCH: Check MASK p2 = %08X\n",
					((long)0x80000000 >> (masklen-1)));
			****/
			printf("DEBUG: RADIX: SEARCH: Check MASK p1 = %08X\n",
					((long)0xFFFFFFFF << (31-node->masklen)));
			printf("DEBUG: RADIX: SEARCH: Check MASK p2 = %08X\n",
					((long)0xFFFFFFFF << (32-masklen)));
			printf("DEBUG: RADIX: SEARCH: Check MASK to = %08X\n", cmask);
#endif
			if ((ntohl(prefix) & cmask) == ntohl(node->key)) break;
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
RADIX_T *clear_rt_history(RADIX_T *curnode, int *upcode)
{
	RADIX_T		*bnode;
	int		rcode;
	R_LIST		*rlist;
	ROUTE_INFO	*rinfo;
	RT_HISTORY	*histnext;

	if (curnode == NULL) {
		/* Show route from TOP Node */
		bnode = radix_top;
		*upcode = 0;
#ifdef DEBUG
		printf("DEBUG: RTHISTCLEAN: Goto Most Left Leaf\n");
#endif
		while(bnode->l_leaf != NULL) {
		    bnode = bnode->l_leaf;
#ifdef DEBUG
		    printf("DEBUG: RTHISTCLEAN:   Move to Node = %08X\n",bnode);
#endif
		}
		if (bnode->rinfo != NULL) {
#ifdef DEBUG
		    printf("DEBUG: RTHISTCLEAN:   Found First Route Data\n");
#endif
		    /** Cleaning Node **/
		    rlist = bnode->rinfo;
		    while(rlist != NULL) {
			rinfo = rlist->route_data;
			if (rinfo->active == false) {
			    withdraw_route(check_id1topn(rinfo->source),
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
				withdraw_route(check_id1topn(rinfo->source),
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

void rthisttimerclear()
{
	pr_log(VLOG_GLOB, LOG_ALL,
			 "Route Change History Clearing by timer\n", ON);
	if ( (bvs_clients+bvs_maxcon-1)->cont_code == -1) {
		/*** Another job is working ***/
		pr_log(VLOG_GLOB, LOG_ALL,
			 " ==> Another job is working...\n", OFF);
		rtclear_id = common_entry_timer(&rthistcleartime, 0,
							 rthisttimerclear);
		if (rtclear_id == -1) {
			pr_log(VLOG_GLOB, LOG_ALL,
				"Timer Set Error : rthisttimerclear - 1\n", ON);
			notify_systemnotify("STOP",
				"Timer Set Error : rthisttimerclear - 1");
			finish(PROC_SHUTDOWN);
			return;
		}
		return;
	}

	strcpy((bvs_clients+bvs_maxcon-1)->cmdline, "clear route history");
	bvs_exec_command(bvs_maxcon-1);

	if (rthistcleartime.tv_sec != 0) {
		rtclear_id = common_entry_timer(&rthistcleartime, 0,
							 rthisttimerclear);
		if (rtclear_id == -1) {
			pr_log(VLOG_GLOB, LOG_ALL,
				"Timer Set Error : rthisttimerclear - 1\n", ON);
			notify_systemnotify("STOP",
				"Timer Set Error : rthisttimerclear - 1");
			finish(PROC_SHUTDOWN);
			return;
		}
	}
}

int start_rthistclear_timer()
{

	if (rthistcleartime.tv_sec == 0) return(0);

	rtclear_id = common_entry_timer(&rthistcleartime, 0, rthisttimerclear);
	if (rtclear_id == -1) return(1);

	return(0);
}

#endif /** RTHISTORY **/
