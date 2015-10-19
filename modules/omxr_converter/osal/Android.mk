LOCAL_PATH:= $(call my-dir)

OMX_TOP := $(LOCAL_PATH)/../../..

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SHARED_LIBRARIES := libutils libcutils libomxr_utility
ifeq ($(TARGET_OS)-$(TARGET_SIMULATOR),linux-true)
LOCAL_LDLIBS += -ldl
endif
ifneq ($(TARGET_SIMULATOR),true)
LOCAL_SHARED_LIBRARIES += libdl
endif

LOCAL_SRC_FILES := \
    cnv_osdep.c

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
	$(OMX_TOP)/modules/omxr_converter/include \
	$(OMX_TOP)/include \
	$(OMX_TOP)/modules/omxr_utility/include \
	$(TOP)/hardware/renesas/omx/OMXR/include

LOCAL_CFLAGS := -DOMXR_BUILD_OS_LINUX

LOCAL_MODULE:= libomxr_cnvosdep
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)
