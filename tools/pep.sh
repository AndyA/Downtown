#!/bin/bash

moviemaker='node js/sig.js'
pip_size='432:324'
pip_x='0.8'
pip_y='0.1'
#time_limit=10

for obj in "$@"; do
    { [ -d "$obj" ]                           \
        && find "$obj" -type f -name '*.dat'  \
        || echo "$obj"
    } | while read dat; do

    mpeg="$( echo "$dat" | perl -pe 's/\.mpeg\..*/.mpeg/' )"

    mjpeg="${mpeg%.*}.mjpeg"
    mjpeg_t="${mpeg%.*}.tmp.mjpeg"

    m4v="${mpeg%.*}.warp.m4v"
    m4v_t="${mpeg%.*}.warp.tmp.m4v"

    extra=
    [ -n "$time_limit" ] && extra="-t $time_limit"

    if [ "$dat" -nt "$m4v" ]; then

      echo "$dat -> $m4v"

      $moviemaker $extra "$dat" "$mjpeg_t" && mv "$mjpeg_t" "$mjpeg" || exit

      ffmpeg                                                                                                       \
        -nostdin                                                                                                   \
        $extra -i "$mjpeg"                                                                                         \
        $extra -i "$mpeg"                                                                                          \
        -filter_complex                                                                                            \
          "[1:v]scale=$pip_size[pip], [0:v][pip]overlay=$pip_x*(main_w-overlay_w):$pip_y*(main_h-overlay_h)[out]"  \
        -map '[out]'                                                                                               \
        -pix_fmt yuv420p -aspect 16:9                                                                              \
        -preset ultrafast -c:v libx264 -b:v 8000k                                                                  \
        -y "$m4v_t" && mv "$m4v_t" "$m4v" || exit

    fi

  done
done

exit
# vim:ts=2:sw=2:sts=2:et:ft=sh

