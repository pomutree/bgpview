/********************************************************************/
/* This is IRR Route Checker that checks received routes by BGPView */
/* using IRR.                                                       */
/*                                                                  */
/* Usage:                                                           */
/*  irrcheck [BGPView Prefix Log File]                              */
/********************************************************************/
/* irrcheck.c,v 1.1 2004/12/08 05:41:07 kuniaki Exp */

#include "irrcheck.h"


/*** Global Values ***/
int	db_soc;
int	irr_port;
char	*irr_host;

u_long	base_cnt;
u_long	notexact_cnt;
u_long	notless_cnt;
u_long	asmiss_cnt;
u_long	asmiss_route;
u_long	asmissincorrect;

int	flg_notexact;
int	flg_notless;
int	flg_missas;
int	flg_printall;
int	flg_routemode;

void init()
{
	db_soc = 0;
	irr_port = DEFAULT_IRR_PORT;
	irr_host = strdup(DEFAULT_IRR_HOST);
	flg_notexact = 0;
	flg_notless = 0;
	flg_missas = 0;
	flg_printall = 0;
	flg_routemode = 0;
	base_cnt = 0;
	notexact_cnt = 0;
	notless_cnt = 0;
	asmiss_cnt = 0;
	asmiss_route = 0;
	asmissincorrect = 0;
}

void finish(int exitcode)
{
	if (irr_host) free(irr_host);

	exit(exitcode);
}

