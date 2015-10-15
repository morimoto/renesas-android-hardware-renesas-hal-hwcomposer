LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES:= $(LOCAL_PATH)/../include

LOCAL_SRC_FILES := \
	mmngr_buf_if.c

#LOCAL_CFLAGS += -O3 -fstrict-aliasing -fprefetch-loop-arrays

LOCAL_MODULE:= libmmngrbuf

include $(BUILD_SHARED_LIBRARY)

