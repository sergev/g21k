#ifndef _X1REGS_H
#define _X1REGS_H
/*
   THE REG ENUM spelling must be the same as actual register name prepended by
   REG_ hence we can redefine

   SETr (x, y) (regs[CONCAT(REG_,x)])

   we need to be able to perform this mapping with the C-preprocess
   or. */

typedef enum regt {
  REG_AX0, REG_AX1, REG_MX0, REG_MX1, REG_AY0, REG_AY1, REG_MY0,
  REG_MY1, REG_MR2, REG_SR2, REG_AR, REG_SI, REG_MR1, REG_SR1,
  REG_MR0, REG_SR0, REG_I0, REG_I1, REG_I2, REG_I3, REG_M0, REG_M1,
  REG_M2, REG_M3, REG_L0, REG_L1, REG_L2, REG_L3, REG_IMASK,
  REG_IRPTL, REG_ICNTL, REG_STACKA, REG_I4, REG_I5, REG_I6, REG_I7,
  REG_M4, REG_M5, REG_M6, REG_M7, REG_L4, REG_L5, REG_L6, REG_L7,
  REG_TX0, REG_TX1, REG_CNTR, REG_OWRCNTR, REG_ASTAT, REG_MSTAT,
  REG_SSTAT, REG_FLAGS, REG_CCODE, REG_SE, REG_SB, REG_PX, REG_DMPG1,
  REG_DMPG2, REG_IOPG, REG_IJPG, REG_RX0, REG_RX1, REG_DX, REG_STACKP,
  REG_AF, REG_MR, REG_SR,
  
  REG_PC, REG_FADDR, REG_DADDR, 
  REG_EMUSTAT, REG_EMUCTL, REG_CYCLES, REG_DATAB, REG_ADDRB,
  STACK_PC, STACK_STS, STACK_LOOP, STACK_CNTR, STACK_LPTERM, LATCH_IRPTL, REG_IMASKPTR,


/* generated from (ac0)c0ioregs.gperf */
  SYS_B0, SYS_B1, SYS_B2, SYS_B3, 
  SYS_B4, SYS_B5, SYS_B6, SYS_B7, 
  SYS_SYSCNTL, SYS_MEMWAIT, SYS_MEMMODE, SYS_LPTERM, 
  SYS_SPORTPAGE, SYS_DMAPAGE, SYS_DMAPAGE1, SYS_BOOTPAGE, 
  SYS_STCTL0, SYS_SRCTL0, SYS_ABFCTL0, SYS_MCCTL0, 
  SYS_TDIV0, SYS_TCNT0, SYS_RDIV0, SYS_RCNT0, 
  SYS_RFSDIV0, SYS_RFSCNT0, SYS_MTCSL0, SYS_MTCSM0, 
  SYS_MRCSL0, SYS_MRCSM0, SYS_MCCSL0, SYS_MCCSM0, 
  SYS_STCTL1, SYS_SRCTL1, SYS_ABFCTL1, SYS_MCCTL1, 
  SYS_TDIV1, SYS_TCNT1, SYS_RDIV1, SYS_RCNT1, 
  SYS_RFSDIV1, SYS_RFSCNT1, SYS_MTCSL1, SYS_MTCSM1, 
  SYS_MRCSL1, SYS_MRCSM1, SYS_MCCSL1, SYS_MCCSM1, 
  SYS_TPERIOD, SYS_TCOUNT, SYS_TSCALE, SYS_TSCALECNT, 
  SYS_EMUCLK1, SYS_EMUCLK2, SYS_EMUCLK3, SYS_TX0ADDR, 
  SYS_TX0NXTADDR, SYS_TX0CNT, SYS_TX0CURCNT, SYS_RX0ADDR, 
  SYS_RX0NXTADDR, SYS_RX0CNT, SYS_RX0CURCNT, SYS_TX1ADDR, 
  SYS_TX1NXTADDR, SYS_TX1CNT, SYS_TX1CURCNT, SYS_RX1ADDR, 
  SYS_RX1NXTADDR, SYS_RX1CNT, SYS_RX1CURCNT, SYS_DMAADDR, 
  SYS_DMANXTADDR, SYS_DMACNT, SYS_DMACURCNT, SYS_DMA1ADDR, 
  SYS_DMA1NXTADDR, SYS_DMA1CNT, SYS_DMA1CURCNT, SYS_DMA2ADDR, 
  SYS_DMA2NXTADDR, SYS_DMA2CNT, SYS_DMA2CURCNT, SYS_DMA3ADDR, 
  SYS_DMA3NXTADDR, SYS_DMA3CNT, SYS_DMA3CURCNT, SYS_CADIS, 
  SYS_CAENA, SYS_A0H, SYS_A0L, SYS_A1H, 
  SYS_A1L, SYS_D0H, SYS_D0L, SYS_D1H, 
  SYS_D1L, SYS_SET0, SYS_SET1, SYS_SET2, 
  SYS_SET3, SYS_SET4, SYS_SET5, SYS_SET6, 
  SYS_SET7, SYS_SET8, SYS_SET9, SYS_SET10, 
  SYS_SET11, SYS_SET12, SYS_SET13, SYS_SET14, 
  SYS_SET15, SYS_SET16, SYS_SET17, SYS_SET18, 
  SYS_SET19, SYS_SET20, SYS_SET21, SYS_SET22, 
  SYS_SET23, SYS_SET24, SYS_SET25, SYS_SET26, 
  SYS_SET27, SYS_SET28, SYS_SET29, SYS_SET30, 
  SYS_SET31, 

  LASTREG
} reg;


