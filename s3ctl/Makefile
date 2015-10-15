CFILES = s3ctl_drv.c

obj-m += s3ctl.o
s3ctl-objs := $(CFILES:.c=.o)

all:
	make -C $(KERNELDIR) M=$(PWD) modules
clean:
	make -C $(KERNELDIR) M=$(PWD) clean

