/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 2000/01                         */
/*                                                              */
/* BGPView Shell / Defines and Structs                          */
/****************************************************************/
/* bvshell.h,v 1.1 2003/03/31 05:51:04 kuniaki Exp */

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

#ifndef _BVSHELL_H
#define _BVSHELL_H

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "bgp.h"

#define BVS_PORT_DEFAULT	3000
#define BVS_MAXCON_DEFAULT	4
#define BVS_MAXIMUM		128

#define SET_BVS_PORT		1
#define SET_BVS_MAXCON		2
#define SET_BVS_PASSWD		3
#define SET_BVS_ROWSIZE		4

#define MAXIMUM_CMD_LENGTH	128

/* #define BVS_PROMPT		"BGPView# " */
#define BVSUSER_USER		0
#define BVSUSER_ADMIN		1
#define BVSPROMPTMKLIST		"%#"
#define DEFUSERLEVEL		BVSUSER_ADMIN

#define DEF_ROWSIZE		24

/* Client Status */
#define BVSC_LOGIN		1
#define BVSC_INPUT_CMD		2
#define BVSC_EXEC_CMD		3
#define BVSC_LOGOUT		4

/* Command Status */
#define BVSC_CMD_WAIT		1
#define BVSC_CMD_EXEC		2

/* Commands */
#define BVSC_WRCMAX		64
#define BVSC_WORDMAX		128
#define BVSC_HELPMAXC		128
#define BVSC_MAXHISTORY		8

/* Char Edit Mode */
#define BVSE_INS		1
#define BVSE_DEL		2
#define BVSE_BS			3

/* Continue Process Code */
#define BVS_CNT_END		0
#define BVS_CNT_CONTINUE	1
#define BVS_CNT_FINISH		2
#define BVS_CNT_NOPROMPT	3

typedef union address {
	char		v6addr[IPV6_ADDR_LEN];
	net_ulong	v4addr;
} COMMON_ADDRESS;

typedef union {
        struct {
                int     clno;
        } noparam;
        struct {
                int     clno;
                u_long  addr;
        } adrs;
        struct {
                int     clno;
                u_long  neighbor;
                u_long  prefix;
                int     prefixlen;
                int     timeout;
        } routertt;
        struct {
                int     clno;
                int     num;
        } one_num;
        struct {
                int     clno;
                u_long  prefix;
                int     prefixlen;
        } one_route;
        struct {
                int     clno;
                char    str[64];
        } one_str64;
        struct {
                int     clno;
                char    str[64];
                u_long  addr;
        } one_str64_adrs;
        struct {
                int     clno;
                char    longstr[128];
        } longstr128;
        struct {
                int     clno;
                u_long  addr;
                boolean swc;
        } adrs_boolean;
        struct {
        	int	clno;
        	char	v6addr[IPV6_ADDR_LEN];
        } ipv6addr;
        struct {
        	int	clno;
        	char	v6addr[IPV6_ADDR_LEN];
        	int	prefixlen;
        } ipv6prefix;
        struct {
        	int		clno;
		COMMON_ADDRESS	address;
        	char	afi;
        } ipv46addr;
        struct {
        	int		clno;
        	char		str[64];
        	COMMON_ADDRESS	address;
        	char		afi;
        } onestr_ipv46addr;
} BVS_CMD_PARAM;


typedef struct {
	int		clsock;	/* Client Socket */
	u_long		addr;	/* Client Address */
	int		status; /* Client Status */
	struct timeval	lgtime;	/* Login time */
	boolean		cmd_lock; /* Command input Lock */
	char		cmdline[MAXIMUM_CMD_LENGTH+1];
	char		history[BVSC_MAXHISTORY][MAXIMUM_CMD_LENGTH+1];
	char		userlevel;
	int		curhitory;
	int		histnum;
	int		col;
	int		esc;
	int		rowsize;
	int		nextmore;
	int		cont_code;
	int		(*cont_func)(BVS_CMD_PARAM *);
	BVS_CMD_PARAM	cont_param;
	boolean		enable_more;
} BVS_CLIENT;

typedef struct {
	char		word[BVSC_WRCMAX];
	int		(*function)(BVS_CMD_PARAM *);
	void		*next;
	void		*lower;
	char		help[BVSC_HELPMAXC];
	boolean		more;
} BVS_CMDS;

/* Valiables */
extern int		bvs_maxcon;
extern BVS_CLIENT	*bvs_clients;
extern char		bvs_prompt[32];

/* Functions */
extern void bvs_init_set_default();
extern int  bvs_init_set_values(/* int type, char *datas */);
extern int  bvs_init_start();
extern void bvs_finish();
extern void bvs_readsocset(/* fd_set *fds */);
extern int  bvs_cont_proc();
extern int  bvs_input(/* fd_set *fds */);
extern int  bvs_open_new_shell(/* fd_set *fds */);
extern int  bvs_write(/* int clno, char *str */);

#endif /* _BVSHELL_H */
