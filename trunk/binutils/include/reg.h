/* @(#)reg.h	2.12 2/17/95 2 */
#ifndef __REG_H__
#define __REG_H__


/*************************************************************************

   File:          reg.h

   Description:   Contains all common defines and declarations for the 
                  register function

*************************************************************************/


#include "regid.h"  /* the actual register ids */


/********************************************************* Structures */

typedef union {
              unsigned long type_32;   /* 32 bit register */
              UNS40 type_40;           /* 40 bit register */
              UNS48 type_48;           /* 48 bit register */
              } REG_VALUE;




/********************************************************* Defines */

/* Define type tags for registers that have the same values */

#define REG_TYPE_NULL     (0x0)     /* not a register (starting marker) */
#define REG_TYPE_UNIV     (0x1)     /* Universal registers */
#define REG_TYPE_CU       (0x2)     /* Computation Unit Registers */
#define REG_TYPE_CACHE    (0x3)     /* Cache Registers */
#define REG_TYPE_PC_STACK (0x4)     /* pc stack */
#define REG_TYPE_LA_STACK (0x5)     /* loop address stack */
#define REG_TYPE_LC_STACK (0x6)     /* loop count stack */
#define REG_TYPE_ST_STACK (0x7)     /* status stack */

#ifdef ADSPZ3
#define REG_TYPE_IOP      (0x8)     /* IOP registers */
#define REG_IOP_MAX       (IOP_SPCNT1 + 1)
#define REG_TYPE_END      (0x9)     /* not a register (ending marker) */
#else
#define REG_TYPE_END      (0x8)     /* not a register (ending marker) */
#endif

#define REG_UNIV_MAX (REG_ALT_B15 + 1)

#define IS_REG_FILE(x)  (((x) >= REG_R0 && (x) <= REG_R15) ||        \
			 ((x) >= REG_ALT_R0 && (x) <= REG_ALT_R15))

#define IS_DAG1_INDEX_REG(x)  (((x) >= REG_I0 && (x) <= REG_I7) || \
			       ((x) >= REG_ALT_I0 && (x) <= REG_ALT_I7))
#define IS_DAG2_INDEX_REG(x)  (((x) >= REG_I8 && (x) <= REG_I15) || \
			       ((x) >= REG_ALT_I8 && (x) <= REG_ALT_I15))
#define IS_DAG1_MODIFY_REG(x) (((x) >= REG_M0 && (x) <= REG_M7) || \
			       ((x) >= REG_ALT_M0 && (x) <= REG_ALT_M7))
#define IS_DAG2_MODIFY_REG(x) (((x) >= REG_M8 && (x) <= REG_M15) || \
			       ((x) >= REG_ALT_M8 && (x) <= REG_ALT_M15))
#define IS_DAG1_LENGTH_REG(x) (((x) >= REG_L0 && (x) <= REG_L7) || \
			       ((x) >= REG_ALT_L0 && (x) <= REG_ALT_L7))
#define IS_DAG2_LENGTH_REG(x) (((x) >= REG_L8 && (x) <= REG_L15) || \
			       ((x) >= REG_ALT_L8 && (x) <= REG_ALT_L15))
#define IS_DAG1_BASE_REG(x)   (((x) >= REG_B0 && (x) <= REG_B7) || \
			       ((x) >= REG_ALT_B0 && (x) <= REG_ALT_B7))
#define IS_DAG2_BASE_REG(x)   (((x) >= REG_B8 && (x) <= REG_B15) || \
			       ((x) >= REG_ALT_B8 && (x) <= REG_ALT_B15))

#define IS_DAG1_REG(x)       (IS_DAG1_INDEX_REG(x) || IS_DAG1_MODIFY_REG(x) || \
                              IS_DAG1_LENGTH_REG(x) || IS_DAG1_BASE_REG(x))
#define IS_DAG2_REG(x)       (IS_DAG2_INDEX_REG(x) || IS_DAG2_MODIFY_REG(x) || \
                              IS_DAG2_LENGTH_REG(x) || IS_DAG2_BASE_REG(x))
#define IS_DAG_REG(x)        (IS_DAG1_REG(x) || IS_DAG2_REG(x))

#define IS_SYSTEM_REG(x) ((x) >= REG_USTAT1 && (x) <= REG_IMASKP)

#define IS_DM_CTRL_REG(x) ((x) >= REG_DMWAIT )

#define IS_PM_CTRL_REG(x) ((x) >= REG_PMWAIT )

#define IS_ALT_REG(x)   (((x) >= REG_UNIV_ALT_START) && ((x) < REG_UNIV_MAX))

#define IS_A_REG_TYPE(x) (((x) > REG_TYPE_NULL) && ((x) < REG_TYPE_END))

#define IS_REG_TYPE_STACK(x) (((x) >= REG_TYPE_PC_STACK) && ((x) <= REG_TYPE_ST_STACK))

#define IS_SWAPPABLE(x) (IS_REG_FILE(x) || IS_DAG1_REG(x) || IS_DAG2_REG(x) || IS_ALT_REG(x))

