/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013 All rights reserved.
 */

#ifndef _VSP_DRV_H_
#define _VSP_DRV_H_

/* sub error code */
#define E_VSP_DEF_INH				(-100)
#define E_VSP_DEF_REG				(-101)
#define E_VSP_NO_MEM				(-102)
#define E_VSP_NO_INIT				(-103)
#define E_VSP_INVALID_STATE			(-105)
#define E_VSP_PARA_CH				(-150)
#define E_VSP_PARA_CB				(-201)
#define E_VSP_PARA_INPAR			(-202)

#define E_VSP_PARA_RPFNUM			(-210)
#define E_VSP_PARA_RPFORDER			(-211)
#define E_VSP_PARA_USEMODULE		(-212)
#define E_VSP_PARA_OUTPAR			(-213)
#define E_VSP_PARA_CTRLPAR			(-214)
#define E_VSP_PARA_NOIN				(-215)
#define E_VSP_PARA_CONNECT			(-216)	/* illegal connection */
#define E_VSP_PARA_NOPARENT			(-217)	/* not found parent layer */
#define E_VSP_PARA_NOINPUT			(-218)	/* no input */
#define E_VSP_PARA_NOMODULE			(-219)	/* no module */

#define E_VSP_PARA_IN_ADR			(-220)
#define E_VSP_PARA_IN_ADRC0			(-221)
#define E_VSP_PARA_IN_ADRC1			(-222)
#define E_VSP_PARA_IN_WIDTH			(-223)
#define E_VSP_PARA_IN_HEIGHT		(-224)
#define E_VSP_PARA_IN_WIDTHEX		(-225)
#define E_VSP_PARA_IN_HEIGHTEX		(-226)
#define E_VSP_PARA_IN_XOFFSET		(-227)
#define E_VSP_PARA_IN_YOFFSET		(-228)
#define E_VSP_PARA_IN_FORMAT		(-229)
#define E_VSP_PARA_IN_SWAP			(-230)	/* not used */
#define E_VSP_PARA_IN_XPOSI			(-231)
#define E_VSP_PARA_IN_YPOSI			(-232)
#define E_VSP_PARA_IN_CIPM			(-233)
#define E_VSP_PARA_IN_CEXT			(-234)
#define E_VSP_PARA_IN_CSC			(-235)
#define E_VSP_PARA_IN_ITURBT		(-236)
#define E_VSP_PARA_IN_CLRCNG		(-237)
#define E_VSP_PARA_IN_VIR			(-238)
#define E_VSP_PARA_IN_ALPHA			(-239)
#define E_VSP_PARA_IN_CONNECT		(-240)
#define E_VSP_PARA_IN_PWD			(-241)

#define E_VSP_PARA_OSD_CLUT			(-250)
#define E_VSP_PARA_OSD_SIZE			(-251)

#define E_VSP_PARA_ALPHA_ADR		(-260)	/* illegal address pointer */
#define E_VSP_PARA_ALPHA_ALPHAN		(-261)
#define E_VSP_PARA_ALPHA_ASWAP		(-262)	/* not used */
#define E_VSP_PARA_ALPHA_ASEL		(-263)
#define E_VSP_PARA_ALPHA_AEXT		(-264)
#define E_VSP_PARA_ALPHA_IROP		(-265)
#define E_VSP_PARA_ALPHA_MSKEN		(-266)
#define E_VSP_PARA_ALPHA_BSEL		(-267)

#define E_VSP_PARA_OUT_ADR			(-270)
#define E_VSP_PARA_OUT_ADRC0		(-271)
#define E_VSP_PARA_OUT_ADRC1		(-272)
#define E_VSP_PARA_OUT_WIDTH		(-273)
#define E_VSP_PARA_OUT_HEIGHT		(-274)
#define E_VSP_PARA_OUT_XOFFSET		(-275)
#define E_VSP_PARA_OUT_YOFFSET		(-276)
#define E_VSP_PARA_OUT_XCLIP		(-277)
#define E_VSP_PARA_OUT_YCLIP		(-278)
#define E_VSP_PARA_OUT_FORMAT		(-279)
#define E_VSP_PARA_OUT_SWAP			(-280)	/* not used */
#define E_VSP_PARA_OUT_PXA			(-281)
#define E_VSP_PARA_OUT_XCOFFSET		(-282)
#define E_VSP_PARA_OUT_YCOFFSET		(-283)
#define E_VSP_PARA_OUT_CSC			(-284)
#define E_VSP_PARA_OUT_ITURBT		(-285)
#define E_VSP_PARA_OUT_CLRCNG		(-286)
#define E_VSP_PARA_OUT_CBRM			(-287)
#define E_VSP_PARA_OUT_ABRM			(-288)
#define E_VSP_PARA_OUT_CLMD			(-289)
#define E_VSP_PARA_OUT_DITH			(-291)
#define E_VSP_PARA_OUT_INHSV		(-292)	/* illegal input color space */
#define E_VSP_PARA_OUT_INWIDTH		(-293)	/* illegal input image width */
#define E_VSP_PARA_OUT_INHEIGHT		(-294)	/* illegal input image height */
#define E_VSP_PARA_OUT_NOTCOLOR		(-295)	/* not equal color space */

#define E_VSP_PARA_BRU_LAYORDER		(-300)
#define E_VSP_PARA_BRU_ADIV			(-301)
#define E_VSP_PARA_BRU_QNT			(-302)
#define E_VSP_PARA_BRU_DITH			(-303)
#define E_VSP_PARA_BRU_CONNECT		(-304)
#define E_VSP_PARA_BRU_INHSV		(-305)	/* illegal input color space */

