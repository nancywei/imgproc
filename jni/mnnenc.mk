# Generate the libwelsencdemo.so file
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := mnnenc

ifneq (,$(wildcard $(LOCAL_PATH)/$(LOCAL_SRC_FILES)))
include $(PREBUILT_STATIC_LIBRARY) 
endif 


include $(CLEAR_VARS)

#
# Module Settings
#
LOCAL_MODULE := mnnenc

#
# Source Files
#
LOCAL_SRC_FILES := \
            segment.cpp

#
# Header Includes
#
LOCAL_C_INCLUDES := \
            $(LOCAL_PATH)/include \
            $(LOCAL_PATH)/include/MNN/ \


#
# Compile Flags and Link Libraries
#
LOCAL_CFLAGS += -DANDROID_NDK  -fPIE -pie
LOCAL_LDFLAGS += -fPIE -pie
#LOCAL_LDLIBS := -llog
LOCAL_STATIC_LIBRARIES := mnnenc

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_EXECUTABLE)
include $(CLEAR_VARS)
