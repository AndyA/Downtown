#!/bin/bash

moviemaker='node js/sig.js'
pip_size='432:324'
pip_x='0.95'
pip_y='0.1'

mm_extra=
ff_extra=

while getopts "t:s:" opt; do
  case $opt in
    t)
      mm_extra="$mm_extra -t $OPTARG"
      ff_extra="$ff_extra -t $OPTARG"
      ;;
    s)
      mm_extra="$mm_extra -ss $OPTARG"
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
  esac
done

shift $((OPTIND - 1))

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

    if [ "$dat" -nt "$m4v" ]; then

      echo "$dat -> $m4v"

      $moviemaker $mm_extra "$dat" "$mjpeg_t" && mv "$mjpeg_t" "$mjpeg" || exit

      ffmpeg                                                                                                       \
        -nostdin                                                                                                   \
        $ff_extra -i "$mjpeg"                                                                                      \
        $mm_extra -i "$mpeg"                                                                                       \
        -filter_complex                                                                                            \
          "[1:v]scale=$pip_size[pip], [0:v][pip]overlay=$pip_x*(main_w-overlay_w):$pip_y*(main_h-overlay_h)[out]"  \
        -map '[out]'                                                                                               \
        -pix_fmt yuv420p -aspect 16:9                                                                              \
        -c:v libx264 -b:v 8000k                                                                                    \
        -y "$m4v_t" && mv "$m4v_t" "$m4v" || exit

    fi

  done
done

exit
# vim:ts=2:sw=2:sts=2:et:ft=sh

