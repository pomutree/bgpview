#!/usr/bin/perl
#
# Prefix Summarizer for BGPView
#
# sumroute.pl,v 1.1 2003/03/31 05:56:42 kuniaki Exp
#
# Usage:
#   sumroute.pl <Prefix Length> <File Name>
#     Prefix Length : 8 - 32
#     File Name     : File Name of BGPView Route File
#
#
# Copyright (c) 2001 Internet Initiative Japan Inc.
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

$length = 24;
@pp = ();

{
	$length = $ARGV[0];
	$fname = $ARGV[1];

	if (($length > 32) || ($length < 8)) {
		print "Prefix length is out of range.\n";
		exit;
	}

	unless(open(FL, "$fname")) {
		printf "Could not open '$fname'\n";
		exit;
	}

	while(<FL>) {
		chomp;
		next if (/^$/);
		next if (/^\s+$/);

		@data1 = split(/\s+/, $_);
		@data2 = split(/\//, $data1[1]);
		@data3 = split(/\./, $data2[0]);

		if ($data2[1] == $length) {
			$pp[$data3[0]]++;
		}
	}
	close(FL);

	$cnt = 1;
	while($cnt < 256) {
		print "$cnt : $pp[$cnt]\n";
		$cnt++;
	}
}

