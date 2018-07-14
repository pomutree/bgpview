/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 2000/8                          */
/*                                                              */
/* Utility Sub-routines                                         */
/****************************************************************/
/* utils.h,v 1.1 2003/03/31 05:51:04 kuniaki Exp */

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

#ifndef _UTILS_H
#define _UTILS_H

#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include "bgp.h"
#include "bvtypes.h"
#include "route.h"
#include "route_ipv6.h"


#define OFF                     0
#define ON                      1

#define CHKFMT_ADR	1
#define CHKFMT_PRF	2
#define CHKFMT_INT	3
#define CHKFMT_V6A	4	/* IPv6 Address */
#define CHKFMT_V6P	5	/* IPv6 Prefix */

#define ADR_CSET	"0123456789."
#define PRF_CSET	"0123456789./"
#define INT_CSET	"0123456789"
#define V6A_CSET	"0123456789ABCDEF:"
#define V6P_CSET	"0123456789ABCDEF:/"

/* Functions */
extern u_short 	buf2ushort(/*ptr*/);
extern u_long 	buf2ulong(/*ptr*/);
extern void 	store_int16(/*ptr, var*/);
extern void 	store_int32(/*ptr, var*/);
extern boolean	chkfmt(/* int type, char *str */);
extern int	checkpw(/* char *phrase, char *pass */);
extern void 	wait_sigchld();
extern int 	bintostrv6(/* char *ptr, char *str */);
extern char 	*strtobinv6(/* char *ptr, char *str */);
extern void	store_v6addr(/* char *dest, char *src */);
extern void 	v6addrmask(/* char *adrs, int masklen */);
extern int 	v6addrcmp(/* char *adrs1, char *adrs2, int masklen */);
extern char	checkafi(/* char *adrs */);
extern u_long 	fourbyteastoulong(char *asstr);
extern char 	*ulongtofourbyteas(u_long as4);

#endif /* _UTILS_H */
