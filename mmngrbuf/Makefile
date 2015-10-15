CFILES = mmngr_buf_drv.c

obj-m += mmngrbuf.o
mmngrbuf-objs := $(CFILES:.c=.o)

CP ?= cp
KERNELINC ?= $(KERNELDIR)/include/linux
EXTRA_CFLAGS += -I$(KERNELINC)

all:
	$(CP) ./include/mmngr_buf_private.h $(KERNELINC)
	make -C $(KERNELDIR) M=$(PWD) modules
clean:
	make -C $(KERNELDIR) M=$(PWD) clean

