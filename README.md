# Generate frequency domain signatures from video

```shell
$ ./setup.sh && ./configure && make test
```

Requires: https://github.com/AndyA/jsondata

```shell
$ ffmpeg -i infile.mov -f yuv4mpegpipe - | downtown | \
    ffmpeg -f yuv4mpegpipe -i - -pix_fmt yuv420p -c:v libx264 -b:v 4000k out.mp4
```

Andy Armstrong, andy@hexten.net
