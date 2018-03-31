#ifndef _ASOC_STA_32X_H
#define _ASOC_STA_32X_H

/* STA326 register addresses */

#define STA33X_REGISTER_COUNT   0x2d
#define STA33X_COEF_COUNT 62

#define STA33X_CONFA    0x00
#define STA33X_CONFB    0x01
#define STA33X_CONFC    0x02
#define STA33X_CONFD    0x03
#define STA33X_CONFE    0x04
#define STA33X_CONFF    0x05
#define STA33X_MMUTE    0x06
#define STA33X_MVOL     0x07
#define STA33X_C1VOL    0x08
#define STA33X_C2VOL    0x09
#define STA33X_C3VOL    0x0a
#define STA33X_AUTO1    0x0b
#define STA33X_AUTO2    0x0c
#define STA33X_AUTO3    0x0d
#define STA33X_C1CFG    0x0e
#define STA33X_C2CFG    0x0f
#define STA33X_C3CFG    0x10
#define STA33X_TONE     0x11
#define STA33X_L1AR     0x12
#define STA33X_L1ATRT   0x13
#define STA33X_L2AR     0x14
#define STA33X_L2ATRT   0x15
#define STA33X_CFADDR2  0x16
#define STA33X_B1CF1    0x17
#define STA33X_B1CF2    0x18
#define STA33X_B1CF3    0x19
#define STA33X_B2CF1    0x1a
#define STA33X_B2CF2    0x1b
#define STA33X_B2CF3    0x1c
#define STA33X_A1CF1    0x1d
#define STA33X_A1CF2    0x1e
#define STA33X_A1CF3    0x1f
#define STA33X_A2CF1    0x20
#define STA33X_A2CF2    0x21
#define STA33X_A2CF3    0x22
#define STA33X_B0CF1    0x23
#define STA33X_B0CF2    0x24
#define STA33X_B0CF3    0x25
#define STA33X_CFUD     0x26
#define STA33X_MPCC1    0x27
#define STA33X_MPCC2    0x28

#define STA33X_FDRC1    0x2b
#define STA33X_FDRC2    0x2c

#define STA33X_DEVSTA   0x2d

#define STA33X_EQDRCCONFIG    0x31
#define STA33X_EXTEND_CONFIG   0x36



/* STA326 register field definitions */

/* 0x00 CONFA */
#define STA33X_CONFA_MCS_MASK   0x07
#define STA33X_CONFA_MCS_SHIFT  0
#define STA33X_CONFA_IR_MASK    0x18
#define STA33X_CONFA_IR_SHIFT   3
#define STA33X_CONFA_TWRB       0x20
#define STA33X_CONFA_TWAB       0x40
#define STA33X_CONFA_FDRB       0x80

/* 0x01 CONFB */
#define STA33X_CONFB_SAI_MASK   0x0f
#define STA33X_CONFB_SAI_SHIFT  0
#define STA33X_CONFB_SAIFB      0x10
#define STA33X_CONFB_DSCKE      0x20
#define STA33X_CONFB_C1IM       0x40
#define STA33X_CONFB_C2IM       0x80

/* 0x02 CONFC */
#define STA33X_CONFC_OM_MASK    0x03
#define STA33X_CONFC_OM_SHIFT   0
#define STA33X_CONFC_CSZ_MASK   0x7c
#define STA33X_CONFC_CSZ_SHIFT  2

/* 0x03 CONFD */
#define STA33X_CONFD_HPB        0x01
#define STA33X_CONFD_HPB_SHIFT  0
#define STA33X_CONFD_DEMP       0x02
#define STA33X_CONFD_DEMP_SHIFT 1
#define STA33X_CONFD_DSPB       0x04
#define STA33X_CONFD_DSPB_SHIFT 2
#define STA33X_CONFD_PSL        0x08
#define STA33X_CONFD_PSL_SHIFT  3
#define STA33X_CONFD_BQL        0x10
#define STA33X_CONFD_BQL_SHIFT  4
#define STA33X_CONFD_DRC        0x20
#define STA33X_CONFD_DRC_SHIFT  5
#define STA33X_CONFD_ZDE        0x40
#define STA33X_CONFD_ZDE_SHIFT  6
#define STA33X_CONFD_MME        0x80
#define STA33X_CONFD_MME_SHIFT  7