#define E_VSP_PARA_VIR_ADR			(-310)	/* illegal address pointer */
#define E_VSP_PARA_VIR_WIDTH		(-311)
#define E_VSP_PARA_VIR_HEIGHT		(-312)
#define E_VSP_PARA_VIR_XPOSI		(-313)
#define E_VSP_PARA_VIR_YPOSI		(-314)
#define E_VSP_PARA_VIR_PWD			(-315)	/* illegal pwd */

#define E_VSP_PARA_BLEND_RBC		(-320)
#define E_VSP_PARA_BLEND_CROP		(-321)
#define E_VSP_PARA_BLEND_AROP		(-322)
#define E_VSP_PARA_BLEND_FORM		(-323)
#define E_VSP_PARA_BLEND_COEFX		(-324)
#define E_VSP_PARA_BLEND_COEFY		(-325)
#define E_VSP_PARA_BLEND_AFORM		(-326)
#define E_VSP_PARA_BLEND_ACOEFX		(-327)
#define E_VSP_PARA_BLEND_ACOEFY		(-328)

#define E_VSP_PARA_ROP_CROP			(-330)
#define E_VSP_PARA_ROP_AROP			(-331)

#define E_VSP_PARA_SRU_MODE			(-340)
#define E_VSP_PARA_SRU_PARAM		(-341)
#define E_VSP_PARA_SRU_ENSCL		(-342)
#define E_VSP_PARA_SRU_CONNECT		(-343)
#define E_VSP_PARA_SRU_WIDTH		(-344)	/* illegal image width */
#define E_VSP_PARA_SRU_HEIGHT		(-345)	/* illegal image height */
#define E_VSP_PARA_SRU_INHSV		(-346)	/* illegal input color space */

#define E_VSP_PARA_UDS_AMD			(-350)
#define E_VSP_PARA_UDS_FMD			(-351)
#define E_VSP_PARA_UDS_CLIP			(-352)
#define E_VSP_PARA_UDS_ALPHA		(-353)
#define E_VSP_PARA_UDS_COMP			(-354)
#define E_VSP_PARA_UDS_CONNECT		(-355)
#define E_VSP_PARA_UDS_XRATIO		(-356)	/* illegal x_ratio */
#define E_VSP_PARA_UDS_YRATIO		(-357)	/* illegal y_ratio */
#define E_VSP_PARA_UDS_OUTCWIDTH	(-358)	/* illegal clipping width */
#define E_VSP_PARA_UDS_OUTCHEIGHT	(-359)	/* illegal clipping height */
#define E_VSP_PARA_UDS_INWIDTH		(-360)	/* illegal input width */
#define E_VSP_PARA_UDS_INHEIGHT		(-361)	/* illegal input height */

#define E_VSP_PARA_LUT				(-600)
#define E_VSP_PARA_LUT_SIZE			(-601)
#define E_VSP_PARA_LUT_CONNECT		(-602)

#define E_VSP_PARA_CLU_MODE			(-610)
#define E_VSP_PARA_CLU_ADR			(-611)
#define E_VSP_PARA_CLU_DATA			(-612)
#define E_VSP_PARA_CLU_SIZE			(-613)
#define E_VSP_PARA_CLU_CONNECT		(-614)

#define E_VSP_PARA_HST_NOTRGB		(-630)	/* illegal input color space */
#define E_VSP_PARA_HST_CONNECT		(-631)	/* illegal connecting module */

#define E_VSP_PARA_HSI_NOTHSV		(-640)	/* illegal input color space */
#define E_VSP_PARA_HSI_CONNECT		(-641)	/* illegal connecting module */

#define E_VSP_PARA_HGO_ADR			(-660)	/* illegal addres pointer */
#define E_VSP_PARA_HGO_WIDTH		(-661)	/* illegal widht */
#define E_VSP_PARA_HGO_HEIGHT		(-662)	/* illegal height */
#define E_VSP_PARA_HGO_XOFFSET		(-663)	/* illegal x_offset */
#define E_VSP_PARA_HGO_YOFFSET		(-664)	/* illegal y_offset */
#define E_VSP_PARA_HGO_BINMODE		(-665)	/* illegal binary mode */
#define E_VSP_PARA_HGO_MAXRGB		(-669)	/* illegal source component */
#define E_VSP_PARA_HGO_XSKIP		(-666)	/* illegal skip mode */
#define E_VSP_PARA_HGO_YSKIP		(-667)	/* illegal skip mode */
#define E_VSP_PARA_HGO_SMMPT		(-668)	/* illegal sampling module */

#define E_VSP_PARA_HGT_ADR			(-670)	/* illegal addres pointer */
#define E_VSP_PARA_HGT_WIDTH		(-671)	/* illegal width */
#define E_VSP_PARA_HGT_HEIGHT		(-672)	/* illegal height */
#define E_VSP_PARA_HGT_XOFFSET		(-673)	/* illegal x_offset */
#define E_VSP_PARA_HGT_YOFFSET		(-674)	/* illegal y_offset */
#define E_VSP_PARA_HGT_AREA			(-675)	/* illegal hue area pointer */
#define E_VSP_PARA_HGT_XSKIP		(-676)	/* illegal skip mode */
#define E_VSP_PARA_HGT_YSKIP		(-677)	/* illegal skip mode */
#define E_VSP_PARA_HGT_SMMPT		(-678)	/* illegal sampling module */

#define E_VSP_PARA_NOSRU			(-650)
#define E_VSP_PARA_NOUDS			(-651)
#define E_VSP_PARA_NOLUT			(-652)
#define E_VSP_PARA_NOCLU			(-653)
#define E_VSP_PARA_NOHST			(-654)
#define E_VSP_PARA_NOHSI			(-655)
#define E_VSP_PARA_NOBRU			(-656)
#define E_VSP_PARA_NOHGO			(-657)
#define E_VSP_PARA_NOHGT			(-658)

#define E_VSP_PARA_BRU_INCOLOR		(-660)	/* different color space */
#define E_VSP_PARA_UDS_SERIAL		(-661)	/* be arranged in series */

#define E_VSP_BUSY_RPF_OVER			(-700)
#define E_VSP_BUSY_MODULE_OVER		(-701)

/* T_VSP_START.rpf_order */
enum {
	VSP_RPF0 = 0,
	VSP_RPF1,
	VSP_RPF2,
	VSP_RPF3,
	VSP_RPF4,
	VSP_RPF_MAX
};

/* T_VSP_START.use_module */
#define VSP_SRU_USE				(0x0001)	/* super-resolution */
#define VSP_UDS_USE				(0x0002)	/* up down scaler */
#define VSP_UDS1_USE			(0x0004)	/* up down scaler */
#define VSP_UDS2_USE			(0x0008)	/* up down scaler */
#define VSP_LUT_USE				(0x0010)	/* look up table */
#define VSP_CLU_USE				(0x0020)	/* cubic look up table */
#define VSP_HST_USE				(0x0040)	/* hue saturation value transform */
#define VSP_HSI_USE				(0x0080)	/* hue saturation value inverse */
#define VSP_BRU_USE				(0x0100)	/* blend rop */
#define VSP_HGO_USE				(0x0200)	/* histogram generator-one */
#define VSP_HGT_USE				(0x0400)	/* histogram generator-two */

/* RPF module parameter */
/* input format */
#define VSP_IN_RGB332				(0x0100) /* RGB332 */
#define VSP_IN_XRGB4444				(0x0201) /* XRGB4444 */
#define VSP_IN_RGBX4444				(0x0202) /* RGBX4444 */
#define VSP_IN_XRGB1555				(0x0204) /* XRGB1555 */
#define VSP_IN_RGBX5551				(0x0205) /* RGBX5551 */
#define VSP_IN_RGB565				(0x0206) /* RGB565 */
#define VSP_IN_AXRGB86666			(0x0407) /* AXRGB86666 */
#define VSP_IN_RGBXA66668			(0x0408) /* RGBXA66668 */
#define VSP_IN_XRGBA66668			(0x0409) /* XRGBA66668 */
#define VSP_IN_ARGBX86666			(0x040A) /* ARGBX86666 */
#define VSP_IN_AXXXRGB82666			(0x040B) /* AXXXRGB82666 */
#define VSP_IN_XXXRGBA26668			(0x040C) /* XXXRGBA26668 */
#define VSP_IN_ARGBXXX86662			(0x040D) /* ARGBXXX86662 */
#define VSP_IN_RGBXXXA66628			(0x040E) /* RGBXXXA66628 */
#define VSP_IN_XRGB6666				(0x030F) /* XRGB6666 */
#define VSP_IN_RGBX6666				(0x0310) /* RGBX6666 */
#define VSP_IN_XXXRGB2666			(0x0311) /* XXXRGB2666 */
#define VSP_IN_RGBXXX6662			(0x0312) /* RGBXXX6662 */
#define VSP_IN_ARGB8888				(0x0413) /* ARGB8888 */
#define VSP_IN_RGBA8888				(0x0414) /* RGBA8888 */
#define VSP_IN_RGB888				(0x0315) /* RGB888 */
#define VSP_IN_XXRGB7666			(0x0416) /* XXRGB7666 */
#define VSP_IN_XRGB14666			(0x0417) /* XRGB14666 */
#define VSP_IN_BGR888				(0x0318) /* BGR888 */
#define VSP_IN_ARGB4444				(0x0219) /* ARGB4444 */
#define VSP_IN_RGBA4444				(0x021A) /* RGBA4444 */
#define VSP_IN_ARGB1555				(0x021B) /* ARGB1555 */
#define VSP_IN_RGBA5551				(0x021C) /* RGBA5551 */
#define VSP_IN_ABGR4444				(0x021D) /* ABGR4444 */
#define VSP_IN_BGRA4444				(0x021E) /* BGRA4444 */
#define VSP_IN_ABGR1555				(0x021F) /* ABGR1555 */
#define VSP_IN_BGRA5551				(0x0220) /* BGRA5551 */
#define VSP_IN_XXXBGR2666			(0x0321) /* XXXBGR2666 */
#define VSP_IN_ABGR8888				(0x0422) /* ABGR8888 */
#define VSP_IN_XRGB16565			(0x0423) /* XRGB16565 */
#define VSP_IN_RGB_CLUT_DATA		(0x013F) /* RGB_CLUT_DATA */
#define VSP_IN_YUV444_SEMI_PLANAR	(0x0040) /* YCbCr4:4:4 Semi-Planar */
#define VSP_IN_YUV422_SEMI_PLANAR	(0x0041) /* YCbCr4:2:2 Semi-Planar */
#define VSP_IN_YUV420_SEMI_PLANAR	(0x0042) /* YCbCr4:2:0 Semi-Planar */
#define VSP_IN_YUV444_INTERLEAVED	(0x0046) /* YCbCr4:4:4 Interleaved */
#define VSP_IN_YUV422_INTERLEAVED0	(0x0047) /* YCbCr4:2:2 Interleaved type0 */
#define VSP_IN_YUV422_INTERLEAVED1	(0x0048) /* YCbCr4:2:2 Interleaved type1 */
#define VSP_IN_YUV420_INTERLEAVED	(0x0049) /* YCbCr4:2:0 Interleaved */
#define VSP_IN_YUV444_PLANAR		(0x004A) /* YCbCr4:4:4 Planar */
#define VSP_IN_YUV422_PLANAR		(0x004B) /* YCbCr4:2:2 Planar */
#define VSP_IN_YUV420_PLANAR		(0x004C) /* YCbCr4:2:0 Planar */
#define VSP_IN_YUV_CLUT_DATA		(0x017F) /* YCbCr_CLUT_DATA */

