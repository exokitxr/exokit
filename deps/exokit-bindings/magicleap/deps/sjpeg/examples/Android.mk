LOCAL_PATH := $(call my-dir)

################################################################################
# utils library

include $(CLEAR_VARS)

LOCAL_SRC_FILES := utils.cc

LOCAL_CFLAGS := $(SJPEG_CFLAGS)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../src

LOCAL_MODULE := utils_static

include $(BUILD_STATIC_LIBRARY)

ifeq ($(ENABLE_SHARED),1)
  include $(CLEAR_VARS)
  LOCAL_WHOLE_STATIC_LIBRARIES := utils_static
  LOCAL_MODULE := utils

  include $(BUILD_SHARED_LIBRARY)
endif

################################################################################
# sjpeg

include $(CLEAR_VARS)

LOCAL_SRC_FILES := sjpeg.cc

LOCAL_CFLAGS := $(SJPEG_CFLAGS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../src
LOCAL_STATIC_LIBRARIES := sjpeg_static utils_static

LOCAL_MODULE := sjpeg

include $(BUILD_EXECUTABLE)
