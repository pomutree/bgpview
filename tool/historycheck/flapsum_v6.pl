#!/usr/bin/perl
# Flapped Route Summary for BGPView for IPv6 routes
# flapsum_v6.pl,v 1.1 2002/04/19 05:15:41 kuniaki Exp

#
# Copyright (c) 2002 Internet Initiative Japan Inc.
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

#
# Copyright (c) 2003 Intec NetCore, Inc.
# All rights reserved.
#

$ORIGINAS = 18146;

sub checkas {
	local($oneline) = @_;
	local(@dd, @dd2, $origin, $plen, $ot, $tempc);

	$tempc = 0;
	@dd = split(/\s+/, $oneline);
	while(defined($dd[$tempc])) {
		$tempc++;
	}
	$tempc--;

	if ($dd[$tempc] =~ /^\?$/) {
		$ot = 1;
	} elsif ($dd[$tempc] =~ /^i$/) {
		$ot = 2;
	} elsif ($dd[$tempc] =~ /^e$/) {
		$ot = 3;
	}

	$tempc--;
	$origin = $dd[$tempc];
	$plen=0;

	while(!($dd[$tempc] =~ /[:dhwy]/)) {
		$plen++;
		$tempc--;
	}

	# DEBUG
	#print "$oneline\n";
	#print "--> Origin:$origin / Type:$ot / Leng:$plen\n";

	return($origin, $ot, $plen);
}

