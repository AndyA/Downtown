#!/usr/bin/env perl

use autodie;
use strict;
use warnings;

use Getopt::Long;
use JSON;

use constant CHUNK => 60;

die "Syntax: json2c.pl <symbol> < in.json > out.c\n" unless @ARGV == 1;

my $sym = shift @ARGV;
my $ds  = JSON->new->canonical->encode(
  JSON->new->decode(
    do { local $/; <> }
  )
);

print "const char *$sym =\n";
while ( length $ds ) {
  my $chunk = CHUNK;
  $chunk = length $ds if $chunk > length $ds;
  my $frag = substr $ds, 0, $chunk;
  $ds = substr $ds, $chunk;
  print "  \"", to_c($frag), "\"";
  print ";" if $chunk < CHUNK;
  print "\n";
}

sub to_c {
  my $s = shift;
  $s =~ s/(["\\])/\\$1/g;
  return $s;
}

# vim:ts=2:sw=2:sts=2:et:ft=perl

