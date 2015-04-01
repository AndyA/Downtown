#!/bin/bash

for obj in "$@"; do
    { [ -d "$obj" ]                           \
        && find "$obj" -type f -name '*.mpeg'  \
        || echo "$obj"
    } | while read mov; do

    sig="$mov.sig"

    if [ "$mov" -nt "$sig" ]; then

      tmp="$sig.tmp"

      ffmpeg -nostdin -i "$mov"  -pix_fmt yuv420p -s 256x256 -f yuv4mpegpipe - \
        | ./downtown-sig --output "$tmp" && mv "$tmp" "$sig" || exit

    fi

  done
done

# vim:ts=2:sw=2:sts=2:et:ft=sh