{

	local($filename, $cnt, @eachplen, $updated, $total, $active,
	      $totalupdate, $totalwithdraw, $withdrawed, $adaynew,
	      @etimcnt, $actflg, $unreal, $chkday, $ecnt);

	$filename = $ARGV[0];
	if (!defined($ARGV[1])) {
		$chkyea = 0;
		$chkmon = 0;
		$chkday = 0;
	} else {
		@chks = split(/\//, $ARGV[1]);
		$chkyea = $chks[0];
		$chkmon = $chks[1];
		$chkday = $chks[2];
	}

	$cnt = 0;
	while($cnt <= 64) {
		$eachplen[$cnt] = 0;
		$cnt++;
	}
	$cnt = 0;
	while($cnt <= 24) {
		$etimecnt[$cnt] = 0;
		$cnt++;
	}
	$cnt = 1;
	while($cnt <= 3) {
		$origintype[$cnt] = 0;
		$cnt++;
	}

	$updated 	= 0;
	$total		= 0;
	$active		= 0;
	$totalupdate	= 0;
	$totalwithdraw	= 0;
	$withdrawed	= 0;
	$adaynew	= 0;
	$nflag		= 0;
	$unreal		= 0;
	$unstablenew    = 0;
        @pathl_sd       = ();
        $ptc            = -1;
        @ptotal         = ();

	@originroute = ();
	@oasupdate = ();
	@oaswithdraw = ();

	unless(open(FL, "$filename")) {
		print "Could not open log file : $filename\n";
		exit;
	}

	while(<FL>) {
		next if (/^$/);
		next if (/^\s+$/);
		next if (/AS_Path/);
		last if (/^\%\ No\ Route/);
		chomp;
		if (/==\ Prefix\ Update\ History\ /) {
			$ptc++;
                        $ptotal[$ptc] = 0;
			# Flapped Route
			$updated++;
			$eachplen[$lstack]++;

			$nflag = 0;
			if ($chkday != 0) {
				# Unreal Withdraw
				@data = split(/\s+/, $_);
				@data2 = split(/\//, $data[6]);
				if (($data2[0] == $chkyea) && ($data2[1] == $chkmon) &&
				    ($data2[2] == $chkday) && ($actflg == 0)) {
					$unreal++;
				}
				if (($data2[0] == $chkyea) && ($data2[1] == $chkmon) &&
				    ($data2[2] == $chkday) && ($actflg == 1)) {
					$unstablenew++;
				}
			}
		} elsif (/WITHDRAW/) {
			$totalwithdraw++;
			@data = split(/\s+/, $_);
			@data2 = split(/:/, $data[2]);
			$etimecnt[$data2[0]]++;
			$oaswithdraw[$oas]++;
			$ptotal[$ptc]++;
		} elsif (/UPDATE/) {
			$totalupdate++;
			@data = split(/\s+/, $_);
			@data2 = split(/:/, $data[2]);
			$etimecnt[$data2[0]]++;
			$oasupdate[$oas]++;
			$ptotal[$ptc]++;
		} else {
			$total++;
			if ($nflag == 1) {
				$adaynew++;
				$nflag = 0;
			}
			($oas, $otype, $pathl) = &checkas($_);

			if ($oas == 0) {
				$oas = $ORIGINAS;
			}

			if (!defined($oasupdate[$oas])) {
				$oaswithdraw[$oas] = 0;
				$oasupdate[$oas] = 0;
			}
			if (/^\!/) {
				$withdrawed++;
				$actflg = 0;
			} else {
				$pathl_sd[$active] = $pathl;
				$active++;
				@data3 = split(/\s+/, $_);
				$ecnt = 3;
				while(defined($data3[$ecnt])) {
					if ($data3[$ecnt] =~ /:/) {
						$nflag = 1;
						last;
					} else {
						$nflag = 0;
					}
					$ecnt++;
				}
				$actflg = 1;
				$origintype[$otype]++;
				$originroute[$oas]++;
			}
			@data = split(/\s+/, $_);
			@data2 = split(/\//, $data[1]);
			$pstack = $data2[0];
			$lstack = $data2[1];
		}
	}
	if ($nflag == 1) { $adaynew++; }

	close(FL);

	for($cnt=0; $cnt<65536; $cnt++) {
		if (defined($originroute[$cnt])) {
			$norg++;
		}
	}

	@oaslist = ();
	@oastotalup = ();
	$cnt = 0;
	while(!defined($oasupdate[$cnt])) {
		if ($cnt >= 70000) {
			print "Data Illegal\n";
			exit;
		}
		if (!defined($oasupdate[$cnt])) {
			$cnt++;
			next;
		}
		if (($oasupdate[$cnt] + $oaswithdraw[$cnt]) > 1) {
			last;
		}
		$cnt++;
	}
	$oaslist[0] = $cnt;
	$oastotalup[0] = $oasupdate[$cnt] + $oaswithdraw[$cnt];

	for(; $cnt<65536; $cnt++) {
		if (!defined($oasupdate[$cnt])) {
			next;
		}
		$temptotal = $oasupdate[$cnt] + $oaswithdraw[$cnt];
		if ($temptotal <= 1) {
			next;
		}
		$cnt1 = 0;
		while(defined($oastotalup[$cnt1])) {
			if ($oastotalup[$cnt1] < $temptotal) {
				undef($temp1);
				undef($temp2);
				$astemp = $cnt;
				$ttemp = $temptotal;;
				while(defined($oastotalup[$cnt1])) {
					if (defined($temp1)) {
						$ttemp = $temp1;
						$astemp = $temp2;
					}
					$temp1 = $oastotalup[$cnt1];
					$temp2 = $oaslist[$cnt1];
					$oastotalup[$cnt1] = $ttemp;
					$oaslist[$cnt1] = $astemp;
					$cnt1++;
				}
				$oastotalup[$cnt1] = $temp1;
				$oaslist[$cnt1] = $temp2;
				last;
			}
			$cnt1++;
		}
		if (!defined($oastotalup[$cnt1])) {
			$oastotalup[$cnt1] = $temptotal;
			$oaslist[$cnt1] = $cnt;
		}
	}


	print "=== Summary ===\n";
	printf(" Total Route           : %7d routes\n",$total);
	printf(" Total Active Route    : %7d routes\n",$active);
	printf(" Total In-Active Route : %7d routes\n",$withdrawed);
	if ($chkday != 0) {
	if ($adaynew < ($total-$updated)) {
		printf("     In briefly appear : %7d routes\n",$unreal);
		printf("     In real withdraw  : %7d routes\n",$withdrawed - $unreal);
	}
	}
	printf(" Stable Route          : %7d routes\n",$total-$updated);
	# printf("          In new route : ");
	if ($adaynew >= ($total-$updated)) {
		printf("       Total New Route :       0 routes\n");
		printf("  (Uncount: Too many new routes, probably reconnect)\n");
	} else {
		printf("       In unstable new : %7d routes\n", $unstablenew);
		printf("       In stable new   : %7d routes\n", $adaynew);
		printf("       Total New Route : %7d routes\n",
							$unstablenew+$adaynew);
		printf(" Incresed Routes       : %7d routes\n",
				($unstablenew+$adaynew) - ($withdrawed-$unreal));
	}

        # Calc Changes Info
        $min = $ptotal[0];
        $max = $ptotal[0];
        $cnt = 0;
        $totalchn = 0;
        while(defined($ptotal[$cnt])) {
                $totalchn += $ptotal[$cnt];
                $min = $ptotal[$cnt] if ($min > $ptotal[$cnt]);
                $max = $ptotal[$cnt] if ($max < $ptotal[$cnt]);
                $cnt++;
        }
	if ($totalchn == 0) {
		$upd_avg = 0;
	} else {
        	$upd_avg = $totalchn / $updated;
	}
        $cnt = 0;
        $sd = 0;
        while(defined($ptotal[$cnt])) {
                $sd += ($ptotal[$cnt] - $upd_avg)**2;
                $cnt++;
        }
	if ($sd == 0) {
		$psd = 0;
	} else {
		$psd = sqrt($sd / $updated);
	}

	printf(" Total Updated Route   : %7d routes\n",$updated);
	printf(" Updated Still Active  : %7d routes\n", $updated-$withdrawed);
	printf(" Total Update Times    : %7d times\n",$totalupdate);
	printf(" Total Withdraw Times  : %7d times\n",$totalwithdraw);
	printf(" Total Changes         : %7d times\n",
						 $totalupdate+$totalwithdraw);
#	printf("               Average : %7.1f times/updated route\n",
#					($totalupdate+$totalwithdraw)/$updated);
        printf("               Average : %7.1f times/updated route\n", $upd_avg);
        printf("                            Max : %7.1f times\n", $max);
        printf("                            Min : %7.1f times\n", $min);
        printf("                            SD  : %7.1f\n", $psd);
	print "\n";

	printf(" AS Path Analysis\n");
	printf("   Incomplete Route    : %7d routes\n", $origintype[1]);
	printf("   IGP Route           : %7d routes\n", $origintype[2]);
	printf("   EGP Route           : %7d routes\n", $origintype[3]);
	printf("   Num. of origin AS   : %7d origins\n", $norg);
	printf("   Num. of Route per AS: %5.1f routes\n", $active/$norg);

        # Calc Min/Avg/Max/SD
        $min = $pathl_sd[0];
        $max = $pathl_sd[0];
        $total_lenght = 0;
        $cnt=0;
        while(defined($pathl_sd[$cnt])) {
                $total_length += $pathl_sd[$cnt];
                $min = $pathl_sd[$cnt] if ($min > $pathl_sd[$cnt]);
                $max = $pathl_sd[$cnt] if ($max < $pathl_sd[$cnt]);
                $cnt++;
        }
        $pathl_avg = $total_length / $active;
        $sd = 0;
        $cnt = 0;
        while(defined($pathl_sd[$cnt])) {
                $sd += ($pathl_sd[$cnt]-$pathl_avg)**2;
                $cnt++;
        }
        $psd = sqrt($sd / $active);
        printf("   AS Path length avg. : %5.1f\n", $pathl_avg);
        printf("                            Max : %5.1f\n", $max);
        printf("                            Min : %5.1f\n", $min);
        printf("                            SD  : %5.1f\n", $psd);
	printf("\n");


	print " Updated routes for each prefix length\n";
	$cnt=1;
	while($cnt<=32) {
		printf("   /%2d : %5d routes  |  /%2d : %5d routes\n", 
				$cnt, $eachplen[$cnt],
				$cnt+32, $eachplen[$cnt+32]);
		$cnt++;
	}
	print "\n";

	print " Update count for each hour\n";
	$cnt = 0;
	while($cnt<=11) {
		printf("   %02d : %5d routes  |  %02d : %5d routes\n",
				$cnt, $etimecnt[$cnt],
				$cnt+12, $etimecnt[$cnt+12]);
		$cnt++;
	}
	print "\n";


	print " Update Ranking\n";
	print "   Rank  : AS Num  : Update + Withdr = Total\n";
	$cnt = 0;
	while(defined($oastotalup[$cnt])) {
		last if ($cnt >= 50);
		printf("   %5d : AS%-5d : %6d + %6d = %6d\n",
			$cnt+1, $oaslist[$cnt], $oasupdate[$oaslist[$cnt]],
			$oaswithdraw[$oaslist[$cnt]], $oastotalup[$cnt]);
		$cnt++;
	}
	print "\n";

}