#define VSP_IN_YUV422_SEMI_NV16		VSP_IN_YUV422_SEMI_PLANAR
#define VSP_IN_YUV422_SEMI_NV61		(0x4041) /* YCbCr4:2:2 Semi(NV61) */
#define VSP_IN_YUV420_SEMI_NV12		VSP_IN_YUV420_SEMI_PLANAR
#define VSP_IN_YUV420_SEMI_NV21		(0x4042) /* YCbCr4:2:0 semi(NV21) */
#define VSP_IN_YUV422_INT0_UYVY		VSP_IN_YUV422_INTERLEAVED0
#define VSP_IN_YUV422_INT0_YUY2		(0x8047) /* YCbCr4:2:2 type0(YUY2) */
#define VSP_IN_YUV422_INT0_YVYU		(0xC047) /* YCbCr4:2:2 type0(YVYU) */

/* swap setting */
#define VSP_SWAP_NO					(0x00)	/* disable */
#define VSP_SWAP_B					(0x01)	/* byte units */
#define VSP_SWAP_W					(0x02)	/* word units */
#define VSP_SWAP_L					(0x04)	/* longword units */
#define VSP_SWAP_LL					(0x08)	/* LONG LWORD units */

/* parent setting */
#define VSP_LAYER_CHILD				(0x01)	/* child */
#define VSP_LAYER_PARENT			(0x02)	/* parent */

/* horizontal chrominance interpolation method setting */
#define VSP_CIPM_0_HOLD				(0x00)	/* nearest-neighbor method */
#define VSP_CIPM_BI_LINEAR			(0x01)	/* bilinear method */

/* lower-bit color data extension method setting */
#define VSP_CEXT_EXPAN				(0x00)	/* extended with 0 */
#define VSP_CEXT_COPY				(0x01)	/* copied to the lower-order bits */
#define VSP_CEXT_EXPAN_MAX			(0x02)	/* extended with 0 */
										/* maximum value is limitede to 0xFF */

/* color space conversion enable */
#define VSP_CSC_OFF					(0x00)	/* disable */
#define VSP_CSC_ON					(0x01)	/* enable */

/* select of color space conversion ITU-R BT */
#define VSP_ITURBT_601				(0x00)	/* ITU-R BT601 */
#define VSP_ITURBT_709				(0x01)	/* ITU-R BT709 */

/* select of color space conversion scale */
#define VSP_ITU_COLOR				(0x00)	/* YUV[16,235/140] <-> RGB[0,255] */
#define VSP_FULL_COLOR				(0x01)	/* Full scale */

/* virtual input enable */
#define VSP_NO_VIR					(0x00)	/* disable */
#define VSP_VIR						(0x01)	/* enable */

/* comparison color data setting */
#define VSP_ALPHA_NO				(0x00)	/* disable */
#define VSP_ALPHA_AL1				(0x01)	/* comparision alpha1 enable */
#define VSP_ALPHA_AL2				(0x02)	/* comparision alpha2 enable */

/* alpha format and processing method select */
#define VSP_ALPHA_NUM1				(0x00)	/* 1,4 or 8bit packed alpha
												+ plane alpha */
#define VSP_ALPHA_NUM2				(0x01)	/* 8-bit plane alpha */
#define VSP_ALPHA_NUM3				(0x02)	/* 1-bit packed alpha
												+ plane alpha */
#define VSP_ALPHA_NUM4				(0x03)	/* 1-bit plane alpha */
#define VSP_ALPHA_NUM5				(0x04)	/* Fixed alpha */

/* lower-bit alpha value extension method set */
#define VSP_AEXT_EXPAN				(0x00)	/* extended with 0 */
#define VSP_AEXT_COPY				(0x01)	/* copied to the lower-order bits */
#define VSP_AEXT_EXPAN_MAX			(0x02)	/* extended with 0.
												max limited 0xFF */

/* ROP oprator */
enum {
	VSP_IROP_NOP = 0,						/* NOP(D) */
	VSP_IROP_AND,							/* AND(S & D) */
	VSP_IROP_AND_REVERSE,					/* AND_REVERSE(S & ~D) */
	VSP_IROP_COPY,							/* COPY(S) */
	VSP_IROP_AND_INVERTED,					/* AND_INVERTED(~S & D) */
	VSP_IROP_CLEAR,							/* CLEAR(0) */
	VSP_IROP_XOR,							/* XOR(S ^ D) */
	VSP_IROP_OR,							/* OR(S | D) */
	VSP_IROP_NOR,							/* NOR(~(S | D)) */
	VSP_IROP_EQUIV,							/* EQUIV(~(S ^ D)) */
	VSP_IROP_INVERT,						/* INVERT(~D) */
	VSP_IROP_OR_REVERSE,					/* OR_REVERSE(S | ~D) */
	VSP_IROP_COPY_INVERTED,					/* COPY_INVERTED(~S) */
	VSP_IROP_OR_INVERTED,					/* OR_INVERTED(~S | D) */
	VSP_IROP_NAND,							/* NAND(~(S & D)) */
	VSP_IROP_SET,							/* SET(all 1) */
	VSP_IROP_MAX
};

