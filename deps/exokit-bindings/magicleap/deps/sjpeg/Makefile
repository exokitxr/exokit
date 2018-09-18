# This makefile is a simpler alternative to the CMake build system, for simple
# local building of the libraries and tools.
# It will not install the libraries system-wide, but just create the 'sjpeg',
# and 'vjpeg' tools in the examples/ directory, along with the static
# library 'src/libsjpeg.a'.
#
# To build the library and examples, use 'make' from this top directory.

#### Customizable part ####

VERSION=1.0
ARCHIVE_FILE=sjpeg-$(VERSION).tar.gz

EXTRA_FLAGS= -DSJPEG_HAVE_PNG -DSJPEG_HAVE_JPEG
UTILS_LIBS= -lpng -ljpeg

ifeq ($(strip $(shell uname)), Darwin)
  EXTRA_FLAGS += -I/opt/local/include
  EXTRA_FLAGS += -Wno-deprecated-declarations
  EXTRA_LIBS  += -L/opt/local/lib
  GL_LIBS = -framework GLUT -framework OpenGL
  EXTRA_FLAGS += -DHAVE_GLUT_GLUT_H
else
  EXTRA_FLAGS += -I/usr/local/include
  EXTRA_LIBS  += -L/usr/local/lib
  GL_LIBS = -lglut -lGL
  EXTRA_FLAGS += -DHAVE_GL_GLUT_H
endif

# Uncomment for build for 32bit platform
# Alternatively, you can just use the command
# 'make EXTRA_FLAGS=-m32' to that effect.
# EXTRA_FLAGS += -m32

# Extra strictness flags
EXTRA_FLAGS += -Wextra
EXTRA_FLAGS += -Wunused
EXTRA_FLAGS += -Wshadow
EXTRA_FLAGS += -Wformat-security -Wformat-nonliteral

# EXTRA_FLAGS += -Wvla

# SSE4.1-specific flags:
ifeq ($(HAVE_SSE41), 1)
EXTRA_FLAGS += -DSJPEG_HAVE_SSE41
src/%_sse41.o: EXTRA_FLAGS += -msse4.1
endif

# AVX2-specific flags:
ifeq ($(HAVE_AVX2), 1)
EXTRA_FLAGS += -DSJPEG_HAVE_AVX2
src/%_avx2.o: EXTRA_FLAGS += -mavx2
endif

# NEON-specific flags:
# EXTRA_FLAGS += -march=armv7-a -mfloat-abi=hard -mfpu=neon -mtune=cortex-a8
# -> seems to make the overall lib slower: -fno-split-wide-types

#### Nothing should normally be changed below this line ####

AR = ar
ARFLAGS = r
CXX = g++
CXXFLAGS = -Isrc/ -Wall $(EXTRA_FLAGS)
ifeq ($(DEBUG), 1)
  CXXFLAGS += -g
else
  CXXFLAGS += -O3 -DNDEBUG
endif
INSTALL = install
GROFF = /usr/bin/groff
COL = /usr/bin/col
LDFLAGS = $(EXTRA_LIBS) $(EXTRA_FLAGS) -lm

SJPEG_OBJS = \
    src/bit_writer.o \
    src/colors_rgb.o \
    src/dichotomy.o \
    src/enc.o \
    src/fdct.o \
    src/headers.o \
    src/jpeg_tools.o \
    src/score_7.o  \
    src/yuv_convert.o \

UTILS_OBJS = \
    examples/utils.o \

HDRS_INSTALLED = \
    src/sjpeg.h \

HDRS = \
    examples/utils.h \
    src/sjpegi.h \
    src/bit_writer.h \
    $(HDRS_INSTALLED) \

OUT_LIBS = src/libsjpeg.a examples/libutils.a
OUT_EXAMPLES =  examples/sjpeg
OUT_EXAMPLES += examples/vjpeg

OUTPUT = $(OUT_LIBS) $(OUT_EXAMPLES)

ex: $(OUT_EXAMPLES)
all: ex

