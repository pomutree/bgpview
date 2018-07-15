/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1997/12                         */
/*                                                              */
/* Timer Routine                                                */
/****************************************************************/
/* timer.c,v 1.2 2004/07/13 08:58:44 kuniaki Exp */

/*
 * Copyright (c) 1998-2001 Internet Initiative Japan Inc.
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

/*
 * Copyright (c) 2006 Atelier Mahoroba
 * All rights Reserved.
 */

#include "timer.h"

CRON_LIST	*cron_list[MAX_CRON];
char		cron_file[1024];

static TIMERLIST tl_head = {
	&tl_head, &tl_head
};

static COMMON_TIMER_LIST common_tmlist[MAX_TIMER_TABLE];

/* t1>t2:1 / t1=t2:0 / t1<t2:-1 */
int timer_cmp(t1, t2)
struct timeval *t1, *t2;
{
	if (t1->tv_sec > t2->tv_sec) return 1;
	if (t1->tv_sec < t2->tv_sec) return -1;

	if (t1->tv_usec > t2->tv_usec) return 1;
	if (t1->tv_usec < t2->tv_usec) return -1;

	return 0;
}

/* time val add */
void timer_add(t1, t2, tv)
struct timeval *t1, *t2, *tv;
{
	tv->tv_sec  = t1->tv_sec  + t2->tv_sec;
	tv->tv_usec = t1->tv_usec + t2->tv_usec;

	if (tv->tv_usec >= TIME_SEC_USEC) {
		tv->tv_usec -= TIME_SEC_USEC;
		tv->tv_sec++;
	}
}

/* time sub */
void timer_sub(t1, t2, tv)
struct timeval *t1, *t2, *tv;
{
	tv->tv_sec  = t1->tv_sec  - t2->tv_sec;
	tv->tv_usec = t1->tv_usec - t2->tv_usec;

	if (tv->tv_usec < 0) {
		tv->tv_usec += TIME_SEC_USEC;
		tv->tv_sec--;
	}
}

/* time diff */
void timer_diff(t1, t2, tv)
struct timeval *t1, *t2, *tv;
{
	switch(timer_cmp(t1, t2)) {
	case 1:
		timer_sub(t1, t2, tv);
		break;
	case -1:
		timer_sub(t2, t1, tv);
		break;
	case 0:
		tv->tv_sec = tv->tv_usec = 0;
		break;
	}
}

void clear_timer()
{
	TIMERLIST *tl;

#ifdef DEBUG
	printf("Clearing Timer\n");
#endif 

	while(tl_head.next != &tl_head) {
		tl = tl_head.next;
		tl_head.next = tl->next;
		free((char *)tl);
	}
	tl_head.prev = &tl_head;

	bzero(common_tmlist, sizeof(COMMON_TIMER_LIST)*MAX_TIMER_TABLE);
}

boolean entry_timer(struct timeval *et_time, net_ulong id1, int id2)
{
	TIMERLIST *tl, *new_tl;
	struct timeval adder;
#ifdef DEBUG
	struct in_addr adr;
#endif

	if ((new_tl = (TIMERLIST *)malloc(sizeof(TIMERLIST))) == NULL)
		return false;

#ifdef DEBUG
	adr.s_addr = id1;
	printf("Entrying Timer :\n");
	printf("  Time : %d . %d\n", et_time->tv_sec, et_time->tv_usec);
	printf("  ID   : ID1->%s  ID2->%d\n", inet_ntoa(adr), id2);
#endif

	gettimeofday(&adder, (struct timezone *)0);
	timer_add(&adder, et_time, &adder);

#ifdef DEBUG
	printf("  Set Time : %d . %d\n", adder.tv_sec, adder.tv_usec);
#endif

	    /* linking */
	tl = tl_head.next;
	while(tl != &tl_head) {
		if (timer_cmp(&tl->timeout, &adder) == 1)
			break;
		tl = tl->next;
	}
	new_tl->prev = tl->prev;
	new_tl->next = tl;
	tl->prev->next = new_tl;
	tl->prev = new_tl;

	new_tl->id1 = id1;
	new_tl->id2 = id2;
	new_tl->timeout = adder;
	return true;
}