/* mask generation specification */
#define VSP_MSKEN_ALPHA				(0x00)	/* use alpha plane */
#define VSP_MSKEN_COLOR				(0x01)	/* use comparison value */

/* alpha bit counte conversion selection */
#define VSP_ALPHA_8BIT				(0x00) /* 8bit alpha is converted to 1bit */
#define VSP_ALPHA_1BIT				(0x01) /* alpha value goes through */

/* WPF module parameter */
/* output format */
#define VSP_OUT_RGB332				VSP_IN_RGB332
#define VSP_OUT_XRGB4444			VSP_IN_XRGB4444
#define VSP_OUT_RGBX4444			VSP_IN_RGBX4444
#define VSP_OUT_XRGB1555			VSP_IN_XRGB1555
#define VSP_OUT_RGBX5551			VSP_IN_RGBX5551
#define VSP_OUT_RGB565				VSP_IN_RGB565
#define VSP_OUT_PXRGB86666			VSP_IN_AXRGB86666
#define VSP_OUT_RGBXP66668			VSP_IN_RGBXA66668
#define VSP_OUT_XRGBP66668			VSP_IN_XRGBA66668
#define VSP_OUT_PRGBX86666			VSP_IN_ARGBX86666
#define VSP_OUT_PXXXRGB82666		VSP_IN_AXXXRGB82666
#define VSP_OUT_XXXRGBP26668		VSP_IN_XXXRGBA26668
#define VSP_OUT_PRGBXXX86662		VSP_IN_ARGBXXX86662
#define VSP_OUT_RGBXXXP66628		VSP_IN_RGBXXXA66628
#define VSP_OUT_XRGB6666			VSP_IN_XRGB6666
#define VSP_OUT_RGBX6666			VSP_IN_RGBX6666
#define VSP_OUT_XXXRGB2666			VSP_IN_XXXRGB2666
#define VSP_OUT_RGBXXX6662			VSP_IN_RGBXXX6662
#define VSP_OUT_PRGB8888			VSP_IN_ARGB8888
#define VSP_OUT_RGBP8888			VSP_IN_RGBA8888
#define VSP_OUT_RGB888				VSP_IN_RGB888
#define VSP_OUT_XXRGB7666			VSP_IN_XXRGB7666
#define VSP_OUT_XRGB14666			VSP_IN_XRGB14666
#define VSP_OUT_BGR888				VSP_IN_BGR888
#define VSP_OUT_PRGB4444			VSP_IN_ARGB4444
#define VSP_OUT_RGBP4444			VSP_IN_RGBA4444
#define VSP_OUT_PRGB1555			VSP_IN_ARGB1555
#define VSP_OUT_RGBP5551			VSP_IN_RGBA5551
#define VSP_OUT_PBGR4444			VSP_IN_ABGR4444
#define VSP_OUT_BGRP4444			VSP_IN_BGRA4444
#define VSP_OUT_PBGR1555			VSP_IN_ABGR1555
#define VSP_OUT_BGRP5551			VSP_IN_BGRA5551
#define VSP_OUT_XXXBGR2666			VSP_IN_XXXBGR2666
#define VSP_OUT_PBGR8888			VSP_IN_ABGR8888
#define VSP_OUT_XRGB16565			VSP_IN_XRGB16565
#define VSP_OUT_YUV444_SEMI_PLANAR	VSP_IN_YUV444_SEMI_PLANAR
#define VSP_OUT_YUV422_SEMI_PLANAR	VSP_IN_YUV422_SEMI_PLANAR
#define VSP_OUT_YUV420_SEMI_PLANAR	VSP_IN_YUV420_SEMI_PLANAR
#define VSP_OUT_YUV444_INTERLEAVED	VSP_IN_YUV444_INTERLEAVED
#define VSP_OUT_YUV422_INTERLEAVED0	VSP_IN_YUV422_INTERLEAVED0
#define VSP_OUT_YUV422_INTERLEAVED1	VSP_IN_YUV422_INTERLEAVED1
#define VSP_OUT_YUV420_INTERLEAVED	VSP_IN_YUV420_INTERLEAVED
#define VSP_OUT_YUV444_PLANAR		VSP_IN_YUV444_PLANAR
#define VSP_OUT_YUV422_PLANAR		VSP_IN_YUV422_PLANAR
#define VSP_OUT_YUV420_PLANAR		VSP_IN_YUV420_PLANAR

#define VSP_OUT_YUV422_SEMI_NV16	VSP_IN_YUV422_SEMI_NV16
#define VSP_OUT_YUV422_SEMI_NV61	VSP_IN_YUV422_SEMI_NV61
#define VSP_OUT_YUV420_SEMI_NV12	VSP_IN_YUV420_SEMI_NV12
#define VSP_OUT_YUV420_SEMI_NV21	VSP_IN_YUV420_SEMI_NV21
#define VSP_OUT_YUV422_INT0_UYVY	VSP_IN_YUV422_INT0_UYVY
#define VSP_OUT_YUV422_INT0_YUY2	VSP_IN_YUV422_INT0_YUY2
#define VSP_OUT_YUV422_INT0_YVYU	VSP_IN_YUV422_INT0_YVYU

/* PAD data select */
#define VSP_PAD_P					(0x00)	/* PAD value */
#define VSP_PAD_IN					(0x01)	/* alpha value */

/* bit count reduction method selection for data storage in packed RGB */
#define VSP_CSC_ROUND_DOWN			(0x00)	/* truncated */
#define VSP_CSC_ROUND_OFF			(0x01)	/* rounding */