%.o: %.cc $(HDRS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

examples/libutils.a: $(UTILS_OBJS)
src/libsjpeg.a: $(SJPEG_OBJS)

%.a:
	$(AR) $(ARFLAGS) $@ $^

examples/sjpeg: examples/sjpeg.o
examples/sjpeg: examples/libutils.a
examples/sjpeg: src/libsjpeg.a
examples/sjpeg: EXTRA_LIBS += $(UTILS_LIBS)

examples/vjpeg: examples/vjpeg.o
examples/vjpeg: examples/libutils.a
examples/vjpeg: src/libsjpeg.a
examples/vjpeg: EXTRA_LIBS += $(UTILS_LIBS)
# warning! -lpthread must be declared first, before GL_LIBS:
examples/vjpeg: EXTRA_LIBS += -lpthread $(GL_LIBS)
examples/vjpeg: EXTRA_FLAGS += -DSJPEG_HAVE_OPENGL

$(OUT_EXAMPLES):
	$(CXX) -o $@ $^ $(LDFLAGS)

test: test_cmd test_png_jpg

test_cmd: $(OUT_EXAMPLES)
	cd tests && ./test_cmd.sh

test_png_jpg: $(OUT_EXAMPLES)
	cd tests && ./test_png_jpg.sh

dist: DESTDIR := dist
dist: all
	$(INSTALL) -m755 -d $(DESTDIR)/include/ \
	           $(DESTDIR)/bin $(DESTDIR)/doc $(DESTDIR)/lib
	$(INSTALL) -m755 -s $(OUT_EXAMPLES) $(DESTDIR)/bin
	$(INSTALL) -m644 $(HDRS_INSTALLED) $(DESTDIR)/include
	$(INSTALL) -m644 src/libsjpeg.a $(DESTDIR)/lib
	umask 022; \
	for m in man/sjpeg.1 man/vjpeg.1; do \
	  basenam=$$(basename $$m .1); \
	  $(GROFF) -t -e -man -T utf8 $$m \
	    | $(COL) -bx >$(DESTDIR)/doc/$${basenam}.txt; \
	  $(GROFF) -t -e -man -T html $$m \
	    | $(COL) -bx >$(DESTDIR)/doc/$${basenam}.html; \
	done

clean:
	$(RM) $(OUTPUT) *~ \
              examples/*.o examples/*~ man/*~ src/*.o src/*~ tests/*~ \
              examples/*.lo src/*.lo

DIST_FILES= \
         Application.mk  \
         Android.mk  \
         AUTHORS  \
         ChangeLog  \
         COPYING  \
         INSTALL  \
         Makefile \
         NEWS  \
         README  \
         CMakeLists.txt \
         cmake/cpu.cmake \
         cmake/sjpegConfig.cmake.in \
         src/libsjpeg.pc.in \
         src/bit_writer.cc  \
         src/bit_writer.h  \
         src/colors_rgb.cc  \
         src/dichotomy.cc  \
         src/enc.cc  \
         src/fdct.cc  \
         src/headers.cc \
         src/jpeg_tools.cc  \
         src/score_7.cc  \
         src/sjpeg.h  \
         src/sjpegi.h  \
         src/yuv_convert.cc \
         man/sjpeg.1  \
         man/vjpeg.1  \
         examples/Android.mk  \
         examples/sjpeg.cc  \
         examples/vjpeg.cc  \
         examples/utils.h  \
         examples/utils.cc  \
         tests/test_cmd.sh  \
         tests/test_png_jpg.sh  \
         tests/testdata/source1.png \
         tests/testdata/source1.itl.png \
         tests/testdata/source2.jpg \
         tests/testdata/source3.jpg \
         tests/testdata/source4.ppm \
         tests/testdata/test_exif_xmp.png \
         tests/testdata/test_icc.jpg \

pak: clean
ifeq ($(strip $(shell uname)), Darwin)
	COPYFILE_DISABLE=1 tar -s ,.*,sjpeg-$(VERSION)\/~,p -czf $(ARCHIVE_FILE) $(DIST_FILES)
else
	tar --transform="s#^#sjpeg-1.0/#" -czf $(ARCHIVE_FILE) $(DIST_FILES)
endif
	@echo "GENERATED ARCHIVE: $(ARCHIVE_FILE)"

.PHONY: all clean dist ex pak
.SUFFIXES:
