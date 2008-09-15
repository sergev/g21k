/* register identifiers */

/************************************************** Universal Registers */
/*  Note: The ordering of these register definitions is significant     */


#ifdef ADSPZ3
/* SPORT pseudo-registers used by the z3 simulator core to emulate
   various bit-fields of actual IOP registers. */

#define REG_MCE0        0x5a
#define REG_ISCLK0      0x5b
#define REG_RFSR0       0x5c
#define REG_RFSW0       0x5d
#define REG_TFSR0       0x5e
#define REG_TFSW0       0x5f
#define REG_ITFS0       0x60
#define REG_IRFS0       0x61
#define REG_INVTFS0     0x62
#define REG_INVRFS0     0x63
#define REG_DTYPE0      0x64
#define REG_SLEN0       0x65
#define REG_SCLKDIV0    0x66
#define REG_RFSDIV0     0x67
#define REG_TIREG0      0x68
#define REG_TMREG0      0x69
#define REG_RIREG0      0x6a
#define REG_RMREG0      0x6b
#define REG_TBUF0       0x6c
#define REG_RBUF0       0x6d
#define REG_ISCLK1      0x6e
#define REG_RFSR1       0x6f
#define REG_RFSW1       0x70
#define REG_TFSR1       0x71
#define REG_TFSW1       0x72
#define REG_ITFS1       0x73
#define REG_IRFS1       0x74
#define REG_INVTFS1     0x75
#define REG_INVRFS1     0x76
#define REG_DTYPE1      0x77
#define REG_SLEN1       0x78
#define REG_SCLKDIV1    0x79
#define REG_RFSDIV1     0x7a
#define REG_TIREG1      0x7b
#define REG_TMREG1      0x7c
#define REG_RIREG1      0x7d
#define REG_RMREG1      0x7e
#define REG_TBUF1       0x7f
#define REG_RBUF1       0x80
#define REG_MCRE1       0x81
#define REG_MCRE0       0x82
#define REG_MCTE1       0xf4
#define REG_MCTE0       0xf5
#define REG_MCFD        0xe8
#define REG_T0          0x31
#define REG_T1          0x33


/* IOP registers... */

/* primary control registers */
#define IOP_SYSCON  0x00
#define IOP_VIRPT   0x01
#define IOP_WAIT    0x02
#define IOP_SYSTAT  0x03

/* external port DMA fifos (six-deep each) */
#define IOP_EPB0    0x04
#define IOP_EPB1    0x05
#define IOP_EPB2    0x06
#define IOP_EPB3    0x07

/* message registers */
#define IOP_MSGR0   0x08
#define IOP_MSGR1   0x09
#define IOP_MSGR2   0x0a
#define IOP_MSGR3   0x0b
#define IOP_MSGR4   0x0c
#define IOP_MSGR5   0x0d
#define IOP_MSGR6   0x0e
#define IOP_MSGR7   0x0f

/* bus timeout registers */
#define IOP_BMAX    0x18
#define IOP_BCNT    0x19

/* last external bank0 address */
#define IOP_ELAST   0x1b

/* DMA control registers */
#define IOP_DMAC6   0x1c
#define IOP_DMAC7   0x1d
#define IOP_DMAC8   0x1e
#define IOP_DMAC9   0x1f

/* DMA channel 4 registers (LBUF2) */
#define IOP_II4     0x30
#define IOP_IM4     0x31
#define IOP_C4      0x32
#define IOP_CP4     0x33
#define IOP_GP4     0x34
#define IOP_DB4     0x35
#define IOP_DA4     0x36

/* DMA status register */
#define IOP_DMASTAT 0x37

/* DMA5 registers (LBUF3) */
#define IOP_II5     0x38
#define IOP_IM5     0x39
#define IOP_C5      0x3a
#define IOP_CP5     0x3b
#define IOP_GP5     0x3c
#define IOP_DB5     0x3d
#define IOP_DA5     0x3e

