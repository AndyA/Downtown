#!/usr/bin/env perl

use v5.10;

use autodie;
use strict;
use warnings;

use constant SCALE_RATE => 0.02;
use constant MERGE      => 36;

my ( $total, $count ) = ( 0, 0 );

sub spew {
  print $total/ $count, "\n";
  ( $total, $count ) = ( 0, 0 );
}

sub put {
  my $n = shift;
  $total += $n;
  $count++;
}

while (<>) {
  next unless /raw_rate:\s+(\d+\.\d+),/;
  my $raw_rate = $1;
  next if $raw_rate <= 0;
  my $rms2 = SCALE_RATE / $raw_rate;
  my $rms  = sqrt $rms2;
  put($rms);
  spew if $count >= MERGE;
}
spew;

# vim:ts=2:sw=2:sts=2:et:ft=perl