#define REG_GET_WIDTH(type,number)  ((type == REG_TYPE_UNIV) ? ((IS_REG_FILE (number)) ? (40) : ((number == REG_PX) ? (48) : (32))) : (32))


/*************************************************************************

                        Bit Mask Values 

*************************************************************************/
                                   /* MODE1 */
#ifdef ADSPZ3
#define REG_MODE1_BR8      (0x00000001uL)   /* bit revers for I8 */
#endif /* ADSPZ3 */

#define REG_MODE1_BR0      (0x00000002uL)   /* bit revers for I0 */
#define REG_MODE1_SRCU     (0x00000004uL)   /* alt reg select for CU */
#define REG_MODE1_SRD1H    (0x00000008uL)   /* DAG1 alt reg select 7-4 */
#define REG_MODE1_SRD1L    (0x00000010uL)   /* DAG1 alt reg select 3-0 */
#define REG_MODE1_SRD2H    (0x00000020uL)   /* DAG2 alt reg select 15-12 */
#define REG_MODE1_SRD2L    (0x00000040uL)   /* DAG2 alt reg select 11-8 */
#define REG_MODE1_SRRFH    (0x00000080uL)   /* RF alt reg select R(15-8) */
#define REG_MODE1_SRRFL    (0x00000400uL)   /* RD alt reg select R(7-0) */
#define REG_MODE1_NESTM    (0x00000800uL)   /* interrupt nesting enable */
#define REG_MODE1_IRPTEN   (0x00001000uL)   /* global interrupt enable */
#define REG_MODE1_ALUSAT   (0x00002000uL)   /* enable ALU saturation */

#ifdef ADSPZ3
#define REG_MODE1_SSE	   (0x00004000uL)   /* enable short word sign extension*/
#endif /* ADSPZ3 */

#define REG_MODE1_TRUNC    (0x00008000uL)   /* 1-flt pt trunc, 0-rnd nearest */
#define REG_MODE1_RND32    (0x00010000uL)   /* 1-rnd flt pt to 32, 0-rnd flt pt to 40 */

                                   /* MODE2 */
#define REG_MODE2_IRQ0E    (0x00000001uL)   /* irq0 sens 1-edge, 0-level */
#define REG_MODE2_IRQ1E    (0x00000002uL)   /* irq1 "                  " */
#define REG_MODE2_IRQ2E    (0x00000004uL)   /* irq2 "                  " */
#define REG_MODE2_IRQ3E    (0x00000008uL)   /* irq3 "                  " */
#define REG_MODE2_CADIS    (0x00000010uL)   /* cache disable */
#define REG_MODE2_TIMEN    (0x00000020uL)   /* timer enable */
#define REG_MODE2_DMA1W    (0x00000100uL)   /* dma1 mode 1-when avail, 0-on demand */
#define REG_MODE2_DMA1R    (0x00000200uL)   /* dma1 mode 1-read, 0-write */
#define REG_MODE2_DMA1E    (0x00000400uL)   /* dma1 enable */
#define REG_MODE2_DMA2W    (0x00000800uL)   /* dma2 mode 1-when avail, 0-on demand */
#define REG_MODE2_DMA2R    (0x00001000uL)   /* dma2 mode 1-read, 0-write */
#define REG_MODE2_DMA2E    (0x00002000uL)   /* dma2 enable */
#define REG_MODE2_F0CNT    (0x00004000uL)   /* Connect flag0 to event counter */
#define REG_MODE2_FLGIO0   (0x00008000uL)   /* flag0  1-output 0-input */
#define REG_MODE2_FLGIO1   (0x00010000uL)   /* flag1  "              " */
#define REG_MODE2_FLGIO2   (0x00020000uL)   /* flag2  "              " */
#define REG_MODE2_FLGIO3   (0x00040000uL)   /* flag3  "              " */
#define REG_MODE2_CAFRZ    (0x00080000uL)   /* cache freeze */

                                   /* ASTAT */
