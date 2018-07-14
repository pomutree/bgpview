#!/usr/local/bin/perl
# Prefix Length Counter for BGPView
# mlcnt.pl,v 1.1 2003/03/31 05:56:40 kuniaki Exp

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

##################################################################
#
# Would you like these valiables as you like.
#
##################################################################
#
# This is BGPView prefix logfile name configuration.
$LOGFILE = "/usr/local/bgpview/log/prefix.prefix.bak";
#
# This is this scripts' log file name conifigurations.
#   STACKF1 is Current logfile name.
#   STACKF2 is Previous logfile name.
$STACKF1 = "/usr/local/bgpview/log/mlcnt.log";
$STACKF2 = "/usr/local/bgpview/log/mlcnt.old";
#
# This is Output HTML Configurations.
#  This is output HTML file name configuration.
$HTMLFILE = "/usr/local/www/data/bgpview/mlcnt.html";
#
#  This is the part of HTML Header Block configuration.
@HEADER  = ("<html>\n<head>\n<title>BGPView Prefix Cnt</title>\n",
            "</head>\n",
            "<body  BGCOLOR=\"#FFFFFF\" TEXT=\"#00004F\" LINK=\"#0000FF\" VLINK=\"#0000FF\" ALINK=\"#0000FF\">\n",
            "<font size=+2>BGPView Prefix Length Count</font>\n",
            "<hr>\n");
#
#  This is the part of HTML Footer Block configuratuon.
@FOOTER  = ("<hr>\n",
	    "<address><a href=\"mailto:admin\@domain.name\">\n",
	    "admin\@domain.name</a></address>\n",
	    "</body></html>\n" );
#
# This is configuration of the number of lines of BGPView Datas..
$VIEWCOUNT = 96;	# View One Day datas
#
# This is prefix length range configuration.
@PRRANGE = (1,32);

##################################################################

{
	unless(open(LG, "$LOGFILE")) {
		print "Could not open logfile : $LOGFILE\n";
		exit(1);
	}

	@updcnt = (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	@wtdcnt = (0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
	while(<LG>) {
		chomp;
		next if (/^$/);
		next if (/^\s+$/);
		next if (/^#/);

		@datas = split(/\s+/, $_);
		if ($datas[2] =~ /^UPDATE/) {
			@dat2 = split(/\//, $datas[4]);
			$updcnt[$dat2[1]]++;
		} elsif ($datas[2] =~ /^WITHDRAW/) {
			@dat2 = split(/\//, $datas[3]);
			$wtdcnt[$dat2[1]]++;
		}
	}
	close(LG);

	## Conver HTML
	$oldfile = 1;
	rename($STACKF1, $STACKF2);
	unless(open(OL, "$STACKF2")) {
		print "Could not Open Old MLCNT log file : $STACKF2\n";
		$oldfile = 0;
	}
	unless(open(ST, "> $STACKF1")) {
		print "Could Not Open MLCNT log file : $STACKF1\n";
		close(OL);
		exit(3);
	}
	unless(open(HTML, "> $HTMLFILE")) {
		print "Could not open HTML File : $HTMLFILE\n";
		close(OL);
		close(ST);
		exit(4);
	}

	($sec,$min,$hour,$day,$mon,$year) = localtime(time);
	$td = sprintf("%.4d/%.2d/%.2d %.2d:%.2d",
		$year+1900, $mon+1,$day, $hour, $min);
#	print HTML "Content-Type: text/html\n";
#	print HTML "\n";
	print HTML "@HEADER";
	print HTML "<table border=1>\n";
	print HTML "<tr bgcolor=#E0E0E0>";
	print HTML "<th colspan=2>Time</th>";
	$cols = $PRRANGE[1] - $PRRANGE[0] + 1;
	print HTML "<th colspan=$cols>Update Count</th><th colspan=$cols>Withdraw Count</th></tr>\n";
	print HTML "<tr bgcolor=#E0E0E0>";
	print HTML "<th>Date</th><th>Time</th>";
	for($cnt=$PRRANGE[0];$cnt<=$PRRANGE[1];$cnt++) {
		print HTML "<th>$cnt</th>";
	}
	for($cnt=$PRRANGE[0];$cnt<=$PRRANGE[1];$cnt++) {
		print HTML "<th>$cnt</th>";
	}
	print HTML "</tr>\n";
	print ST "$td ";
	@td2 = split(/\s+/,$td);
	print HTML "<tr><td>$td2[0]</td><td>$td2[1]</td>";
	for($cnt=1;$cnt<=32;$cnt++) {
		print ST "$updcnt[$cnt] ";
		if (($PRRANGE[0] <= $cnt) && ($PRRANGE[1] >= $cnt)) {
			print HTML "<td align=right>$updcnt[$cnt]</td>";
		}
	}
	for($cnt=1;$cnt<=32;$cnt++) {
		print ST "$wtdcnt[$cnt] ";
		if (($PRRANGE[0] <= $cnt) && ($PRRANGE[1] >= $cnt)) {
			print HTML "<td align=right>$wtdcnt[$cnt]</td>";
		}
	}
	print ST "\n";
	print HTML "\n";

	if ($oldfile == 1) {
		$cnt1 = 0;
		while(<OL>) {
			chomp;
			print ST "$_\n";
			@data = split(/\s+/,$_);
			print HTML "<tr><td>$data[0]</td><td>$data[1]</td>";
			for($cnt=2;$cnt<=65;$cnt++) {
				if (($PRRANGE[0] <= ($cnt-1)) && 
				    ($PRRANGE[1] >= ($cnt-1)) &&
				    ($cnt <= 34)) {
					print HTML "<td align=right>$data[$cnt]</td>";
				} elsif (($PRRANGE[0] <= ($cnt-33)) &&
				         ($PRRANGE[1] >= ($cnt-33))) {
					print HTML "<td align=right>$data[$cnt]</td>";
				}
			}
			print HTML "\n";
			$cnt1++;
			last if ($cnt1 >= $VIEWCOUNT);
		}
		close(OL);
	}
	print HTML "</table>\n";
	print HTML "@FOOTER";

	close(ST);
	close(HTML);
}

