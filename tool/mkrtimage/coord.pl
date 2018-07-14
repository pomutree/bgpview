#!/usr/bin/perl

# This Line add by Kuniaki Kondo
# This tool is released on http://micho.mimora.com/routegraph/ ,
# which is made by Shuichi Okubo.

$width = 800;
$height = 600;

$hspace = 13;
$wspace = 14;

$hmargin = ($height - $hspace * 2) / 33;

while(<>) {
	chomp;
	@a = split(/\s+/);
	($x, $y, $rx1, $ry1, $rx2, $ry2) = &coord($a[0], $a[1]);
	print "point: $x $y\n";
	print "rect: $rx1 $ry1 $rx2 $ry2\n";
}
exit;

sub coord {
	my ($prefix, $len) = @_;

	my @prefix = split(/\./, $prefix);

	my $yrand = rand($hmargin);
	my $y = $hspace + $len * $hmargin + $yrand;

	my $z = ($prefix[0] * 2**24 + $prefix[1] * 2**16 + $prefix[2] * 2**8 + $prefix[3]) / 2**(32-$len);
	my $n = 2 ** $len;
	my $x = (($width - $wspace * 2) / (2 * $n)) + $z * (($width - $wspace * 2)/ $n) + $wspace;

	my $rx1 = $x - (($width - $wspace * 2) / (2 * $n));
	my $ry1 = $y;

	my $rx2 = $x + (($width - $wspace * 2) / (2 * $n));
	my $ry2 = $height - 1;

	return(int($x), int($y), int($rx1), int($ry1), int($rx2), int($ry2));
}