/* DMA6 registers (EPB0/LBUF4) */
#define IOP_II6     0x40
#define IOP_IM6     0x41
#define IOP_C6      0x42
#define IOP_CP6     0x43
#define IOP_GP6     0x44
#define IOP_EI6     0x45
#define IOP_EM6     0x46
#define IOP_EC6     0x47

/* DMA7 registers (EPB1/LBUF5) */
#define IOP_II7     0x48
#define IOP_IM7     0x49
#define IOP_C7      0x4a
#define IOP_CP7     0x4b
#define IOP_GP7     0x4c
#define IOP_EI7     0x4d
#define IOP_EM7     0x4e
#define IOP_EC7     0x4f

/* DMA8 registers (EPB2) */
#define IOP_II8     0x50
#define IOP_IM8     0x51
#define IOP_C8      0x52
#define IOP_CP8     0x53
#define IOP_GP8     0x54
#define IOP_EI8     0x55
#define IOP_EM8     0x56
#define IOP_EC8     0x57

/* DMA9 registers (EPB3) */
#define IOP_II9     0x58
#define IOP_IM9     0x59
#define IOP_C9      0x5a
#define IOP_CP9     0x5b
#define IOP_GP9     0x5c
#define IOP_EI9     0x5d
#define IOP_EM9     0x5e
#define IOP_EC9     0x5f

/* DMA0 registers (SPORT0-RX) */
#define IOP_II0     0x60
#define IOP_IM0     0x61
#define IOP_C0      0x62
#define IOP_CP0     0x63
#define IOP_GP0     0x64
#define IOP_DB0     0x65
#define IOP_DA0     0x66

/* DMA1 registers (SPORT1-RX/LBUF0) */
#define IOP_II1     0x68
#define IOP_IM1     0x69
#define IOP_C1      0x6a
#define IOP_CP1     0x6b
#define IOP_GP1     0x6c
#define IOP_DB1     0x6d
#define IOP_DA1     0x6e

/* DMA2 registers (SPORT0-TX) */
#define IOP_II2     0x70
#define IOP_IM2     0x71
#define IOP_C2      0x72
#define IOP_CP2     0x73
#define IOP_GP2     0x74
#define IOP_DB2     0x75
#define IOP_DA2     0x76

/* DMA3 registers (SPORT1-TX/LBUF1) */
#define IOP_II3     0x78
#define IOP_IM3     0x79
#define IOP_C3      0x7a
#define IOP_CP3     0x7b
#define IOP_GP3     0x7c
#define IOP_DB3     0x7d
#define IOP_DA3     0x7e

/* Emulation HWBP Bounds Registers */
#define IOP_IOAS    0xac
#define IOP_IOAE    0xad
#define IOP_EPAS    0xae
#define IOP_EPAE    0xaf

/* link buffer DMA fifos (two-deep each) */
#define IOP_LBUF0   0xc0
#define IOP_LBUF1   0xc1
#define IOP_LBUF2   0xc2
#define IOP_LBUF3   0xc3
#define IOP_LBUF4   0xc4
#define IOP_LBUF5   0xc5

/* link port control registers */
#define IOP_LCTL    0xc6
#define IOP_LCOM    0xc7
#define IOP_LAR     0xc8
#define IOP_LSRQ    0xc9
#define IOP_LPATH1  0xca
#define IOP_LPATH2  0xcb
#define IOP_LPATH3  0xcc
#define IOP_LPCNT   0xcd
#define IOP_CNST1   0xce
#define IOP_CNST2   0xcf

/* SPORT0 control registers */
#define IOP_STCTL0  0xe0
#define IOP_SRCTL0  0xe1
#define IOP_TX0     0xe2  /* two-deep fifo */
#define IOP_RX0     0xe3  /* two-deep fifo */
#define IOP_TDIV0   0xe4
#define IOP_TCNT0   0xe5
#define IOP_RDIV0   0xe6
#define IOP_RCNT0   0xe7
#define IOP_MTCS0   0xe8
#define IOP_MRCS0   0xe9
#define IOP_MTCCS0  0xea
#define IOP_MRCCS0  0xeb
#define IOP_SPATH0  0xee
#define IOP_SPCNT0  0xef

