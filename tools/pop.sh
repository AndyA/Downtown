#!/bin/bash

DURATION=240

for src in "$@"; do
  for gain in 1 2 4 8 16; do
    for chans in 'y' 'yuv'; do
      for permute in 'zigzag' 'raster' 'weave'; do

        tag="$permute-$gain-$chans"
        dst="$src.$tag.mov"

        if [ "$src" -nt "$dst" ]; then
          echo "$src -> $dst"
          tmp="$dst.tmp.mov"
          dt_config="--permute $permute --gain $gain"
          [ "$chans" = 'y' ] && dt_config="$dt_config --mono"

          ffmpeg -i "$src" -pix_fmt yuv444p -s 256x256 -t $DURATION -f yuv4mpegpipe - \
            | ./downtown $dt_config \
            | ffmpeg \
                -f yuv4mpegpipe -i - \
                -t $DURATION -i "$src" \
                -filter_complex '[0:v][1:v]overlay=x=30:y=40[out]' -map '[out]' \
                -aspect 16:9 -c:v libx264 -b:v 8000k -y "$tmp" && mv "$tmp" "$dst"

        fi

      done
    done
  done

done

# vim:ts=2:sw=2:sts=2:et:ft=sh

