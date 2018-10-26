LOCAL_PATH:= $(call my-dir)

SJPEG_CFLAGS := -Wall -DANDROID -DHAVE_MALLOC_H -DHAVE_PTHREAD
SJPEG_CFLAGS += -fvisibility=hidden

ifeq ($(APP_OPTIM),release)
  SJPEG_CFLAGS += -finline-functions -ffast-math \
                  -ffunction-sections -fdata-sections
  ifeq ($(findstring clang,$(NDK_TOOLCHAIN_VERSION)),)
    SJPEG_CFLAGS += -frename-registers -s
  endif
endif

ifneq ($(findstring armeabi-v7a, $(TARGET_ARCH_ABI)),)
  # Setting LOCAL_ARM_NEON will enable -mfpu=neon which may cause illegal
  # instructions to be generated for armv7a code. Instead target the neon code
  # specifically.
  NEON := cc.neon
  USE_CPUFEATURES := yes
else
  NEON := cc
endif

enc_srcs := \
        src/bit_writer.cc \
	src/colors_rgb.$(NEON) \
	src/enc.$(NEON) \
	src/fdct.$(NEON) \
	src/headers.cc \
	src/dichotomy.cc \
	src/jpeg_tools.cc \
	src/yuv_convert.$(NEON) \
	src/score_7.cc \

################################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    $(enc_srcs) \

LOCAL_CFLAGS := $(SJPEG_CFLAGS)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src

# prefer arm over thumb mode for performance gains
LOCAL_ARM_MODE := arm

LOCAL_MODULE := sjpeg_static

include $(BUILD_STATIC_LIBRARY)

ifeq ($(ENABLE_SHARED),1)
  include $(CLEAR_VARS)
  LOCAL_WHOLE_STATIC_LIBRARIES := sjpeg_static
  LOCAL_MODULE := sjpeg

  include $(BUILD_SHARED_LIBRARY)
endif

################################################################################

include $(LOCAL_PATH)/examples/Android.mk

$(call import-module,android/cpufeatures)