int dbopen(const char *dbhost, int port)
{
	struct hostent		*hp;
	struct sockaddr_in	sin;

	if (db_soc != 0) {
		printf("IRR DB already connected.\n");
		return(0);
	}

	if ((hp = gethostbyname(dbhost)) == NULL) {
		printf("%s Unknown Host.\n", dbhost);
		return(0);
	}

	if ((db_soc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Socket Error");
		return(0);
	}

	if (db_soc == 0) {
		printf("Socket Error\n");
		close(db_soc);
		return(0);
	}

	sin.sin_family	= AF_INET;
	sin.sin_port	= htons(port);
	bcopy(hp->h_addr, &sin.sin_addr, hp->h_length);

	if (connect(db_soc, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		perror("Connect Error");
		close(db_soc);
		return(0);
	}
	return(1);
}

void dbclose()
{
	write(db_soc, "q\n", 2);
	close(db_soc);
	db_soc = 0;
}

void batchmode()
{
	write(db_soc, "!!\n", 3);
}

int getcontrol(char *type)
{
	char	line[1024];
	char	c;
	int	cnt;
	int	ret;

	bzero(line, 1024);
	cnt = 0;
	while(1) {
		read(db_soc, &c, 1);
		if (c == '\n') break;
		line[cnt] = c;
		cnt++;
		if (cnt >= 1024) {
			printf("Line Buffer Overflow\n");
			finish(1);
		}
	}

	ret = 0;
	switch(line[0]) {
	case 'A':
		*type = 'A';
		ret = atoi(&line[1]);
		break;
	case 'C':
	case 'D':
		ret = -1;
	case 'B':
	case 'E':
	case 'F':
		*type = line[0];
		break;
	default:
		*type = 'Z';
	}
	return(ret);
}

int sread(char *buf, int len)
{
	int	cnt;
	int	rel;

	cnt = 0;
	rel = 0;
	bzero(buf, len);
	while(cnt < len) {
		rel = read(db_soc, (char *)(buf + cnt), (len-cnt));
		cnt += rel;
#ifdef DEBUG
		printf("Reading %d/%d/%d\n", rel, cnt, len);
#endif
	}
	*(buf+len) = '\0';
#ifdef DEBUG
	printf("%d:%s\n", strlen(buf), buf);
#endif
	return(cnt);
}

char *readdata()
{
	char	type;
	int	result;
	char	*data;
	int	stop;
	int	getdata;

	data = NULL;
	stop = 0;
	getdata = 0;
	while(stop == 0) {
		result = getcontrol(&type);
#ifdef DEBUG
		printf("DEBUG: DATA Type %c\n", type);
#endif
		switch(type) {
		case 'A':
#ifdef DEBUG
			printf("DEBUG: Reading Data = %d byte\n", result);
#endif
			data = (char *)malloc(result+1);
#ifdef DEBUG
			printf("DEBUG: Malloced Address: %p\n", data);
#endif
			sread(data, result);
			getdata = 1;
			break;

		case 'B':
			stop = 1;
			break;
		case 'C':
#ifdef DEBUG
			printf("DEBUG: Complete\n");
#endif
			stop = 1;
			break;
		case 'D':
			/* Key Not Found */
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

void separate_line(char *line, char **cmd, char **val)
{
	char	*linebuf;
	char	*sep;
	int	len;

	len = strlen(line);
	linebuf = (char *)malloc(len+1);
	bzero(linebuf, len+1);
	strcpy(linebuf, line);

	sep = strchr(linebuf, ':');
	if (sep == NULL) {
		*cmd = NULL;
		*val = NULL;
		return;
	}
	*sep = '\0';
	*cmd = strdup(linebuf);
	sep++;
	while(*sep != '\0') {
		if ((*sep == ' ') || (*sep == '\t')) {
			sep++;
		} else {
			break;
		}
	}
	if (strlen(sep) == 0) {
		free(*cmd);
		*cmd = NULL;
		*val = NULL;
		return;
	}
	*val = strdup(sep);

	free(linebuf);
	return;
}

int analy_result(char *data, SEARCH_RESULT *result)
{
	char		*dbuf;
	char		*curp;
	char		*pbuf;
	int		rcnt;
	int		lcnt;
	OBJ_DATA	*rdata, *optr;
	char		*cmd,*val;

	lcnt = strlen(data);
	dbuf = (char *)malloc(lcnt+1);
	if (dbuf == NULL) {
		printf("Malloc Error\n");
		finish(1);
	}
	bzero(dbuf, lcnt+1);
	strncpy(dbuf, data, lcnt);
	rcnt = 0;
	rdata = NULL;
	result->items = 0;
	result->item = NULL;
	curp = strtok(dbuf, "\n");
	while(curp != NULL) {
	    separate_line(curp, &cmd, &val);
	    if (cmd != NULL) {
		if (strcmp(cmd, "route") == 0) {
		    rcnt++;
		    if (rdata == NULL) {
			rdata = (OBJ_DATA *)malloc(sizeof(OBJ_DATA));
		    } else {
			rdata = (OBJ_DATA *)realloc(rdata,
						 rcnt * sizeof(OBJ_DATA));
		    }
		    optr = (rdata+(rcnt-1));

		    bzero(optr, sizeof(OBJ_DATA));
		    pbuf = strchr(val, '/');
		    if (pbuf == NULL) {
			optr->plen = 32;
		    } else {
		    	*pbuf = '\0';
		    	optr->plen = atoi(pbuf+1);
		    }
		    inet_aton(val, &(optr->addr));
		} else if (strcmp(cmd, "origin") == 0) {
		    optr = (rdata+(rcnt-1));
		    if ((*val == 'A') && (*(val+1) == 'S')) {
			optr->asn = strtoul((val+2), NULL, 10);
		    } else {
			optr->asn = strtoul(val, NULL, 10);
		    }
		} else if (strcmp(cmd, "source") == 0) {
		    strncpy((rdata+(rcnt-1))->src, val, 63);
		}
	    }
	    if (cmd) free(cmd);
	    if (val) free(val);
	    curp = strtok(NULL, "\n");
	}
	free(dbuf);

	result->items = rcnt;
	result->item  = rdata;

	return(rcnt);
}

int getroute_exact(char *rstr, SEARCH_RESULT *result)
{
	char	cmd[128];
	char	*data;
	int	rcnt;

	sprintf(cmd, "!r%s\n", rstr);

#ifdef DEBUG
	printf("DEBUG: Command: %s", cmd);
#endif

	result->items = 0;
	result->item = NULL;
	write(db_soc, cmd, strlen(cmd));
	data = readdata();
	if (data == NULL) {
#ifdef DEBUG
		printf("DEBUG: Search Fail\n");
#endif
		rcnt = 0;
	} else {
		rcnt = analy_result(data, result);
		free(data);
	}

	return(rcnt);
}


int getroute_less(char *rstr, SEARCH_RESULT *result)
{
	char	cmd[128];
	char	*data;
	int	rcnt;

	sprintf(cmd, "!r%s,L\n", rstr);

#ifdef DEBUG
	printf("DEBUG: Command: %s", cmd);
#endif

	write(db_soc, cmd, strlen(cmd));
	data = readdata();
	if (data == NULL) {
		rcnt = 0;
	} else {
		rcnt = analy_result(data, result);
		free(data);
	}

	return(rcnt);
}

int analy_routelog_line(char *line, char **prefix, u_long *asn)
{
	char	*lbuf, *ptr;
	char	*pstr, *oas, *oas2;

	if (strstr(line, "UPDATE") != NULL) return(0);
	if (strstr(line, "WITHDRAW") != NULL) return(0);
	if (strstr(line, "History") != NULL) return(0);
	if (line[0] == '!') return(0);

	lbuf = strdup(line+2);
	ptr = strtok(lbuf, " ");
	if (ptr != NULL) {
		pstr = strdup(ptr);
	} else {
		free(lbuf);
		return(-1);
	}
	oas = NULL;
	oas2 = NULL;
	while(ptr != NULL) {
		oas2 = oas;
		oas = ptr;
		ptr = strtok(NULL, " ");
	}
	if (oas2 == NULL) {
		free(lbuf);
		return(-1);
	}
	*prefix = pstr;
	*asn = strtoul(oas2, NULL, 10);
	base_cnt++;

	return(1);
}

int analy_preflog_line(char *line, char **prefix, u_long *asn)
{
	char	mode[10];
	char	*lbuf, *ptr;
	char	*pstr, *oas, *oas2;

#ifdef DEBUG
	printf("DEBUG: Line: %s", line);
#endif

	bzero(mode, 10);
	strncpy(mode, line+21, 9);


	if (strcmp(mode, "WITHDRAW:") == 0) {
#ifdef DEBUG
		printf("DEBUG: Withdraw\n");
#endif
		return(0);
	}
	if (strcmp(mode, "UPDATE  :") != 0) {
		/* Format Error */
		return(-1);
	}
	lbuf = strdup(line+31);
	ptr = strtok(lbuf, " ");
	if (ptr != NULL) {
		pstr = strdup(ptr);
	} else {
		free(lbuf);
		return(-1);
	}
	oas = NULL;
	oas2 = NULL;
	while(ptr != NULL) {
		oas2 = oas;
		oas = ptr;
		ptr = strtok(NULL, " ");
	}
	if (oas2 == NULL) {
		free(lbuf);
		return(-1);
	}
	*prefix = pstr;
	*asn = strtoul(oas2, NULL, 10);
	base_cnt++;
	
	return(1);
}

int count_result(char *s_pref, u_long s_asn, SEARCH_RESULT *s_result, int *asmisses)
{
	int		cnt;
	OBJ_DATA	*objs;
	int		asmiscnt;

	if (s_result->items <= 0) {
		return(0);
	}

	asmiscnt = 0;
	for(cnt = 0; cnt < s_result->items; cnt++) {
		objs = (s_result->item)+cnt;
		if (flg_printall)
			printf("    %02d: %s/%d From AS%lu Entried in %s",
				cnt, inet_ntoa(objs->addr),
				objs->plen, objs->asn, objs->src);
		if (s_asn != objs->asn) {
			if (flg_printall) 
				printf(" (!!ORIGIN AS MISMATCHED!!)\n");
			if (flg_missas)
				printf("%s Origin AS Mismatch Recv:AS%lu -> IRR:AS%lu(%s)\n",
					s_pref, s_asn, objs->asn, objs->src);
			asmiscnt++;
		} else {
			if (flg_printall) printf("\n");
		}
	}
	*asmisses = asmiscnt;
	return(cnt);
}

int main_loop(char *filename)
{
	FILE		*fp;
	char		linebuf[1024];
	char		*s_pref;
	u_long		s_asn;
	int		ret;
	SEARCH_RESULT	s_result;
	int		asmiss;

	if ((fp = fopen(filename, "r")) == NULL) {
		printf("File Could not Open: %s\n", filename);
		finish(0);
	}
	while(!feof(fp)) {
		bzero(linebuf, 1024);
		fgets(linebuf, 1023, fp);
	
		if (flg_routemode) {
			ret = analy_routelog_line(linebuf, &s_pref, &s_asn);
		} else {
			ret = analy_preflog_line(linebuf, &s_pref, &s_asn);
		}
		if (ret == -1) {
#ifdef DEBUG
			printf("Data Format Error\n");
			printf(" -> %s", linebuf);
#endif
			/* Ignore Line */
			continue;
		} else if (ret == 0) {
			continue;
		}
		if (flg_printall)
			 printf("SEARCH ROUTE: %s AS%lu\n", s_pref, s_asn);

		/* Exect Match */
		getroute_exact(s_pref, &s_result);
		/* Precess s_resutl  and free s_result */
		if (s_result.items > 0) {
			if (flg_printall) printf("  Exect Match Routes:\n");
			count_result(s_pref, s_asn, &s_result, &asmiss);
		} else {
			if (flg_notexact)
			 	printf("%s(AS%lu) dosen't match EXECT\n",
							s_pref, s_asn);
			notexact_cnt++;
		}
		if (s_result.item) free(s_result.item);

		/* Less Prefix Match */
		getroute_less(s_pref, &s_result);
		/* Precess s_resutl  and free s_result */
		if (s_result.items > 0) {
			if (flg_printall) printf("  Less Prefix Match Routes:\n");
			count_result(s_pref, s_asn, &s_result, &asmiss);
			if (asmiss != 0) {
				asmiss_cnt += asmiss;
				asmiss_route++;
				if (asmiss < s_result.items) {
					asmissincorrect++;
				}
			}
		} else {
			if (flg_notless)
			 	printf("%s(AS%lu) dosen't match LESS_PREFIX\n",
							s_pref, s_asn);
			notless_cnt++;
		}
		if (s_result.item) free(s_result.item);


		free(s_pref);
	}
	fclose(fp);
	return(1);

}

void show_version()
{
	printf("\n%s\n", PRODUCTNAME);
	printf("Version %s\n", VERSION);
	printf("%s\n\n", COPYRIGHT);
}

void show_usage()
{
	printf("Usage:\n");
	printf("  irrcheck [-aehlrsv] [-d IRR_host] [-p IRR_port] <BGPView Prefix Log File Name>\n");
	printf("\n");
	printf("  -a          : Print AS Number if prefix log origin as number and IRR\n");
	printf("                object's origin as number mismatched.\n");
	printf("\n");
	printf("  -d IRR_host : Indicate search IRR host\n");
	printf("\n");
	printf("  -e          : Print routes if prefix log's prefix didn't match with\n");
	printf("                IRR objects Exectly.\n");
	printf("\n");
	printf("  -h          : Show this message.\n");
	printf("\n");
	printf("  -l          : Print routes if prefix log's prefix didn't match with\n");
	printf("                less prefix in IRR objects.\n");
	printf("\n");
	printf("  -p IRR_port : Indicate IRR search TCP port number.\n");
	printf("\n");
	printf("  -r          : Analyze BGPView Route log file mode.\n");
	printf("                In default, it analyze prefix log.\n");
	printf("\n");
	printf("  -s          : Print all comparison messages.\n");
	printf("\n");
	printf("  -v          : Print Version Number.\n");
	printf("\n");
}

void show_summary()
{
	printf("IRR Check Summary:\n");
	printf("  Search Routes : %lu\n", base_cnt);
	printf("    # of NOT EXECT MATCH ROUTES     : %lu\n", notexact_cnt);
	printf("    # of NOT LESS_PREFIX ROUTES     : %lu\n", notless_cnt);
	printf("    # of AS Number Mis-Match Objects: %lu\n", asmiss_cnt);
	printf("    # of AS Number Mis-Match Routes : %lu\n", asmiss_route);
	printf("        in one of correct AS entried: %lu\n", asmissincorrect);
	printf("\n");
}


int main(int argc, char *argv[])
{
	int	cnt;
	char	*fname;

	init();

#ifdef DEBUG
	printf("DEBUG: Parameter Count = %d\n", argc);
#endif
	if (argc < 2) {
		printf("Parmameter Not Enough\n");
		show_usage();
		
		finish(1);
	}

	cnt = 1;
	while(cnt < argc) {
		if (strcmp(argv[cnt], "-h") == 0) {
			show_version();
			show_usage();
			finish(0);
		}
		if (strcmp(argv[cnt], "-v") == 0) {
			show_version();
			finish(0);
		}
		if (strcmp(argv[cnt], "-d") == 0) {
			if ((cnt+2) >= argc) {
				printf("Parameter Error: IRR HOST\n");
				finish(1);
			}
			cnt++;
			if (irr_host) free(irr_host);
			irr_host = strdup(argv[cnt]);
		}
		if (strcmp(argv[cnt], "-p") == 0) {
			if ((cnt+2) >= argc) {
				printf("Parameter Error: IRR PORT\n");
				finish(1);
			}
			cnt++;
			irr_port = atoi(argv[cnt]);
		}
		if (strcmp(argv[cnt], "-s") == 0) {
			flg_printall = 1;
		}
		if (strcmp(argv[cnt], "-e") == 0) {
			flg_notexact = 1;
		}
		if (strcmp(argv[cnt], "-l") == 0) {
			flg_notless = 1;
		}
		if (strcmp(argv[cnt], "-a") == 0) {
			flg_missas = 1;
		}
		if (strcmp(argv[cnt], "-r") == 0) {
			flg_routemode = 1;
		}
		cnt++;
	}
	fname = strdup(argv[cnt-1]);

	if (!(dbopen(irr_host, irr_port))) {
		printf("Connectio Fail with IRR Server: %s\n", irr_host);
		finish(1);
	}
	batchmode();

#ifdef DEBUG
	printf("Entering Mail Loop\n");
#endif
	main_loop(fname);

	dbclose();

	show_summary();

	finish(0);

	return(0);
}
