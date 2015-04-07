#!/usr/bin/env perl

use v5.10;

use autodie;
use strict;
use warnings;

use Getopt::Long;
use JSON;
use Path::Class;

use constant usage => <<EOT;
Usage: $0 [options] <average.json>

Options:

  -p, --params <params>   JSON format params

EOT

my %O = ( params => undef, );
my $json = JSON->new->pretty->canonical;

GetOptions( 'p|params:s' => \$O{params} ) or die usage;

for my $avg (@ARGV) {

  my $avg = shift @ARGV;
  my %p = ( sampler => { name => 'spiral' } );

  $p{width} = $p{height} = 0 + $1 if $avg =~ /\bs(\d+)\./;
  $p{sampler}{p}{a_rate} = 0 + $1 if $avg =~ /\ba_rate=(\d+(?:\.\d+))/;
  $p{sampler}{p}{r_rate} = 0 + $1 if $avg =~ /\br_rate=(\d+(?:\.\d+))/;

  %p = ( %p, %{ $json->decode( $O{params} ) } ) if defined $O{params};

  my $baseline = $json->decode( scalar file($avg)->slurp );
  my %prof = ( %p, baseline => $baseline );

  say $json->encode( \%prof );
}

# vim:ts=2:sw=2:sts=2:et:ft=perl