/* bit count reduction method selection for data storage in PAD */
#define VSP_CONVERSION_ROUNDDOWN	(0x00)	/* truncated */
#define VSP_CONVERSION_ROUNDING		(0x01)	/* rounding */
#define VSP_CONVERSION_THRESHOLD	(0x02)	/* comparison */

/* color data clipping method */
#define VSP_CLMD_NO					(0x00)	/* not clipped */
#define VSP_CLMD_MODE1				(0x01)	/* YCbCr mode1 */
#define VSP_CLMD_MODE2				(0x02)	/* YCbCr mode2 */

/* dithering enable */
#define VSP_NO_DITHER				(0x00)	/* disable */
#define VSP_DITHER					(0x03)	/* enable */

/* SRU module parameter */
/* super-resolution mode */
#define VSP_SRU_MODE1				(0x00)	/* without scaling */
#define VSP_SRU_MODE2				(0x40)	/* double scale-up */

/* super-resolution color */
#define VSP_SRU_RCR					(0x08)	/* R or Cr */
#define VSP_SRU_GY					(0x04)	/* G or Y */
#define VSP_SRU_BCB					(0x02)	/* B or Cb */

/* super-resolution level */
enum {
	VSP_SCL_LEVEL1 = 0,		/* weak */
	VSP_SCL_LEVEL2,
	VSP_SCL_LEVEL3,
	VSP_SCL_LEVEL4,
	VSP_SCL_LEVEL5,
	VSP_SCL_LEVEL6,			/* strong */
	VSP_SCL_LEVEL_MAX
};

/* UDS module parameter */
/* pixel count at scale-up */
#define VSP_AMD_NO					(0x00)	/* 1 + int((n-1)*scale-up factor) */
#define VSP_AMD						(0x01)	/* int(n*scale-up factor) */

/* padding for insufficient clipping size */
#define VSP_FMD_NO					(0x00)	/* copying pixels
												at the right and botom edge */
#define VSP_FMD						(0x01)	/* filled by filcolor parameter */

/* alpha data threshold comparision enable/disable */
#define VSP_CLIP_OFF				(0x00)	/* disable */
#define VSP_CLIP_ON					(0x01)	/* enable */

/* scale-up/down of alpha plane */
#define VSP_ALPHA_OFF				(0x00)	/* disable */
#define VSP_ALPHA_ON				(0x01)	/* enable */

/* pixel component interpolation method */
#define VSP_COMPLEMENT_BIL			(0x00)	/* bilinear */
#define VSP_COMPLEMENT_NN			(0x01)	/* nearest neighbor */
#define VSP_COMPLEMENT_BC			(0x02)	/* multi-tap mode */

/* CLU module parameter */
/* LUT dimension number */
#define VSP_CLU_MODE_3D				(0x00)	/* 3D mode */
#define VSP_CLU_MODE_2D				(0x01)	/* 2D mode */
#define VSP_CLU_MODE_3D_AUTO		(0x80)	/* 3D mode with auto addr inc */
#define VSP_CLU_MODE_2D_AUTO		(0x81)	/* 2D mode with auto addr inc */

/* BRU module parameter */
/* select input layer */
#define VSP_LAY_NO					(0x00)	/* */
#define VSP_LAY_1					(0x01)	/* input source 1 */
#define VSP_LAY_2					(0x02)	/* input source 2 */
#define VSP_LAY_3					(0x03)	/* input source 3 */
#define VSP_LAY_4					(0x04)	/* input source 4 */
#define VSP_LAY_VIRTUAL				(0x05)	/* input virtual */

/* color data normalization */
#define VSP_DIVISION_OFF			(0x00)	/* disable */
#define VSP_DIVISION_ON				(0x01)	/* enable */

/* tone(color) reduction enable */
#define VSP_QNT_OFF					(0x00)	/* disable */
#define VSP_QNT_ON					(0x01)	/* enable */

/* dither processing enable */
#define VSP_DITH_OFF				(0x00)	/* disable */
#define VSP_DITH_18BPP				(0x01)	/* 18bpp(RGB666:260000 colors) */
#define VSP_DITH_16BPP				(0x02)	/* 16bpp(RGB565:65535 colors) */
#define VSP_DITH_15BPP				(0x03)	/* 15bpp(RGB555:32768 colors) */
#define VSP_DITH_12BPP				(0x04)	/* 12bpp(RGB666:4096 colors) */
#define VSP_DITH_8BPP				(0x05)	/* 8bpp(RGB666:256 colors) */

/* ROP Unit method */
#define VSP_RBC_ROP					(0x00)	/* raster opration */
#define VSP_RBC_BLEND				(0x01)	/* blend opration */

/* blending expression selection */
#define VSP_FORM_BLEND0				(0x00)	/* + */
#define VSP_FORM_BLEND1				(0x01)	/* - */

/* blending coefficient X selection */
enum {
	VSP_COEFFICIENT_BLENDX1 = 0,			/* (DST) */
	VSP_COEFFICIENT_BLENDX2,				/* 255 - (DST) */
	VSP_COEFFICIENT_BLENDX3,				/* (SRC) */
	VSP_COEFFICIENT_BLENDX4,				/* 255 - (SRC) */
	VSP_COEFFICIENT_BLENDX5,				/* (acoefx_fix) */
	VSP_COEFFICIENT_BLENDX_MAX
};

/* blending coefficient Y selection */
enum {
	VSP_COEFFICIENT_BLENDY1 = 0,			/* (DST) */
	VSP_COEFFICIENT_BLENDY2,				/* 255 - (DST) */
	VSP_COEFFICIENT_BLENDY3,				/* (SRC) */
	VSP_COEFFICIENT_BLENDY4,				/* 255 - (SRC) */
	VSP_COEFFICIENT_BLENDY5,				/* (acoefx_fix) */
	VSP_COEFFICIENT_BLENDY_MAX
};