/* SPORT1 control registers */
#define IOP_STCTL1  0xf0
#define IOP_SRCTL1  0xf1
#define IOP_TX1     0xf2 /* two-deep fifo */
#define IOP_RX1     0xf3 /* two-deep fifo */
#define IOP_TDIV1   0xf4
#define IOP_TCNT1   0xf5
#define IOP_RDIV1   0xf6
#define IOP_RCNT1   0xf7
#define IOP_MTCS1   0xf8
#define IOP_MRCS1   0xf9
#define IOP_MTCCS1  0xfa
#define IOP_MRCCS1  0xfb
#define IOP_SPATH1  0xfe
#define IOP_SPCNT1  0xff

#endif /* z3 */


/* universal registers... */
#define REG_R0    (0x0)
#define REG_R1    (0x1)
#define REG_R2    (0x2)
#define REG_R3    (0x3)
#define REG_R4    (0x4)
#define REG_R5    (0x5)
#define REG_R6    (0x6)
#define REG_R7    (0x7)
#define REG_R8    (0x8)
#define REG_R9    (0x9)
#define REG_R10   (0xA)
#define REG_R11   (0xB)
#define REG_R12   (0xC)
#define REG_R13   (0xD)
#define REG_R14   (0xE)
#define REG_R15   (0xF)

#define REG_F0    (0x0)  /* These F registers should be identical to the R values */
#define REG_F1    (0x1)
#define REG_F2    (0x2)
#define REG_F3    (0x3)
#define REG_F4    (0x4)
#define REG_F5    (0x5)
#define REG_F6    (0x6)
#define REG_F7    (0x7)
#define REG_F8    (0x8)
#define REG_F9    (0x9)
#define REG_F10   (0xA)
#define REG_F11   (0xB)
#define REG_F12   (0xC)
#define REG_F13   (0xD)
#define REG_F14   (0xE)
#define REG_F15   (0xF)

#define REG_I0    (0x10)
#define REG_I1    (0x11)
#define REG_I2    (0x12)
#define REG_I3    (0x13)
#define REG_I4    (0x14)
#define REG_I5    (0x15)
#define REG_I6    (0x16)
#define REG_I7    (0x17)
#define REG_I8    (0x18)
#define REG_I9    (0x19)
#define REG_I10   (0x1A)
#define REG_I11   (0x1B)
#define REG_I12   (0x1C)
#define REG_I13   (0x1D)
#define REG_I14   (0x1E)
#define REG_I15   (0x1F)

#define REG_M0    (0x20)
#define REG_M1    (0x21)
#define REG_M2    (0x22)
#define REG_M3    (0x23)
#define REG_M4    (0x24)
#define REG_M5    (0x25)
#define REG_M6    (0x26)
#define REG_M7    (0x27)
#define REG_M8    (0x28)
#define REG_M9    (0x29)
#define REG_M10   (0x2A)
#define REG_M11   (0x2B)
#define REG_M12   (0x2C)
#define REG_M13   (0x2D)
#define REG_M14   (0x2E)
#define REG_M15   (0x2F)

#define REG_L0    (0x30)
#define REG_L1    (0x31)
#define REG_L2    (0x32)
#define REG_L3    (0x33)
#define REG_L4    (0x34)
#define REG_L5    (0x35)
#define REG_L6    (0x36)
#define REG_L7    (0x37)
#define REG_L8    (0x38)
#define REG_L9    (0x39)
#define REG_L10   (0x3A)
#define REG_L11   (0x3B)
#define REG_L12   (0x3C)
#define REG_L13   (0x3D)
#define REG_L14   (0x3E)
#define REG_L15   (0x3F)

#define REG_B0    (0x40)
#define REG_B1    (0x41)
#define REG_B2    (0x42)
#define REG_B3    (0x43)
#define REG_B4    (0x44)
#define REG_B5    (0x45)
#define REG_B6    (0x46)
#define REG_B7    (0x47)
#define REG_B8    (0x48)
#define REG_B9    (0x49)
#define REG_B10   (0x4A)
#define REG_B11   (0x4B)
#define REG_B12   (0x4C)
#define REG_B13   (0x4D)
#define REG_B14   (0x4E)
#define REG_B15   (0x4F)

