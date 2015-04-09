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
        && find "$obj" -type f -name '*.dat'  \
        || echo "$obj"
    } | while read dat; do

    eval $( tools/decode-dat.pl "$dat" )

    mpeg="$dat_mpeg"
    profile="profiles/$dat_prof"
    m4v="$dat_m4v"

    if [ -e "$profile" ]; then

      m4v_t="$m4v.tmp.m4v"
      mjpeg="$m4v.mjpeg"
      mjpeg_t="$m4v.tmp.mjpeg"

      if [ "$dat" -nt "$m4v" ]; then

        echo "$dat -> $m4v (profile = $profile)"

        $moviemaker $mm_extra "$dat" "$profile" "$mjpeg_t" && mv "$mjpeg_t" "$mjpeg" || exit

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

        rm -f "$mjpeg_t" "$mjpeg"
      fi

    fi

  done
done

# vim:ts=2:sw=2:sts=2:et:ft=sh

