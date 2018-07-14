#!/usr/local/bin/perl

# Preifx Counter for every ASes
# asprefcnt.pl,v 1.1 2003/03/31 05:56:35 kuniaki Exp

#
# Copyright (c) 1999 Internet Initiative Japan Inc.
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

sub printusage {
	print "\nUsage :\n";
	print "  sprefcnt.pl <target file> <mode> <ASN>......\n";
	print "        target file : BGPView Prefix Log File Name\n";
	print "        mode : origin-as : count for origin-as\n";
	print "               peer-as   : count for peer-as\n";
}

sub analy_pref {
	local($fname, $mode, @ases) = @_;
	local(@ret, @dat1, @dat2, $cnt, %ascnt, $tas);

	undef(@ret);
	unless(open(TGF, "$fname")) {
		print "Could not open target file : $fname\n";
		return(@ret);
	}
	undef(%ascnt);
	while(<TGF>) {
		chomp;
		@dat1 = split(/:/, $_);
		if ($dat1[3] =~ /UPDATE/) {
			@dat2 = split(/\s+/, $dat1[4]);
			if ($mode =~ /^peer-as$/) {
				$tas = $dat2[3];
			} else {
				$cnt = 3;
				while(defined($dat2[$cnt])) {
					unless (defined($dat2[$cnt+1])) {
						$tas = $dat2[$cnt];
					}
					$cnt++;
				}
			}
			$cnt = 0;
			while(defined($ases[$cnt])) {

				if ($ases[$cnt] == $tas) {
					$ascnt{$tas}++;
				}
				$cnt++;
			}
		}
	}
	close(TGF);
	$cnt = 0;
	while(defined($ases[$cnt])) {
		push(@ret, $ascnt{$ases[$cnt]});
		$cnt++;
	}
	return(@ret);
}

{
	local(@result, @ases, $tgfile, $mode, $cnt);

	unless (defined($ARGV[0]) && defined($ARGV[1]) && defined($ARGV[2])) {
		print "Paramer not enough\n";
		&printusage;
		exit;
	}
	if (($ARGV[1] =~ /^origin-as$/) || ($ARGV[1] =~ /^peer-as$/)) {
		$mode = $ARGV[1];
	} else {
		print "Unknown analize mode\n";
		&printusage;
		exit;
	}
	$tgfile = $ARGV[0];
	undef(@ases);
	$cnt = 2;
	while(defined($ARGV[$cnt])) {
		push(@ases, $ARGV[$cnt]);
		$cnt++;
	}

	@result = &analy_pref($tgfile, $mode, @ases);

	unless (defined(@result)) {
		&printusage;
		exit;
	}

	$cnt = 0;
	while(defined($ases[$cnt])) {
		printf(" %7s : %5d\n", "AS$ases[$cnt]", $result[$cnt]);
		$cnt++;
	}
}

