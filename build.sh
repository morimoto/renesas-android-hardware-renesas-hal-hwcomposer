
export ARCH=arm
export CROSS_COMPILE=arm-eabi-
export MMNGR_CONFIG=MMNGR_LAGER
export MMNGRBUF_CONFIG=MMNGRBUF_LAGER
export VSPM_CONFIG=H2CONFIG
export FDPM_CONFIG=H2CONFIG

CONFIG_MACH="CONFIG_MACH_LAGER=y"
CONFIG_ARCH="CONFIG_ARCH_R8A7790=y"
MACH_GREP_COUNT=20
ARCH_GREP_COUNT=22

ls ${KERNELDIR} > /dev/null || exit 1

pushd ${ANDROID_ROOT}/hardware/renesas/mmp_km

cd mmngr && make clean && make && cd .. || exit 1
cd mmngrbuf && make clean && make && cd .. || exit 1
cd s3ctl && make clean && make && cd .. || exit 1
cd vspm && make clean && make && cd .. || exit 1
cd composer && make clean && make && cd .. || exit 1
cd fdpm && make clean && make && cd .. || exit 1
cd uvcs/source/makefile/ndk_4_6 && make clean && make && cd ../../../../ || exit 1

popd
