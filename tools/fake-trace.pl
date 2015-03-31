#!/usr/bin/env perl

use v5.10;

use autodie;
use strict;
use warnings;

use JSON;
use Math::Trig ':pi';

use constant FRAMES        => 100;
use constant DATA          => 200;
use constant PHASE_ADVANCE => 0.217;
use constant FREQUENCY     => 0.321;
use constant OFFSET        => 1;
use constant SCALE         => 20;

for my $frame ( 0 .. FRAMES - 1 ) {
  my $angle  = PHASE_ADVANCE * $frame;
  my @series = ();
  for my $pt ( 0 .. DATA - 1 ) {
    push @series, sin($angle) + OFFSET * SCALE;
    $angle += FREQUENCY;
  }
  print JSON->new->canonical->encode(
    { frame  => $frame,
      planes => [[@series]],
    }
   ),
   "\n";
}

# vim:ts=2:sw=2:sts=2:et:ft=perl

