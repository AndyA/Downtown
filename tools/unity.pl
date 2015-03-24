#!/usr/bin/env perl

use v5.10;

use autodie;
use strict;
use warnings;

use constant NOWT => 0.000000001;

for my $kernel (@ARGV) {
  my @coef = load_coef($kernel);
  my $area = calc_area(@coef);
  printf "\n%-30s: area = %f\n\n", $kernel, $area;
  print "  coef ", join( ' ', @coef ), "\n";
  my @fcoef = fix_coef(@coef);
  print "  fixed ", join( ' ', @fcoef ), "\n";
  my @scoef = set_scale( 1, @fcoef );
  print "  scaled ", join( ' ', @scoef ), "\n";
  ( my $unity = $kernel ) =~ s/\.dat$/.unity/g;
  print "  writing $unity\n";
  save_coef( $unity, @scoef );
}
print "\n";

sub fix_coef {
  my @coef = @_;
  if ( @coef % 2 ) {
    my $mid = @coef / 2;
    splice @coef, $mid, 0, $coef[$mid];
  }
  return @coef;
}

sub set_scale {
  my ( $scale, @coef ) = @_;
  while () {
    print "  trying: ", join( ' ', @coef ), "\n";
    my $area = calc_area(@coef);
    die "Can't scale zero" if $area < NOWT;
    my $adj = $scale / $area;
    print "    area = $area, adj = $adj\n";
    last if abs( 1 - $adj ) < NOWT;
    @coef = map { $_ * $adj } @coef;
  }
  return @coef;
}

sub save_coef {
  my ( $file, @coef ) = @_;
  open my $fh, '>', $file;
  print $fh join( ' ', @coef ), "\n";
}

sub load_coef {
  my $file = shift;
  open my $fh, '<', $file;
  my @coef = ();
  while (<$fh>) {
    chomp;
    push @coef, split /\s+/;
  }
  return @coef;
}

sub calc_area {
  my @coef = @_;
  return 0 if @coef < 2;

  my $area = 0;
  my $prev = shift @coef;
  while (@coef) {
    my $next = shift @coef;
    $area += ( $prev + $next ) / 2;
    $prev = $next;
  }
  return $area;
}

# vim:ts=2:sw=2:sts=2:et:ft=perl

