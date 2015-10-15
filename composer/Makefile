# Makefile for composer/imgpctrl driver

ifneq ($(KERNELRELEASE),)
# kbuild part of makefile

# common build option
subdir-ccflags-y += -Wall
subdir-ccflags-y += -I$(src)/include/

# top level modules
obj-y += composer/ imgpctrl/

else
# normal makefile

MODULE_SYMVERS_FILES = "$(PWD)/../vspm/Module.symvers $(PWD)/../mmngr/Module.symvers"

default:
	make -C $(KERNELDIR) M=$(PWD) KBUILD_EXTRA_SYMBOLS=$(MODULE_SYMVERS_FILES)
	make -C $(KERNELDIR) M=$(PWD) KBUILD_EXTRA_SYMBOLS=$(MODULE_SYMVERS_FILES) modules


clean:
	make -C $(KERNELDIR) M=$(PWD) clean

endif
