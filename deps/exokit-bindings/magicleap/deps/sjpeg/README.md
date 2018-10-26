                     ____ __   __ __  ____ ____  ____
                    /  __\__) /  \  \/  _ \   / /  _ \
                   _\_   \  (/      /   __/  /_/   __/
                   \_____/___/__//_/\__/_/_____/_____/
                          (__)_/  _ \/  _ \/ .__\
                          _)  \   __/   __/  /  \
                         /____/__/  \_____/_____/v1.0

* sjpeg: 's' stands for 'simple'.

* sjpeg is a simple encoding library for encoding baseline JPEG files.
  It's also a command-line tool that takes PNG or JPEG images as input
  to produce JPEG files.

* Why 'simple'? Well, the encoder used to be one single source file. And
  not a large one, moreover. Over time it grew and was split, for readability,
  but the spirit remains the same.

# The sjpeg library:

The main header is src/sjpeg.h.
This is mainly just one function to call, defined in src/sjpeg.h

```
size_t SjpegCompress(const uint8_t* rgb, int width, int height,
                     int quality, uint8_t** out_data);
```

This function will make a lot of automatic decisions based on input samples.

There is a more fine-tuned variant available too:

```
size_t SjpegEncode(const uint8_t* const data,
                   int W, int H, int stride,
                   uint8_t** const out_data,
                   int quality, int compression_method,
                   int yuv_mode);
```

as well as functions with a C++ string-based API.

The library comes primarily with a plain-C API, but a C++ version is available
too, that uses `std::string` as interface.

`SjpegFreeBuffer()` can be called release the buffers returned by `SjpegEncode()`
and `SjpegCompress()`.

Also included in the library: some helper functions to inspect JPEG bitstream.
They are meant to help re-processing a JPEG source:
  `SjpegDimensions()`: quickly get the JPEG picture dimensions.
  `SjpegFindQuantizer()`: return the quantization matrices
  `SjpegEstimateQuality()`: return an estimate of the encoding quality

## SjpegEncodeParam interface:

This should be the most fine-tuned use of the library: SjpegEncodeParam gives
access to a lot of fine controls (C++ only). See `src/sjpeg.h` for details.

In particular, using SjpegEncodeParam is the recommended way of re-compressing
a JPEG input. Here is the recipe:

* use `SjpegFindQuantizer` to extract the quantization matrices from the source
* use `SjpegEncodeParam::SetQuantMatrix()` to transfer them to the encoding parameters (with a reduction factor if necessary)
* call `sjpeg::SjpegEncode()` with this parameter

Alternately, one can use `SjpegEstimateQuality()` and
`SjpegEncodeParam::SetQuality()` to pass a quality factor instead of the matrices.

`SjpegEncodeParam` is also useful for transfering some metadata to the final
JPEG bitstream.

## 'Riskiness' score:

The riskiness score is a decision score to help decide between using YUV 4:2:0
and YUV 4:4:4 downsampling. The function to call is:

```
 int SjpegRiskiness(const uint8_t* rgb, int width, int height, int step, float* risk);
```

which returns a recommendation for 'yuv_mode' to use in the above API.

## The 'sjpeg' command line tool:

`examples/sjpeg` is a very simple tool to compress a PNG/JPEG file to JPEG.

`sjpeg in.png -o out.jpg -q 70     # compress the PNG source with quality 70`

`sjpeg in.jpg -o out.jpg -r 90     # recompress JPEG input to ~90% `

sjpeg has various options to change the encoding method. Just try `sjpeg -h`.

## The 'vjpeg' command line tool:

![vjpeg UI](https://github.com/webmproject/sjpeg/blob/master/examples/vjpeg_sample.jpg "the vjpeg interface")

examples/vjpeg is a (non-essential) visualization tool, built with OpenGL/GLUT.
It allows changing the compression parameters with the keyboard, and visually
inspect the result. This is mostly educational, since most of the parameters
from `SjpegEncodeParam` can be triggered from the keyboard.

# Building the library and tools:

Please read the instructions in the INSTALL file.

Basically, you can use the supplied Makefile or generate your build with
cmake:

* `make -j`, or
* `mkdir build && cd build && cmake ../ && make -j`

# Discussion list

The following forum can be used to report bugs or just discuss sjpeg:

[https://groups.google.com/forum/#!forum/sjpeg]([https://groups.google.com/forum/#!forum/sjpeg)
