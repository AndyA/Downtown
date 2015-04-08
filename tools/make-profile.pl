#!/usr/bin/env perl

use v5.10;

use autodie;
use strict;
use warnings;

use Getopt::Long;
use JSON;
use Path::Class;
use Scalar::Util qw( looks_like_number );

use constant usage => <<EOT;
Usage: $0 [options] <average.json>

Options:

  -p, --params <params>   JSON format params

EOT

my %SAMPLER = (
  spiral => 'r_rate=5,a_rate=5,area_limit=1.2,edge_trim=1',
  zigzag => '',
  weave  => '',
  raster => '',
);

my %O = ( params => undef, );
my $json = JSON->new->pretty->canonical;

GetOptions( 'p|params:s' => \$O{params} ) or die usage;

for my $avg (@ARGV) {

  my $avg = shift @ARGV;
  ( my $prof = $avg ) =~ s/\.json$/.profile/;

  say "$avg -> $prof";

  my %p = ();
  my %s = ( name => 'spiral', parse_parms( $SAMPLER{spiral} ) );

  $p{width} = $p{height} = 0 + $1 if $avg =~ /\bs(\d+)\./;
  $s{a_rate} = 0 + $1 if $avg =~ /\ba_rate=(\d+(?:\.\d+))/;
  $s{r_rate} = 0 + $1 if $avg =~ /\br_rate=(\d+(?:\.\d+))/;

  %p = ( %p, %{ $json->decode( $O{params} ) } ) if defined $O{params};

  my $baseline = $json->decode( scalar file($avg)->slurp );
  my %prof = ( %p, sampler => mk_sampler( \%s ), baseline => $baseline );

  my $fh = file($prof)->openw;
  say $fh $json->encode( \%prof );
}

# Q&D
sub parse_parms { split /[=,]/, $_[0] }

sub mk_sampler {
  my $spec = shift;
  my $name = delete $spec->{name} // die "Missing name";

  return join ':', $name, join ',', map {
    "$_="
     . (
      looks_like_number( $spec->{$_} )
      ? $spec->{$_}
      : "'$spec->{$_}'"
     )
  } sort keys %$spec;
}

# vim:ts=2:sw=2:sts=2:et:ft=perl
