LOCAL_PATH:= $(call my-dir)

OMX_TOP := $(LOCAL_PATH)/../..

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SHARED_LIBRARIES := libutils libcutils libmmngr
ifeq ($(TARGET_OS)-$(TARGET_SIMULATOR),linux-true)
LOCAL_LDLIBS += -ldl
endif
ifneq ($(TARGET_SIMULATOR),true)
LOCAL_SHARED_LIBRARIES += libdl
endif

LOCAL_SRC_FILES := \
	config/omxr_utility_config.c \
	hw_dep/omxr_debug_func.c \
	hw_dep/omxr_dll_loader.c \
	hw_dep/omxr_file_loader.c \
	os_dep/omxr_mem_osdep_func.c \
	os_dep/omxr_os_wrapper.c \
	os_dep/omxr_string_osdep_func.c \
	os_dep/omxr_workbuffer_func.c

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := \
	$(TOP)/frameworks/native/include/media/openmax \
	$(OMX_TOP)/include \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/os_dep \
	$(LOCAL_PATH)/hw_dep \
	$(TOP)/hardware/renesas/omx/OMXR/include \
	$(TOP)/hardware/renesas/mmp/include

LOCAL_CFLAGS := -DOMXR_BUILD_OS_LINUX -DOMXR_BUILD_LINUX_NON_ROOT -DOMXR_BUILD_USE_MEMORY_MANAGER
LOCAL_CFLAGS += -DOMXR_DEFAULT_CONFIG_FILE_NAME=\"/system/etc/omxr_config_base.txt\"

LOCAL_MODULE:= libomxr_utility
LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)