#define REG_FADDR    (0x60)
#define REG_DADDR    (0x61)
#define REG_PC       (0x63)
#define REG_PCSTK    (0x64)
#define REG_PCSTKP   (0x65)
#define REG_LADDR    (0x66)
#define REG_CURLCNTR (0x67)
#define REG_LCNTR    (0x68)

#define REG_USTAT1   (0x70)
#define REG_USTAT2   (0x71)
#define REG_IRPTL    (0x79)
#define REG_MODE2    (0x7A)
#define REG_MODE1    (0x7B)
#define REG_ASTAT    (0x7C)
#define REG_IMASK    (0x7D)
#define REG_STKY     (0x7E)
#define REG_IMASKP   (0x7F)

/* holdover form original 21000 design: never implimented */
#ifdef UNIMPLIMENTED
#define REG_DAI1     (0x90)
#define REG_DAI2     (0x91)
#define REG_DAL1     (0xB0)
#define REG_DAL2     (0xB1)
#define REG_DAB1     (0xC0)
#define REG_DAB2     (0xC1)
#endif

#define REG_PSA1S    (0xD0) /* hwbd registers */
#define REG_PSA1E    (0xD1)
#define REG_PSA2S    (0xD2)
#define REG_PSA2E    (0xD3)
#define REG_PSA3S    (0xD4)
#define REG_PSA3E    (0xD5)
#define REG_PSA4S    (0xD6)
#define REG_PSA4E    (0xD7)

#define REG_EMUCLK   (0xD9)

#define REG_EMUCLK2  (0xD8)  /* double-precision cycle counter for z3 */
#define REG_EMUN     (0xDA)  /* hwbp hit-count register */

#define REG_PX       (0xDB)
#define REG_PX1      (0xDC)
#define REG_PX2      (0xDD)
#define REG_TPERIOD  (0xDE)
#define REG_TCOUNT   (0xDF)

/* Although the various PM/DM-WAIT/BANK registers are not present on
the z3, the z3 simulator still uses them: be careful.  The z3 emulator
will use the new IOP WAIT register.  */
#define REG_PMWAIT   (0xE0)
#define REG_PMBANK1  (0xE1)
#define REG_PMADR    (0xE2)

#define REG_PMDAS    (0xE3)  /* hwbp registers */
#define REG_PMDAE    (0xE4)

/* Full-featured emulator support (pm overlay) on the '020 only */
#ifndef ADSPZ3
#define REG_POVL0    (0xE5)
#define REG_POVL1    (0xE6)
#endif

#define REG_DMWAIT   (0xF0)
#define REG_DMBANK1  (0xF1)
#define REG_DMBANK2  (0xF2)
#define REG_DMBANK3  (0xF3)

#ifdef ADSPZ3
#define REG_UNBANK   (0xE0)
#endif /* ADSPZ3 */

#define REG_DMADR    (0xF4)

#define REG_DMA1S    (0xF5)  /* hwbp registers */
#define REG_DMA1E    (0xF6)
#define REG_DMA2S    (0xF7)
#define REG_DMA2E    (0xF8)

/* Full-featured emulator support (pm overlay) on the '020 only */
#ifndef ADSPZ3
#define REG_DOVL     (0xF9)
#endif
                                                        
#define REG_UNIV_ALT_START  (0xFA)                 /* These define the alternate regs */

