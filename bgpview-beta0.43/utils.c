/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1999/08                         */
/*                                                              */
/* Utility functions                                            */
/****************************************************************/
/* utils.c,v 1.1 2003/03/31 05:51:04 kuniaki Exp */

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

#include "utils.h"

/*
 * converters between digits and figures
 */
u_short buf2ushort(u_char *ptr)
{
	return (u_short)((*ptr<<8)| *(ptr+1));
}

u_long buf2ulong(u_char *ptr)
{
	return (u_long)((*(ptr)<<24) | (*(ptr+1)<<16) |
			(*(ptr+2)<<8)| *(ptr+3));
}

void store_int16(ptr, var)
u_char *ptr;
u_short var;
{
	*ptr     = (var>>8) & 0xff;
	*(ptr+1) =  var     & 0xff;
}

void store_int32(u_char *ptr, u_long var)
{
	*(ptr)   = (var>>24) & 0xff;
	*(ptr+1) = (var>>16) & 0xff;
	*(ptr+2) = (var>>8 ) & 0xff;
	*(ptr+3) =  var      & 0xff;
}

boolean chkfmt(int type, char *str)
{
	int	cnt1, cnt2;
	boolean	ret;
	char	cstr[50];

	ret = true;
	switch(type) {
	case CHKFMT_ADR:
		strcpy(cstr, ADR_CSET);
		break;
	case CHKFMT_PRF:
		strcpy(cstr, PRF_CSET);
		break;
	case CHKFMT_INT:
		strcpy(cstr, INT_CSET);
		break;
	case CHKFMT_V6A:
		strcpy(cstr, V6A_CSET);
		break;
	case CHKFMT_V6P:
		strcpy(cstr, V6P_CSET);
		break;
	default:
		ret = false;
	}

#ifdef DEBUG
	printf("DEBUG: CHKFMT: Type = %d, String = '%s', chkstr = '%s'\n",
		type, str, cstr);
#endif

	if (ret == false) return(ret);

	cnt1 = 0;
	while(1) {
		if (*(str+cnt1) == '\0') break;
		cnt2 = 0;
		while(1) {
			if (*(cstr+cnt2) == '\0') break;
			if (toupper(*(str+cnt1)) == *(cstr+cnt2)) break;
			cnt2++;
		}
		if (strlen(cstr) <= cnt2) {
#ifdef DEBUG
			printf("DEBUG: CHKFMT: false = %c(%c)\n",
				*(str+cnt1), *(cstr+cnt2));
#endif
			ret = false;
			break;
		}
		cnt1++;
	}
	return(ret);
}

int checkpw(char *encripted_strings, char *pass)
{

        char    salt[9];
        char    encoded[128];

	bzero(salt, 9);
	bzero(encoded, 128);

	strncpy(salt, encripted_strings, 2);

        strcpy(&encoded[0], (char *)crypt(pass, salt));

#ifdef DEBUG
        printf("DEBUG: CHACKPW: Phrase : %s\n",encripted_strings);
        printf("DEBUG: CHACKPW: Encoded: %s\n",encoded);
#endif

        if(strncmp(encripted_strings, encoded,sizeof(salt)) == 0) {
#ifdef DEBUG
                printf("DEBUG: CHACKPW: Password OK!\n");
#endif
                return(1);
        } else {
#ifdef DEBUG
                printf("DEBUG: CHACKPW: Password Mismatch!\n");
#endif
                return(0);
        }
}

void wait_sigchld()
{
	while( waitpid(-1,NULL,WNOHANG) > 0 );
}

int bintostrv6(char *ptr, char *str)
{
        int     cnt;
        int     mzerop;
        int     mzero;
        int     mf;
        int     zc;
        short   wrd[8];
        char    sbuf[6];

        cnt = 0;
        mzerop = -1;
        mzero = 0;
        mf = 0;
        zc = 0;
        while(cnt < 8) {
                wrd[cnt] = ntohs(*(short *)(ptr+(cnt*2)));

                if (wrd[cnt] == 0) {
                        if (mf == 0) {
                                mf = 1;
                                zc = 1;
                        } else {
                                zc++;
                        }
                } else {
                        if (mf == 1) {
                                mf = 0;
                                if (mzero < zc) {
                                        mzero = zc;
                                        mzerop = cnt-zc;
                                }
                        }
                }
                cnt++;
        }
        if (mf == 1) {
                if (mzero < zc) {
                        mzero = zc;
                        mzerop = cnt-zc;
                }
        }

        strcpy(str, "");
        cnt = 0;
        while(cnt < 8) {
                if (mzerop == cnt) {
                        strcat(str, "::");
                        cnt+=mzero;
                        continue;
                }
                if (cnt != 0) {
                        if (str[strlen(str)-1] != ':') {
                                strcat(str, ":");
                        }
                }
                sprintf(sbuf, "%X", wrd[cnt]);
                if (strlen(sbuf) > 4) {
                        strcat(str, &sbuf[4]);
                } else {
                        strcat(str, sbuf);
                }
                cnt++;
        }
        return(strlen(str));
}

