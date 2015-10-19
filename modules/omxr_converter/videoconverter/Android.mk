LOCAL_PATH:= $(call my-dir)

OMX_TOP := $(LOCAL_PATH)/../../..

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SHARED_LIBRARIES := libomxr_cnvosdep 
ifeq ($(TARGET_OS)-$(TARGET_SIMULATOR),linux-true)
LOCAL_LDLIBS += -ldl
endif
ifneq ($(TARGET_SIMULATOR),true)
LOCAL_SHARED_LIBRARIES += libdl
endif

LOCAL_SRC_FILES := \
    cnv_config.c \
    cnv_interface.c 

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
	$(OMX_TOP)/modules/omxr_converter/include \
	$(OMX_TOP)/modules/omxr_converter/osal
	
LOCAL_CFLAGS := -DOMXR_BUILD_OS_LINUX -DCNV_BUILD_FDPM_ENABLE

LOCAL_MODULE:= libomxr_videoconverter
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)
