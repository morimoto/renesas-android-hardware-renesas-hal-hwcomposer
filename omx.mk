
PRODUCT_COPY_FILES += \
	hardware/renesas/omx/config/omxr_config_base.txt:system/etc/omxr_config_base.txt \
	hardware/renesas/omx/config/omxr_config_h264d.txt:system/etc/omxr_config_h264d.txt \
	hardware/renesas/omx/config/omxr_config_vdcmn.txt:system/etc/omxr_config_vdcmn.txt \
	hardware/renesas/omx/lib/android/ndk_4_6/libomxr_core.so:system/lib/libomxr_core.so \
	hardware/renesas/omx/lib/android/ndk_4_6/libomxr_mc_cmn.so:system/lib/libomxr_mc_cmn.so \
	hardware/renesas/omx/lib/android/ndk_4_6/libomxr_mc_h264d.so:system/lib/libomxr_mc_h264d.so \
	hardware/renesas/omx/lib/android/ndk_4_6/libomxr_mc_vcmn.so:system/lib/libomxr_mc_vcmn.so \
	hardware/renesas/omx/lib/android/ndk_4_6/libomxr_mc_vdcmn.so:system/lib/libomxr_mc_vdcmn.so \
	hardware/renesas/omx/lib/android/ndk_4_6/libuvcs_dec.so:system/lib/libuvcs_dec.so \
	hardware/renesas/omx/lib/android/ndk_4_6/libvcp3_avcd.so:system/lib/libvcp3_avcd.so \
	hardware/renesas/omx/lib/android/ndk_4_6/libvcp3_mcvd.so:system/lib/libvcp3_mcvd.so

PRODUCT_PACKAGES += \
	libomxr_adapter \
	libomxr_utility \
	libomxr_uvcs_udf \
	libomxr_cnvosdep \
	libomxr_cnvpfdp \
	libomxr_videoconverter \
	libstagefrighthw