int off_timer(id1, id2)
net_ulong id1;
int id2;
{
	TIMERLIST *tl;
#ifdef DEBUG
	struct in_addr adr;

	adr.s_addr = id1;
#endif

	tl = tl_head.next;
	while(tl != &tl_head) {
		if ((tl->id1 == id1) && (tl->id2 == id2)) {
#ifdef DEBUG
			printf("OFF Timer : %s / %d\n", inet_ntoa(adr), id2);
#endif
			    /* unlinking */
			tl->prev->next = tl->next;
			tl->next->prev = tl->prev;
			free((char *)tl);
			return 1;
		}
		tl = tl->next;
	}
#ifdef DEBUG
	printf("OFF Timer Fail : %s / %d\n", inet_ntoa(adr), id2);
#endif
	return 0;
}

int peer_off_timer(id1)
net_ulong id1;
{
	TIMERLIST *tl, *tmp;
#ifdef DEBUG
	struct in_addr adr;

	adr.s_addr = id1;
#endif

	tl = tl_head.next;
	while(tl != &tl_head) {
		if ((tl->id1 == id1) && (tl->id2 != ID_BGPINFO_TIMER)) {
#ifdef DEBUG
			printf("OFF Timer : %s / %d\n",
						inet_ntoa(adr), tl->id2);
#endif
			tmp = tl;
			    /* unlinking */
			tl->prev->next = tl->next;
			tl->next->prev = tl->prev;
			free((char *)tl);
			tl = tmp;
		}
		tl = tl->next;
	}
	return 0;
}

/* Check Timer */
void check_timer(id1, id2, tv)
net_ulong *id1;
int *id2;
struct timeval *tv;
{
	struct timeval nowtime;
	TIMERLIST *tl;
#ifdef DEBUG
	struct in_addr adr;
#endif

#ifdef DEBUG
	printf("Checking Timer :");
#endif

	tl = tl_head.next;
	if (tl == &tl_head) {
#ifdef DEBUG
		printf(" Timer is Not Set\n");
#endif
		*id1 = 0;
		*id2 = 0;
		tv->tv_sec = tv->tv_usec = 0;
		return;
	}

	gettimeofday(&nowtime, (struct timezone *)0);

	if (timer_cmp(&tl->timeout, &nowtime) >= 0) {
		timer_sub(&tl->timeout, &nowtime, tv);
		/* tv->tv_sec++; */
#ifdef DEBUG
		adr.s_addr = tl->id1;
		printf(" Suply Wait time : %ld . %ld : %s / %d\n",
			tv->tv_sec, tv->tv_usec, inet_ntoa(adr), tl->id2);
#endif
		*id1 = 0;
		*id2 = 0;
	} else {
		tv->tv_sec = tv->tv_usec = 0;
		*id1 = tl->id1;
		*id2 = tl->id2;
		off_timer(*id1, *id2);

#ifdef DEBUG
		adr.s_addr = *id1;
		printf(" Cause Timeout: %s / %d\n", inet_ntoa(adr), *id2);
#endif
	}
}

int common_entry_timer(struct timeval *tv,int param,void (*function)(int, int))
{
	int	comid;
	int	cnt;

	cnt = 0;
	while(common_tmlist[cnt].function != NULL) {
		cnt++;
		if (cnt >= MAX_TIMER_TABLE) break;
	}
	if (cnt >= MAX_TIMER_TABLE) {
#ifdef DEBUG
		printf("Common timer could not set.\n");
#endif
		return(-1);
	}
	comid = cnt;
	if (entry_timer(tv, COMMON_ID1, comid) == false) {
#ifdef DEBUG
		printf("Common timer could not set by entry_timer\n");
#endif
		return(-1);
	}
	common_tmlist[comid].param = param;
	common_tmlist[comid].function = function;

	return(comid);
}
		
int common_off_timer(int common_id)
{
	int ret;
	ret = off_timer(COMMON_ID1, common_id);
	common_tmlist[common_id].param = 0;
	common_tmlist[common_id].function = NULL;
	return(ret);
}

void common_timer_expire(int common_id)
{
	int 	prm;
	void    (*function)(int, int);

#ifdef DEBUG
	printf("DEBUG: COM_TIMER: Expire Process ID = %d\n", common_id);
#endif
	prm = common_tmlist[common_id].param;
	function = common_tmlist[common_id].function;

	common_tmlist[common_id].param = 0;
	common_tmlist[common_id].function = NULL;

	if (function != NULL) {
		(*function)(common_id, prm);
	}
#ifdef DEBUG
	if (function == NULL) {
		printf("DEBUG: COM_TIMER: Function is NULL\n");
	}
#endif
	

}