#define REG_ASTAT_AZ   (0x00000001uL)   /* alu result 0 or flt pt underflow */
#define REG_ASTAT_AV   (0x00000002uL)   /* alu overflow */
#define REG_ASTAT_AN   (0x00000004uL)   /* alu result negative */
#define REG_ASTAT_AC   (0x00000008uL)   /* alu fix pt carry */
#define REG_ASTAT_AS   (0x00000010uL)   /* alu X input sign (ABS operation) */
#define REG_ASTAT_AI   (0x00000020uL)   /* alu flt pt invalid operation */
#define REG_ASTAT_MN   (0x00000040uL)   /* multiplier result negative */
#define REG_ASTAT_MV   (0x00000080uL)   /* multiplier overflow */
#define REG_ASTAT_MU   (0x00000100uL)   /* multiplier flt pt underflow */
#define REG_ASTAT_MI   (0x00000200uL)   /* multiplier flt pt invalid operation */
#define REG_ASTAT_AF   (0x00000400uL)   /* alu floating point operation */
#define REG_ASTAT_SV   (0x00000800uL)   /* shifter overflow */
#define REG_ASTAT_SZ   (0x00001000uL)   /* shifter result 0 */
#define REG_ASTAT_SS   (0x00002000uL)   /* shifter input sign */
#define REG_ASTAT_FV   (0x00004000uL)   /* function unit overflow */
#define REG_ASTAT_FU   (0x00008000uL)   /* function unit underflow */
#define REG_ASTAT_FI   (0x00010000uL)   /* function unit invalid operation (includes divide by 0)*/
#define REG_ASTAT_FB   (0x00020000uL)   /* function unit busy */
#define REG_ASTAT_BTF  (0x00040000uL)   /* bit test flag for system registers (read only) */
#define REG_ASTAT_FLG0 (0x00080000uL)   /* flag0 value */
#define REG_ASTAT_FLG1 (0x00100000uL)   /* flag1 "   " */
#define REG_ASTAT_FLG2 (0x00200000uL)   /* flag2 "   " */
#define REG_ASTAT_FLG3 (0x00400000uL)   /* flag3 "   " */
#define REG_ASTAT_CACC (0xff000000uL)   /* compare accumulation bits */


                                   /* STKY */
#define REG_STKY_AUS   (0x00000001uL)   /* alu flt pt underflow */
#define REG_STKY_AVS   (0x00000002uL)   /* alu flt pt overflow */
#define REG_STKY_AOS   (0x00000004uL)   /* alu fix pt overflow */
#define REG_STKY_AIS   (0x00000020uL)   /* alu flt pt invalid operation */
#define REG_STKY_MOS   (0x00000040uL)   /* multiplier fix pt overflow */
#define REG_STKY_MVS   (0x00000080uL)   /* multiplier flt pt overflow */
#define REG_STKY_MUS   (0x00000100uL)   /* multiplier flt pt underflow */
#define REG_STKY_MIS   (0x00000200uL)   /* multiplier flt pt invalid operation */
#define REG_STKY_FVS   (0x00004000uL)   /* function unit overflow */
#define REG_STKY_FUS   (0x00008000uL)   /* function unit underflow (division only) */
#define REG_STKY_FIS   (0x00010000uL)   /* function unit invalid operation (includes divide by 0)*/
#define REG_STKY_DAG1S (0x00020000uL)   /* dag1 cb 7 overflow */
#define REG_STKY_DAG2S (0x00040000uL)   /* dag2 cb 15 overflow */
#define REG_STKY_DMA1S (0x00080000uL)   /* dma1 overflow */
#define REG_STKY_DMA2S (0x00100000uL)   /* dma2 overflow */
#define REG_STKY_PCFL  (0x00200000uL)   /* pc stack full (read only) */
#define REG_STKY_PCEM  (0x00400000uL)   /* pc stack empty (read only) */
#define REG_STKY_SSOV  (0x00800000uL)   /* status stack overflow (mode1 and astat) (read only) */
#define REG_STKY_SSEM  (0x01000000uL)   /* status stack empty (read only) */
#define REG_STKY_LSOV  (0x02000000uL)   /* loop stack overflow (loop address and loop counter) (read only) */
#define REG_STKY_LSEM  (0x04000000uL)   /* loop stack empty (read only) */


/* IRPTL */
#ifdef ADSPZ3
#define REG_IRPTL_EMUI   (0x00000001uL)   /* emulator interrupt */
#define REG_IRPTL_RSTI   (0x00000002uL)   /* reset */
#define REG_IRPTL_SOVFI  (0x00000008uL)   /* status or loop stack overflow or pc stack full */
#define REG_IRPTL_TMZOI  (0x00000010uL)   /* timer=0 (high priority option) */
#define REG_IRPTL_VIRPTI (0x00000020uL)   /*  VIRPTI asserted  for Z3*/
#define REG_IRPTL_IRQ2I  (0x00000040uL)   /* irq2 asserted */
#define REG_IRPTL_IRQ1I  (0x00000080uL)   /* irq1 asserted */
#define REG_IRPTL_IRQ0I  (0x00000100uL)   /* irq0 asserted */
#define REG_IRPTL_SPR0I  (0x00000400uL)   /* Z3 DMA Channel 0*/
#define REG_IRPTL_SPR1I  (0x00000800uL)   /* Z3 DMA Channel 1*/
#define REG_IRPTL_SPT0I  (0x00001000uL)   /* Z3 DMA Channel 2*/
#define REG_IRPTL_SPT1I  (0x00002000uL)   /* Z3 DMA Channel 3*/
#define REG_IRPTL_LP0I   (0x00004000uL)   /* Z3 DMA Channel 4*/
#define REG_IRPTL_LP1I   (0x00008000uL)   /* Z3 DMA Channel 5*/
#define REG_IRPTL_EP0I   (0x00010000uL)   /* Z3 DMA Channel 6*/
#define REG_IRPTL_EP1I   (0x00020000uL)   /* Z3 DMA Channel 7*/
#define REG_IRPTL_EP2I   (0x00040000uL)   /* Z3 DMA Channel 8*/
#define REG_IRPTL_EP3I   (0x00080000uL)   /* Z3 DMA Channel 9*/
#define REG_IRPTL_CB7I   (0x00200000uL)   /* cb 7 overflow interrupt */
#define REG_IRPTL_CB15I  (0x00400000uL)   /* cb 15 overflow interrupt */
#define REG_IRPTL_TMZI   (0x00800000uL)   /* timer=0 (low priority option) */
#define REG_IRPTL_FIXI   (0x01000000uL)   /* fix pt overflow */
#define REG_IRPTL_FLTOI  (0x02000000uL)   /* flt pt overflow exception */
#define REG_IRPTL_FLTUI  (0x04000000uL)   /* flt pt underflow exception */
#define REG_IRPTL_FLTII  (0x08000000uL)   /* flt pt invalid exception */
#define REG_IRPTL_USI0   (0x10000000uL)   /* user software interrupt 0 */
#define REG_IRPTL_USI1   (0x20000000uL)   /* user software interrupt 1 */
#define REG_IRPTL_USI2   (0x40000000uL)   /* user software interrupt 2 */
#define REG_IRPTL_USI3   (0x80000000uL)   /* user software interrupt 3 */

