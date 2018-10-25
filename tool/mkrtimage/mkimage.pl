#!/usr/bin/perl

# This Line add by Kuniaki Kondo
# This tool is released on http://micho.mimora.com/routegraph/ ,
# which is made by Shuichi Okubo.

use GD;

$width = 800;
$height = 600;

$hspace = 13;
$wspace = 14;

$im = new GD::Image($width, $height, 1);

$black = $im->colorAllocate(0, 0, 0);
$white = $im->colorAllocate(255, 255, 255);

$im->fill(0, 0, $white);

# xlabel
for ($i = 10; $i < 256; $i += 10) {
	$x = $i * ($width - $wspace * 2) / 256 + $wspace - 10;
	$im->string(gdSmallFont, $x, 0, $i, $black);
}

# ylabel
for ($i = 0; $i <= 32; $i ++) {
	$y = $hspace + $i * ($height - $hspace * 2) / 33;
	$str = sprintf("%2d", $i);
	$im->string(gdSmallFont, 2, $y, $str, $black);
}
# x upper line
$im->line(0, 13, $width-1, 13, $black);

# y left line
$im->line(14, 0, 14, $height-1, $black);

# each first octet line
for ($i = 10; $i < 256; $i += 10) {
	$x = $wspace + ($width - $wspace * 2) / 256 * $i;
	$im->line($x, 13, $x, $height - 1, $black);
}

# each len line
for ($i = 1; $i <= 33; $i++) {
	$y = $hspace + ($height - $hspace * 2) / 33 * $i;
	$im->line(0, $y, $width-1, $y, $black);
}

# used block color
$color = $im->colorAllocate(0, 229, 53);
while(<>) {
	chomp;
	if (/^point: (.*)$/) {
		@a = split(/\s+/, $1);
		$plot[int($a[0]) + int($a[1]) * $width]++;
		next;
	}
	if (/^rect: (.*)$/) {
		@a = split(/\s+/, $1);
		$im->filledRectangle($a[0], $a[1], $a[2], $a[3], $color);
	}
}

$max = 0;
for ($i = 0; $i < $width * $height; $i++) {
	$max = $plot[$i] if ($plot[$i] > $max);
}
print STDERR "max = $max\n";

for ($i = 0; $i < $width * $height; $i++) {
	next unless ($plot[$i]);
	$weight = $plot[$i] * 511 / $max;

	if ($weight <= 255) {
		$color = $im->colorAllocate($weight, 0, 255);
	} else {
		$color = $im->colorAllocate(255, 0, 511-$weight);
	}

	$im->setPixel($i % $width, int($i / $width), $color);
}

print $im->png;