void execute_cron()
{
	time_t		cclock;
	struct tm	*tm;
	struct timeval	tv;
	int		cnt;
	int		ret;

	time(&cclock);
	for(cnt=0; cnt<MAX_CRON; cnt++) {
		if (cron_list[cnt] == NULL) {
			continue;
		}
		ret = cron_setnext(cnt, cclock);
		if (ret == 0) {
			pr_log(VLOG_GLOB, LOG_ALL,
				 "Cron Job Execute Error.\n", ON);
		}
#ifdef DEBUG
		if (ret == 2) {
			printf("DEBUG: EXECUTE_CRON: Different Time(Normal)\n");
		} else {
			printf("DEBUG: EXECUTE_CRON: Match Time?\n");
		}
#endif
	}

	time(&cclock);
	tm = localtime(&cclock);
	tv.tv_sec = 60 - tm->tm_sec;
	tv.tv_usec = 0;
	cron_id = common_entry_timer(&tv, 0, execute_cron);
	if (cron_id == -1) {
		pr_log(VLOG_GLOB, LOG_ALL, 
				"Cron Timer Set Fail : execute_cron\n", ON);
		notify_systemnotify("STOP", 
					"Cron Timer Set Fail : execute_cron");
		finish(PROC_SHUTDOWN);
		return;
	}

	exec_cmdqueue();
}

void init_cron()
{
	struct timeval	tv;
	time_t		clock;
	struct tm	*tm;

	bzero(cron_list, sizeof(CRON_LIST *) * MAX_CRON);
	strcpy(cron_file, DEFAULT_CRONF);
	bzero(cmd_queue, MAX_CRON*1024);

	time(&clock);
	tm = localtime(&clock);
	tv.tv_sec = 60 - tm->tm_sec;
	tv.tv_usec = 0;
	cron_id = common_entry_timer(&tv, 0, execute_cron);
	if (cron_id == -1) {
		pr_log(VLOG_GLOB, LOG_ALL,
				 "Cron Timer Set Error\n", ON);
		exit(1);
	}

	qretry_id = -1;
}

void set_cronfile(char *setf)
{
	strcpy(cron_file, setf);
}

void clear_cron()
{
	int	cnt;

#ifdef DEBUG
	printf("DEBUG: CEALRCRON: clear cron command queue\n");
#endif
	/* bzero(cmd_queue, MAX_CRON*1024); */
	for(cnt = 0; cnt < MAX_CRON; cnt++) {
		if (cron_list[cnt] == NULL) continue;
		free(cron_list[cnt]);
		cron_list[cnt] = NULL;
	}
#ifdef DEBUG
	printf("DEBUG: CEALRCRON: clear cron list\n");
#endif
}

int nexttime(char *cstr, int dtime)
{
	char	*cptr;
	char	line[64];
	int	ibuf,cnt;
	int	ret;

	strcpy(line, cstr);

	/*** strcheck ***/
	cnt = 0;
	while(line[cnt] != '\0') {
		if (!(((line[cnt] >= '0') && (line[cnt] <= '9')) || 
		    (line[cnt] == ',') || (line[cnt] == '*'))) {
			pr_log(VLOG_GLOB, LOG_ALL,
				 "Cron Time Field Syntax Error.\n", ON);
			return(-1);
		}
		cnt++;
	}

	/** Time Check **/
	ret = -1;
	cptr = strtok(line, ",");
	while(cptr != NULL) {
		if (*cptr == '*') {
			ret = dtime;
			break;
		}
		ibuf = atoi(cptr);
		if (dtime == ibuf) {
			ret = ibuf;
			break;
		}
		cptr = strtok(NULL, ",");
	}

	return(ret);

}

