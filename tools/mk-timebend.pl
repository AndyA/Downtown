#!/usr/bin/env perl

use v5.10;

use autodie;
use strict;
use warnings;

use List::Util qw( min max sum );

use Getopt::Long;

use constant RATE_MIN   => 1;
use constant RATE_MAX   => 50;
use constant RATE_BASE  => 100;
use constant RATE_BOOST => 100;

use constant NOWT => 0.0000000001;

use constant USAGE => <<EOT;
Usage: $0 <stats.csv>
EOT

GetOptions() or die USAGE;

my $stats_csv = shift // die USAGE;

my $stats = read_stats($stats_csv);

my @rms = @{ $stats->{delta_Y_rms} };

my $min = min(@rms);
my $max = max(@rms);
my $avg = average(@rms);

#print "min=$min, avg=$avg, max=$max\n";

my $zero = $avg / 2;

my @rate = ();

for my $v (@rms) {
  my $vv = RATE_MAX * RATE_BASE**( ( $zero - $v ) * RATE_BOOST );
  push @rate, max( RATE_MIN, min( $vv, RATE_MAX ) );
}

my @scaled = drain( timebend( i_seq(@rate), i_seq(@rate) ) );

my @jiffed = drain(
  timebend(
    i_min( i_avg( i_seq(@scaled), 100 ), 25 ),
    i_recip( i_seq(@scaled) )
  )
);

my @biffed
 = drain( timebend( i_seq(@jiffed), i_const( @jiffed / @rate ) ) );

printf "%8.3g\n", $_ for @biffed;

sub read_stats {
  my $file = shift;
  open my $fl, '<', $file;
  my @field  = ();
  my %series = ();
  while (<$fl>) {
    chomp;
    my @c = split /\s*,\s*/;

    unless (@field) {
      @field = @c;
      next;
    }

    push @{ $series{$_} }, shift @c for @field;

  }
  return \%series;
}

sub drain {
  my $i   = shift;
  my @buf = ();
  while ( defined( my $v = $i->() ) ) {
    push @buf, $v;
  }
  return @buf;
}

sub average {
  my @l = @_;
  return sum(@l) / @l;
}

sub i_append {
  my @iter = @_;
  return sub {
    return unless @iter;
    while () {
      my $next = $iter[0]->();
      return $next if defined $next;
      shift @iter;
    }
  };
}

sub i_rep_const {
  my ( $n, $count ) = @_;
  sub {
    return unless $count-- > 0;
    return $n;
  };
}

sub i_const {
  my $n = shift;
  sub { $n };
}

sub i_seq {
  my @s = @_;
  sub { shift @s };
}

sub i_min { i_window( @_, \&min ) }
sub i_avg { i_window( @_, \&average ) }

sub i_window {
  my ( $i, $width, $func ) = @_;
  my ( $done, @buf );
  return sub {
    until ( $done || @buf >= $width / 2 ) {
      my $next = $i->();
      unless ( defined $next ) {
        $done++;
        last;
      }
      push @buf, $next;
    }

    return unless @buf;

    my $v = $func->(@buf);
    shift @buf;
    return $v;
  };
}

sub i_recip {
  my $i = shift;
  sub {
    my $v = $i->();
    return unless defined $v;
    return 1 / $v;
  };
}

sub timebend {
  my ( $vi, $ri ) = @_;

  my ( $init, $prev, $sample, $rate, $duration );
  my $buf_time = 0;

  my $refill = sub {
    $prev   = $sample;
    $sample = $vi->();
    if ( defined $sample ) {
      $rate = $ri->();
      die "Run out of rates" unless defined $rate;
    }
    $duration = 1;
  };

  return sub {

    unless ($init) {
      $refill->();
      return unless defined $sample;
      $refill->();
      $init++;
    }

    my $buf_time = 0;
    my $val      = 0;

    while ( $buf_time < 1 ) {

      if ( $duration < NOWT ) {
        $refill->();
        return unless defined $prev;
      }

      my $got      = $duration / $rate;
      my $need     = min( 1 - $buf_time, $got );
      my $p_weight = $duration * $need;
      my $c_weight = ( 1 - $duration ) * $need;

      $val += $prev * $p_weight;
      $val += ( $sample // $prev ) * $c_weight;

      $duration -= $need * $rate;
      $buf_time += $need;
    }

    return $val;
  };
}

# vim:ts=2:sw=2:sts=2:et:ft=perl

