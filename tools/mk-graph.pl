#!/usr/bin/env perl

use v5.10;

use autodie;
use strict;
use warnings;

my ( $out, @dat ) = @ARGV;
my $first = $dat[0];

die "Nothing to plot" unless @dat;

print <<EOS;
set bmargin 8
set key left

set offsets graph 0, 0, 0.05, 0.05

set term unknown

plot "$first" with lines title '$first'

set yr [-0.1:GPVAL_DATA_Y_MAX*1.1]

set term png size 1920,1080
set output "$out"

EOS

my @scp = ();

for my $df (@dat) {
  push @scp, "\"$df\" with lines title '$df'";
}

print "plot ", join( ", \\\n     ", @scp ), "\n";

# vim:ts=2:sw=2:sts=2:et:ft=perl

