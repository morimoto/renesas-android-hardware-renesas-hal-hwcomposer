# Copyright (C) 2016 The Android Open Source Project
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

# Include only for Renesas ones.
ifneq (,$(filter $(TARGET_PRODUCT), salvator ulcb kingfisher))

LOCAL_PATH := $(call my-dir)

# HAL module implementation stored in
# hw/<OVERLAY_HARDWARE_MODULE_ID>.<ro.product.board>.so
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)/hw
LOCAL_SHARED_LIBRARIES := libcutils libhardware libutils libhardware_legacy liblog libhidlbase
LOCAL_SHARED_LIBRARIES += android.hardware.graphics.mapper@3.0
LOCAL_SHARED_LIBRARIES += libhidltransport
LOCAL_SHARED_LIBRARIES += android.hardware.graphics.common@1.2
LOCAL_SHARED_LIBRARIES += libhwbinder
LOCAL_SHARED_LIBRARIES += android.hardware.graphics.composer@2.1
LOCAL_SHARED_LIBRARIES += android.hardware.graphics.composer@2.2
LOCAL_SHARED_LIBRARIES += android.hardware.graphics.composer@2.3
LOCAL_SHARED_LIBRARIES += vendor.renesas.graphics.composer@2.0
LOCAL_SHARED_LIBRARIES += libfmq
LOCAL_SHARED_LIBRARIES += libsync
LOCAL_SHARED_LIBRARIES += libion
LOCAL_SHARED_LIBRARIES += libdrm
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libbase
LOCAL_SHARED_LIBRARIES += libui

# source (base class)
#
LOCAL_C_INCLUDES += $(LOCAL_PATH)

# source (implement)
#
LOCAL_SRC_FILES += drm/DRMMode.cpp
LOCAL_SRC_FILES += drm/DRMPlane.cpp
LOCAL_SRC_FILES += drm/DRMProperty.cpp
LOCAL_SRC_FILES += ComposerClient.cpp
LOCAL_SRC_FILES += DrmDisplayComposition.cpp
LOCAL_SRC_FILES += Hwc.cpp
LOCAL_SRC_FILES += HwcBuffer.cpp
LOCAL_SRC_FILES += HwcDisplay.cpp
LOCAL_SRC_FILES += HwcHotPlug.cpp
LOCAL_SRC_FILES += HwcDump.cpp
LOCAL_SRC_FILES += HwcLayer.cpp
LOCAL_SRC_FILES += platformrcar.cpp
LOCAL_SRC_FILES += service.cpp
LOCAL_SRC_FILES += vsyncworker.cpp
LOCAL_SRC_FILES += worker.cpp
LOCAL_SRC_FILES += PrimeCache.cpp
LOCAL_SRC_FILES += ReadbackBuffer.cpp

# target
#
LOCAL_MODULE := android.hardware.graphics.composer@2.3-service.renesas
LOCAL_CFLAGS += -DLOG_TAG=\"hwcomposer\"
LOCAL_CFLAGS += -DHWC2_USE_CPP11
LOCAL_CFLAGS += -DHWC2_INCLUDE_STRINGIFICATION
LOCAL_CFLAGS += -DDEBUG_FRAMERATE=1
LOCAL_CFLAGS += -Wall -Werror
LOCAL_INIT_RC := android.hardware.graphics.composer@2.3-service.renesas.rc
LOCAL_VINTF_FRAGMENTS := android.hardware.graphics.composer@2.3-service.renesas.xml

ifeq ($(TARGET_ENABLE_HOTPLUG_SUPPORT),true)
    LOCAL_CFLAGS += -DHWC_HOTPLUG_SUPPORT=1
endif

ifeq ($(TARGET_DISABLE_PRIME_CACHE),true)
    LOCAL_CFLAGS += -DHWC_PRIME_CACHE=0
else
    LOCAL_CFLAGS += -DHWC_PRIME_CACHE=1
endif

ifeq ($(TARGET_DEVICE),salvator)
LOCAL_CFLAGS += -DTARGET_BOARD_SALVATOR
endif

ifeq ($(TARGET_DEVICE),ulcb)
LOCAL_CFLAGS += -DTARGET_BOARD_ULCB
endif

ifeq ($(TARGET_DEVICE),kingfisher)
LOCAL_CFLAGS += -DTARGET_BOARD_KINGFISHER
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
LOCAL_C_INCLUDES+= $(TOP)/system/core/libsync/
LOCAL_C_INCLUDES+= $(TOP)/system/core/base/include/
LOCAL_C_INCLUDES+= $(TOP)/system/libhidl/base/
LOCAL_C_INCLUDES+= $(TOP)/system/libhidl/base/include/
LOCAL_C_INCLUDES+= $(TOP)/system/core/base/include/
LOCAL_C_INCLUDES+= $(TOP)/system/core/base/
LOCAL_C_INCLUDES+= $(TOP)/system/libfmq/
LOCAL_C_INCLUDES+= $(TOP)/system/libfmq/include

LOCAL_MULTILIB := 64
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)

endif # Include only for Renesas ones.
