# ESExtractor

Elementary stream frame extractor:
The purpose of this library is to parse streams and extract frames with a simple API inspired from [FFmpeg](https://ffmpeg.org) API
[av_read_frame](https://ffmpeg4d.dpldocs.info/ffmpeg.libavformat.avformat.av_read_frame.html).

It supports:
  - NAL based streams which can be [NAL](https://en.wikipedia.org/wiki/Network_Abstraction_Layer) or [AUs](https://en.wikipedia.org/wiki/Network_Abstraction_Layer#Access_Units) aligned
  - IVF based streams

## Setup

### Ubuntu/Debian/etc

```sh
$ apt install python3-pip ninja-build pkg-config
$ pip3 install --user meson
```


## Build

### Windows

1. Open a Developer Command Prompt for VS201x
2. Go to root of the cloned git repository

### Common

```sh
$ meson builddir
$ ninja -C builddir
$ meson test -C builddir
```

### Test with a sample

```
$ ./builddir/tests/testesextractor -d -d -f samples/clip-a.h264
```