#define REG_ALT_R0     (0x0 + REG_UNIV_ALT_START)
#define REG_ALT_R1     (0x1 + REG_UNIV_ALT_START)
#define REG_ALT_R2     (0x2 + REG_UNIV_ALT_START)
#define REG_ALT_R3     (0x3 + REG_UNIV_ALT_START)
#define REG_ALT_R4     (0x4 + REG_UNIV_ALT_START)
#define REG_ALT_R5     (0x5 + REG_UNIV_ALT_START)
#define REG_ALT_R6     (0x6 + REG_UNIV_ALT_START)
#define REG_ALT_R7     (0x7 + REG_UNIV_ALT_START)
#define REG_ALT_R8     (0x8 + REG_UNIV_ALT_START)
#define REG_ALT_R9     (0x9 + REG_UNIV_ALT_START)
#define REG_ALT_R10    (0xa + REG_UNIV_ALT_START)
#define REG_ALT_R11    (0xb + REG_UNIV_ALT_START)
#define REG_ALT_R12    (0xc + REG_UNIV_ALT_START)
#define REG_ALT_R13    (0xd + REG_UNIV_ALT_START)
#define REG_ALT_R14    (0xe + REG_UNIV_ALT_START)
#define REG_ALT_R15    (0xf + REG_UNIV_ALT_START)

#define REG_ALT_F0     (0x0 + REG_UNIV_ALT_START)
#define REG_ALT_F1     (0x1 + REG_UNIV_ALT_START)
#define REG_ALT_F2     (0x2 + REG_UNIV_ALT_START)
#define REG_ALT_F3     (0x3 + REG_UNIV_ALT_START)
#define REG_ALT_F4     (0x4 + REG_UNIV_ALT_START)
#define REG_ALT_F5     (0x5 + REG_UNIV_ALT_START)
#define REG_ALT_F6     (0x6 + REG_UNIV_ALT_START)
#define REG_ALT_F7     (0x7 + REG_UNIV_ALT_START)
#define REG_ALT_F8     (0x8 + REG_UNIV_ALT_START)
#define REG_ALT_F9     (0x9 + REG_UNIV_ALT_START)
#define REG_ALT_F10    (0xa + REG_UNIV_ALT_START)
#define REG_ALT_F11    (0xb + REG_UNIV_ALT_START)
#define REG_ALT_F12    (0xc + REG_UNIV_ALT_START)
#define REG_ALT_F13    (0xd + REG_UNIV_ALT_START)
#define REG_ALT_F14    (0xe + REG_UNIV_ALT_START)
#define REG_ALT_F15    (0xf + REG_UNIV_ALT_START)

#define REG_ALT_I0     (0x10 + REG_UNIV_ALT_START)
#define REG_ALT_I1     (0x11 + REG_UNIV_ALT_START)
#define REG_ALT_I2     (0x12 + REG_UNIV_ALT_START)
#define REG_ALT_I3     (0x13 + REG_UNIV_ALT_START)
#define REG_ALT_I4     (0x14 + REG_UNIV_ALT_START)
#define REG_ALT_I5     (0x15 + REG_UNIV_ALT_START)
#define REG_ALT_I6     (0x16 + REG_UNIV_ALT_START)
#define REG_ALT_I7     (0x17 + REG_UNIV_ALT_START)
#define REG_ALT_I8     (0x18 + REG_UNIV_ALT_START)
#define REG_ALT_I9     (0x19 + REG_UNIV_ALT_START)
#define REG_ALT_I10    (0x1a + REG_UNIV_ALT_START)
#define REG_ALT_I11    (0x1b + REG_UNIV_ALT_START)
#define REG_ALT_I12    (0x1c + REG_UNIV_ALT_START)
#define REG_ALT_I13    (0x1d + REG_UNIV_ALT_START)
#define REG_ALT_I14    (0x1e + REG_UNIV_ALT_START)
#define REG_ALT_I15    (0x1f + REG_UNIV_ALT_START)