/* 0x04 CONFE */
#define STA33X_CONFE_MPCV       0x01
#define STA33X_CONFE_MPCV_SHIFT 0
#define STA33X_CONFE_MPC        0x02
#define STA33X_CONFE_MPC_SHIFT  1
#define STA33X_CONFE_AME        0x08
#define STA33X_CONFE_AME_SHIFT  3
#define STA33X_CONFE_PWMS       0x10
#define STA33X_CONFE_PWMS_SHIFT 4
#define STA33X_CONFE_ZCE        0x40
#define STA33X_CONFE_ZCE_SHIFT  6
#define STA33X_CONFE_SVE        0x80
#define STA33X_CONFE_SVE_SHIFT  7

/* 0x05 CONFF */
#define STA33X_CONFF_OCFG_MASK  0x03
#define STA33X_CONFF_OCFG_SHIFT 0
#define STA33X_CONFF_IDE        0x04
#define STA33X_CONFF_IDE_SHIFT  3
#define STA33X_CONFF_BCLE       0x08
#define STA33X_CONFF_ECLE       0x20
#define STA33X_CONFF_PWDN       0x40
#define STA33X_CONFF_EAPD       0x80

/* 0x06 MMUTE */
#define STA33X_MMUTE_MMUTE      0x01

/* 0x0b AUTO1 */
#define STA33X_AUTO1_AMGC_MASK  0x30
#define STA33X_AUTO1_AMGC_SHIFT 4

/* 0x0c AUTO2 */
#define STA33X_AUTO2_AMAME      0x01
#define STA33X_AUTO2_AMAM_MASK  0x0e
#define STA33X_AUTO2_AMAM_SHIFT 1
#define STA33X_AUTO2_XO_MASK    0xf0
#define STA33X_AUTO2_XO_SHIFT   4

/* 0x0e 0x0f 0x10 CxCFG */
#define STA33X_CxCFG_TCB        0x01    /* only C1 and C2 */
#define STA33X_CxCFG_TCB_SHIFT  0
#define STA33X_CxCFG_EQBP       0x02    /* only C1 and C2 */
#define STA33X_CxCFG_EQBP_SHIFT 1
#define STA33X_CxCFG_VBP        0x03
#define STA33X_CxCFG_VBP_SHIFT  2
#define STA33X_CxCFG_BO         0x04
#define STA33X_CxCFG_LS_MASK    0x30
#define STA33X_CxCFG_LS_SHIFT   4
#define STA33X_CxCFG_OM_MASK    0xc0
#define STA33X_CxCFG_OM_SHIFT   6

/* 0x11 TONE */
#define STA33X_TONE_BTC_SHIFT   0
#define STA33X_TONE_TTC_SHIFT   4

/* 0x12 0x13 0x14 0x15 limiter attack/release */
#define STA33X_LxA_SHIFT        0
#define STA33X_LxR_SHIFT        4

/* 0x26 CFUD */
#define STA33X_CFUD_W1          0x01
#define STA33X_CFUD_WA          0x02
#define STA33X_CFUD_R1          0x04
#define STA33X_CFUD_RA          0x08

#define STA33X_2_0_HP_Config	0x10
#define STA33X_2_1_SE_Config	0x01
#define STA33X_2_1_BTL_Config	0x02
#define STA33X_0_1_Mono_Config	0x03

/* biquad filter coefficient table offsets */
#define STA33X_C1_BQ_BASE       0
#define STA33X_C2_BQ_BASE       20
#define STA33X_CH_BQ_NUM        4
#define STA33X_BQ_NUM_COEF      5
#define STA33X_XO_HP_BQ_BASE    40
#define STA33X_XO_LP_BQ_BASE    45
#define STA33X_C1_PRESCALE      50
#define STA33X_C2_PRESCALE      51
#define STA33X_C1_POSTSCALE     52
#define STA33X_C2_POSTSCALE     53
#define STA33X_C3_POSTSCALE     54
#define STA33X_TW_POSTSCALE     55
#define STA33X_C1_MIX1          56
#define STA33X_C1_MIX2          57
#define STA33X_C2_MIX1          58
#define STA33X_C2_MIX2          59
#define STA33X_C3_MIX1          60
#define STA33X_C3_MIX2          61
#endif


