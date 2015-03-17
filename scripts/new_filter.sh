#!/bin/bash

proto="passthru"
base="$( dirname "$0" )"

name="$1"
if [ -z "$name" ]; then
  echo "Syntax: $0 <filter_name>" 1>&2
  exit 1
fi

for ext in '.c' '.h'; do
  in="$proto$ext"
  out="$name$ext"
  perl "$base/refactorish.pl" "$proto" "$name" < "$in" > "$out"
done



# vim:ts=2:sw=2:sts=2:et:ft=sh