char *strtobinv6(char *ptr, char *str)
{
        short   wrd[7], wrd1[7];
        char    adrbuf[40];
        char    strbuf[5];
        char    cnt,c2,c3;
        char    *cptr;
        char    fstr[40],estr[40];

        bzero(wrd, 16);
        strcpy(adrbuf, str);

        cptr = strstr(adrbuf, "::");
        if (cptr != NULL) {
                bzero(fstr, 40);
                strncpy(fstr, &adrbuf[0], (cptr - adrbuf));
                strcpy(estr, cptr+2);
        } else {
                strcpy(fstr, adrbuf);
                strcpy(estr, "");
        }

        cnt = 0;
        c2 = 0;
        c3 = 0;
        bzero(strbuf, 6);
        while(fstr[cnt] != '\0') {
                if (fstr[cnt] == ':') {
                        c2 = 0;
                        sscanf(strbuf, "%X", &wrd[c3]);
                        bzero(strbuf, 6);
                        c3++;
                } else {
                        strbuf[c2] = fstr[cnt];
                        c2++;
                }
                cnt++;
        }
        if (cnt != 0) {
                sscanf(strbuf, "%X", &wrd[c3]);
        }

        cnt = 0;
        c2 = 0;
        c3 = 0;
        bzero(strbuf, 6);
        while(estr[cnt] != '\0') {
                if (estr[cnt] == ':') {
                        c2 = 0;
                        sscanf(strbuf, "%X", &wrd1[c3]);
                        bzero(strbuf, 6);
                        c3++;
                } else {
                        strbuf[c2] = estr[cnt];
                        c2++;
                }
                cnt++;
        }
        if (cnt != 0) {
                sscanf(strbuf, "%X", &wrd1[c3]);
                c3++;
        }

        c2 = 7;
        while(c3 > 0){
                wrd[c2] = wrd1[c3-1];
                c3--;
                c2--;
        }

        cnt = 0;
        while(cnt < 8) {
                *((short *)(ptr + (cnt*2))) = htons(wrd[cnt]);
                cnt++;
        }

        return(ptr);
}

void store_v6addr(char *dest, char *src)
{
	memcpy(dest, src, IPV6_ADDR_LEN);
}

void v6addrmask(char *adrs, int masklen)
{
	char	adrbuf[IPV6_ADDR_LEN];
	int	cnt;
	int	fulloct, leftbits;
	char	moct;

	fulloct = (int)(masklen/8);
	leftbits = masklen % 8;

	cnt=0;
	bzero(adrbuf, IPV6_ADDR_LEN);
	for(cnt = 0; cnt<fulloct; cnt++) {
		adrbuf[cnt] = *(adrs+cnt);
	}
	if (leftbits != 0) {
		switch(leftbits) {
		case 1:	moct = 0x80;
			break;
		case 2:	moct = 0xC0;
			break;
		case 3:	moct = 0xE0;
			break;
		case 4:	moct = 0xF0;
			break;
		case 5:	moct = 0xF8;
			break;
		case 6:	moct = 0xFC;
			break;
		case 7:	moct = 0xFE;
			break;
		}
		adrbuf[cnt] = (*(adrs+cnt)) & moct;
	}
	store_v6addr(adrs, adrbuf);
}

int v6addrcmp(char *adrs1, char *adrs2, int masklen)
{
	char	adrbuf1[IPV6_ADDR_LEN], adrbuf2[IPV6_ADDR_LEN];
	int	rt;

	store_v6addr(adrbuf1, adrs1);
	store_v6addr(adrbuf2, adrs2);

	if (masklen != 0) {
		v6addrmask(adrbuf1, masklen);
		v6addrmask(adrbuf2, masklen);
	} else {
		return(1);
	}

	rt = memcmp(adrbuf1, adrbuf2, IPV6_ADDR_LEN);

	if (rt == 0) return(1);
	return(0);
}

char checkafi(char *adrs)
{
	char	afitmp;

	if (strchr(adrs, ':') != NULL) {
#ifdef DEBUG
 printf("DEBUG: Check IPv6\n");
#endif
		if (chkfmt(CHKFMT_V6A, adrs)) {
			afitmp = AF_IPV6;
		} else {
			afitmp = 0;
		}
	} else if (strchr(adrs, '.') != NULL) {
#ifdef DEBUG
 printf("DEBUG: Check IPv4\n");
#endif
		if (chkfmt(CHKFMT_ADR, adrs)) {
#ifdef DEBUG
 printf("DEBUG: Format IPv4 OK\n");
#endif
			afitmp = AF_IPV4;
		} else {
			afitmp = 0;
		}
	} else {
		afitmp = 0;
	}

	return(afitmp);
}

u_long fourbyteastoulong(char *asstr)
{
	char	*ptr1, *ptr2;
	u_long	as4, tmpl;

	as4 = 0;
	tmpl = 0;
	ptr1 = strdup(asstr);
	ptr2 = strchr(ptr1, '.');
	
	if (ptr2 == NULL) {
		sscanf(ptr1, "%lu", &as4);
	} else {
		*ptr2 = '\0';
		sscanf(ptr1, "%hu", &tmpl);
		sscanf(ptr2+1, "%hu", &as4);
		as4 += tmpl * 0x10000;
	}
	free(ptr1);
#ifdef DEBUG
	printf("DEBUG: STR:%s->NUM:%lu\n", asstr, as4);
#endif
	return(as4);
}

char *ulongtofourbyteas(u_long as4)
{
	u_long		tmp1, tmp2;
	static char	res[20];

	bzero(res, 20);

	tmp1 = (int)(as4 / 0x10000);
	tmp2 = as4 % 0x10000;
	sprintf(&res[0], "%lu.%lu", tmp1, tmp2);

#ifdef DEBUG
	printf("DEBUG: NUM:%lu->%s\n", as4, res);
	printf("DEBUG: tmp1=%lu / tmp2=%lu\n", tmp1, tmp2);
#endif

	return(&res[0]);
}
