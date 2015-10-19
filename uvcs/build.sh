
export ARCH=arm
export CROSS_COMPILE=arm-eabi-

CONFIG_MACH="CONFIG_MACH_LAGER=y"
CONFIG_ARCH="CONFIG_ARCH_R8A7790=y"
MACH_GREP_COUNT=20
ARCH_GREP_COUNT=22

ls ${KERNELDIR} > /dev/null || exit 1

pushd ${ANDROID_ROOT}/hardware/renesas/uvcs/source/makefile/ndk_4_6
make clean && make || exit 1
popd
