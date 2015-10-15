CFILES = vspm_main.c vspm_ioctl.c vspm_sub.c
CFILES += frame.c
CFILES += manager/vspm_control.c manager/vspm_drv_2ddmac.c manager/vspm_drv_vsp.c manager/vspm_exec_manager.c
CFILES += manager/vspm_job_manager.c manager/vspm_lib.c manager/vspm_sort_queue.c manager/vspm_task.c
CFILES += tddmac/tddmac_drv.c tddmac/tddmac_drv_table.c
CFILES += vsp/vsp_drv.c vsp/vsp_drv_phy.c vsp/vsp_drv_par.c

obj-m += vspm.o
vspm-objs := $(CFILES:.c=.o)

CP ?= cp
KERNELINC ?= $(KERNELDIR)/include/linux
U_INCLUDE := -I$(PWD)
U_INCLUDE += -I$(KERNELINC)
EXTRA_CFLAGS += $(U_INCLUDE)

ifeq ($(VSPM_CONFIG), H2CONFIG)
 EXTRA_CFLAGS += -DUSE_VSPS_UDS
 EXTRA_CFLAGS += -DUSE_VSPR
endif
#EXTRA_CFLAGS += -DUSE_VSPD0
ifneq ($(VSPM_CONFIG), E2CONFIG)
#EXTRA_CFLAGS += -DUSE_VSPD1
endif

all:
	$(CP) ./include/vspm_public.h $(KERNELINC)
	$(CP) ./include/vsp_drv.h $(KERNELINC)
	$(CP) ./include/tddmac_drv.h $(KERNELINC)
	$(CP) ./include/vspm_if.h $(KERNELINC)
	make -C $(KERNELDIR) M=$(PWD) modules
clean:
	make -C $(KERNELDIR) M=$(PWD) clean