#define REG_ALT_M0     (0x20 + REG_UNIV_ALT_START)
#define REG_ALT_M1     (0x21 + REG_UNIV_ALT_START)
#define REG_ALT_M2     (0x22 + REG_UNIV_ALT_START)
#define REG_ALT_M3     (0x23 + REG_UNIV_ALT_START)
#define REG_ALT_M4     (0x24 + REG_UNIV_ALT_START)
#define REG_ALT_M5     (0x25 + REG_UNIV_ALT_START)
#define REG_ALT_M6     (0x26 + REG_UNIV_ALT_START)
#define REG_ALT_M7     (0x27 + REG_UNIV_ALT_START)
#define REG_ALT_M8     (0x28 + REG_UNIV_ALT_START)
#define REG_ALT_M9     (0x29 + REG_UNIV_ALT_START)
#define REG_ALT_M10    (0x2a + REG_UNIV_ALT_START)
#define REG_ALT_M11    (0x2b + REG_UNIV_ALT_START)
#define REG_ALT_M12    (0x2c + REG_UNIV_ALT_START)
#define REG_ALT_M13    (0x2d + REG_UNIV_ALT_START)
#define REG_ALT_M14    (0x2e + REG_UNIV_ALT_START)
#define REG_ALT_M15    (0x2f + REG_UNIV_ALT_START)

#define REG_ALT_L0     (0x30 + REG_UNIV_ALT_START)
#define REG_ALT_L1     (0x31 + REG_UNIV_ALT_START)
#define REG_ALT_L2     (0x32 + REG_UNIV_ALT_START)
#define REG_ALT_L3     (0x33 + REG_UNIV_ALT_START)
#define REG_ALT_L4     (0x34 + REG_UNIV_ALT_START)
#define REG_ALT_L5     (0x35 + REG_UNIV_ALT_START)
#define REG_ALT_L6     (0x36 + REG_UNIV_ALT_START)
#define REG_ALT_L7     (0x37 + REG_UNIV_ALT_START)
#define REG_ALT_L8     (0x38 + REG_UNIV_ALT_START)
#define REG_ALT_L9     (0x39 + REG_UNIV_ALT_START)
#define REG_ALT_L10    (0x3a + REG_UNIV_ALT_START)
#define REG_ALT_L11    (0x3b + REG_UNIV_ALT_START)
#define REG_ALT_L12    (0x3c + REG_UNIV_ALT_START)
#define REG_ALT_L13    (0x3d + REG_UNIV_ALT_START)
#define REG_ALT_L14    (0x3e + REG_UNIV_ALT_START)
#define REG_ALT_L15    (0x3f + REG_UNIV_ALT_START)

#define REG_ALT_B0     (0x40 + REG_UNIV_ALT_START)
#define REG_ALT_B1     (0x41 + REG_UNIV_ALT_START)
#define REG_ALT_B2     (0x42 + REG_UNIV_ALT_START)
#define REG_ALT_B3     (0x43 + REG_UNIV_ALT_START)
#define REG_ALT_B4     (0x44 + REG_UNIV_ALT_START)
#define REG_ALT_B5     (0x45 + REG_UNIV_ALT_START)
#define REG_ALT_B6     (0x46 + REG_UNIV_ALT_START)
#define REG_ALT_B7     (0x47 + REG_UNIV_ALT_START)
#define REG_ALT_B8     (0x48 + REG_UNIV_ALT_START)
#define REG_ALT_B9     (0x49 + REG_UNIV_ALT_START)
#define REG_ALT_B10    (0x4a + REG_UNIV_ALT_START)
#define REG_ALT_B11    (0x4b + REG_UNIV_ALT_START)
#define REG_ALT_B12    (0x4c + REG_UNIV_ALT_START)
#define REG_ALT_B13    (0x4d + REG_UNIV_ALT_START)
#define REG_ALT_B14    (0x4e + REG_UNIV_ALT_START)
#define REG_ALT_B15    (0x4f + REG_UNIV_ALT_START)

/********************************************* Computational Unit Registers */
/*  Note: The ordering of these register definitions is significant     */

#define REG_MR0F     (0x0)
#define REG_MR1F     (0x1)
#define REG_MR2F     (0x2)
#define REG_MR0B     (0x4)
#define REG_MR1B     (0x5)
#define REG_MR2B     (0x6)
#define REG_CU_MAX   (0x7)

/******************************************************* Cache Registers */
/*  Note: The ordering of these register definitions is significant     */

#define REG_CH        (0x0)
#define REG_CL        (0x1)
#define REG_CA        (0x2)
#define REG_CACHE_MAX (0x3)


