LOCAL_PATH:= $(call my-dir)

OMX_TOP := $(LOCAL_PATH)/../../../..

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SHARED_LIBRARIES := libomxr_cnvosdep libfdpm libs3ctl
ifeq ($(TARGET_OS)-$(TARGET_SIMULATOR),linux-true)
LOCAL_LDLIBS += -ldl
endif
ifneq ($(TARGET_SIMULATOR),true)
LOCAL_SHARED_LIBRARIES += libdl
endif

LOCAL_SRC_FILES := \
    ../cmn/cnvp_cmn.c \
    cnvp_fdp_core.c \
    cnvp_fdp_interface.c

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
	$(OMX_TOP)/modules/omxr_converter/include \
	$(OMX_TOP)/modules/omxr_converter/plugin/cmn \
	$(OMX_TOP)/modules/omxr_converter/osal \
	$(TOP)/hardware/renesas/mmp/include \
	$(TOP)/hardware/renesas/mmp/fdpm \

LOCAL_CFLAGS := -DOMXR_BUILD_OS_LINUX

LOCAL_MODULE:= libomxr_cnvpfdp
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)