#else  /* 21020... */
#define REG_IRPTL_EMUI   (0x00000001uL)   /* emulator interrupt */
#define REG_IRPTL_RSTI   (0x00000002uL)   /* reset */
#define REG_IRPTL_SOVFI  (0x00000008uL)   /* status or loop stack overflow or pc stack full */
#define REG_IRPTL_TMZOI  (0x00000010uL)   /* timer=0 (high priority option) */
#define REG_IRPTL_IRQ3I  (0x00000020uL)   /* irq3 asserted */
#define REG_IRPTL_IRQ2I  (0x00000040uL)   /* irq2 asserted */
#define REG_IRPTL_IRQ1I  (0x00000080uL)   /* irq1 asserted */
#define REG_IRPTL_IRQ0I  (0x00000100uL)   /* irq0 asserted */
#define REG_IRPTL_DMA1I  (0x00000200uL)   /* dma1 overflow interrupt (also shuts off !dmag) */
#define REG_IRPTL_DMA2I  (0x00000400uL)   /* dma2 overflow interrupt (also shuts off !dmag) */
#define REG_IRPTL_CB7I   (0x00000800uL)   /* cb 7 overflow interrupt */
#define REG_IRPTL_CB15I  (0x00001000uL)   /* cb 15 overflow interrupt */
#define REG_IRPTL_ECNTZI (0x00002000uL)   /* event counter = 0 */
#define REG_IRPTL_TMZI   (0x00004000uL)   /* timer=0 (low priority option) */
#define REG_IRPTL_FIXI   (0x00008000uL)   /* fix pt overflow */
#define REG_IRPTL_FLTOI  (0x00010000uL)   /* flt pt overflow exception */
#define REG_IRPTL_FLTUI  (0x00020000uL)   /* flt pt underflow exception */
#define REG_IRPTL_FLTII  (0x00040000uL)   /* flt pt invalid exception */
#define REG_IRPTL_USI0   (0x01000000uL)   /* user software interrupt 0 */
#define REG_IRPTL_USI1   (0x02000000uL)   /* user software interrupt 1 */
#define REG_IRPTL_USI2   (0x04000000uL)   /* user software interrupt 2 */
#define REG_IRPTL_USI3   (0x08000000uL)   /* user software interrupt 3 */
#define REG_IRPTL_USI4   (0x10000000uL)   /* user software interrupt 4 */
#define REG_IRPTL_USI5   (0x20000000uL)   /* user software interrupt 5 */
#define REG_IRPTL_USI6   (0x40000000uL)   /* user software interrupt 6 */
#define REG_IRPTL_USI7   (0x80000000uL)   /* user software interrupt 7 */
#endif /* ADSPZ3 */