int cron_setnext(int cron_id, time_t curtime)
{
	struct tm	*tm;
	int		dret,buf;

	tm = localtime(&curtime);

	if (nexttime(cron_list[cron_id]->min, tm->tm_min) < 0) {
		return(2);
	} else if (nexttime(cron_list[cron_id]->hour, tm->tm_hour) < 0) {
		return(2);
	} else if (nexttime(cron_list[cron_id]->day, tm->tm_mday) < 0) {
		return(2);
	} else if (nexttime(cron_list[cron_id]->month, tm->tm_mon) < 0) {
		return(2);
	} else if (nexttime(cron_list[cron_id]->year, tm->tm_year) < 0) {
		return(2);
	}
	buf = 0;
	while(strlen(cmd_queue[buf]) != 0) {
		buf++;
		if (buf >= MAX_CRON) return(0);
	}
#ifdef DEBUG
	printf("DEBUG: CRON SET QUEUE: %d:%s\n", 
					buf, cron_list[cron_id]->command);
#endif
	strcpy(cmd_queue[buf], cron_list[cron_id]->command);
	return(1);
}

int read_cron_file()
{
	FILE 		*fp;
	char		line[1024];
	char		cmdl[1024];
	char		*ptr;
	int		cnt;
	CRON_LIST	*cl;
	time_t		cclock;

#ifdef DEBUG
	printf("DEBUG: READCRONCONF: Called\n");
#endif
	clear_cron();
	if ((fp = fopen(cron_file, "r")) == NULL) {
		pr_log(VLOG_GLOB, LOG_ALL, "Cron File could not open.\n", ON);
#ifdef DEBUG
		printf("DEBUG: CRONFILE: %s\n", cron_file);
#endif
		return(-1);
	}

	cnt = 0;
	fgets(line, 1024, fp);
	while(!feof(fp)) {
#ifdef DEBUG
		printf("DEBUG: READCRON: READ:%s", line);
#endif
		if (line[0] == '#') {
			fgets(line, 1024, fp);
			continue;
		}

		/*** initial and getting min ***/
		ptr = strtok(line, " \n");
		if (ptr == NULL) {
#ifdef DEBUG
			printf("DEBUG: READCRON: Illegal Line\n");
#endif
			fgets(line, 1024, fp);
			continue;
		} else {
#ifdef DEBUG
			printf("DEBUG: READCRON: New Cron : /min/ %s\n", ptr);
#endif
			cl = (CRON_LIST *)malloc(sizeof(CRON_LIST));
			if (cl == NULL) {
				pr_log(VLOG_GLOB, LOG_ALL,
					 "Cron list could not make.\n", ON);
				clear_cron();
				return(-1);
			}
			strcpy(cl->min, ptr);
		}

		/*** getting hour ***/
		ptr = strtok(NULL, " \n");
		if (ptr == NULL) {
			pr_log(VLOG_GLOB, LOG_ALL, "Cron entry error.\n", ON);
			free(cl);
			fgets(line, 1024, fp);
			continue;
		} else {
			strcpy(cl->hour, ptr);
		}

		/*** getting day ***/
		ptr = strtok(NULL, " \n");
		if (ptr == NULL) {
			pr_log(VLOG_GLOB, LOG_ALL, "Cron entry error.\n", ON);
			free(cl);
			fgets(line, 1024, fp);
			continue;
		} else {
			strcpy(cl->day, ptr);
		}

		/*** getting month ***/
		ptr = strtok(NULL, " \n");
		if (ptr == NULL) {
			pr_log(VLOG_GLOB, LOG_ALL, "Cron entry error.\n", ON);
			free(cl);
			fgets(line, 1024, fp);
			continue;
		} else {
			strcpy(cl->month, ptr);
		}

		/**** getting year ***/
		ptr = strtok(NULL, " \n");
		if (ptr == NULL) {
			pr_log(VLOG_GLOB, LOG_ALL, "Cron entry error.\n", ON);
			free(cl);
			fgets(line, 1024, fp);
			continue;
		} else {
			strcpy(cl->year, ptr);
		}

		/*** getting command string ***/
		strcpy(cmdl, "");
		ptr = strtok(NULL, " \n");
		if (ptr == NULL) {
			pr_log(VLOG_GLOB, LOG_ALL, "Cron entry error.\n", ON);
			free(cl);
			fgets(line, 1024, fp);
			continue;
		} else {
			while(ptr != NULL) {
				strcat(cmdl, ptr);
				strcat(cmdl, " ");
				ptr = strtok(NULL, " \n");
			}
			strcpy(cl->command, cmdl);
#ifdef DEBUG
			printf("DEBUG: READCRON: CMD: %s\n", cmdl);
#endif
		}
		cron_list[cnt] = cl;
		cnt++;
		fgets(line, 1024, fp);
	}
	fclose(fp);

	return(0);
}


