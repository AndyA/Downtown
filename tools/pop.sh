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

      for gain in 50; do
        for chans in 'y'; do
          for sampler in 'spiral'; do
            for delta in 'n'; do
              for centre in 'n'; do
                for graph in 'n'; do

                  tag="chans=${chans}"
                  tag="${tag}.graph=${graph}"
                  tag="${tag}.centre=${centre}"
                  tag="${tag}.delta=${delta}"
                  tag="${tag}.gain=${gain}"
                  tag="${tag}.merge=${merge}"
                  tag="${tag}.sampler=${sampler}"
                  tag="${tag}.size=${size}"


                  dst="$src.$tag.mov"
                  dat="$src.$tag.dat"

                  if [ "$graph" = "n" ]; then
                    # Rescue output from previous version
                    otag="chans=${chans}"
                    otag="${otag}.centre=${centre}"
                    otag="${otag}.delta=${delta}"
                    otag="${otag}.gain=${gain}"
                    otag="${otag}.merge=${merge}"
                    otag="${otag}.sampler=${sampler}"
                    otag="${otag}.size=${size}"
                    odst="$src.$otag.mov"
                    if [ -e "$odst" ]; then
                      mv "$odst" "$dst"
                      continue
                    fi
                  fi

                  if [ "$src" -nt "$dst" ]; then

                    tmp="$dst.tmp.mov"

                    if [ ! -e "$raw" ]; then
                      dt_f_config="--merge $merge"
                      ffmpeg -nostdin -i "$src" -t $DURATION -f yuv4mpegpipe - \
                        | ./downtown_filter $dt_f_config > "$raw" || exit
                    fi

                    if [ ! -e "$scaled" ]; then
                      ffmpeg -nostdin -f yuv4mpegpipe -i "$raw" \
                        -pix_fmt yuv420p -s ${size}x${size} -f yuv4mpegpipe -y "$scaled" || exit
                    fi

                    dt_config="--size $outsize --sampler $sampler --gain $gain"
                    [ "$chans" = 'y' ] && dt_config="$dt_config --mono"
                    [ "$centre" = 'y' ] && dt_config="$dt_config --centre"
                    [ "$delta" = 'y' ] && dt_config="$dt_config --delta"
                    [ "$graph" = 'y' ] && \
      dt_config="$dt_config --graph rms:#f00 --graph energy:#0f0 --graph min:#00f --graph average:#44f --graph max:#88f"

                    echo "$src -> $dst ($dt_config)"

                    # Pipe source via downtown_filter into two pipes; one for downtown, one for the overlay

                    ffmpeg \
                      -nostdin \
                      -f yuv4mpegpipe \
                      -i <( cat "$scaled" | ./downtown --output "$dat" $dt_config ) \
                      -i "$raw" \
                      -filter_complex '[0:v][1:v]overlay=x=30:y=40[out]' -map '[out]' \
                      -pix_fmt yuv420p -aspect 16:9 -c:v libx264 -b:v 8000k -y "$tmp" && mv "$tmp" "$dst" || exit

                  fi

                done #graph
              done #centre
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

