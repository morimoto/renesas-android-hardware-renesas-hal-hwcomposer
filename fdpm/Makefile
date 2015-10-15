CFILES = fdpm_main.c fdpm_sub.c fdpm_ioctl.c fdpm_ctrl.c fdpm_task.c manager/fdpm_seq.c manager/fdpm_resource.c
CFILES += fdp/fdp_drv.c fdp/fdp_drv_l.c fdp/fdp_drv_imgset.c

obj-m += fdpm.o
fdpm-objs := $(CFILES:.c=.o)

CP ?= cp

U_INCLUDE := -I$(PWD) -Iinclude/linux
U_INCLUDE += -D$(FDPM_CONFIG)
KERNELINC ?= $(KERNELDIR)/include/linux
ccflags-y := $(U_INCLUDE)
ccflags-y += -I$(KERNELINC)

all:
	$(CP) ./include/fdpm_drv.h $(KERNELINC)
	$(CP) fdpm_public.h $(KERNELINC)
	$(CP) fdpm_api.h $(KERNELINC)
	make -C $(KERNELDIR) M=$(PWD) KBUILD_EXTRA_SYMBOLS=$(PWD)/../mmngr/Module.symvers modules
clean:
	make -C $(KERNELDIR) M=$(PWD) clean