void exec_cmdqueue()
{
	int	cnt;
	struct timeval tv;
	char	prline[1024];

	if (strlen(cmd_queue[0]) == 0) {
#ifdef DEBUG
		printf("DEBUG: EXECUTE_CRON_QUEUE: No Command List in Queue\n");
#endif
		return;
	}

	if ((bvs_clients+bvs_maxcon-1)->cont_code == -1) {
		tv.tv_sec = CRON_QRETRY;
		tv.tv_usec = 0;
		if (qretry_id != -1) {
			common_off_timer(qretry_id);
		}
#ifdef DEBUG
		printf("DEBUG: EXECUTE_CRON_QUEUE: Job Working Retry Timer Set\n");
#endif
		qretry_id = common_entry_timer(&tv, 0, exec_cmdqueue);
		if (qretry_id == -1) {
			pr_log(VLOG_GLOB, LOG_ALL,
					 "Cron Retry Timer Set Fail\n", ON);
		}
		return;
	}

#ifdef DEBUG
	printf("DEBUG: EXECUTE_CRON_QUEUE: CMD: %s\n", cmd_queue[0]);
#endif

	strcpy((bvs_clients+bvs_maxcon-1)->cmdline, cmd_queue[0]);
	bvs_exec_command(bvs_maxcon-1);
	sprintf(prline, "Cron: %s\n", cmd_queue[0]);
	pr_log(VLOG_GLOB, LOG_ALL, prline, ON);


#ifdef DEBUG
	cnt = 0;
	while(strlen(cmd_queue[cnt]) != 0){
		printf("DEBUG: EXECUTE_CRON_QUEUE: %d:%s\n",
			cnt, cmd_queue[cnt]);
		cnt++;
		if (cnt >= MAX_CRON) break;
	}
#endif

	cnt = 0;
	while(strlen(cmd_queue[cnt]) != 0){
#ifdef DEBUG
		printf("DEBUG: EXECUTE_CRON_QUEUE: QUEUE Re-list\n");
#endif
		strcpy(cmd_queue[cnt], cmd_queue[cnt+1]);
		if (cnt+1>=MAX_CRON) {
			strcpy(cmd_queue[cnt+1], "");
		}
		cnt++;
	}
	if (strlen(cmd_queue[0]) != 0) {
#ifdef DEBUG
		printf("DEBUG: EXECUTE_CRON_QUEUE: Re-execute CRON Queue\n");
#endif
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		qretry_id = common_entry_timer(&tv, 0, exec_cmdqueue);
		if (qretry_id == -1) {
			pr_log(VLOG_GLOB, LOG_ALL,
					 "Cron Retry Timer Set Fail.\n", ON);
		}
	}
	return;
}

#ifdef TEST
main()
{
	struct timeval stopper;
	net_ulong i1;
	int i2;
	TIMERLIST *tl;

	clear_timer();
	stopper.tv_sec = 10;
	stopper.tv_usec = 0;
	if (!entry_timer(&stopper, 1, 1)) {
		printf("failed to allocate memory.\n");
		exit(1);
	}
	stopper.tv_sec = 15;
	stopper.tv_usec = 0;
	if (!entry_timer(&stopper, 1, 2) {
		printf("failed to allocate memory.\n");
		exit(1);
	}
	stopper.tv_sec = 20;
	stopper.tv_usec = 0;
	if (!entry_timer(&stopper, 1, 10) {
		printf("failed to allocate memory.\n");
		exit(1);
	}
	stopper.tv_sec = 5;
	stopper.tv_usec = 0;
	if (!entry_timer(&stopper, 2, 1) {
		printf("failed to allocate memory.\n");
		exit(1);
	}

	tl = tl_head.next;
	while(tl != &tl_head) {
		printf("ID1:%d ID2:%d TSEC:%d TSEC:%d\n", tl->id1, tl->id2,
			tl->timeout.tv_sec, tl->timeout.tv_usec);
		tl = tl->next;
	}

	while(1){	
		check_timer(&i1, &i2, &stopper);

		if ((i1 == 0) && (stopper.tv_sec == 0)) break;

		if (i1 == 0) {
			sleep(stopper.tv_sec+1);
		} else {
			printf("Time Out ID1: %d  / ID2: %d\n", i1, i2);
		}
	}

	exit(0);
}
#endif
