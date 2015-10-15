# Makefile for composer driver

include $(src)/../config

# file list
rcar_composer-y := src/composer.o
rcar_composer-y += atrace/atrace.o

# build with Imgpctrl driver
ifeq ($(BUILD_WITH_IMGPCTRL_DRV),y)
obj-y += ../imgpctrl/
rcar_composer-y += ../imgpctrl/lib.a
endif

# build option.

ifneq ($(M),)
CONFIG_MISC_R_CAR_COMPOSER_DEVATTR=1
CONFIG_MISC_R_CAR_COMPOSER=m
CONFIG_MISC_R_CAR_COMPOSER_MAX_BLENDUNIT=$(CONFIG_MAX_BLENDUNIT)
CONFIG_MISC_R_CAR_COMPOSER_MAX_ROTBUFFER=$(CONFIG_MAX_ROTBUFFER)
ccflags-y += -DCONFIG_MISC_R_CAR_COMPOSER_DEVATTR=1
ccflags-y += -DCONFIG_MISC_R_CAR_COMPOSER=m
ccflags-y += -DCONFIG_MISC_R_CAR_COMPOSER_MAX_BLENDUNIT=$(CONFIG_MAX_BLENDUNIT)
ccflags-y += -DCONFIG_MISC_R_CAR_COMPOSER_MAX_ROTBUFFER=$(CONFIG_MAX_ROTBUFFER)
endif
ccflags-y += -Werror
#ccflags-y += -Wextra -Wno-unused-parameter
ccflags-y += -I$(src)/
ccflags-y += -Idrivers/staging/android/

# build
obj-$(CONFIG_MISC_R_CAR_COMPOSER) := rcar_composer.o
