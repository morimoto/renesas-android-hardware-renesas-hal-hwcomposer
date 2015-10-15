/*
 * Renesas Proprietary Information.
 * The information contained herein is confidential property of
 * Renesas Electronics Corporation
 *
 * Copyright (C) Renesas Electronics Corporation 2013 All rights reserved.
 */

#ifndef __FDPM_PUBLIC_H__
#define __FDPM_PUBLIC_H__

typedef void (*PFN_FDPM_COMPLETE_CALLBACK)(unsigned long uwUserData);

#define R_FDPM_OK (0)
#define R_FDPM_NG (-1)

/* drv_FDPM_Init */
#define E_FDP_PARA_INTLVL        (-100)
#define E_FDP_DEF_INH            (-101)

/* drv_FDPM_Open */
#define E_FDP_PARA_CB3           (-200)
#define E_FDP_PARA_CB4           (-201)
#define E_FDP_PARA_OPENPAR       (-202)
#define E_FDP_PARA_REFBUFMODE    (-203)
#define E_FDP_PARA_BUFREF0       (-204)
#define E_FDP_PARA_BUFREF1       (-205)
#define E_FDP_PARA_BUFREF2       (-206)
#define E_FDP_PARA_BUFREFPRG     (-207)
#define E_FDP_PARA_VMODE         (-208)
#define E_FDP_PARA_CLKMODE       (-209)
#define E_FDP_PARA_OCMODE        (-210)
#define E_FDP_PARA_INSIZE        (-211)

#define E_FDP_PARA_CB1           (-250)
#define E_FDP_PARA_CB2           (-251)
#define E_FDP_PARA_VCNT          (-252)
#define E_FDP_PARA_REFBUF        (-253)
#define E_FDP_PARA_BUFADDR       (-254)
#define E_FDP_PARA_BUFADDRC      (-255)
#define E_FDP_PARA_BUFADDRC1     (-256)
#define E_FDP_PARA_BUFSTRIDE     (-257)
#define E_FDP_PARA_BUFHEIGHT     (-258)
#define E_FDP_PARA_BUFHEIGHTC    (-259)

/* drv_FDPM_Start */
#define E_FDP_PARA_STARTPAR      (-300)
#define E_FDP_PARA_FDPGO         (-301)
#define E_FDP_PARA_FPROCPAR      (-302)
#define E_FDP_PARA_SEQPAR        (-303)
#define E_FDP_PARA_IMGSETPAR     (-304)
#define E_FDP_PARA_INPIC         (-305)
#define E_FDP_PARA_OUTBUF        (-306)
#define E_FDP_PARA_SEQMODE       (-307)
#define E_FDP_PARA_TELECINEMODE  (-308)
#define E_FDP_PARA_INWIDTH       (-309)
#define E_FDP_PARA_INHEIGHT      (-310)
#define E_FDP_PARA_PICPAR        (-311)
#define E_FDP_PARA_INBUF1        (-312)
#define E_FDP_PARA_INBUF2        (-313)
#define E_FDP_PARA_PICWIDTH      (-314)
#define E_FDP_PARA_PICHEIGHT     (-315)
#define E_FDP_PARA_CHROMA        (-316)
#define E_FDP_PARA_PROGSEQ       (-317)
#define E_FDP_PARA_PICSTRUCT     (-318)
#define E_FDP_PARA_REPEATTOP     (-319)
#define E_FDP_PARA_BUFREFWR      (-320)
#define E_FDP_PARA_BUFREFRD0     (-321)
#define E_FDP_PARA_BUFREFRD1     (-322)
#define E_FDP_PARA_BUFREFRD2     (-323)
#define E_FDP_PARA_BUFIIRWR      (-324)
#define E_FDP_PARA_BUFIIRRD      (-325)
#define E_FDP_PARA_SEQOVERLAP    (-326)
#define E_FDP_PARA_FIELD_PARITY  (-327)
#define E_FDP_PARA_STATUS        (-328)
#define E_FDP_PARA_LASTSTART     (-329)
#define E_FDP_PARA_CF            (-330)
#define E_FDP_PARA_FDECODE       (-331)
#define E_FDP_PARA_OUTFORMAT     (-332)

/* drv_FDPM_Cancel */
#define E_FDP_CANCEL_NOID        (-400)
#define E_FDP_CANCEL_ID_PROCESSING (-401)

/* timer */
#define E_FDP_TIMER_CB           (-80)
#define E_FDP_TIMER_TO           (-81)

/* #define TO_VCNT    (10*10000) */
#define TO_VCNT    (1*10000)

#endif
