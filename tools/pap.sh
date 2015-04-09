#!/bin/bash

moviemaker='node js/profile.js'
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
        && find "$obj" -type f -name '*.mpeg.dat'  \
        || echo "$obj"
    } | while read dat; do

    dir="$( dirname "$dat" )"
    mpeg="$dir/$( basename "$dat" .dat )"

    mov="$dir/$( basename "$mpeg" .mpeg ).smooth.mov"
    mov_t="$mov.tmp.mov"

    mjpeg="$mov.mjpeg"
    mjpeg_t="$mov.tmp.mjpeg"

    if [ "$dat" -nt "$mov" ]; then

      echo "$dat -> $mov"

      if [ "$dat" -nt "$mjpeg" ]; then
        $moviemaker $mm_extra "$dat" "$mjpeg_t" && mv "$mjpeg_t" "$mjpeg" || exit
      fi

      ffmpeg                                                                                                       \
        -nostdin                                                                                                   \
        $ff_extra -i "$mjpeg"                                                                                      \
        $mm_extra -i "$mpeg"                                                                                       \
        -filter_complex                                                                                            \
          "[1:v]scale=$pip_size[pip], [0:v][pip]overlay=$pip_x*(main_w-overlay_w):$pip_y*(main_h-overlay_h)[out]"  \
        -map '[out]'                                                                                               \
        -pix_fmt yuv420p -aspect 16:9                                                                              \
        -c:v libx264 -b:v 8000k                                                                                    \
        -y "$mov_t" && mv "$mov_t" "$mov" || exit

      rm -f "$mjpeg_t" "$mjpeg"
    fi


  done
done

# vim:ts=2:sw=2:sts=2:et:ft=sh

