#!/bin/bash

for dir in "$@"; do
  json="$dir.json"
  [ -e "$json" ] && continue
  tmp="$dir.tmp.json"
  node tools/summate.js "$dir"/* | tee >( grep -v '^#' > "$tmp" ) && mv "$tmp" "$json"
done

# vim:ts=2:sw=2:sts=2:et:ft=sh

