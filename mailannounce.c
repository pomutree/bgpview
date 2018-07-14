/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 2000/03                         */
/*                                                              */
/* Mail Notification                                            */
/****************************************************************/
/* mailannounce.c,v 1.1 2003/03/31 05:51:04 kuniaki Exp */

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

#include "mailannounce.h"

char sendmail_command[FNAMESIZE];
char mail_addr[FNAMESIZE];
char admin_addr[FNAMESIZE];

void init_mailannounce()
{
	strcpy(sendmail_command, DEFAULT_SENDMAILCMD);
	strcpy(mail_addr, "");
	strcpy(admin_addr, "");
}

int open_mail_pipe(int *wfp, int *rfp, char *maddr)
{
	int 	wri[2],rea[2];
	int	pid;

	/* Create Pipe */
	if (pipe(wri) < 0){
		return(1);
	}
	if (pipe(rea) < 0) {
		close(wri[0]);
		close(wri[1]);
		return(1);
	}
	if ((pid = fork()) < 0) {
		close(wri[0]);
		close(wri[1]);
		close(rea[0]);
		close(rea[1]);
		return(2);
	}

	/* chile process execute */
	if (pid == 0) {
		close(wri[1]);
		close(rea[0]);

		close(0);
		dup(wri[0]);
		close(wri[0]);

		close(1);
		dup(rea[1]);
		close(rea[1]);

		execl( sendmail_command, "sendmail", maddr, (char *)NULL);
		perror("exec error");
		return(3);
	}

	close(wri[0]);
	close(rea[1]);

	*wfp = wri[1];
	*rfp = rea[0];

	return(0);
}

int mail_enable(int pn)
{
	int	mcnt;

	if (strlen(admin_addr) == 0) return(1);

	if (pn != -1) {
		mcnt = 0;
		if (pn >= 0) {
			if (strlen(peer[pn].admin_addr) != 0) mcnt++;
		}
	}
	if (strlen(mail_addr) != 0) mcnt++;

	if  (mcnt == 0) return(1);
	return(0);
}

int notify_peer_status_change(int pn)
{
	time_t		clock;
	struct tm	*tm;
	int 		wfp, rfp;
	char		message[FNAMESIZE];
	char		buf[TIME_BUF];
	int		ret;
	struct in_addr	adr;
	char		maddr[FNAMESIZE];

	if (mail_enable(pn) != 0) return(0);
	if (peer[pn].status_notify == false) return(0);
	if (strlen(mail_addr) != 0) {
		strcpy(maddr, mail_addr);
		if (strlen(peer[pn].admin_addr) != 0) {
			strcat(maddr, ",");
			strcat(maddr, peer[pn].admin_addr);
		}
	} else {
		strcpy(maddr, peer[pn].admin_addr);
	}

	if (open_mail_pipe(&wfp, &rfp, maddr) != 0) {
		pr_log(VLOG_GLOB, LOG_ALL, 
		  "Executing Sendmail Fail : notify_peer_status_change\n", ON);
		return(1);
	}
	
	sprintf(message, "Subject: [BV_NOTIFY] Peer %d Status Change : %s\n",
				 		pn, peer[pn].description);
	write(wfp, message, strlen(message));
	sprintf(message, "From: BGPView Auto Message <%s>\n", admin_addr);
	write(wfp, message, strlen(message));
	sprintf(message, "To: %s\n\n", maddr);
	write(wfp, message, strlen(message));
	write(wfp, NOTIFY_HEADER, strlen(NOTIFY_HEADER));
	write(wfp, "\n", 1);

	sprintf(message, "  System Name     : %s\n", sysname);
	write(wfp, message, strlen(message));
	time(&clock);
	tm = localtime(&clock);
	ret = strftime(buf, TIME_BUF, "%Y/%m/%d %H:%M:%S", tm);
	if (ret != 0) {
		sprintf(message, "  Notify Time  : %s\n", buf);
		write(wfp, message, strlen(message));
	}
	write(wfp, "  Notify Method: BGP Status Change\n", 35);

	adr.s_addr = peer[pn].local_id;
	sprintf(message, "  Local IDENT  : %s\n", inet_ntoa(adr));
	write(wfp, message, strlen(message));

	adr.s_addr = peer[pn].neighbor;
	sprintf(message, "  Changed Peer : %s (Peer No. %d)\n", inet_ntoa(adr),
				pn);
	write(wfp, message, strlen(message));

	sprintf(message, "  Peer Name    : %s\n", peer[pn].description);
	write(wfp, message, strlen(message));

	switch(peer[pn].status) {
	case IDLE:
		write(wfp, "  BGP Status   : change to IDLE\n", 32);
		break;
	case ESTABLISHED:
		write(wfp, "  BGP Status   : change to ESTABLISHED\n", 39);
		break;
	defalut:
		sprintf(message, "  BGP Status   : un-supported code -> %d\n",
								peer[pn].status);
		write(wfp, message, strlen(message));
	}

	write(wfp, "\n", 1);
	write(wfp, NOTIFY_FOOTER, strlen(NOTIFY_FOOTER));
	
	close(wfp);
	close(rfp);
	return(0);
}

int notify_systemnotify(const char *action, const char *reason)
{
	time_t		clock;
	struct tm	*tm;
	int 		wfp, rfp;
	char		message[FNAMESIZE];
	char		buf[TIME_BUF];
	int		ret;
	struct in_addr	adr;
	char		maddr[FNAMESIZE];

	if (mail_enable(-1) != 0) return(0);
	strcpy(maddr, mail_addr);

	if (open_mail_pipe(&wfp, &rfp, maddr) != 0) {
		pr_log(VLOG_GLOB, LOG_ALL, 
		  "Executing Sendmail Fail : notify_peer_status_change\n", ON);
		return(1);
	}
	
	sprintf(message, "Subject: [BV_NOTIFY] : %s\n", action);
	write(wfp, message, strlen(message));
	sprintf(message, "From: BGPView Auto Message <%s>\n", admin_addr);
	write(wfp, message, strlen(message));
	sprintf(message, "To: %s\n\n", maddr);
	write(wfp, message, strlen(message));
	write(wfp, NOTIFY_HEADER, strlen(NOTIFY_HEADER));
	write(wfp, "\n", 1);

	sprintf(message, "  System Name     : %s\n", sysname);
	write(wfp, message, strlen(message));
	time(&clock);
	tm = localtime(&clock);
	ret = strftime(buf, TIME_BUF, "%Y/%m/%d %H:%M:%S", tm);
	if (ret != 0) {
		sprintf(message, "  Notify Time     : %s\n", buf);
		write(wfp, message, strlen(message));
	}
	sprintf(message, "  BGPView Action  : %s\n", action);
	write(wfp, message, strlen(message));
	sprintf(message, "  Notify Method   : %s\n", reason);
	write(wfp, message, strlen(message));

	write(wfp, "\n", 1);
	write(wfp, NOTIFY_FOOTER, strlen(NOTIFY_FOOTER));
	
	close(wfp);
	close(rfp);
	return(0);
}
