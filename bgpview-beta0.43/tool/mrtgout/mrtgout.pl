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
# mrtgout.pl,v 1.1 2003/03/31 05:56:41 kuniaki Exp
#

$INFODIR = "/bgpview/log/directory";
$ROUTER  = "Router-Name";

{
	$FNAME = $INFODIR . "/" . $ARGV[0];
	$COUNT = $ARGV[1];
	$MOD   = $ARGV[2];

	open(LOG, "$FNAME");
	$cc = 0;
	$lc = 0;
	while(<LOG>){
		last;
	}
	while(<LOG>){
		chomp;
		$lc++;
		$line[$cc] = $_;
		$cc++;
		$cc = 0 if ($cc >= $COUNT);
	}

	if ($lc > $COUNT) {
		$lim = $COUNT;
	} else {
		$lim = $lc;
		$cc = 0;
	}
	$lc =  $lim;
	$cnt1 = 0;
	$cnt2 = 0;
	$date = "";
	while($lc != 0){
#		print "$cc:$line[$cc]\n";
		@data = split(/\s+/, $line[$cc]);
		if ($MOD eq "ROUTE") {
			$cnt1 += $data[7];
			$cnt2 += $data[8];
			$date = $data[0]." ".$data[1];
		}
		$cc++;
		$lc--;
		$cc = 0 if ($cc >= $lim);
	}

	print "$cnt1\n$cnt2\n$date\n$ROUTER\n";
		

}
