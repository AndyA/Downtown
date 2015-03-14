#!/bin/bash

DURATION=240

outsize="1920x1080"

for src in "$@"; do

  for merge in 1 5; do

    raw="$src.raw.$merge.y4m2"
    rm -rf "$raw"

    for size in 256; do

      scaled="$src.scaled.$size.$merge.y4m2"
      rm -rf "$scaled"

      for gain in 5; do
        for chans in 'y'; do
          for sampler in 'zigzag'; do
            for delta in 'n' 'y'; do

              tag="chans=${chans}"
              tag="${tag}.delta=${delta}"
              tag="${tag}.gain=${gain}"
              tag="${tag}.merge=${merge}"
              tag="${tag}.sampler=${sampler}"
              tag="${tag}.size=${size}"

              dst="$src.$tag.mov"

              if [ "$src" -nt "$dst" ]; then

                tmp="$dst.tmp.mov"

                if [ ! -e "$raw" ]; then
                  dt_f_config="--merge $merge"
                  ffmpeg -nostdin -i "$src" -t $DURATION -f yuv4mpegpipe - \
                    | ./downtown_filter $dt_f_config > "$raw"
                fi

                if [ ! -e "$scaled" ]; then
                  ffmpeg -nostdin -f yuv4mpegpipe -i "$raw" \
                    -pix_fmt yuv444p -s ${size}x${size} -f yuv4mpegpipe -y "$scaled"
                fi

                dt_config="--size $outsize --sampler $sampler --gain $gain"
                [ "$chans" = 'y' ] && dt_config="$dt_config --mono"
                [ "$delta" = 'y' ] && dt_config="$dt_config --delta"

                echo "$src -> $dst ($dt_config)"

                # Pipe source via downtown_filter into two pipes; one for downtown, one for the overlay

                ffmpeg \
                  -nostdin \
                  -f yuv4mpegpipe \
                  -i <( cat "$scaled" | ./downtown $dt_config ) \
                  -i "$raw" \
                  -filter_complex '[0:v][1:v]overlay=x=30:y=40[out]' -map '[out]' \
                  -aspect 16:9 -c:v libx264 -b:v 8000k -y "$tmp" && mv "$tmp" "$dst" || exit

              fi

            done #delta
          done #sampler
        done #chans
      done #gain

      rm -rf "$scaled"

    done #size

    rm -rf "$raw"

  done #merge


done #src

# vim:ts=2:sw=2:sts=2:et:ft=sh

