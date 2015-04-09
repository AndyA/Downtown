#!/usr/bin/env perl

use v5.10;

use autodie;
use strict;
use warnings;

my ($dat) = @ARGV;

die "Don't understand $dat" unless $dat =~ /(.+\.mpeg)\.(.+)$/;
my $mpeg      = $1;
my $prof_name = $2;
( my $prof = "s$prof_name" ) =~ s/\.dat$/.profile/;
( my $m4v  = "s$prof_name" ) =~ s/\.dat$/.m4v/;

say "dat_mpeg='$mpeg'";
say "dat_prof='$prof'";
say "dat_m4v='$mpeg.$m4v'";

# vim:ts=2:sw=2:sts=2:et:ft=perl

