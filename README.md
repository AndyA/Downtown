# Generate frequency domain signatures from video

```shell
$ ./setup.sh && ./configure && make test
```

Requires: https://github.com/AndyA/jsondata

```shell
$ ffmpeg -i infile.mov -f yuv4mpegpipe - | downtown config.json 
```

Andy Armstrong, andy@hexten.net