/* blending alpha creation expression */
#define VSP_FORM_ALPHA0				(0x00)	/* + */
#define VSP_FORM_ALPHA1				(0x01)	/* - */

/* alpha creation coefficient X */
enum {
	VSP_COEFFICIENT_ALPHAX1 = 0,			/* (DST) */
	VSP_COEFFICIENT_ALPHAX2,				/* 255 - (DST) */
	VSP_COEFFICIENT_ALPHAX3,				/* (SRC) */
	VSP_COEFFICIENT_ALPHAX4,				/* 255 - (SRC) */
	VSP_COEFFICIENT_ALPHAX5,				/* (acoefx_fix) */
	VSP_COEFFICIENT_ALPHAX_MAX
};

/* alpha creation coefficient Y */
enum {
	VSP_COEFFICIENT_ALPHAY1 = 0,			/* (DST) */
	VSP_COEFFICIENT_ALPHAY2,				/* 255 - (DST) */
	VSP_COEFFICIENT_ALPHAY3,				/* (SRC) */
	VSP_COEFFICIENT_ALPHAY4,				/* 255 - (SRC) */
	VSP_COEFFICIENT_ALPHAY5,				/* (acoefx_fix) */
	VSP_COEFFICIENT_ALPHAY_MAX
};

/* HGO,HGT module parameter */
#define VSP_STRAIGHT_BINARY			(0x00)	/* straight binary mode */
#define VSP_OFFSET_BINARY			(0x50)	/* offset binary mode */

#define VSP_MAXRGB_OFF				(0x00)	/* 3 color components */
#define VSP_MAXRGB_ON				(0x80)	/* max value of RGB */

#define VSP_SKIP_OFF				(0x00)	/* disable */
#define VSP_SKIP_1_2				(0x01)	/* every two pixels */
#define VSP_SKIP_1_4				(0x02)	/* every four pixels */

#define VSP_SMPPT_SRC1				0
#define VSP_SMPPT_SRC2				1
#define VSP_SMPPT_SRC3				2
#define VSP_SMPPT_SRC4				3
#define VSP_SMPPT_SRU				16
#define VSP_SMPPT_UDS				17
#define VSP_SMPPT_UDS1				18
#define VSP_SMPPT_UDS2				19
#define VSP_SMPPT_LUT				22
#define VSP_SMPPT_BRU				27
#define VSP_SMPPT_CLU				29
#define VSP_SMPPT_HST				30
#define VSP_SMPPT_HSI				31

typedef struct {
	unsigned char intlvl[2];	/* not used */
} T_VSP_INIT; 

typedef struct {
	long ercd;					/* error code */
	unsigned long ch;			/* channel number */
	void *userdata;				/* userdata */
} T_VSP_CB;

typedef struct {
	unsigned long *clut;
	short size;
} T_VSP_OSDLUT;

typedef struct {
	void *addr_a;
	unsigned char alphan;
	unsigned long alpha1;
	unsigned long alpha2;
	unsigned short astride;
	unsigned char aswap;
	unsigned char asel;
	unsigned char aext;
	unsigned char anum0;
	unsigned char anum1;
	unsigned char afix;
	unsigned char irop;
	unsigned char msken;
	unsigned char bsel;
	unsigned long mgcolor;
	unsigned long mscolor0;
	unsigned long mscolor1;
} T_VSP_ALPHA;

typedef struct {
	unsigned long color1;
	unsigned long color2;
} T_VSP_CLRCNV;

typedef struct {
	void *addr;					/* Y or RGB buffer address */
	void *addr_c0;				/* CbCr or CB buffer address */
	void *addr_c1;				/* Cr buffer address */
	unsigned short stride;
	unsigned short stride_c;
	unsigned short width;
	unsigned short height;
	unsigned short width_ex;
	unsigned short height_ex;
	unsigned short x_offset;
	unsigned short y_offset;
	unsigned short format;
	unsigned char swap;
	unsigned short x_position;
	unsigned short y_position;
	unsigned char pwd;
	unsigned char cipm;
	unsigned char cext;
	unsigned char csc;
	unsigned char iturbt;
	unsigned char clrcng;
	unsigned char vir;
	unsigned long vircolor;
	T_VSP_OSDLUT *osd_lut;
	T_VSP_ALPHA *alpha_blend;
	T_VSP_CLRCNV *clrcnv;
	unsigned long connect;
} T_VSP_IN;

typedef struct {
	void *addr;					/* Y or RGB buffer address */
	void *addr_c0;				/* CbCr or CB buffer address */
	void *addr_c1;				/* Cr buffer address */
	unsigned short stride;
	unsigned short stride_c;
	unsigned short width;
	unsigned short height;
	unsigned short x_offset;
	unsigned short y_offset;
	unsigned short format;
	unsigned char swap;
	unsigned char pxa;
	unsigned char pad;
	unsigned short x_coffset;
	unsigned short y_coffset;
	unsigned char csc;
	unsigned char iturbt;
	unsigned char clrcng;
	unsigned char cbrm;
	unsigned char abrm;
	unsigned char athres;
	unsigned char clmd;
	unsigned char ln16;			/* no support */
	unsigned char dith;
	unsigned char rotation;		/* no support */
	unsigned char mirror;		/* no support */
} T_VSP_OUT;

/* SRU parameter */
typedef struct {
	unsigned char mode;
	unsigned char param;
	unsigned short enscl;
	unsigned char fxa;
	unsigned long connect;
} T_VSP_SRU;

