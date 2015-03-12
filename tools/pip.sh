#!/bin/bash

DURATION=240

for src in "$@"; do
  dst="$src.downtown.mov"
  if [ "$src" -nt "$dst" ]; then
    echo "$src -> $dst"
    tmp="$dst.tmp.mov"

    ffmpeg -i "$src" -pix_fmt yuv420p -s 256x256 -t $DURATION -f yuv4mpegpipe - \
      | ./downtown \
      | ffmpeg \
          -f yuv4mpegpipe -i - \
          -t $DURATION -i "$src" \
          -filter_complex '[0:v][1:v]overlay=x=30:y=40[out]' -map '[out]' \
          -aspect 16:9 -c:v libx264 -b:v 8000k -y "$tmp" && mv "$tmp" "$dst"

  fi
done

# vim:ts=2:sw=2:sts=2:et:ft=sh