/* ditto for IMASK... */
#ifdef ADSPZ3
#define REG_IMASK_EMUI   (0x00000001uL)   /* emulator interrupt */
#define REG_IMASK_RSTI   (0x00000002uL)   /* reset */
#define REG_IMASK_SOVFI  (0x00000008uL)   /* status or loop stack overflow or pc stack full */
#define REG_IMASK_TMZOI  (0x00000010uL)   /* timer=0 (high priority option) */
#define REG_IMASK_VIRPTI (0x00000020uL)   /*  VIRPTI asserted  for Z3*/
#define REG_IMASK_IRQ2I  (0x00000040uL)   /* irq2 asserted */
#define REG_IMASK_IRQ1I  (0x00000080uL)   /* irq1 asserted */
#define REG_IMASK_IRQ0I  (0x00000100uL)   /* irq0 asserted */
#define REG_IMASK_SPR0I  (0x00000400uL)   /* Z3 DMA Channel 0*/
#define REG_IMASK_SPR1I  (0x00000800uL)   /* Z3 DMA Channel 1*/
#define REG_IMASK_SPT0I  (0x00001000uL)   /* Z3 DMA Channel 2*/
#define REG_IMASK_SPT1I  (0x00002000uL)   /* Z3 DMA Channel 3*/
#define REG_IMASK_LP0I   (0x00004000uL)   /* Z3 DMA Channel 4*/
#define REG_IMASK_LP1I   (0x00008000uL)   /* Z3 DMA Channel 5*/
#define REG_IMASK_EP0I   (0x00010000uL)   /* Z3 DMA Channel 6*/
#define REG_IMASK_EP1I   (0x00020000uL)   /* Z3 DMA Channel 7*/
#define REG_IMASK_EP2I   (0x00040000uL)   /* Z3 DMA Channel 8*/
#define REG_IMASK_EP3I   (0x00080000uL)   /* Z3 DMA Channel 9*/
#define REG_IMASK_CB7I   (0x00200000uL)   /* cb 7 overflow interrupt */
#define REG_IMASK_CB15I  (0x00400000uL)   /* cb 15 overflow interrupt */
#define REG_IMASK_TMZI   (0x00800000uL)   /* timer=0 (low priority option) */
#define REG_IMASK_FIXI   (0x01000000uL)   /* fix pt overflow */
#define REG_IMASK_FLTOI  (0x02000000uL)   /* flt pt overflow exception */
#define REG_IMASK_FLTUI  (0x04000000uL)   /* flt pt underflow exception */
#define REG_IMASK_FLTII  (0x08000000uL)   /* flt pt invalid exception */
#define REG_IMASK_USI0   (0x10000000uL)   /* user software interrupt 0 */
#define REG_IMASK_USI1   (0x20000000uL)   /* user software interrupt 1 */
#define REG_IMASK_USI2   (0x40000000uL)   /* user software interrupt 2 */
#define REG_IMASK_USI3   (0x80000000uL)   /* user software interrupt 3 */

#else  /* 21020... */
#define REG_IMASK_EMUI   (0x00000001uL)   /* emulator interrupt */
#define REG_IMASK_RSTI   (0x00000002uL)   /* reset */
#define REG_IMASK_SOVFI  (0x00000008uL)   /* status or loop stack overflow or pc stack full */
#define REG_IMASK_TMZOI  (0x00000010uL)   /* timer=0 (high priority option) */
#define REG_IMASK_IRQ3I  (0x00000020uL)   /* irq3 asserted */
#define REG_IMASK_IRQ2I  (0x00000040uL)   /* irq2 asserted */
#define REG_IMASK_IRQ1I  (0x00000080uL)   /* irq1 asserted */
#define REG_IMASK_IRQ0I  (0x00000100uL)   /* irq0 asserted */
#define REG_IMASK_DMA1I  (0x00000200uL)   /* dma1 overflow interrupt (also shuts off !dmag) */
#define REG_IMASK_DMA2I  (0x00000400uL)   /* dma2 overflow interrupt (also shuts off !dmag) */
#define REG_IMASK_CB7I   (0x00000800uL)   /* cb 7 overflow interrupt */
#define REG_IMASK_CB15I  (0x00001000uL)   /* cb 15 overflow interrupt */
#define REG_IMASK_ECNTZI (0x00002000uL)   /* event counter = 0 */
#define REG_IMASK_TMZI   (0x00004000uL)   /* timer=0 (low priority option) */
#define REG_IMASK_FIXI   (0x00008000uL)   /* fix pt overflow */
#define REG_IMASK_FLTOI  (0x00010000uL)   /* flt pt overflow exception */
#define REG_IMASK_FLTUI  (0x00020000uL)   /* flt pt underflow exception */
#define REG_IMASK_FLTII  (0x00040000uL)   /* flt pt invalid exception */
#define REG_IMASK_USI0   (0x01000000uL)   /* user software interrupt 0 */
#define REG_IMASK_USI1   (0x02000000uL)   /* user software interrupt 1 */
#define REG_IMASK_USI2   (0x04000000uL)   /* user software interrupt 2 */
#define REG_IMASK_USI3   (0x08000000uL)   /* user software interrupt 3 */
#define REG_IMASK_USI4   (0x10000000uL)   /* user software interrupt 4 */
#define REG_IMASK_USI5   (0x20000000uL)   /* user software interrupt 5 */
#define REG_IMASK_USI6   (0x40000000uL)   /* user software interrupt 6 */
#define REG_IMASK_USI7   (0x80000000uL)   /* user software interrupt 7 */
#endif /* ADSPZ3 */


/* Macros to test or manipulate system registers */

/* MODE1 */

