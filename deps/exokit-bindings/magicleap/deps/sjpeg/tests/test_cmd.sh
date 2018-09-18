#!/bin/sh
# tests for command lines

SJPEG=../examples/sjpeg
TMP_FILE1=/tmp/test.jpg
TMP_FILE2=/tmp/test
BAD_FILE=/tmp/
SRC_FILE1="./testdata/source1.png"
SRC_FILE2="./testdata/source2.jpg"
SRC_FILE3="./testdata/source4.ppm"

# simple coverage of command line arguments. Positive tests.
echo "POSITIVE TESTS"
set -e
${SJPEG} -version
${SJPEG} -h
${SJPEG} --help

${SJPEG} ${SRC_FILE1} -o ${TMP_FILE1} -yuv_mode 2
${SJPEG} ${SRC_FILE1} -o ${TMP_FILE1} -q 3 -no_adapt -no_optim -quiet

${SJPEG} ${SRC_FILE1} -o ${TMP_FILE1} -r 30 -no_adapt -no_optim -quiet
${SJPEG} ${SRC_FILE2} -o ${TMP_FILE1} -r 30 -no_adapt -no_optim -quiet
${SJPEG} ${SRC_FILE3} -o ${TMP_FILE1} -r 30 -no_adapt -no_optim -quiet
${SJPEG} ${SRC_FILE1} -crc

# negative tests (should fail)
echo "NEGATIVE TESTS"
set +e
${SJPEG} -no_adapt -no_optim -quiet
${SJPEG} ${SRC_FILE1} -o ${TMP_FILE1} -yuv_mode -1 -quiet
${SJPEG} ${SRC_FILE1} -o ${TMP_FILE1} -yuv_mode 99 -quiet -no_metadata
${SJPEG} -q 80 -quiet
${SJPEG} ${SRC_FILE1} -risk -quiet
${SJPEG} ${SRC_FILE1} -o
${SJPEG} -o ${TMP_FILE2} -quiet
${SJPEG} ${SRC_FILE1} -o ${TMP_FILE1} -yuv_mode -1 -quiet
${SJPEG} ${SRC_FILE1} -o ${TMP_FILE1} -yuv_mode 99 -quiet
${SJPEG} ${BAD_FILE} -o ${TMP_FILE1} -quiet
${SJPEG} ${SRC_FILE1} -o ${BAD_FILE} -quiet

${SJPEG} ${SRC_FILE2} -o
${SJPEG} -o ${BAD_FILE} -quiet

# this test does not work for very low quality values (q<4)
for q in `seq 4 100`; do
  ${SJPEG} -q $q ${SRC_FILE1} -o ${TMP_FILE1} -no_adapt -no_optim &> /dev/null
  # parse the 'estimated quality' result string, and compare to expected quality
  a=(`${SJPEG} -i ${TMP_FILE1} | grep estimated | grep -Eo '[+-]?[0-9]+(\.0)'`)
  q1="${a[0]}"
  q2="${a[1]}"
  q3="${q}.0"
  if [ "x${q1}" != "x${q3}" ]; then echo "Y-Quality mismatch!"; exit 1; fi
  if [ "x${q2}" != "x${q3}" ]; then echo "UV-Quality mismatch!"; exit 1; fi
done

echo "OK!"
exit 0
