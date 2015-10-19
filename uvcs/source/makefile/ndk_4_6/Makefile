UVCS_DRV_SRC_DIR := ../../uvcs_lkm
UVCS_CMN_SRC_DIR := ../../uvcs_cmn
UVCS_CMN_INC_DIR := ../../../include
DRV_CORE_SRC_DIR := ../../driver_core

obj-m := uvcs_cmn.o
uvcs_cmn-y := uvcs_lkm.o
uvcs_cmn-y += uvcs_lkm_uf_semaphore.o
uvcs_cmn-y += uvcs_lkm_uf_thread.o
uvcs_cmn-y += uvcs_lkm_uf_other.o
uvcs_cmn-y += uvcs_lkm_uf_io.o
uvcs_cmn-y += uvcs_cmn_api.o
uvcs_cmn-y += uvcs_cmn_dump.o
uvcs_cmn-y += mcvx_api.o

#EXTRA_CFLAGS := -DUVCS_DEBUG=1
#EXTRA_CFLAGS += -mno-unaligned-access

.PHONY: all
all:
	@cp -p $(UVCS_DRV_SRC_DIR)/*.c .
	@cp -p $(UVCS_DRV_SRC_DIR)/*.h .
	@cp -p $(UVCS_CMN_SRC_DIR)/*.c .
	@cp -p $(UVCS_CMN_SRC_DIR)/*.h .
	@cp -p $(UVCS_CMN_INC_DIR)/*.h .
	@cp -p $(DRV_CORE_SRC_DIR)/*.c .
	@cp -p $(DRV_CORE_SRC_DIR)/*.h .
	@make --no-print-directory CFLAGS_MODULE=-fno-pic -C $(KERNELDIR) M=$(PWD) modules
	@rm -f *.c
	@rm -f *.h

clean:
	@make --no-print-directory -C $(KERNELDIR) M=$(PWD) clean