#define REG2TAG(x) CONCAT (REG_,x)

/* @section System register Definitions.

    Memory-mapped Registers
    -----------------------

A start prefix means that this registers functionality has been
implemented in the simulator.

ADDR  REGISTER    FUNCTION

*00 B0      Base Register 0
*01 B1      Base Register 1
*02 B2      Base Register 2
*03 B3      Base Register 3
*04 B4      Base Register 4
*05 B5      Base Register 5
*06 B6      Base Register 6
*07 B7      Base Register 7

*08 SYSCNTL     System Control (Powerdown & Boot Control)
 09 MEMWAIT     Memory Wait State Register
 0a MEMMODE     Memory Wait Mode Register
 0b TSTCNTL     Test Control Register

 0c SPORTPAGE   Sport DMA Page Register
 0d DMAPAGE     IDMA Port Page Register
 0e-0f  --- Unused ---

 10 STCTL0      Sport0 Transmit Control Register
 11 SRCTL0      Sport0 Receive Control Register
 12 ABFCTL0     Sport0 Autobuffer & FIFO Control Register
 13 MCCTL0      Sport0 Multichannel Control Register
 14 TDIV0     Sport0 Transmit Clock Divide Register
 15 TCNT0     Sport0 Transmit Clock Divide Count (Readonly Testmode)
 16 RDIV0     Sport0 Receive Clock Divide Register
 17 RCNT0     Sport0 Receive Clock Divide Count (Readonly Testmode)
 18 RFSDIV0     Sport0 Receive Frame Sync Divide Register
 19 RFSCNT0     Sport0 Receive Frame Sync Divide Count (Readonly Testmode)
 1a MTCSL0      Sport0 Transmit Multichannel Word Enable, Channels 0-15
 1b MTCSM0      Sport0 Transmit Multichannel Word Enable, Channels 16-31
 1c MRCSL0      Sport0 Receive Multichannel Word Enable, Channels 0-15
 1d MRCSM0      Sport0 Receive Multichannel Word Enable, Channels 16-31
 1e MCCSL0      Sport0 Multichannel Companding Enable, Channels 0-15
 1f MCCSM0      Sport0 Multichannel Companding Enable, Channels 16-31

 20 STCTL1      Sport1 Transmit Control Register
 21 SRCTL1      Sport1 Receive Control Register
 22 ABFCTL1     Sport1 Autobuffer & FIFO Control Register
 23 MCCTL1      Sport1 Multichannel Control Register
 24 TDIV1     Sport1 Transmit Clock Divide Register
 25 TCNT1     Sport1 Transmit Clock Divide Count (Readonly Testmode)
 26 RDIV1     Sport1 Receive Clock Divide Register
 27 RCNT1     Sport1 Receive Clock Divide Count (Readonly Testmode)
 28 RFSDIV1     Sport1 Receive Frame Sync Divide Register
 29 RFSCNT1     Sport1 Receive Frame Sync Divide Count (Readonly Testmode)
 2a MTCSL1      Sport1 Transmit Multichannel Word Enable, Channels 0-15
 2b MTCSM1      Sport1 Transmit Multichannel Word Enable, Channels 16-31
 2c MRCSL1      Sport1 Receive Multichannel Word Enable, Channels 0-15
 2d MRCSM1      Sport1 Receive Multichannel Word Enable, Channels 16-31
 2e MCCSL1      Sport1 Multichannel Companding Enable, Channels 0-15
 2f MCCSM1      Sport1 Multichannel Companding Enable, Channels 16-31

*30 TPERIOD     Timer Period Register
*31 TCOUNT      Timer Counter Register
*32 TSCALE      Timer Scaling Register
*33 TSCALECNT   Timer Scale Count (Readonly Testmode)
 34 EMUCLK1     Emulation Clock Bits 0-15 (Testmode)
 35 EMUCLK2     Emulation Clock Bits 16-31 (Testmode)
 36 EMUCLK3     Emulation Clock Bits 32-47 (Testmode)
 37 --- Reserved ---
 38-3f  --- Unused ---

 40 TX0ADDR     DMA Address, Sport0 Transmit
 41 TX0NXTADDR  DMA Next Address, Sport0 Transmit
 42 TX0CNT      DMA Count, Sport0 Transmit
 43 TX0CURCNT   DMA Current Count, Sport0 Transmit
 44 RX0ADDR     DMA Address, Sport0 Receive
 45 RX0NXTADDR  DMA Next Address, Sport0 Receive
 46 RX0CNT      DMA Count, Sport0 Receive
 47 RX0CURCNT   DMA Current Count, Sport0 Receive
 48 TX1ADDR     DMA Address, Sport1 Transmit
 49 TX1NXTADDR  DMA Next Address, Sport1 Transmit
 4a TX1CNT      DMA Count, Sport1 Transmit
 4b TX1CURCNT   DMA Current Count, Sport1 Transmit
 4c RX1ADDR     DMA Address, Sport1 Receive
 4d RX1NXTADDR  DMA Next Address, Sport1 Receive
 4e RX1CNT      DMA Count, Sport1 Receive
 4f RX1CURCNT   DMA Current Count, Sport1 Receive
 50 DMAADDR     DMA Address, Sport0 Transmit
 51 DMANXTADDR  DMA Next Address, Sport0 Transmit
 52 DMACNT      DMA Count, Sport0 Transmit
 53 DMACURCNT   DMA Current Count, Sport0 Transmit
 54-5f  --- Reserved for future DMA channels ---

 60-ff  --- Unused ---
*/

