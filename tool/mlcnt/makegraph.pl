#!/usr/local/bin/perl
#
# Prefix Length Count Grapher for BGPView
# makegraph.pl,v 1.1 2003/03/31 05:56:40 kuniaki Exp

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
# Would you like change these valiables as you like.
#
##################################################################
#
# This is gnuplot command path configuration.
$GNUPLOT = "/usr/local/bin/gnuplot";
#
# This is ppmtogif command path configuration.
$PPMTOGIF = "/usr/X11R6/bin/ppmtogif";
#
# This is the log of mlcnt.pl path configuration.
$MLCNTLOG = "/usr/local/bgpview/log/mlcnt.log";
#
# This is the makegraph.pl's log file path configuration.
$DATADIR = "/usr/local/bgpview/log";
#
# This is the output file name configuration.
$OUTGIF = "/usr/local/www/data/bgpview/prefix.gif";
#
# This is a configration of drawing prefix length.
@updatepl   = (32,24,16,8);
@withdrawpl = (32,24,16,8);
#
##################################################################


sub makegif {
    unless(open(GIF, "$PPMTOGIF $DATADIR/prefix.pbm > $OUTGIF |")) {
    	print "Could not execute $PPMTOFIG\n";
    	return;
    }
    while(<GIF>) {
    	print $_;
    }
    close(GIF);
}

sub drawgnuplot {
    unless(open(GP, "| $GNUPLOT")) {
	print "Could not execute gnuplot\n";
	return;
    }
    print GP "set nologscale\n";
    print GP "set noparametric\n";
    print GP "set time\n";
    print GP "set title 'BGPView Prefix Updates'\n";
    print GP "set xlabel 'x:time'\n";
    print GP "set ylabel 'y:prefixes'\n";
    print GP "set xdata time\n";
    print GP "set timefmt \"%Y/%m/%d %H:%M\"\n";
    print GP "set format x \"%m/%d %H:%M\"\n";
    print GP "set xtics rotate\n";
    print GP "set terminal pbm color\n";
    print GP "set output '$DATADIR/prefix.pbm'\n";
    print GP "plot \\\n";
    if (defined($updatepl[0])) {
    	$cnt = 0;
    	while(1) {
	    print GP "  '$DATADIR/ul$updatepl[$cnt].dat' using 1:3 title '/$updatepl[$cnt] update' with line lw 3";
	    $cnt++;
	    if (defined($updatepl[$cnt])) {
		print GP ", \\\n";
		next;
	    }
	    last;
        }
    }
    if (defined($withdrawpl[0])) {
    	if (defined($updatepl[0])) {
    		print GP ", \\\n";
    	}
    	$cnt = 0;
    	while(1) {
	    print GP "  '$DATADIR/dl$withdrawpl[$cnt].dat' using 1:3 title '/$withdrawpl[$cnt] withdraw' with line lw 3";
	    $cnt++;
	    if (defined($withdrawpl[$cnt])) {
		print GP ", \\\n";
		next;
	    }
	    last;
        }
    }
    print GP "\n";
    print GP "quit\n";
    close(GP);
}

sub makedata {
	unless(open(LG, "$MLCNTLOG")) {
		print "Could not open mlcnt.pl log\n";
		return;
	}

	$dt = "";	
	@updatelist = ();
	@withdrawlist = ();
	while(<LG>) {
		chomp;
		next if /^$/;
		next if /^\s+$/;
		next if /^#/;

		@datas = split(/\s+/, $_);
		$dt .= "$datas[0]-$datas[1] ";

		$cnt = 0;
		while(defined($cur = $updatepl[$cnt])) {
			$updatelist[$cnt] .= "$datas[$cur + 1] ";
			$cnt++;
		}
		$cnt = 0;
		while(defined($cur = $withdrawpl[$cnt])) {
			$withdrawlist[$cnt] .= "$datas[$cur + 32] ";
			$cnt++;
		}
	}
	close(LG);

	@dts = split(/\s+/, $dt);
	$cnt = 0;
	while(defined($cur = $updatepl[$cnt])) {
		unless(open(LL, "> $DATADIR/ul$cur.dat")) {
			print "Could not open : ul$cur.dat\n";
			last;
		}
		@datas = split(/\s+/, $updatelist[$cnt]);
		$cnt2 = 0;
		while(defined($datas[$cnt2])) {
			@del = split(/-/, $dts[$cnt2]);
			print LL "$del[0] $del[1] $datas[$cnt2]\n";
			$cnt2++;
		}
		close(LL);
		$cnt++;
	}
	$cnt = 0;
	while(defined($cur = $withdrawpl[$cnt])) {
		unless(open(LL, "> $DATADIR/dl$cur.dat")) {
			print "Could not open : dl$cur.dat\n";
			last;
		}
		@datas = split(/\s+/, $withdrawlist[$cnt]);
		$cnt2 = 0;
		while(defined($datas[$cnt2])) {
			@del = split(/-/, $dts[$cnt2]);
			print LL "$del[0] $del[1] $datas[$cnt2]\n";
			$cnt2++;
		}
		close(LL);
		$cnt++;
	}

}

&makedata;
&drawgnuplot;
&makegif;