#define FLAG_BIT_REV(x)            (((x) & REG_MODE1_BR0) ? 1 : 0)
#define FLAG_CU_ALT_REG(x)         (((x) & REG_MODE1_SRCU) ? 1 : 0)
#define FLAG_DAG1_HI_ALT_REG(x)    (((x) & REG_MODE1_SRD1H) ? 1 : 0)
#define FLAG_DAG1_LO_ALT_REG(x)    (((x) & REG_MODE1_SRD1L) ? 1 : 0)
#define FLAG_DAG2_HI_ALT_REG(x)    (((x) & REG_MODE1_SRD2H) ? 1 : 0)
#define FLAG_DAG2_LO_ALT_REG(x)    (((x) & REG_MODE1_SRD2L) ? 1 : 0)
#define FLAG_REG_HI_ALT_REG(x)     (((x) & REG_MODE1_SRRFH) ? 1 : 0)
#define FLAG_REG_LO_ALT_REG(x)     (((x) & REG_MODE1_SRRFL) ? 1 : 0)
#define FLAG_INTR_NEST(x)          (((x) & REG_MODE1_NESTM) ? 1 : 0)
#define FLAG_GLOBAL_INTR(x)        (((x) & REG_MODE1_IRPTEN) ? 1 : 0)
#define FLAG_ALU_SAT(x)            (((x) & REG_MODE1_ALUSAT) ? 1 : 0)
#define FLAG_TRUNC(x)              (((x) & REG_MODE1_TRUNC) ? 1 : 0)
#define FLAG_RND32(x)              (((x) & REG_MODE1_RND32) ? 1 : 0)

/* ASTAT */

#define FLAG_ALU_ZERO(x)          (((x) & REG_ASTAT_AZ) ? 1 : 0)
#define FLAG_ALU_UNDERFLOW(x)     (((x) & REG_ASTAT_AZ) ? 1 : 0)
#define FLAG_ALU_OVERFLOW(x)      (((x) & REG_ASTAT_AV ) ? 1 : 0)
#define FLAG_ALU_NEGATIVE(x)      (((x) & REG_ASTAT_AN ) ? 1 : 0)
#define FLAG_ALU_CARRY(x)         (((x) & REG_ASTAT_AC ) ? 1 : 0)
#define FLAG_ALU_SIGN_X(x)        (((x) & REG_ASTAT_AS ) ? 1 : 0)
#define FLAG_ALU_FLOAT_INVALID(x) (((x) & REG_ASTAT_AI ) ? 1 : 0)
#define FLAG_MUL_NEGATIVE(x)      (((x) & REG_ASTAT_MN ) ? 1 : 0)
#define FLAG_MUL_OVERFLOW(x)      (((x) & REG_ASTAT_MV ) ? 1 : 0)
#define FLAG_MUL_UNDERFLOW(x)     (((x) & REG_ASTAT_MU ) ? 1 : 0)
#define FLAG_MUL_FLOAT_INVALID(x) (((x) & REG_ASTAT_MI ) ? 1 : 0)
#define FLAG_ALU_FLOAT_OP(x)      (((x) & REG_ASTAT_AF ) ? 1 : 0)
#define FLAG_SHF_OVERFLOW(x)      (((x) & REG_ASTAT_SV ) ? 1 : 0)
#define FLAG_SHF_ZERO(x)          (((x) & REG_ASTAT_SZ ) ? 1 : 0)
#define FLAG_SHF_INPUT_SIGN(x)    (((x) & REG_ASTAT_SS ) ? 1 : 0)
#define FLAG_FUNC_OVERFLOW(x)     (((x) & REG_ASTAT_FV ) ? 1 : 0)
#define FLAG_FUNC_UNDERFLOW(x)    (((x) & REG_ASTAT_FU ) ? 1 : 0)
#define FLAG_FUNC_INVALID(x)      (((x) & REG_ASTAT_FI ) ? 1 : 0)
#define FLAG_FUNC_BUSY(x)         (((x) & REG_ASTAT_FN ) ? 1 : 0)
#define FLAG_BIT_TEST(x)          (((x) & REG_ASTAT_BTF ) ? 1 : 0)
#define FLAG_FLG0(x)              (((x) & REG_ASTAT_FLG0 ) ? 1 : 0)
#define FLAG_FLG1(x)              (((x) & REG_ASTAT_FLG1 ) ? 1 : 0)
#define FLAG_FLG2(x)              (((x) & REG_ASTAT_FLG2 ) ? 1 : 0)
#define FLAG_FLG3(x)              (((x) & REG_ASTAT_FLG3 ) ? 1 : 0)
#define FLAG_CACC(x)              (((x) & REG_ASTAT_CACC ) ? 1 : 0)

