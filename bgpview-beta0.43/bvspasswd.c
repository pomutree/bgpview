/****************************************************************/
/* BGP-4 Protocol Viewer                                        */
/*    Programmed By K.Kondo IIJ 1997/12                         */
/*                                                              */
/* BGPView Password Encoder                                     */
/****************************************************************/
/* bvspasswd.c,v 1.1 2003/03/31 05:51:04 kuniaki Exp */

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

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include "bgp.h"

static unsigned char saltstr[] =
    "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void to64(register char *s, register long v, register int n)
{
	while(--n >= 0) {
		*s++ = saltstr[v & 0x3F];
		v >>= 6;
	}
}

#ifdef GETPASS
char *getpass(const char *prompt)
{
	static char	pass[128];
	char		inpass[128];
	int		cnt;
	char		ic;

	fputs(prompt, stderr);
	cnt = 0;
	while(1) {
		ic = getchar();
		if ((ic == '\n')||(ic == '\r')) {
			inpass[cnt] = '\0';
			break;
		}
		inpass[cnt] = ic;
		cnt++;
		if (cnt == 128) {
			inpass[cnt] = '\0';
			break;
		}
	}

	/* fgets(&inpass[0], 127, stdin); */

	if (strlen(inpass) > 8) {
		inpass[8] = '\0';
	}

	strcpy(pass, inpass);

	if (strlen(pass) <= 0) {
		printf("Please Input Password\n");
		return(0);
	}

	return(pass);
}
#endif /* GETPASS */

int makepstring(char *pass, char *encoded)
{
	char	salt[9];

	srand((int) time((time_t *)NULL));
	to64(&salt[0], rand(), 8);
	salt[8] = '\0';

	strcpy(encoded, (char *)crypt(pass, salt));

	return(strlen(encoded));
}

main()
{
	char	ipass1[9], ipass2[9];
	char	enced[128];

	printf("BGPView Shell Password Encoder for %s\n", BV_VERSION);
	if (strcpy(&ipass1[0], getpass("Password: ")) == 0) {
		exit(1);
	}
	if (strcpy(&ipass2[0], getpass("Password Again: ")) == 0) {
		exit(1);
	}
	if (strcmp(ipass1, ipass2) != 0) {
#ifdef DEBUG
		printf("Password Mismatched!\n");
#endif
		exit(1);
	}
	if (makepstring(ipass1, &enced[0]) == 0) {
#ifdef DEBIG
		printf("Encryption Fail!\n");
#endif
		exit(1);
	}
	printf("Encrypted String : %s\n", enced);

#ifdef DEBUG
	if (checkpw(enced, ipass1)) {
		printf("Password OK!\n");
	} else {
		printf("Password Mismatch!\n");
	}
#endif
}
