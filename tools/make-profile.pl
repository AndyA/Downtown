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

GetOptions( 'p|params:s' => \$O{params} ) or die usage;
@ARGV == 1 or die usage;

my $avg  = shift @ARGV;
my $json = JSON->new->pretty->canonical;
my %p    = ( sampler => { name => 'spiral' } );

$p{size}            = 0 + $1 if $avg =~ /\bs(\d+)\./;
$p{sampler}{a_rate} = 0 + $1 if $avg =~ /\ba_rate=(\d+(?:\.\d+))/;
$p{sampler}{r_rate} = 0 + $1 if $avg =~ /\br_rate=(\d+(?:\.\d+))/;

%p = ( %p, %{ $json->decode( $O{params} ) } ) if defined $O{params};

my $baseline = $json->decode( scalar file($avg)->slurp );
my %prof = ( %p, baseline => $baseline );

say $json->encode( \%prof );

# vim:ts=2:sw=2:sts=2:et:ft=perl

