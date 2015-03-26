#!/usr/bin/env perl

use v5.10;

use autodie;
use strict;
use warnings;

use Data::Dumper;

use constant NOWT => 0.000000001;

for my $kernel (@ARGV) {
  my $coef = load_coef($kernel);
  my $scaled = fixup( $kernel, $coef );
  ( my $unity = $kernel ) =~ s/\.dat$/.unity/g;
  print "  writing $unity\n";
  save_coef( $unity, $scaled );
}

print "\n";

sub fixup {
  my ( $kernel, $coef ) = @_;

  if ( @$coef && ref $coef->[0] ) {
    return $coef; # pass through
    #    die unless @$coef == 2;
    #    my ( $pos, $neg ) = @$coef;
    #    my $parea = calc_area(@$pos);
    #    my $narea = calc_area(@$neg);
    #    my $ratio = $parea / $narea;
    #    my $adj = sqrt($ratio);
    #    return [[set_scale( $adj, @$pos )], [set_scale( 1 / $adj, @$neg )]];
  }

  my @scoef = set_scale( 1, @$coef );
  return \@scoef;
}

sub set_scale {
  my ( $scale, @coef ) = @_;
  while () {
    my $area = calc_area(@coef);
    die "Can't scale zero" if $area < NOWT;
    my $adj = $scale / $area;
    last if abs( 1 - $adj ) < NOWT;
    @coef = map { $_ * $adj } @coef;
  }
  return @coef;
}

sub write_coef {
  my ( $fh, $coef ) = @_;
  print $fh join( ' ', @$coef );
}

sub save_coef {
  my ( $file, $coef ) = @_;
  open my $fh, '>', $file;

  if ( @$coef && ref $coef->[0] ) {
    for my $sc (@$coef) {
      print $fh '[ ';
      write_coef( $fh, $sc );
      print $fh " ]\n";
    }
  }
  else {
    write_coef( $fh, $coef );
    print $fh "\n";
  }

}

sub parse_coef {
  my @coef = @_;
  my @out  = ();
  while (@coef) {
    my $next = shift @coef;
    if ( $next eq '[' ) {
      ( my $list, @coef ) = parse_coef(@coef);
      push @out, $list;
      next;
    }
    elsif ( $next eq ']' ) {
      last;
    }
    push @out, $next;
  }
  return ( \@out, @coef );
}

sub load_coef {
  my $file = shift;
  open my $fh, '<', $file;
  my @coef = ();
  while (<$fh>) {
    chomp;
    push @coef, grep { !/^\s*$/ } split /((?:\[|\]|\s))/;
  }
  my ( $out, @rem ) = parse_coef(@coef);
  die "Junk at end of file: ", join( ' ', @rem ) if @rem;
  return $out;
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

