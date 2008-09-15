/* @(#)addressing.c	2.3 9/16/94 2 */

#include "app.h"

#ifdef TRUE
#undef TRUE
#endif

#include "symbol.h"
#include "expr.h"
#include "lexical.h"
#include "y_tab.h"
#include "reg.h"
#include "error.h"
#include "addressing.h"


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      get_register_number                                             *
*                                                                      *
*   Synopsis                                                           *
*      long get_register_number( long reg )                            *
*                                                                      *
*   Description                                                        *
*      This routine returns the 21000 universal register equivalent of *
*   the parameter.                                                     *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/12/89       created                  -----             *
***********************************************************************/

long get_register_number(register long reg)
{
    switch( (int) reg )
    {
        case R0:      return( REG_R0 );
        case R1:      return( REG_R1 );
        case R2:      return( REG_R2 );
        case R3:      return( REG_R3 );
        case R4:      return( REG_R4 );
        case R5:      return( REG_R5 );
        case R6:      return( REG_R6 );
        case R7:      return( REG_R7 );
        case R8:      return( REG_R8 );
        case R9:      return( REG_R9 );
        case R10:     return( REG_R10 );
        case R11:     return( REG_R11 );
        case R12:     return( REG_R12 );
        case R13:     return( REG_R13 );
        case R14:     return( REG_R14 );
        case R15:     return( REG_R15 );

        case F0:      return( REG_F0 );
        case F1:      return( REG_F1 );
        case F2:      return( REG_F2 );
        case F3:      return( REG_F3 );
        case F4:      return( REG_F4 );
        case F5:      return( REG_F5 );
        case F6:      return( REG_F6 );
        case F7:      return( REG_F7 );
        case F8:      return( REG_F8 );
        case F9:      return( REG_F9 );
        case F10:     return( REG_F10 );
        case F11:     return( REG_F11 );
        case F12:     return( REG_F12 );
        case F13:     return( REG_F13 );
        case F14:     return( REG_F14 );
        case F15:     return( REG_F15 );

        case I0:      return( REG_I0 );
        case I1:      return( REG_I1 );
        case I2:      return( REG_I2 );
        case I3:      return( REG_I3 );
        case I4:      return( REG_I4 );
        case I5:      return( REG_I5 );
        case I6:      return( REG_I6 );
        case I7:      return( REG_I7 );
        case I8:      return( REG_I8 );
        case I9:      return( REG_I9 );
        case I10:     return( REG_I10 );
        case I11:     return( REG_I11 );
        case I12:     return( REG_I12 );
        case I13:     return( REG_I13 );
        case I14:     return( REG_I14 );
        case I15:     return( REG_I15 );

        case M0:      return( REG_M0 );
        case M1:      return( REG_M1 );
        case M2:      return( REG_M2 );
        case M3:      return( REG_M3 );
        case M4:      return( REG_M4 );
        case M5:      return( REG_M5 );
        case M6:      return( REG_M6 );
        case M7:      return( REG_M7 );
        case M8:      return( REG_M8 );
        case M9:      return( REG_M9 );
        case M10:     return( REG_M10 );
        case M11:     return( REG_M11 );
        case M12:     return( REG_M12 );
        case M13:     return( REG_M13 );
        case M14:     return( REG_M14 );
        case M15:     return( REG_M15 );

        case L0:      return( REG_L0 );
        case L1:      return( REG_L1 );
        case L2:      return( REG_L2 );
        case L3:      return( REG_L3 );
        case L4:      return( REG_L4 );
        case L5:      return( REG_L5 );
        case L6:      return( REG_L6 );
        case L7:      return( REG_L7 );
        case L8:      return( REG_L8 );
        case L9:      return( REG_L9 );
        case L10:     return( REG_L10 );
        case L11:     return( REG_L11 );
        case L12:     return( REG_L12 );
        case L13:     return( REG_L13 );
        case L14:     return( REG_L14 );
        case L15:     return( REG_L15 );
        
        case B0:      return( REG_B0 );
        case B1:      return( REG_B1 );
        case B2:      return( REG_B2 );
        case B3:      return( REG_B3 );
        case B4:      return( REG_B4 );
        case B5:      return( REG_B5 );
        case B6:      return( REG_B6 );
        case B7:      return( REG_B7 );
        case B8:      return( REG_B8 );
        case B9:      return( REG_B9 );
        case B10:     return( REG_B10 );
        case B11:     return( REG_B11 );
        case B12:     return( REG_B12 );
        case B13:     return( REG_B13 );
        case B14:     return( REG_B14 );
        case B15:     return( REG_B15 );

        case FADDR:   return( REG_FADDR );
        case DADDR:   return( REG_DADDR );
        case PC:      return( REG_PC );
        case PCSTK:   return( REG_PCSTK );
        case PCSTKP:  return( REG_PCSTKP );
        case LADDR:   return( REG_LADDR );
        case LCNTR:   return( REG_LCNTR );
        case CURLCNTR: return( REG_CURLCNTR );
        case PX:      return( REG_PX );
        case PX1:     return( REG_PX1 );
        case PX2:     return( REG_PX2 );
        case TPERIOD: return( REG_TPERIOD );
        case TCOUNT:  return( REG_TCOUNT );

        case USTAT1:  return( REG_USTAT1 );
        case USTAT2:  return( REG_USTAT2 );
        case IRPTL:   return( REG_IRPTL );
        case MODE2:   return( REG_MODE2 );
        case MODE1:   return( REG_MODE1 );
        case ASTAT:   return( REG_ASTAT );
        case IMASK:   return( REG_IMASK );
        case STKY:    return( REG_STKY );
        case IMASKP:  return( REG_IMASKP );


        case PSA1S:   return( REG_PSA1S );
        case PSA1E:   return( REG_PSA1E );
        case PSA2S:   return( REG_PSA2S );
        case PSA2E:   return( REG_PSA2E );
        case PSA3S:   return( REG_PSA3S );
        case PSA3E:   return( REG_PSA3E );
        case PSA4S:   return( REG_PSA4S );
        case PSA4E:   return( REG_PSA4E );
        case EMUCLK:  return( REG_EMUCLK );
        case EMUCLK2: return( REG_EMUCLK2 );
        case EMUN:    return( REG_EMUN );

        case PMWAIT:  return( REG_PMWAIT );
        case PMBANK1: return( REG_PMBANK1 );
        case PMADR:   return( REG_PMADR );
        case PMDAS:   return( REG_PMDAS );
        case PMDAE:   return( REG_PMDAE );
        case POVL0:   return( REG_POVL0 );
        case POVL1:   return( REG_POVL1 );

        case DMWAIT:  return( REG_DMWAIT );
        case DMBANK1: return( REG_DMBANK1 );
        case DMBANK2: return( REG_DMBANK2 );
        case DMBANK3: return( REG_DMBANK3 );
        case DMADR:   return( REG_DMADR );
        case DMA1S:   return( REG_DMA1S );
        case DMA1E:   return( REG_DMA1E );
        case DMA2S:   return( REG_DMA2S );
        case DMA2E:   return( REG_DMA2E );
        case DOVL:    return( REG_DOVL );

        case MR0F:    return( REG_MR0F );
        case MR1F:    return( REG_MR1F );
        case MR2F:    return( REG_MR2F );
        case MR0B:    return( REG_MR0B );
        case MR1B:    return( REG_MR1B );
        case MR2B:    return( REG_MR2B );
/*
        case SHP:     return( REG_SHP );
        case SHF:     return( REG_SHF );
        case FR:      return( REG_FR );
        case FTA:     return( REG_FTA );
        case FTB:     return( REG_FTB );
        case FTC:     return( REG_FTC );
*/
        default:      ASSEMBLER_ERROR( "get_register_number: Illegal register." );
    }
}
