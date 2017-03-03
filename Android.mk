# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


LOCAL_PATH := $(call my-dir)

# HAL module implementation stored in
# hw/<OVERLAY_HARDWARE_MODULE_ID>.<ro.product.board>.so
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := libcutils libhardware libutils libhardware_legacy liblog
LOCAL_SHARED_LIBRARIES += libsync
LOCAL_SHARED_LIBRARIES += libion
LOCAL_SHARED_LIBRARIES += libdrm

# source (base class)
#
LOCAL_SRC_FILES += base/disp_base.cpp
LOCAL_SRC_FILES += base/hotplug_base.cpp
LOCAL_SRC_FILES += base/hwc_base.cpp
LOCAL_SRC_FILES += base/hwc_thread.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)

# source (implement)
#
LOCAL_SRC_FILES += base/hwc_notice.cpp
LOCAL_SRC_FILES += component/fencemerge.cpp
LOCAL_SRC_FILES += component/composer.cpp
LOCAL_SRC_FILES += component/layersel.cpp
LOCAL_SRC_FILES += component/drm_display.cpp
LOCAL_SRC_FILES += component/hwcglobal.cpp
LOCAL_SRC_FILES += component/SyncTimeline.cpp

LOCAL_SRC_FILES += displays/hwc_primary.cpp
LOCAL_SRC_FILES += displays/hwc_external.cpp
LOCAL_SRC_FILES += displays/hwc_virtual.cpp

LOCAL_SRC_FILES += hwcomposer.cpp


# target
#
LOCAL_MODULE := hwcomposer.$(TARGET_BOARD_PLATFORM)
LOCAL_CFLAGS += -DLOG_TAG=\"hwcomposer\"
# LOCAL_CFLAGS += -Wall -Werror

ifeq ($(TARGET_BOARD_PLATFORM),r8a7790)
LOCAL_CFLAGS += -DTARGET_BOARD_LAGER
endif

ifeq ($(TARGET_DEVICE),salvator)
LOCAL_CFLAGS += -DTARGET_BOARD_SALVATOR
endif

ifeq ($(TARGET_BOARD_PLATFORM),r8a7795)
LOCAL_CFLAGS += -DTARGET_BOARD_PLATFORM_R8A7795
endif

ifeq ($(TARGET_BOARD_PLATFORM),r8a7796)
LOCAL_CFLAGS += -DTARGET_BOARD_PLATFORM_R8A7796
endif

ifeq ($(TARGET_BOARD_PLATFORM),r8a7791)
LOCAL_CFLAGS += -DTARGET_BOARD_KOELSCH
endif

ifeq ($(TARGET_BOARD_PLATFORM),r8a7794)
LOCAL_CFLAGS += -DTARGET_BOARD_ALT
endif

# add flag to distinguish a target.
#
#LOCAL_C_INCLUDES+= $(SOLUTION_VENDOR_PATH)/include
LOCAL_C_INCLUDES+= $(TOP)/vendor/renesas/include
LOCAL_C_INCLUDES+= $(TOP)/vendor/renesas/$(TARGET_BOARD_PLATFORM)/include
LOCAL_C_INCLUDES+= $(TARGET_OUT_HEADERS)/libdrm/
LOCAL_C_INCLUDES+= $(TOP)/system/core/libsync/

# TARGET_FORCE_HWC_FOR_VIRTUAL_DISPLAYS
#   set build option for copy HWC_FRAMEBUFFER_TARGET to output buffer.
ifeq ($(TARGET_FORCE_HWC_FOR_VIRTUAL_DISPLAYS),true)
    LOCAL_CFLAGS += -DFORCE_HWC_COPY_FOR_VIRTUAL_DISPLAYS
endif

# USE_EXTERNAL_DISPLAY
#   comment out next statement is necessary,
#   if support external display.
LOCAL_CFLAGS += -DUSE_EXTERNAL_DISPLAY

LOCAL_MULTILIB := 64

LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)
