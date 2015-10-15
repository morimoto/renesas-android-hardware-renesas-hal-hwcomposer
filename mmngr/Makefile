CFILES = mmngr_drv.c

obj-m += mmngr.o
mmngr-objs := $(CFILES:.c=.o)

CP ?= cp
KERNELINC ?= $(KERNELDIR)/include/linux
EXTRA_CFLAGS += -D$(MMNGR_CONFIG)
EXTRA_CFLAGS += -I$(KERNELINC) -Iinclude/linux

all:
	$(CP) ./include/mmngr_public.h $(KERNELINC)
	make -C $(KERNELDIR) M=$(PWD) modules
clean:
	make -C $(KERNELDIR) M=$(PWD) clean

