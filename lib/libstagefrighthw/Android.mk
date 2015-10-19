#ifeq ($(TARGET_BOARD_PLATFORM),r8a7790)
#ifeq ($(TARGET_BOARD_PLATFORM),r8a7791)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    RenesasOMXPlugin.cpp

LOCAL_C_INCLUDES:= \
        $(TOP)/frameworks/native/include/media \
        $(TOP)/frameworks/native/include/media/openmax \
        $(TOP)/frameworks/native/include/media/hardware

LOCAL_SHARED_LIBRARIES :=       \
        libbinder               \
        libutils                \
        libcutils               \
        libui                   \
        libdl                   \

LOCAL_CFLAGS += -Werror

LOCAL_MODULE := libstagefrighthw

include $(BUILD_SHARED_LIBRARY)

#endif
