# Multimedia log output level INFO=0 LOW=1 MED=2 HIGH=3 ERROR=4 FATAL=5 NONE=255
MLEVEL=4

# VSPM Job priority 1-126( from 1:LOW to 126:High)
VPRI=63

# Extra Debug Buffer Number N (N : 4096 * N)
EXTRA_BUFF_NUM=0

# Debug TraceID (None:0, ErrorLog:1, TraceLog1:2, Hdl Info:3, TraceLog2:4)
TRACE_ID=0

CFILES = imgpctrl_main.c imgpctrl_common.c imgpctrl_api.c
CFILES += ctrl/imgpctrl_ctrl_main.c ctrl/imgpctrl_ctrl_tdd.c ctrl/imgpctrl_ctrl_vsp.c ctrl/imgpctrl_ctrl_vsp_para_gen.c ctrl/imgpctrl_ctrl_dma.c

include $(src)/../config

ifeq ($(BUILD_WITH_IMGPCTRL_DRV),y)
# create library for composer module
lib-m += $(CFILES:.c=.o)

else
obj-m += imgpctrl_drv.o
imgpctrl_drv-objs := $(CFILES:.c=.o) imgpctrl_module.o
endif

VSPM_INCLUDE=$(src)/../../mmngr/include
MMNG_INCLUDE=$(src)/../../vspm/include

U_INCLUDE := -I$(src)
U_INCLUDE += -I$(src)/../include/imgpctrl
U_INCLUDE += -I$(VSPM_INCLUDE)
U_INCLUDE += -I$(MMNG_INCLUDE)
ccflags-y += $(U_INCLUDE) -DMSG_LEVEL=$(MLEVEL) -DIMGPCTRL_JOB_PRIO=$(VPRI)
ccflags-y += -DRT_GRAPHICS_EXTRA_BUFF_NUM=$(EXTRA_BUFF_NUM)
ccflags-y += -DRT_GRAPHICS_TRACE_ID=$(TRACE_ID)

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
