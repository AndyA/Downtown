#!/usr/bin/env perl

use v5.10;

use autodie;
use strict;
use warnings;

use JSON;
use Text::CSV_XS;

my $csv = Text::CSV_XS->new( { binary => 1, auto_diag => 1 } );

my @hdr = ();
my %col = ();
while ( my $row = $csv->getline(*STDIN) ) {

  unless (@hdr) {
    @hdr = @$row;
    next;
  }

  for my $fn ( 0 .. $#$row ) {
    push @{ $col{ $hdr[$fn] } }, 1 * $row->[$fn];
  }
}

print JSON->new->canonical->encode( \%col );

# vim:ts=2:sw=2:sts=2:et:ft=perl

