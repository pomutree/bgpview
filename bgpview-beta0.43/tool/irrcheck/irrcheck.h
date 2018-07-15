/********************************************************************/
/* This is IRR Route Checker that checks received routes by BGPView */
/* using IRR.                                                       */
/*                                                                  */
/* Usage:                                                           */
/*  irrcheck [BGPView Prefix Log File]                              */
/********************************************************************/
/* irrcheck.h,v 1.1 2004/12/08 05:41:07 kuniaki Exp */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define VERSION		"0.1"
#define PRODUCTNAME	"IrrCheck for BGPView"
#define COPYRIGHT	"Copyright (C) 2004 K.Kondo Intec NetCore, Inc."

#define DEFAULT_IRR_HOST	"jpirr.nic.ad.jp"
#define DEFAULT_IRR_PORT	43

typedef struct {
	struct in_addr	addr;
	u_short		plen;
	u_long		asn;
	char		src[64];
} OBJ_DATA;

typedef struct {
	int		items;
	OBJ_DATA	*item;
} SEARCH_RESULT;

