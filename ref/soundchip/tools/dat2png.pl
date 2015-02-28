#!/usr/bin/env perl

use autodie;
use strict;
use warnings;

use GD::Image;
use List::Util qw( sum min max );

use constant MAXHEIGHT => 512;
use constant BOOST     => 30;

my $out = shift || die;

my @dat = ();
while (<>) {
  chomp;
  my ( $chan, @col ) = split /\s+/;
  push @{ $dat[$chan] }, [@col];
}

my %ch = ();
$ch{$_}++ for map { scalar @$_ } map { @$_ } @dat;
my @ch = sort { $a <=> $b } keys %ch;
die "Mixed column heights: ", join( ', ', @ch ), "\n" if @ch > 1;

my $width  = @{ $dat[0] };
my $height = $ch[0];
my $min    = min( map @$_, map @$_, @dat );
my $max    = max( map @$_, map @$_, @dat );
print "$width x $height (min: $min, max: $max)\n";

# Munge data for ease of plotting

my @pt = ();
for my $chan ( 0 .. $#dat ) {
  my @cols = @{ $dat[$chan] };
  for my $col ( 0 .. $#cols ) {
    my @p = @{ $cols[$col] };
    for my $i ( 0 .. $#p ) {
      $pt[$col][$i][$chan] = $p[$i];
    }
  }
}

my @col
 = @dat == 1 ? ( [255, 255, 255] )
 : @dat == 2 ? ( [255, 0, 0], [0, 255, 0] )
 :             die;

my $img = GD::Image->new( $width, $height, 1 );
for my $x ( 0 .. $width - 1 ) {
  for my $y ( 0 .. $height - 1 ) {
    my $pd = $pt[$x][$y];
    my ( $r, $g, $b );
    for my $ch ( 0 .. $#dat ) {
      my $pv = ( $pd->[$ch] - $min ) / ( $max - $min ) * BOOST;
      $pv = max( 0, min( $pv, 1 ) );
      $r += $col[$ch][0] * $pv;
      $g += $col[$ch][1] * $pv;
      $b += $col[$ch][2] * $pv;
    }

    $img->setPixel(
      $x,
      ( $height - 1 - $y ),
      $img->colorAllocate( $r, $g, $b )
    );
  }
}

{
  open my $fh, '>', $out;
  print $fh $img->png;
}

# vim:ts=2:sw=2:sts=2:et:ft=perl
