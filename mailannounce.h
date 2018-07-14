/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 2000/03                         */
/*                                                              */
/* Mail Notification                                            */
/****************************************************************/
/* mailannounce.h,v 1.2 2003/09/25 02:04:06 kuniaki Exp */

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

#ifndef _MAILANNOUNCE_H
#define _MAILANNOUNCE_H

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "bgp.h"
#include "bgplog.h"

#define DEFAULT_SENDMAILCMD	"/usr/sbin/sendmail"
#define NOTIFY_HEADER		"This is BGPView notification mail.\n"
#define NOTIFY_FOOTER		"----\nbgpview-admin@ate-mahoroba.jp\n"

extern char sendmail_command[];
extern char mail_addr[];
extern char admin_addr[];

extern void init_mailannounce();
extern int notify_peer_status_change(/* int pn, int status */);
extern int notify_systemnotify(/* const char *action, const char *reason */);

#endif /* _MAILANNOUNCE_H */
