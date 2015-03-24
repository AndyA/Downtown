#!/usr/bin/env perl

use v5.10;

use autodie;
use strict;
use warnings;

use Math::Trig ':pi';

sub deg($) { $_[0] * 180 / pi; }

for ( my $angle = 0; $angle <= pi * 2; $angle += pi / 12 ) {
  my $coef = sin( $angle - pi / 2 ) + 1;
  print "$coef ";
}
print "\n";

# vim:ts=2:sw=2:sts=2:et:ft=perl

