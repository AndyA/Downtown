#!/bin/bash

graph='graph'
count=10

make || exit
rm -rf "$graph"

tools/unity.pl t/data/kernel.*.dat

for kernel in t/data/kernel.*.unity; do
  for data in t/data/data.*.dat; do
    kn="$( basename "$kernel" .unity )"
    dn="$( basename "$data" .dat )"
    tag="$kn.$dn"
    echo "$tag"
    set -x
    ./test-convolve -c $count -o "$graph/$tag.%03d.dat" "$data" "$kernel" || exit
    ./tools/mk-graph.pl "$graph/$tag.png" "$graph"/"$tag".*.dat | gnuplot || exit
    set +x
  done
done

# vim:ts=2:sw=2:sts=2:et:ft=sh