/* @macro Register Names
These are defined for convienience if a user would like to graph a
his or her own user interface and be able to refer to the registers
with spellings these are defined.
typical use.
  char *reg_names[] = REG_NAMES;

Keep macros so that there inclusion is based soley on need. */

#define REG_NAMES {\
  "AX0", "AX1", "MX0", "MX1", "AY0", "AY1", "MY0", "MY1", \
  "MR2", "SR2", "AR", "SI", "MR1", "SR1", "MR0", "SR0", \
  "I0", "I1", "I2", "I3", "M0", "M1", "M2", "M3", \
  "L0", "L1", "L2", "L3", "IMASK", "IRPTL", "ICNTL", "STACKA", \
  "I4", "I5", "I6", "I7", "M4", "M5", "M6", "M7", \
  "L4", "L5", "L6", "L7", "TX0", "TX1", "CNTR", "OWRCNTR",\
  "ASTAT", "MSTAT", "SSTAT", "FLAGS", "CCODE", "SE", "SB", "PX",\
  "DMPG1", "DMPG2", "IOPG", "IJPG",  "RX0", "RX1", "DX", "STACKP" ,\
  "AF", "MR", "AR", \
  "PC", "FADDR", "DADDR", "EMUSTAT", "EMUCTL", "CYCLES", "DATAB", "ADDRB",\
  "PCSTACK", "STSSTACK", "LOOPSTACK", "CNTRSTACK", "LPTERMSTACK", \
  "LATCHIRPTL", "IMASKPTR",\