#define SET_ALU_ZERO(x)          ((x) |= REG_ASTAT_AZ)
#define SET_ALU_UNDERFLOW(x)     ((x) |= REG_ASTAT_AZ)
#define SET_ALU_OVERFLOW(x)      ((x) |= REG_ASTAT_AV)
#define SET_ALU_NEGATIVE(x)      ((x) |= REG_ASTAT_AN)
#define SET_ALU_CARRY(x)         ((x) |= REG_ASTAT_AC)
#define SET_ALU_SIGN_X(x)        ((x) |= REG_ASTAT_AS)
#define SET_ALU_FLOAT_INVALID(x) ((x) |= REG_ASTAT_AI)
#define SET_MUL_NEGATIVE(x)      ((x) |= REG_ASTAT_MN)
#define SET_MUL_OVERFLOW(x)      ((x) |= REG_ASTAT_MV)
#define SET_MUL_UNDERFLOW(x)     ((x) |= REG_ASTAT_MU)
#define SET_MUL_FLOAT_INVALID(x) ((x) |= REG_ASTAT_MI)
#define SET_ALU_FLOAT_OP(x)      ((x) |= REG_ASTAT_AF)
#define SET_SHF_OVERFLOW(x)      ((x) |= REG_ASTAT_SV)
#define SET_SHF_ZERO(x)          ((x) |= REG_ASTAT_SZ)
#define SET_SHF_INPUT_SIGN(x)    ((x) |= REG_ASTAT_SS)
#define SET_FUNC_OVERFLOW(x)     ((x) |= REG_ASTAT_FV)
#define SET_FUNC_UNDERFLOW(x)    ((x) |= REG_ASTAT_FU)
#define SET_FUNC_INVALID(x)      ((x) |= REG_ASTAT_FI)
#define SET_FUNC_BUSY(x)         ((x) |= REG_ASTAT_FN)
#define SET_BIT_TEST(x)          ((x) |= REG_ASTAT_BTF)
#define SET_FLG0(x)              ((x) |= REG_ASTAT_FLG0)
#define SET_FLG1(x)              ((x) |= REG_ASTAT_FLG1)
#define SET_FLG2(x)              ((x) |= REG_ASTAT_FLG2)
#define SET_FLG3(x)              ((x) |= REG_ASTAT_FLG3)
#define SET_CACC(x)              ((x) |= REG_ASTAT_CACC)

#define CLEAR_ALU_ZERO(x)          ((x) &= ~REG_ASTAT_AZ)
#define CLEAR_ALU_UNDERFLOW(x)     ((x) &= ~REG_ASTAT_AZ)
#define CLEAR_ALU_OVERFLOW(x)      ((x) &= ~REG_ASTAT_AV)
#define CLEAR_ALU_NEGATIVE(x)      ((x) &= ~REG_ASTAT_AN)
#define CLEAR_ALU_CARRY(x)         ((x) &= ~REG_ASTAT_AC)
#define CLEAR_ALU_SIGN_X(x)        ((x) &= ~REG_ASTAT_AS)
#define CLEAR_ALU_FLOAT_INVALID(x) ((x) &= ~REG_ASTAT_AI)
#define CLEAR_MUL_NEGATIVE(x)      ((x) &= ~REG_ASTAT_MN)
#define CLEAR_MUL_OVERFLOW(x)      ((x) &= ~REG_ASTAT_MV)
#define CLEAR_MUL_UNDERFLOW(x)     ((x) &= ~REG_ASTAT_MU)
#define CLEAR_MUL_FLOAT_INVALID(x) ((x) &= ~REG_ASTAT_MI)
#define CLEAR_ALU_FLOAT_OP(x)      ((x) &= ~REG_ASTAT_AF)
#define CLEAR_SHF_OVERFLOW(x)      ((x) &= ~REG_ASTAT_SV)
#define CLEAR_SHF_ZERO(x)          ((x) &= ~REG_ASTAT_SZ)
#define CLEAR_SHF_INPUT_SIGN(x)    ((x) &= ~REG_ASTAT_SS)
#define CLEAR_FUNC_OVERFLOW(x)     ((x) &= ~REG_ASTAT_FV)
#define CLEAR_FUNC_UNDERFLOW(x)    ((x) &= ~REG_ASTAT_FU)
#define CLEAR_FUNC_INVALID(x)      ((x) &= ~REG_ASTAT_FI)
#define CLEAR_FUNC_BUSY(x)         ((x) &= ~REG_ASTAT_FN)
#define CLEAR_BIT_TEST(x)          ((x) &= ~REG_ASTAT_BTF)
#define CLEAR_FLG0(x)              ((x) &= ~REG_ASTAT_FLG0)
#define CLEAR_FLG1(x)              ((x) &= ~REG_ASTAT_FLG1)
#define CLEAR_FLG2(x)              ((x) &= ~REG_ASTAT_FLG2)
#define CLEAR_FLG3(x)              ((x) &= ~REG_ASTAT_FLG3)
#define CLEAR_CACC(x)              ((x) &= ~REG_ASTAT_CACC)

/* STKY */

