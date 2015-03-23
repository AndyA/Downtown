#!/bin/bash

for src in "$@"; do
  dst="$src.timebend.mov"
  tmp="$dst.tmp.mov"
  if [ "$src" -nt "$dst" ]; then
    ffmpeg -nostdin -i "$src" -f yuv4mpegpipe - \
      | ./test-timebend \
      | ffmpeg -nostdin \
          -f yuv4mpegpipe -i - \
          -pix_fmt yuv420p -c:v libx264 -b:v 4000k \
          -y "$tmp" && mv "$tmp" "$dst" || exit
  fi
done


# vim:ts=2:sw=2:sts=2:et:ft=sh

