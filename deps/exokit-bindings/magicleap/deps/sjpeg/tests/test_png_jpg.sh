#!/bin/sh
# tests for JPEG/PNG/PPM -> JPEG -> JPEG chain

SJPEG=../examples/sjpeg
TMP_JPEG1=/tmp/test1.jpg
TMP_JPEG2=/tmp/test2.jpg

LIST="source1.png \
      source1.itl.png \
      source2.jpg \
      source3.jpg \
      source4.ppm \
      test_icc.jpg \
      test_exif_xmp.png"

set -e
for f in ${LIST}; do
  ${SJPEG} testdata/${f} -o ${TMP_JPEG2} -info -q 56.7
done

for f in ${LIST}; do
  ${SJPEG} testdata/${f} -o ${TMP_JPEG1} -quiet -psnr 39
  ${SJPEG} ${TMP_JPEG1} -o ${TMP_JPEG2} -r 88.7 -short -info -size 20000
done

for f in ${LIST}; do
  ${SJPEG} testdata/${f} -o ${TMP_JPEG1} -quiet -no_metadata
  ${SJPEG} ${TMP_JPEG1} -r 76.6542 -o ${TMP_JPEG2} -short
done

echo "OK!"
exit 0
