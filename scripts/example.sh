#!/bin/bash

ffmpeg -i ~/Desktop/d3/mpeg/19631207-000000-dr-who.mpeg -pix_fmt yuv420p -s 256x256 -aspect 1:1 -t 120 -f yuv4mpegpipe - | ./downtown | ffmpeg -f yuv4mpegpipe -i - -c:v libx264 -b:v 8000k -y ~/Desktop/Downtown.mov

# vim:ts=2:sw=2:sts=2:et:ft=sh