/* generated from (ac0)c0ioregs.gperf */\
  "B0", "B1", "B2", "B3", \
  "B4", "B5", "B6", "B7", \
  "SYSCNTL", "MEMWAIT", "MEMMODE", "LPTERM", \
  "SPORTPAGE", "DMAPAGE", "DMAPAGE1", "BOOTPAGE", \
  "STCTL0", "SRCTL0", "ABFCTL0", "MCCTL0", \
  "TDIV0", "TCNT0", "RDIV0", "RCNT0", \
  "RFSDIV0", "RFSCNT0", "MTCSL0", "MTCSM0", \
  "MRCSL0", "MRCSM0", "MCCSL0", "MCCSM0", \
  "STCTL1", "SRCTL1", "ABFCTL1", "MCCTL1", \
  "TDIV1", "TCNT1", "RDIV1", "RCNT1", \
  "RFSDIV1", "RFSCNT1", "MTCSL1", "MTCSM1", \
  "MRCSL1", "MRCSM1", "MCCSL1", "MCCSM1", \
  "TPERIOD", "TCOUNT", "TSCALE", "TSCALECNT", \
  "EMUCLK1", "EMUCLK2", "EMUCLK3", "TX0ADDR", \
  "TX0NXTADDR", "TX0CNT", "TX0CURCNT", "RX0ADDR", \
  "RX0NXTADDR", "RX0CNT", "RX0CURCNT", "TX1ADDR", \
  "TX1NXTADDR", "TX1CNT", "TX1CURCNT", "RX1ADDR", \
  "RX1NXTADDR", "RX1CNT", "RX1CURCNT", "DMAADDR", \
  "DMANXTADDR", "DMACNT", "DMACURCNT", "DMA1ADDR", \
  "DMA1NXTADDR", "DMA1CNT", "DMA1CURCNT", "DMA2ADDR", \
  "DMA2NXTADDR", "DMA2CNT", "DMA2CURCNT", "DMA3ADDR", \
  "DMA3NXTADDR", "DMA3CNT", "DMA3CURCNT", "CADIS", \
  "CAENA", "A0H", "A0L", "A1H", \
  "A1L", "D0H", "D0L", "D1H", \
  "D1L", "SET0", "SET1", "SET2", \
  "SET3", "SET4", "SET5", "SET6", \
  "SET7", "SET8", "SET9", "SET10", \
  "SET11", "SET12", "SET13", "SET14", \
  "SET15", "SET16", "SET17", "SET18", \
  "SET19", "SET20", "SET21", "SET22", \
  "SET23", "SET24", "SET25", "SET26", \
  "SET27", "SET28", "SET29", "SET30", \
  "SET31", }


