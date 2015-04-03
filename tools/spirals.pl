#!/usr/bin/env perl

use v5.10;

use autodie;
use strict;
use warnings;

use Path::Class;

use constant OUT  => dir('spirals');
use constant RISE => 1.5;

OUT->mkpath;

for ( my $sz = 64; $sz < 512; $sz *= 2 ) {
  my $testcard = sprintf 'testcard-%d.y4m2', $sz;
  for ( my $r_rate = 1; $r_rate < 20; $r_rate *= RISE ) {
    for ( my $a_rate = 1; $a_rate < 20; $a_rate *= RISE ) {
      my $tag = sprintf 's%d~r%.3f~a%.3f', $sz, $r_rate, $a_rate;
      my $out = file( OUT, sprintf( 'spiral-%s.png', $tag ) );
      my @cmd = (
        './downtown-sig',
        -i          => $testcard,
        '--sampler' => "spiral:r_rate=$r_rate,a_rate=$a_rate,dump=$out"
      );
      print join( ' ', @cmd ), "\n";
      system @cmd;
    }
  }
}
# vim:ts=2:sw=2:sts=2:et:ft=perl

