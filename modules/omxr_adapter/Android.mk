LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SHARED_LIBRARIES := libutils libcutils libhardware
ifeq ($(TARGET_OS)-$(TARGET_SIMULATOR),linux-true)
LOCAL_LDLIBS += -ldl
endif
ifneq ($(TARGET_SIMULATOR),true)
LOCAL_SHARED_LIBRARIES += libdl
endif

LOCAL_SRC_FILES := \
	omxr_adapter_core.c \
	omxr_adapter_log.c \
	omxr_adapter_base.c \
	omxr_adapter_video_decoder.c

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
	$(TOP)/frameworks/native/include/media/openmax \
	$(TOP)/frameworks/native/include/media/hardware \
	$(LOCAL_PATH)/include \
	$(TOP)/hardware/renesas/hwcomposer
	

LOCAL_CFLAGS := -DMSG_LEVEL=$(UREL_MEG_LEV)

LOCAL_CFLAGS += -DANDROID_USE_METADATA_IN_BUFFER
LOCAL_CFLAGS += -DOMXR_ADAPTER_BUILD_LOGGER_ENABLE
LOCAL_CFLAGS += -DOMXR_ADAPTER_BUILD_ENABLE_VIDEO_DECODER
#LOCAL_CFLAGS += -DOMXR_ADAPTER_BUILD_ENABLE_VIDEO_ENCODER

LOCAL_MODULE:= libomxr_adapter
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)