/* UDS parameter */
typedef struct{
	unsigned char amd;
	unsigned char fmd;
	unsigned long filcolor;
	unsigned char clip;
	unsigned char alpha;
	unsigned char complement;
	unsigned char athres0;
	unsigned char athres1;
	unsigned char anum0;
	unsigned char anum1;
	unsigned char anum2;
	unsigned short x_ratio;
	unsigned short y_ratio;
	unsigned short x_stp;		/* no support */
	unsigned short x_edp;		/* no support */
	unsigned short y_stp;		/* no support */
	unsigned short y_edp;		/* no support */
	unsigned short in_cwidth;	/* no support */
	unsigned short in_cheight;	/* no support */
	unsigned short x_coffset;	/* no support */
	unsigned short y_coffset;	/* no support */
	unsigned short out_cwidth;
	unsigned short out_cheight;
	unsigned long connect;
} T_VSP_UDS;

/* LUT parameter */
typedef struct {
	unsigned long *lut;
	short size;
	unsigned char fxa;
	unsigned long connect;
} T_VSP_LUT;

/* CLU parameter */
typedef struct {
	unsigned char mode;
	unsigned long *clu_addr;
	unsigned long *clu_data;
	short size;
	unsigned char fxa;
	unsigned long connect;
} T_VSP_CLU;

/* HST parameter */
typedef struct {
	unsigned char fxa;
	unsigned long connect;
} T_VSP_HST;

/* HSI parameter */
typedef struct {
	unsigned char fxa;
	unsigned long connect;
} T_VSP_HSI;

/* BRU parameter */
typedef struct {
	unsigned short width;
	unsigned short height;
	unsigned short x_position;
	unsigned short y_position;
	unsigned char pwd;
	unsigned long color;
} T_VSP_BLEND_VIRTUAL;

typedef struct {
	unsigned char rbc;
	unsigned char crop;
	unsigned char arop;
	unsigned char blend_formula;
	unsigned char blend_coefx;
	unsigned char blend_coefy;
	unsigned char aformula;
	unsigned char acoefx;
	unsigned char acoefy;
	unsigned char acoefx_fix;
	unsigned char acoefy_fix;
} T_VSP_BLEND_CONTROL;

typedef struct {
	unsigned char crop;
	unsigned char arop;
} T_VSP_BLEND_ROP;

typedef struct {
	unsigned long lay_order;
	unsigned char adiv;
	unsigned char qnt[4];
	unsigned char dith[4];
	T_VSP_BLEND_VIRTUAL *blend_virtual;
	T_VSP_BLEND_CONTROL *blend_control_a;
	T_VSP_BLEND_CONTROL *blend_control_b;
	T_VSP_BLEND_CONTROL *blend_control_c;
	T_VSP_BLEND_CONTROL *blend_control_d;
	T_VSP_BLEND_ROP *blend_rop;
	unsigned long connect;
} T_VSP_BRU;

/* HGO parameter */
typedef struct {
	/* histogram detection window */
	void *addr;						/* use 768 bytes */
	unsigned short width;			/* horizontal size */
	unsigned short height;			/* vertical size */
	unsigned short x_offset;		/* horizontal offset */
	unsigned short y_offset;		/* vertical offset*/
	unsigned char binary_mode;		/* offset binary mode */
	unsigned char maxrgb_mode;		/* source component setting */
	unsigned short x_skip;			/* horizontal pixel skipping mode */
	unsigned short y_skip;			/* vertical pixel skipping mode */

	unsigned long sampling;			/* sampling module */
} T_VSP_HGO;

/* HGT parameter */
typedef struct {
	unsigned char lower;			/* lower boundary value */
	unsigned char upper;			/* upper boundary value */
} T_VSP_HUE_AREA;

typedef struct {
	/* histogram detection window */
	void *addr;						/* use 768 bytes */
	unsigned short width;			/* horizontal size */
	unsigned short height;			/* vertical size */
	unsigned short x_offset;		/* horizontal offset */
	unsigned short y_offset;		/* vertical offset*/
	unsigned short x_skip;			/* horizontal pixel skipping mode */
	unsigned short y_skip;			/* vertical pixel skipping mode */
	T_VSP_HUE_AREA area[6];			/* hue area */

	unsigned long sampling;			/* sampling module */
} T_VSP_HGT;

typedef struct {
	T_VSP_SRU *sru;					/* super-resolution */
	T_VSP_UDS *uds;					/* up down scaler */
	T_VSP_UDS *uds1;				/* up down scaler */
	T_VSP_UDS *uds2;				/* up down scaler */
	T_VSP_LUT *lut;					/* look up table */
	T_VSP_CLU *clu;					/* cubic look up table */
	T_VSP_HST *hst;					/* hue saturation value transform */
	T_VSP_HSI *hsi;					/* hue saturation value inverse */
	T_VSP_BRU *bru;					/* blend rop */
	T_VSP_HGO *hgo;					/* histogram generator-one */
	T_VSP_HGT *hgt;					/* histogram generator-two */
} T_VSP_CTRL;

typedef struct {
	unsigned char rpf_num;			/* RPF number */
	unsigned long rpf_order;		/* RPF order */
	unsigned long use_module;		/* using module */
	T_VSP_IN *src1_par;				/* source1 parameter */
	T_VSP_IN *src2_par;				/* source2 parameter */
	T_VSP_IN *src3_par;				/* source3 parameter */
	T_VSP_IN *src4_par;				/* source4 parameter */
	T_VSP_OUT *dst_par;				/* destination parameter */
	T_VSP_CTRL *ctrl_par;			/* module parameter */
} T_VSP_START;
#endif
