LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../include

LOCAL_SRC_FILES := fdpm_api.c fdpm_api_sub.c fdpm_api_fd.c fdpm_api_timer.c

LOCAL_SHARED_LIBRARIES := libutils libcutils

LOCAL_MODULE:= libfdpm

include $(BUILD_SHARED_LIBRARY)