/* generated from (ac0)c0ioregs.gperf */
typedef enum {
  B0, B1, B2, B3, 
  B4, B5, B6, B7, 
  SYSCNTL, MEMWAIT, MEMMODE, LPTERM, 
  SPORTPAGE, DMAPAGE, DMAPAGE1, BOOTPAGE, 
  STCTL0, SRCTL0, ABFCTL0, MCCTL0, 
  TDIV0, TCNT0, RDIV0, RCNT0, 
  RFSDIV0, RFSCNT0, MTCSL0, MTCSM0, 
  MRCSL0, MRCSM0, MCCSL0, MCCSM0, 
  STCTL1, SRCTL1, ABFCTL1, MCCTL1, 
  TDIV1, TCNT1, RDIV1, RCNT1, 
  RFSDIV1, RFSCNT1, MTCSL1, MTCSM1, 
  MRCSL1, MRCSM1, MCCSL1, MCCSM1, 
  TPERIOD, TCOUNT, TSCALE, TSCALECNT, 
  EMUCLK1, EMUCLK2, EMUCLK3, _dummy0, 
  _dummy1, _dummy2, _dummy3, _dummy4, 
  _dummy5, _dummy6, _dummy7, _dummy8, 
  TX0ADDR, TX0NXTADDR, TX0CNT, TX0CURCNT, 
  RX0ADDR, RX0NXTADDR, RX0CNT, RX0CURCNT, 
  TX1ADDR, TX1NXTADDR, TX1CNT, TX1CURCNT, 
  RX1ADDR, RX1NXTADDR, RX1CNT, RX1CURCNT, 
  DMAADDR, DMANXTADDR, DMACNT, DMACURCNT, 
  DMA1ADDR, DMA1NXTADDR, DMA1CNT, DMA1CURCNT, 
  DMA2ADDR, DMA2NXTADDR, DMA2CNT, DMA2CURCNT, 
  DMA3ADDR, DMA3NXTADDR, DMA3CNT, DMA3CURCNT, 
  _dummy9, _dummy10, _dummy11, _dummy12, 
  _dummy13, _dummy14, _dummy15, _dummy16, 
  _dummy17, _dummy18, _dummy19, _dummy20, 
  _dummy21, _dummy22, _dummy23, _dummy24, 
  _dummy25, _dummy26, _dummy27, _dummy28, 
  _dummy29, _dummy30, _dummy31, _dummy32, 
  _dummy33, _dummy34, _dummy35, _dummy36, 
  _dummy37, _dummy38, _dummy39, _dummy40, 
  _dummy41, _dummy42, _dummy43, _dummy44, 
  _dummy45, _dummy46, _dummy47, _dummy48, 
  _dummy49, _dummy50, _dummy51, _dummy52, 
  _dummy53, _dummy54, _dummy55, _dummy56, 
  _dummy57, _dummy58, _dummy59, _dummy60, 
  _dummy61, _dummy62, _dummy63, _dummy64, 
  _dummy65, _dummy66, _dummy67, _dummy68, 
  _dummy69, _dummy70, _dummy71, _dummy72, 
  _dummy73, _dummy74, _dummy75, _dummy76, 
  _dummy77, _dummy78, _dummy79, _dummy80, 
  _dummy81, _dummy82, _dummy83, _dummy84, 
  _dummy85, _dummy86, _dummy87, _dummy88, 
  _dummy89, _dummy90, _dummy91, _dummy92, 
  _dummy93, _dummy94, _dummy95, _dummy96, 
  _dummy97, _dummy98, _dummy99, _dummy100, 
  _dummy101, _dummy102, _dummy103, _dummy104, 
  _dummy105, _dummy106, _dummy107, _dummy108, 
  _dummy109, _dummy110, _dummy111, _dummy112, 
  _dummy113, _dummy114, _dummy115, _dummy116, 
  _dummy117, _dummy118, _dummy119, _dummy120, 
  _dummy121, _dummy122, _dummy123, _dummy124, 
  _dummy125, _dummy126, CADIS, CAENA, 
  A0H, A0L, A1H, A1L, 
  D0H, D0L, D1H, D1L, 
  SET0, SET1, SET2, SET3, 
  SET4, SET5, SET6, SET7, 
  SET8, SET9, SET10, SET11, 
  SET12, SET13, SET14, SET15, 
  SET16, SET17, SET18, SET19, 
  SET20, SET21, SET22, SET23, 
  SET24, SET25, SET26, SET27, 
  SET28, SET29, SET30, SET31, 
} sysio_regs_t;

