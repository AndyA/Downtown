#!/bin/bash

source="/data/media/Library/SFX/BBC"
data="dat"
png="png"

#make || exit
mkdir -p "$data" "$png"

find "$source" -not -path '*/.*'  -type f -name "*.WAV" -mmin +1 | while read wav; do
  name="$( basename "$wav" .WAV )"
  df="$data/$name.dat"
  pf="$png/$name.png"
  [ -e "$pf" ] && continue
  echo "$wav -> $pf"
  ./soundchip "$wav" > "$df" && perl tools/dat2png.pl "$pf" "$df"
done



# vim:ts=2:sw=2:sts=2:et:ft=sh

