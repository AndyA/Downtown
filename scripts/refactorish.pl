#!/usr/bin/env perl

use v5.10;

use autodie;
use strict;
use warnings;

@ARGV & 1 && die "Need an even number of args";
my %MAP = ( @ARGV, map uc, @ARGV );

my $find = join '|', map "(?:$_)", map quotemeta, sort keys %MAP;
my $pat = qr{(\b|(?<=_))($find)(\b|(?=_))};

while (<STDIN>) {
  s/$pat/$1 . $MAP{$2} . $3/eg;
  print;
}

# vim:ts=2:sw=2:sts=2:et:ft=perl