#define SET_STKY_ALU_FP_UNDERFLOW(x)         ((x) |= REG_STKY_AUS)
#define SET_STKY_ALU_FP_OVERFLOW(x)          ((x) |= REG_STKY_AVS)
#define SET_STKY_ALU_FIXED_OVERFLOW(x)       ((x) |= REG_STKY_AOS)
#define SET_STKY_ALU_FP_INVALID(x)           ((x) |= REG_STKY_AIS)
#define SET_STKY_MUL_FIXED_OVERFLOW(x)       ((x) |= REG_STKY_MOS)
#define SET_STKY_MUL_FP_OVERFLOW(x)          ((x) |= REG_STKY_MVS)
#define SET_STKY_MUL_FP_UNDERFLOW(x)         ((x) |= REG_STKY_MUS)
#define SET_STKY_MUL_FP_INVALID(x)           ((x) |= REG_STKY_MIS)
#define SET_STKY_FUNC_OVERFLOW(x)            ((x) |= REG_STKY_FVS)
#define SET_STKY_FUNC_UNDERFLOW(x)           ((x) |= REG_STKY_FUS)
#define SET_STKY_FUNC_INVALID(x)             ((x) |= REG_STKY_FIS)
#define SET_STKY_DAG1_CIRC_OVERFLOW(x)       ((x) |= REG_STKY_DAG1S)
#define SET_STKY_DAG2_CIRC_OVERFLOW(x)       ((x) |= REG_STKY_DAG2S)
#define SET_STKY_DMA1_OVERFLOW(x)            ((x) |= REG_STKY_DMA1S)
#define SET_STKY_DMA2_OVERFLOW(x)            ((x) |= REG_STKY_DMA2S)
#define SET_STKY_PCFL(x)                     ((x) |= REG_STKY_PCFL)
#define SET_STKY_PCEM(x)                     ((x) |= REG_STKY_PCEM)
#define SET_STKY_SSOV(x)                     ((x) |= REG_STKY_SSOV)
#define SET_STKY_SSEM(x)                     ((x) |= REG_STKY_SSEM)
#define SET_STKY_LSOV(x)                     ((x) |= REG_STKY_LSOV)
#define SET_STKY_LSEM(x)                     ((x) |= REG_STKY_LSEM)
#define CLEAR_STKY_PCFL(x)                   ((x) &= ~REG_STKY_PCFL)
#define CLEAR_STKY_PCEM(x)                   ((x) &= ~REG_STKY_PCEM)
#define CLEAR_STKY_SSOV(x)                   ((x) &= ~REG_STKY_SSOV)
#define CLEAR_STKY_SSEM(x)                   ((x) &= ~REG_STKY_SSEM)
#define CLEAR_STKY_LSOV(x)                   ((x) &= ~REG_STKY_LSOV)
#define CLEAR_STKY_LSEM(x)                   ((x) &= ~REG_STKY_LSEM)
#define CLEAR_STKY_DAG1_CIRC_OVERFLOW(x)     ((x) &= ~REG_STKY_DAG1S)
#define CLEAR_STKY_DAG2_CIRC_OVERFLOW(x)     ((x) &= ~REG_STKY_DAG2S)


                                   /* PMWAIT */
#define REG_PMWAIT_WTMODE_0  (0x00000003uL)   /* Wait mode 0 */
#define REG_PMWAIT_WAITCNT_0 (0x0000001cuL)   /* Wait count 0 */
#define REG_PMWAIT_WTMODE_1  (0x00000060uL)   /* Wait mode 1 */
#define REG_PMWAIT_WAITCNT_1 (0x00000380uL)   /* Wait count 1 */
#define REG_PMWAIT_PGSZ      (0x00001c00uL)   /* Page size */
#define REG_PMWAIT_PGWTEN    (0x00002000uL)   /* Page wait enable */


                                   /* DMWAIT */
#define REG_DMWAIT_WTMODE_0  (0x00000003uL)   /* Wait mode 0 */
#define REG_DMWAIT_WAITCNT_0 (0x0000001cuL)   /* Wait count 0 */
#define REG_DMWAIT_WTMODE_1  (0x00000060uL)   /* Wait mode 1 */
#define REG_DMWAIT_WAITCNT_1 (0x00000380uL)   /* Wait count 1 */
#define REG_DMWAIT_WTMODE_2  (0x00000c00uL)   /* Wait mode 2 */
#define REG_DMWAIT_WAITCNT_2 (0x00007000uL)   /* Wait count 2 */
#define REG_DMWAIT_WTMODE_3  (0x00018000uL)   /* Wait mode 3 */
#define REG_DMWAIT_WAITCNT_3 (0x000e0000uL)   /* Wait count 3 */

#ifdef ADSPZ3
#define REG_DMWAIT_WTMODE_U  (0x00300000uL)   /* Wait mode unbank */
#define REG_DMWAIT_WAITCNT_U (0x01c00000uL)   /* Wait count unbank */
#define REG_DMWAIT_PGSZ      (0x0e000000uL)   /* Page size */
#define REG_DMWAIT_PGWTEN    (0x10000000uL)   /* Page wait enable */
#else
#define REG_DMWAIT_PGSZ      (0x00700000uL)   /* Page size */
#define REG_DMWAIT_PGWTEN    (0x00800000uL)   /* Page wait enable */
#endif /* ADSPZ3 */

#endif /* __REG_H__ */

