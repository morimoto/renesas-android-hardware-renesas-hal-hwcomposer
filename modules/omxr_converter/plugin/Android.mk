LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

CONVERTER_PLUGIN_TOP := $(LOCAL_PATH)

include $(CONVERTER_PLUGIN_TOP)/fdp/Android.mk
#include $(CONVERTER_PLUGIN_TOP)/vsp/Android.mk