#define SYSIO_ABYSS _dummy0



/* generated from (ac0)c0ioregs.gperf */
#define SYS2REG {\
  SYS_B0, SYS_B1, SYS_B2, SYS_B3, \
  SYS_B4, SYS_B5, SYS_B6, SYS_B7, \
  SYS_SYSCNTL, SYS_MEMWAIT, SYS_MEMMODE, SYS_LPTERM, \
  SYS_SPORTPAGE, SYS_DMAPAGE, SYS_DMAPAGE1, SYS_BOOTPAGE, \
  SYS_STCTL0, SYS_SRCTL0, SYS_ABFCTL0, SYS_MCCTL0, \
  SYS_TDIV0, SYS_TCNT0, SYS_RDIV0, SYS_RCNT0, \
  SYS_RFSDIV0, SYS_RFSCNT0, SYS_MTCSL0, SYS_MTCSM0, \
  SYS_MRCSL0, SYS_MRCSM0, SYS_MCCSL0, SYS_MCCSM0, \
  SYS_STCTL1, SYS_SRCTL1, SYS_ABFCTL1, SYS_MCCTL1, \
  SYS_TDIV1, SYS_TCNT1, SYS_RDIV1, SYS_RCNT1, \
  SYS_RFSDIV1, SYS_RFSCNT1, SYS_MTCSL1, SYS_MTCSM1, \
  SYS_MRCSL1, SYS_MRCSM1, SYS_MCCSL1, SYS_MCCSM1, \
  SYS_TPERIOD, SYS_TCOUNT, SYS_TSCALE, SYS_TSCALECNT, \
  SYS_EMUCLK1, SYS_EMUCLK2, SYS_EMUCLK3, 0,\
  0,0,0,0,\
  0,0,0,0,\
  SYS_TX0ADDR, SYS_TX0NXTADDR, SYS_TX0CNT, SYS_TX0CURCNT, \
  SYS_RX0ADDR, SYS_RX0NXTADDR, SYS_RX0CNT, SYS_RX0CURCNT, \
  SYS_TX1ADDR, SYS_TX1NXTADDR, SYS_TX1CNT, SYS_TX1CURCNT, \
  SYS_RX1ADDR, SYS_RX1NXTADDR, SYS_RX1CNT, SYS_RX1CURCNT, \
  SYS_DMAADDR, SYS_DMANXTADDR, SYS_DMACNT, SYS_DMACURCNT, \
  SYS_DMA1ADDR, SYS_DMA1NXTADDR, SYS_DMA1CNT, SYS_DMA1CURCNT, \
  SYS_DMA2ADDR, SYS_DMA2NXTADDR, SYS_DMA2CNT, SYS_DMA2CURCNT, \
  SYS_DMA3ADDR, SYS_DMA3NXTADDR, SYS_DMA3CNT, SYS_DMA3CURCNT, \
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,0,0,\
  0,0,SYS_CADIS, SYS_CAENA, \
  SYS_A0H, SYS_A0L, SYS_A1H, SYS_A1L, \
  SYS_D0H, SYS_D0L, SYS_D1H, SYS_D1L, \
  SYS_SET0, SYS_SET1, SYS_SET2, SYS_SET3, \
  SYS_SET4, SYS_SET5, SYS_SET6, SYS_SET7, \
  SYS_SET8, SYS_SET9, SYS_SET10, SYS_SET11, \
  SYS_SET12, SYS_SET13, SYS_SET14, SYS_SET15, \
  SYS_SET16, SYS_SET17, SYS_SET18, SYS_SET19, \
  SYS_SET20, SYS_SET21, SYS_SET22, SYS_SET23, \
  SYS_SET24, SYS_SET25, SYS_SET26, SYS_SET27, \
  SYS_SET28, SYS_SET29, SYS_SET30, SYS_SET31, }

#endif
