#!/usr/bin/perl

# this script converts BGPView route file to rib file of 
# Shuichi's tool.

# Copyrights (C) 2006 Kuniaki Kondo, Bugest-Network
# All rights Reserved.
# makerib.pl,v 1.1 2006/02/21 01:59:10 kuniaki Exp

{
	$fname = $ARGV[0];

	unless(open(RF, $fname)) {
		print "File Could not open '$fname'\n";
		exit(1);
	}

	while(<RF>) {
		chomp;
		next if /^\!/;
		next if /^$/;
		next if /^\s+$/;
		@data = split(/\s+/, $_);
		next if ($data[1] =~ /==/);
		next if ($data[1] =~ /Prefix/);
		@data2 = split(/\//, $data[1]);
		next if (defined($data2[2]));

		print "$data2[0] $data2[1]\n";
	}
	close(RF);
}

