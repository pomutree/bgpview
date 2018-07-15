#!/usr/local/bin/perl

#
# Copyright (c) 1998 Internet Initiative Japan Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistribution with functional modification must include
#    prominent notice stating how and when and by whom it is
#    modified.
# 3. Redistributions in binary form have to be along with the source
#    code or documentation which include above copyright notice, this
#    list of conditions and the following disclaimer.
# 4. All commercial advertising materials mentioning features or use
#    of this software must display the following acknowledgement:
#      This product includes software developed by Internet
#      Initiative Japan Inc.
#
# THIS SOFTWARE IS PROVIDED BY ``AS IS'' AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED.
#
# checkprefix.pl,v 1.1 2003/03/31 05:56:37 kuniaki Exp
#

$ABUSEL = "abuseprefix.dat";
$SUFFIX = "prefix.bak";
$LOGDIR = "/usr/local/bgpview/log";

$MAILCM = "/usr/sbin/sendmail";
$TOADDR = "kuniaki\@iij.ad.jp";
$FROMAD = "BGPVIEW <kuniaki\@iij.ad.jp>";
$SUBJEC = "BGPVIEW Abnormal Prefix Report";

sub read_abuse {
	unless(open(ABU, "$ABUSEL")){
		retuen(2);
	}
	@abusepref = ();
	while(<ABU>) {
		next if /^#/;
		next if /^$/;
		next if /^\s+$/;
		chomp;
		push(@abusepref,$_);
	}
	close(ABU);
	return(0);
}

sub mailsend {
	open(MAIL, "| $MAILCM $TOADDR");
	while(defined($cur = shift(@mail))){
		print MAIL "$cur";
	}
	close(MAIL);
}

{

	$NEIGHBOR = $ARGV[0];

	$logfile = $LOGDIR. "/" . $NEIGHBOR . "." . $SUFFIX;

	@mail = ();
	push(@mail, "Reply-To: $FROMAD\n");
	push(@mail, "From: $FROMAD\n");
	push(@mail, "To: $TOADDR\n");
	push(@mail, "Subject: $SUBJEC\n");
	push(@mail, "\n");

	unless(open(LGF, "$logfile")) {
		push(@mail, "Log file Could not Open\n");
		push(@mail, "File : $logfile\n");
		&mailsend;
		exit(1);
	}

	$ret = &read_abuse;
	if ($ret != 0) {
		push(@mail, "ABUSE Prefix Data file could not open\n");
		push(@mail, "File : $ABUSE\n");
		&mailsend;
		exit(1);
	}

	($sec,$min,$hour,$day,$mon,$year) = localtime(time);

	if ($mon == 12) {
    		$mon = 1;
    	} else {
		$mon++;
	}
	$fna = sprintf("%.4d/%.2d/%.2d %.2d:%.2d", 1900+$year, $mon, $day, $hour, $min);
	push(@mail, "Abuse Report:\n");
	push(@mail, "  LOG Name   : $NEIGHBOR\n");
	push(@mail, "  Check Time : $fna\n\n");

	$sendflg = 0;
	while(<LGF>) {
		chomp;
		$line = $_;
		@data = split(/:/, $line);

		@prefix = split(/\s+/, $data[4]);

		@abusetmp = @abusepref;
		while(defined($ss = shift(@abusetmp))) {
			if ($prefix[1] =~ /$ss/) {
			  push(@mail, "$line\n");
			  $sendflg = 1;
			}
		}
	}
	close(LGF);
	if ($sendflg == 1) {
		&mailsend;
	}
}
