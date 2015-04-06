#!/bin/bash

DURATION=240

for obj in "$@"; do
    { [ -d "$obj" ]                            \
        && find "$obj" -type f -name '*.mpeg'  \
        || echo "$obj"
    } | while read src; do

    for parm in                                         \
      "size=128; spiral='r_rate=1.500,a_rate=3.375'"    \
      "size=128; spiral='r_rate=2.250,a_rate=3.375'"    \
      "size=128; spiral='r_rate=2.250,a_rate=5.062'"    \
      "size=128; spiral='r_rate=3.375,a_rate=5.062'"    \
      "size=128; spiral='r_rate=3.375,a_rate=5.062'"    \
      "size=128; spiral='r_rate=5.062,a_rate=5.062'"    \
      "size=128; spiral='r_rate=5.062,a_rate=7.594'"    \
      "size=128; spiral='r_rate=7.594,a_rate=5.062'"    \
      "size=128; spiral='r_rate=7.594,a_rate=7.594'"    \
      "size=128; spiral='r_rate=11.391,a_rate=7.594'"   \
      "size=128; spiral='r_rate=11.391,a_rate=11.391'"  \
      "size=128; spiral='r_rate=17.086,a_rate=7.594'"   \
      "size=256; spiral='r_rate=1.500,a_rate=3.375'"    \
      "size=256; spiral='r_rate=2.250,a_rate=3.375'"    \
      "size=256; spiral='r_rate=2.250,a_rate=5.062'"    \
      "size=256; spiral='r_rate=2.250,a_rate=7.594'"    \
      "size=256; spiral='r_rate=3.375,a_rate=5.062'"    \
      "size=256; spiral='r_rate=5.062,a_rate=5.062'"    \
      "size=256; spiral='r_rate=5.062,a_rate=7.594'"    \
      "size=256; spiral='r_rate=7.594,a_rate=5.062'"    \
      "size=256; spiral='r_rate=7.594,a_rate=7.594'"    \
      "size=256; spiral='r_rate=7.594,a_rate=11.391'"   \
      "size=256; spiral='r_rate=11.391,a_rate=7.594'"   \
      "size=256; spiral='r_rate=11.391,a_rate=11.391'"  \
      "size=512; spiral='r_rate=1.500,a_rate=3.375'"    \
      "size=512; spiral='r_rate=2.250,a_rate=5.062'"    \
      "size=512; spiral='r_rate=3.375,a_rate=5.062'"    \
      "size=512; spiral='r_rate=3.375,a_rate=7.594'"    \
      "size=512; spiral='r_rate=5.062,a_rate=5.062'"    \
      "size=512; spiral='r_rate=5.062,a_rate=7.594'"    \
      "size=512; spiral='r_rate=7.594,a_rate=7.594'"    \
      "size=512; spiral='r_rate=7.594,a_rate=11.391'"   \
      "size=512; spiral='r_rate=11.391,a_rate=7.594'"   \
      "size=512; spiral='r_rate=11.391,a_rate=11.391'"; do
      eval $parm;
      echo "src: $src, size: $size, spiral: $spiral"

      dat="$src.$size.$spiral.dat"
      if [ "$src" -nt "$dat" ]; then

        y4m2="$src.$size.y4m2"
        if [ "$src" -nt "$y4m2" ]; then
          y4m2_t="$y4m2.tmp.y4m2"
          ffmpeg                  \
            -nostdin              \
            -t "$DURATION"        \
            -i "$src"             \
            -s "${size}x${size}"  \
            -pix_fmt yuv420p      \
            -f yuv4mpegpipe       \
            -y "$y4m2_t"          \
              && mv "$y4m2_t" "$y4m2" || exit
        fi

        dat_t="$dat.tmp.dat"
        ./downtown-sig --input "$y4m2" --raw "$dat_t" --sampler "spiral:$spiral" \
          && mv "$dat_t" "$dat" || exit
      fi

    done

  done
done

# vim:ts=2:sw=2:sts=2:et:ft=sh

