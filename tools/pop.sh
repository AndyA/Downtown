#!/bin/bash

DURATION=240

outsize="1920x1080"

for src in "$@"; do
  for gain in 1 4 16; do
    for chans in 'y' 'yuv'; do
      for size in 64 128 256 512; do
        for permute in 'zigzag' 'raster' 'weave'; do

          tag="$permute-$gain-$chans-$size"
          dst="$src.$tag.mov"

          if [ "$src" -nt "$dst" ]; then

            tmp="$dst.tmp.mov"

            dt_config="--size $outsize --permute $permute --gain $gain"
            [ "$chans" = 'y' ] && dt_config="$dt_config --mono"

            echo "$src -> $dst ($dt_config)"

            ffmpeg -i "$src" -pix_fmt yuv444p -s ${size}x${size} -t $DURATION -f yuv4mpegpipe - \
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

done

# vim:ts=2:sw=2:sts=2:et:ft=sh

