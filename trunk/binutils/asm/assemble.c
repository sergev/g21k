/* @(#)assemble.c	2.10 4/22/94 2 */

#include "app.h"
#include "symbol.h"
#include "expr.h"
#include "codegen.h"
#include "action.h"
#include "tempio.h"
#include "ilnode.h"
#include "opcodes.h"
#include "error.h"
#include "pass1.h"
#include "ieee.h"
#include "lexical.h"
#include "listing.h"
#include "main.h"
#include "assemble.h"
#include "reg.h"
#include <string.h>

#include "make_ilnode_fp.h"

static PM_WORD this_is_it = {0, 0, 0, 0, 0, 0};
extern short Z3;

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      assemble_instruction                                            *
*                                                                      *
*   Synopsis                                                           *
*      assemble_instruction( ILNODE *instr )                           *
*                                                                      *
*   Description                                                        *
*      This routine is the entry point for assembling instructions.    *
*   It simply hands the ball off to the appropriate instruction        *
*   handler.                                                           *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/13/89       created                  -----             *
***********************************************************************/

void assemble_instruction( register ILNODE *instr )
{
    if( instr->error_occurred == ASM_ERROR )
        return;

    if( last_minute_error_checking( instr ) )
        return;

    switch( (int) instr->instruction_type )
    {
            case INSTRUCTION_TYPE1:
                instr_type1( instr );
                break;

            case INSTRUCTION_TYPE2:
                instr_type2( instr );
                break;

            case INSTRUCTION_TYPE3:
                instr_type3( instr );
                break;

            case INSTRUCTION_TYPE4:
                instr_type4( instr );
                break;

            case INSTRUCTION_TYPE5:
                instr_type5( instr );
                break;

            case INSTRUCTION_TYPE6:
                instr_type6( instr );
                break;

            case INSTRUCTION_TYPE7:
                FATAL_ERROR( "Internal assembler error. Type 7 instruction no longer exists." );
/*                instr_type7( instr );  */
                break;

            case INSTRUCTION_TYPE8:
                instr_type8( instr );
                break;

            case INSTRUCTION_TYPE9:
                instr_type9( instr );
                break;

            case INSTRUCTION_TYPE10:
                instr_type10( instr );
                break;

            case INSTRUCTION_TYPE11:
                instr_type11( instr );
                break;

            case INSTRUCTION_TYPE12:
                instr_type12( instr );
                break;

            case INSTRUCTION_TYPE13:
                instr_type13( instr );
                break;

            case INSTRUCTION_TYPE14:
                instr_type14( instr );
                break;

            case INSTRUCTION_TYPE15:
                instr_type15( instr );
                break;

            case INSTRUCTION_TYPE16:
                instr_type16( instr );
                break;

            case INSTRUCTION_TYPE17:
                instr_type17( instr );
                break;

            case INSTRUCTION_TYPE18:
                instr_type18( instr );
                break;

            case INSTRUCTION_TYPE19:
                instr_type19( instr );
                break;

            case INSTRUCTION_TYPE20:
                instr_type20( instr );
                break;

            case INSTRUCTION_TYPE21:
                instr_type21( instr );
                break;

            case INSTRUCTION_TYPE22:
                instr_type22( instr );
                break;

            case INSTRUCTION_TYPE23:
                instr_type23( instr );
                break;

            case INSTRUCTION_TYPE24:
                instr_type24( instr );
                break;

            case INSTRUCTION_TYPE25:
                instr_type25( instr );
                break;

            case INSTRUCTION_TYPE25A:
	        instr_type25a( instr );
                break;

            case INSTRUCTION_TYPE27:
                instr_type27( instr );
                break;

            case INSTRUCTION_TYPE28:
                instr_type28( instr );
                break;

            case INSTRUCTION_TYPE29:
                instr_type29( instr );
                break;

            default:
                ASSEMBLER_ERROR( "Illegal instruction type." );
                break;
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      format_compute                                                  *
*                                                                      *
*   Synopsis                                                           *
*      long format_compute( ILNODE *instr )                            *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting the compute part of a   *
*   instruction together, based on the compute type.                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/13/89       created                  -----             *
***********************************************************************/

long format_compute( register ILNODE *instr )
{
    register long compute;

    compute = 0L;

    switch( (int) instr->compute.compute_type )
    {
            case COMPUTE_NULL:
               compute = COMPUTE_NOT_PRESENT;
               break;

            case COMPUTE_TYPE1:

               /* Special case of shift immediate */

               if( instr->compute.computation_unit == SHIFTER &&
                   instr->shift_type == SHIFT_IMMEDIATE )
               {
                   compute |= (((instr->compute.opcode & 0xFCL) >> 2) << 16) | 
                              ((instr->shift_immediate & 0xFFL) << 8) |
                              (DF_REG(instr->compute.result1_reg) << 4) | 
                               DF_REG(instr->compute.x1_reg);
               }
               else
               {
                   compute |= (instr->compute.computation_unit << 20L) | 
                              (instr->compute.opcode << 12) | 
                              (DF_REG(instr->compute.result1_reg) << 8) | 
                              (DF_REG(instr->compute.x1_reg) << 4) | (DF_REG(instr->compute.y1_reg));
               }
               break;

            case COMPUTE_TYPE2:
               compute |= (instr->compute.computation_unit << 20) | (instr->compute.opcode << 16) |
                          (DF_REG(instr->compute.result2_reg) << 12) | 
                          (DF_REG(instr->compute.result1_reg) << 8) |
                          (DF_REG(instr->compute.x1_reg) << 4) | DF_REG(instr->compute.y1_reg);
               break;

            case COMPUTE_TYPE3:
               compute |= (0x1L << 22) | (instr->compute.opcode << 16) | 
                          (DF_REG(instr->compute.result1_reg) << 12) |
                          (DF_REG(instr->compute.result2_reg) << 8) |
                          (MULTI_REG(instr->compute.x1_reg) << 6) | 
                          (MULTI_REG(instr->compute.y1_reg) << 4) |
                          (MULTI_REG(instr->compute.x2_reg) << 2) | MULTI_REG(instr->compute.y2_reg);
               break;

            case COMPUTE_TYPE4:
               compute |= (0x1L << 22) | ((instr->compute.opcode & 0x03L) << 20) | 
                          (DF_REG(instr->compute.result3_reg) << 16) |
                          (DF_REG(instr->compute.result1_reg) << 12) |
                          (DF_REG(instr->compute.result2_reg) << 8) |
                          (MULTI_REG(instr->compute.x1_reg) << 6) | 
                          (MULTI_REG(instr->compute.y1_reg) << 4) |
                          (MULTI_REG(instr->compute.x2_reg) << 2) | MULTI_REG(instr->compute.y2_reg);
               break;

            case COMPUTE_TYPE5:
               compute |= (0x1L << 22) | (instr->compute.opcode << 17) | ( (long)instr->compute.t << 16)
                           | (instr->compute.cu_reg << 12) | (DF_REG(instr->compute.data_reg) << 8);
               break;

            default:
               ASSEMBLER_ERROR( "Illegal compute instruction type." );
               break;
     }

     return( compute );
}

    

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type1                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type1( ILNODE *instr )                               *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 1 instruction, and then writing the instruction to the      *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/13/89       created                  -----             *
***********************************************************************/

void instr_type1( register ILNODE *instr )
{
    register long compute;

    memset( this_is_it, 0, sizeof(PM_WORD) );
    compute = format_compute( instr );

    this_is_it[0] = (char) (0x20L | (instr->dmd << 4) | ((DAG_REG(instr->dm_ireg)) << 1) | 
                    ((DAG_REG(instr->dm_mreg) & 0x4L) >> 2));
    this_is_it[1] = (char) (((DAG_REG(instr->dm_mreg) & 0x3L) << 6) | (instr->pmd << 5) | 
                    (DF_REG(instr->dm_dreg) << 1) | ((DAG_REG(instr->pm_ireg) & 0x4L) >> 2));
    this_is_it[2] = (char) (((DAG_REG(instr->pm_ireg) & 0x3L) << 6) | (DAG_REG(instr->pm_mreg) << 3)
                    | ((DF_REG(instr->pm_dreg) & 0xEL) >> 1));
    this_is_it[3] = (char) (((DF_REG(instr->pm_dreg) & 0x1L) << 7) | COMPUTE_TOP7( compute ));
    this_is_it[4] = (char) COMPUTE_MIDDLE8( compute );
    this_is_it[5] = (char) COMPUTE_BOTTOM8( compute );

    if( listing_flag )
        listing_output_instruction( this_is_it );

    WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type2                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type2( ILNODE *instr )                               *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 2 instruction, and then writing the instruction to the      *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/13/89       created                  -----             *
***********************************************************************/
          
void instr_type2( register ILNODE *instr )
{
    register long compute;

    memset( this_is_it, 0, sizeof(PM_WORD) );
    compute = format_compute( instr );

    this_is_it[0] = (char) 0x01L;
    this_is_it[1] = (char) (instr->cond << 1);
    this_is_it[3] = (char) COMPUTE_TOP7( compute );
    this_is_it[4] = (char) COMPUTE_MIDDLE8( compute );
    this_is_it[5] = (char) COMPUTE_BOTTOM8( compute );

    if( listing_flag )
        listing_output_instruction( this_is_it );

    WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type3                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type3( ILNODE *instr )                               *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 3 instruction, and then writing the instruction to the      *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/17/89       created                  -----             *
***********************************************************************/

void instr_type3( register ILNODE *instr )
{
    register long compute;
    register short ireg;
    register short mreg;

    memset( this_is_it, 0, sizeof(PM_WORD) );


    compute = format_compute( instr );

    if( instr->g == G_DATA_MEMORY )
    {
        ireg = instr->dm_ireg;
        mreg = instr->dm_mreg;
    }
    else if(instr->g == G_PROGRAM_MEMORY )
    {
       ireg = instr->pm_ireg;
       mreg = instr->pm_mreg;
    }

    if( (IS_DAG1_REG(ireg) && IS_DAG1_REG(instr->ureg)) || 
        (IS_DAG2_REG(ireg) && IS_DAG2_REG(instr->ureg)) )
    {
        if( !(instr->u == U_PRE_MODIFIED && instr->d == D_READ) ) 
            USER_ERROR( "Multiple accesses of the same DAG are illegal." );	
    }

    if( !Z3 && (instr->ureg == REG_PX && instr->g == G_DATA_MEMORY) )
        USER_ERROR( "Transfers between Data Memory and the PX register are illegal." );

    this_is_it[0] = (char) (0x40L | (instr->u << 4) | (DAG_REG(ireg) << 1) | 
                    ((DAG_REG(mreg) & 0x04L) >> 2));
    this_is_it[1] = (char) (((DAG_REG(mreg) & 0x03L) << 6) | (instr->cond << 1) | instr->g);
    this_is_it[2] = (char) ((instr->d << 7) | ((instr->ureg & 0xFEL) >> 1));
    this_is_it[3] = (char) (((instr->ureg & 0x01L) << 7) | COMPUTE_TOP7( compute ));
    this_is_it[4] = (char) COMPUTE_MIDDLE8( compute );
    this_is_it[5] = (char ) COMPUTE_BOTTOM8( compute );

    if( listing_flag )
        listing_output_instruction( this_is_it );

    WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type4                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type4( ILNODE *instr )                               *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 4 instruction, and then writing the instruction to the      *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/17/89       created                  -----             *
***********************************************************************/

void instr_type4( register ILNODE *instr )
{
    char           buff[5];
    char           buff1[5];
    register long  compute;
    register short ireg;
    register long  disp;
    short    dreg;

    memset( this_is_it, 0, sizeof(PM_WORD) );

    if( instr->g == G_DATA_MEMORY )
    {
        ireg = (short) DAG_REG(instr->dm_ireg);
        disp = instr->dm_disp.value.int_val;
        dreg = instr->dm_dreg;
    }
    else
    {
        ireg = (short) DAG_REG(instr->pm_ireg);
        disp = instr->pm_disp.value.int_val;
        dreg = instr->pm_dreg;
    }

    if( (disp < -32) || (disp > 31) )
    {
	if( disp >= 32 && disp <= 63 )
        {
            sprintf( buff, "%d", disp );
            sprintf( buff1, "%d", disp - 64 );
          USER_WARN2( "The displacement %s will be treated as %s in 6-bit twos-complement notation.",
                     buff, buff1 );
	}
        else
           USER_ERROR( "Relative displacement is out of range." );
    }


    if( (instr->g == G_DATA_MEMORY && instr->dm_disp.sym_ptr) || 
        (instr->g == G_PROGRAM_MEMORY && instr->pm_disp.sym_ptr) )
    {
        USER_ERROR( "Relative displacement must be a constant." );
    }

    compute = format_compute( instr );

    this_is_it[0] = (char) (0x60L | (ireg << 1) | instr->g);
    this_is_it[1] = (char) ((instr->d << 7) | (instr->u << 6) | (instr->cond << 1) | 
                    ((disp & 0x20L) >> 5));
    this_is_it[2] = (char) (((disp & 0x1FL) << 3) | ((DF_REG(dreg) & 0x0EL) >> 1));
    this_is_it[3] = (char) (((DF_REG(dreg) & 0x01L) << 7) | COMPUTE_TOP7( compute ));
    this_is_it[4] = (char) COMPUTE_MIDDLE8( compute );
    this_is_it[5] = (char) COMPUTE_BOTTOM8( compute );

    if( listing_flag )
        listing_output_instruction( this_is_it );

    WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}
    

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type5                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type5( ILNODE *instr )                               *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 5 instruction, and then writing the instruction to the      *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/17/89       created                  -----             *
***********************************************************************/

void instr_type5( register ILNODE *instr )
{
    register long compute;

    memset( this_is_it, 0, sizeof(PM_WORD) );
    compute = format_compute( instr );

    if( !Z3 && (instr->src_ureg == REG_PX || instr->dest_ureg == REG_PX) )
    {
	USER_ERROR( "Transfers between univeral registers and the PX register are illegal." );
    }

    this_is_it[0] = (char) (0x70L | ((instr->src_ureg & 0xF0L) >> 4));
    this_is_it[1] = (char) (((instr->src_ureg & 0x0FL) << 4) | ((instr->cond & 0x1EL) >> 1));
    this_is_it[2] = (char) (((instr->cond & 0x1L) << 7) | ((instr->dest_ureg & 0xFEL) >> 1));
    this_is_it[3] = (char) (((instr->dest_ureg & 0x1L) << 7) | COMPUTE_TOP7( compute ));
    this_is_it[4] = (char) COMPUTE_MIDDLE8( compute );
    this_is_it[5] = (char) COMPUTE_BOTTOM8( compute );

    if( listing_flag )
        listing_output_instruction( this_is_it );

    WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type6                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type6( ILNODE *instr )                               *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 6 instruction, and then writing the instruction to the      *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/17/89       created                  -----             *
***********************************************************************/

void instr_type6( register ILNODE *instr )
{
    register long compute;
    register short ireg;
    register short mreg;
             short dreg;  

    memset( this_is_it, 0, sizeof(PM_WORD) );

    compute = format_compute( instr );

    if( instr->shift_data_move == PRESENT )
    {
        if( instr->g == G_DATA_MEMORY )
        {
            dreg = (short) DF_REG( instr->dm_dreg );
            ireg = (short) DAG_REG( instr->dm_ireg );
            mreg = (short) DAG_REG( instr->dm_mreg );
        }
        else if(instr->g == G_PROGRAM_MEMORY )
        {
            dreg = (short) DF_REG( instr->pm_dreg );
            ireg = (short) DAG_REG( instr->pm_ireg );
            mreg = (short) DAG_REG( instr->pm_mreg );
        }

        this_is_it[0] = (char) (0x80L | (ireg << 1) | ((mreg & 0x4L) >> 2));
        this_is_it[1] = (char) (((mreg & 0x3L) << 6) | (instr->cond << 1) | instr->g);
        this_is_it[2] = (char) ((instr->d << 7) | (((instr->shift_immediate & 0xF00L) >> 8) << 3) | 
                               ((dreg & 0xEL) >> 1));
        this_is_it[3] = (char) (((dreg & 0x1L) << 7) | COMPUTE_TOP7( compute ));
    }
    else
    {
        this_is_it[0] = (char) 0x02L;
        this_is_it[1] = (char) (instr->cond << 1);
        this_is_it[2] = (char) (((instr->shift_immediate & 0xF00L) >> 8) << 3);
        this_is_it[3] = (char) COMPUTE_TOP7( compute );
    }

    this_is_it[4] = (char) COMPUTE_MIDDLE8( compute );
    this_is_it[5] = (char) COMPUTE_BOTTOM8( compute );

    if( listing_flag )
        listing_output_instruction( this_is_it );

    WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type7                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type7( ILNODE *instr )                               *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 7 instruction, and then writing the instruction to the      *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/18/89       created                  -----             *
***********************************************************************/

void instr_type7( register ILNODE *instr )
{

    instr;
/*
    register long compute;
    if( !IS_21040 && instr->c == C_EVENT_COUNTER )
    {
	USER_ERROR( "Event counter does not exist on the 21020." );
        return;
    }

    memset( this_is_it, 0, sizeof(PM_WORD) );
    compute = format_compute( instr );

    this_is_it[0] = (char) 0x03L;
    this_is_it[1] = (char) ((instr->cond << 1) | instr->inc);
    this_is_it[2] = (char) ((instr->dec << 7) | (instr->c << 6));
    this_is_it[3] = (char) COMPUTE_TOP7( compute );
    this_is_it[4] = (char) COMPUTE_MIDDLE8( compute );
    this_is_it[5] = (char) COMPUTE_BOTTOM8( compute );

    if( listing_flag )
        listing_output_instruction( this_is_it );

    WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
*/
}
    

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type8                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type8( ILNODE *instr )                               *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 8 instruction, and then writing the instruction to the      *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/17/89       created                  -----             *
***********************************************************************/

void instr_type8( register ILNODE *instr )
{
    register long compute;
    register short ireg;
    register short mreg;

    memset( this_is_it, 0, sizeof(PM_WORD) );
    compute = format_compute( instr );

    if( instr->g == G_DATA_MEMORY )
    {
        ireg = (short) DAG_REG(instr->dm_ireg);
        mreg = (short) DAG_REG(instr->dm_mreg);
    }
    else if(instr->g == G_PROGRAM_MEMORY )
    {
        ireg = (short) DAG_REG(instr->pm_ireg);
        mreg = (short) DAG_REG(instr->pm_mreg);
    }

    this_is_it[0] = (char) 0x04L;
    this_is_it[1] = (char) ((instr->g << 6) | (instr->cond << 1) | ((ireg & 0x04L) >> 2));
    this_is_it[2] = (char) (((ireg & 0x03L) << 6) | (mreg << 3));
    this_is_it[3] = (char) COMPUTE_TOP7( compute );
    this_is_it[4] = (char) COMPUTE_MIDDLE8( compute );
    this_is_it[5] = (char) COMPUTE_BOTTOM8( compute );

    if( listing_flag )
        listing_output_instruction( this_is_it );

    WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type9                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type9( ILNODE *instr )                               *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 9 instruction, and then writing the instruction to the      *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/17/89       created                  -----             *
***********************************************************************/

void instr_type9( register ILNODE *instr )
{
    register long address;

    memset( this_is_it, 0, sizeof(PM_WORD) );

    if( instr->b == B_CALL && instr->a != A_NO_LOOP_ABORT )
    {
        USER_ERROR( "The loop abort modifier is illegal on call instructions." );
        return;
    }

    if( instr->address_type == ADDRESS_DIRECT)
    {
        address = PM_ADDRESS(instr->pm_addr.value.int_val);
        this_is_it[0] = 0x06L;
    }
    else if( instr->address_type == ADDRESS_PC_RELATIVE )
    {
        address = PM_ADDRESS( instr->pc_disp.value.int_val );
        this_is_it[0] = 0x07L;
    }
    else
    {
        ASSEMBLER_ERROR( "Illegal address type in type 9 instruction." );
        return;
    }

    this_is_it[1] = (char) ((instr->b << 7) | (instr->a << 6) | (instr->cond << 1));
    this_is_it[2] = (char) ((instr->j << 2) | (instr->cios));
    this_is_it[3] = (char) PM_ADDRESS_TOP8(address);
    this_is_it[4] = (char) PM_ADDRESS_MIDDLE8(address);
    this_is_it[5] = (char) PM_ADDRESS_BOTTOM8(address);

    if( listing_flag )
        listing_output_instruction( this_is_it );
     
    if( (instr->address_type == ADDRESS_DIRECT) && (instr->pm_addr.sym_ptr != (SYMBOL *) NULL) )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_ADDR24, instr->pm_addr.sym_ptr );
    else if( instr->pc_disp.sym_ptr != (SYMBOL *) NULL )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_PC_REL_LONG, 
                             instr->pc_disp.sym_ptr );
    else
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type10                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type10( ILNODE *instr )                              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 10 instruction, and then writing the instruction to the     *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/17/89       created                  -----             *
***********************************************************************/

void instr_type10( register ILNODE *instr )
{
    register long compute;

    memset( this_is_it, 0, sizeof(PM_WORD) );
    compute = format_compute( instr );

    if( instr->address_type == ADDRESS_INDIRECT )
    {
        this_is_it[0] = (char) 0x08L;
        this_is_it[1] = (char) ((instr->b << 7) | (instr->a << 6) | (instr->cond << 1) |
                               ((DAG_REG(instr->pm_ireg) & 0x04L) >> 2));
        this_is_it[2] = (char) (((DAG_REG(instr->pm_ireg) & 0x03L) << 6) | 
                        (DAG_REG(instr->pm_mreg) << 3) | (instr->j << 2) | 
			(instr->oelse << 1) | (instr->cios) );
    }
    else if( instr->address_type == ADDRESS_PC_RELATIVE )
    {
        this_is_it[0] = (char) 0x09L;
        this_is_it[1] = (char) ((instr->b << 7) | (instr->a << 6) | (instr->cond << 1) |
                               ((instr->pc_disp.value.int_val & 0x20L) >> 5));
        this_is_it[2] = (char) (((instr->pc_disp.value.int_val & 0x1FL) << 3) | 
				 (instr->j << 2) | (instr->oelse << 1) 
                               | (instr->cios) );
    }
    else
    {
        ASSEMBLER_ERROR( "Illegal address type in type 10 instruction." );
        return;
    }
    
    this_is_it[3] = (char) COMPUTE_TOP7( compute );
    this_is_it[4] = (char) COMPUTE_MIDDLE8( compute );
    this_is_it[5] = (char) COMPUTE_BOTTOM8( compute );

    if( listing_flag )
        listing_output_instruction( this_is_it );

    if( instr->address_type == ADDRESS_INDIRECT )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
    else if( instr->address_type == ADDRESS_PC_RELATIVE && instr->pc_disp.sym_ptr != NULL )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_PC_REL_SHORT, 
                            instr->pc_disp.sym_ptr  );
    else
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type11                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type11( ILNODE *instr )                              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 11 instruction, and then writing the instruction to the     *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/17/89       created                  -----             *
***********************************************************************/

void instr_type11( register ILNODE *instr )
{
    register long compute;

    memset( this_is_it, 0, sizeof(PM_WORD) );
    compute = format_compute( instr );

    if( instr->address_type == ADDRESS_INDIRECT )
    {
        this_is_it[0] = (char) (0xC0L | (instr->d << 4) | (DAG_REG(instr->dm_ireg) << 1) |
                               ((DAG_REG(instr->dm_mreg) & 0x04L) >> 2));
        this_is_it[1] = (char) ((DAG_REG(instr->dm_mreg & 0x03L) << 6) | (instr->cond << 1) |
                               ((DAG_REG(instr->pm_ireg) & 0x04L) >> 2));
        this_is_it[2] = (char) (((DAG_REG(instr->pm_ireg) & 0x03L) << 6) | 
                               (DAG_REG(instr->pm_mreg) << 3) | 
                               ((DF_REG(instr->dm_dreg) & 0x0EL) >> 1));
    }
    else if( instr->address_type == ADDRESS_PC_RELATIVE )
    {
        this_is_it[0] = (char) (0xE0L | (instr->d << 4) | (DAG_REG(instr->dm_ireg) << 1) |
                               ((DAG_REG(instr->dm_mreg) & 0x04L) >> 2));
        this_is_it[1] = (char) ((DAG_REG(instr->dm_mreg & 0x03L) << 6) | (instr->cond << 1) |
                               ((instr->pc_disp.value.int_val & 0x20L) >> 5));
        this_is_it[2] = (char) (((instr->pc_disp.value.int_val & 0x1FL) << 3) |
                               ((DF_REG(instr->dm_dreg) & 0x0EL) >> 1));

    }
    else
    {
        ASSEMBLER_ERROR( "Illegal address type in type 11 instruction." );
        return;
    }

    this_is_it[3] = (char) (((DF_REG(instr->dm_dreg) & 0x01L) << 7) | COMPUTE_TOP7( compute ));
    this_is_it[4] = (char) COMPUTE_MIDDLE8( compute );
    this_is_it[5] = (char) COMPUTE_BOTTOM8( compute );

    if( listing_flag )
        listing_output_instruction( this_is_it );

    if( instr->address_type == ADDRESS_INDIRECT )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
    else if( instr->address_type == ADDRESS_PC_RELATIVE && instr->pc_disp.sym_ptr != NULL )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_PC_REL_SHORT, 
                            instr->pc_disp.sym_ptr );
    else
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type12                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type12( ILNODE *instr )                              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 12 instruction, and then writing the instruction to the     *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/17/89       created                  -----             *
***********************************************************************/

void instr_type12( register ILNODE *instr )
{
    register long compute;

    memset( this_is_it, 0, sizeof(PM_WORD) );
    compute = format_compute( instr );

    this_is_it[0] = (char) ((instr->ret == RET_TYPE_RTS) ? 0x0AL : 0x0BL);
    this_is_it[1] = (char) ((instr->a << 6) | (instr->cond << 1));
    this_is_it[2] = (char) (instr->j << 2) | (instr->oelse << 1) | (instr->cios);
    this_is_it[3] = (char) COMPUTE_TOP7( compute );
    this_is_it[4] = (char) COMPUTE_MIDDLE8( compute );
    this_is_it[5] = (char) COMPUTE_BOTTOM8( compute );

    if( listing_flag )
        listing_output_instruction( this_is_it );

     WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type13                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type13( ILNODE *instr )                              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 13 instruction, and then writing the instruction to the     *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/17/89       created                  -----             *
***********************************************************************/

void instr_type13( register ILNODE *instr )
{
    register long address;

    memset( this_is_it, 0, sizeof(PM_WORD) );

    if( instr->address_type == ADDRESS_DIRECT)
    {
        address = PM_ADDRESS(instr->pm_addr.value.int_val);
    }
    else if( instr->address_type == ADDRESS_PC_RELATIVE )
    {
        address = PM_ADDRESS( instr->pc_disp.value.int_val );
    }
    else
    {
        ASSEMBLER_ERROR( "Illegal address type in type 9 instruction." );
        return;
    }

    if( instr->linit == LCNTR_INIT_IMMED )
    {
        this_is_it[0] = (char) 0x0CL;
        this_is_it[1] = (char) ((instr->immediate.value.int_val & 0xFF00L) >> 8);
        this_is_it[2] = (char) (instr->immediate.value.int_val & 0x00FFL);
    }
    else if( instr->linit == LCNTR_INIT_UNIV_REG )
    {
        this_is_it[0] = (char) 0x0DL;
        this_is_it[1] = (char) instr->ureg;
    }

    this_is_it[3] = (char) PM_ADDRESS_TOP8(address);
    this_is_it[4] = (char) PM_ADDRESS_MIDDLE8(address);
    this_is_it[5] = (char) PM_ADDRESS_BOTTOM8(address);

    if( listing_flag )
        listing_output_instruction( this_is_it );

    if( instr->pm_addr.sym_ptr != (SYMBOL *) NULL )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_PC_REL_LONG, 
                             instr->pm_addr.sym_ptr );
    else if( instr->pc_disp.sym_ptr != (SYMBOL *) NULL )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_PC_REL_LONG, 
                             instr->pc_disp.sym_ptr );
    else
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type14                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type14( ILNODE *instr )                              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 14 instruction, and then writing the instruction to the     *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/19/89       created                  -----             *
***********************************************************************/

void instr_type14( register ILNODE *instr )
{
    register long address;

    memset( this_is_it, 0, sizeof(PM_WORD) );

    if( instr->address_type == ADDRESS_DIRECT)
    {
        address = PM_ADDRESS(instr->pm_addr.value.int_val);
    }
    else if( instr->address_type == ADDRESS_PC_RELATIVE )
    {
        address = PM_ADDRESS( instr->pc_disp.value.int_val );
    }
    else
    {
        ASSEMBLER_ERROR( "Illegal address type in type 9 instruction." );
        return;
    }

    this_is_it[0] = (char) 0x0EL;
    this_is_it[1] = (char) (instr->cond) << 1;
    this_is_it[3] = (char) PM_ADDRESS_TOP8(address);
    this_is_it[4] = (char) PM_ADDRESS_MIDDLE8(address);
    this_is_it[5] = (char) PM_ADDRESS_BOTTOM8(address);

    if( listing_flag )
        listing_output_instruction( this_is_it );

    if( instr->pm_addr.sym_ptr != (SYMBOL *) NULL )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_PC_REL_LONG, 
                             instr->pm_addr.sym_ptr );
    else if( instr->pc_disp.sym_ptr != (SYMBOL *) NULL )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_PC_REL_LONG, 
                             instr->pc_disp.sym_ptr );
    else
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type15                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type15( ILNODE *instr )                              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 15 instruction, and then writing the instruction to the     *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/19/89       created                  -----             *
***********************************************************************/

void instr_type15( register ILNODE *instr )
{
    register long address;

    memset( this_is_it, 0, sizeof(PM_WORD) );
    address = (instr->g == G_DATA_MEMORY) ? instr->dm_addr.value.int_val : 
              PM_ADDRESS(instr->pm_addr.value.int_val);

    if( !Z3 && (instr->ureg == REG_PX && instr->g == G_DATA_MEMORY) )
        USER_ERROR( "Transfers between Data Memory and the PX register are illegal." );

    this_is_it[0] = (char) (0x10L | (instr->g << 1) | instr->d);
    this_is_it[1] = (char) instr->ureg;
    this_is_it[2] = (char) DATA32_TOP8(address);
    this_is_it[3] = (char) DATA32_TOP_MIDDLE8(address);
    this_is_it[4] = (char) DATA32_MIDDLE8(address);
    this_is_it[5] = (char) DATA32_BOTTOM8(address);

    if( listing_flag )
        listing_output_instruction( this_is_it );

    if( (instr->g == G_DATA_MEMORY) && (instr->dm_addr.sym_ptr != (SYMBOL *) NULL) )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_ADDR32, instr->dm_addr.sym_ptr );
    else if( instr->pm_addr.sym_ptr != (SYMBOL *) NULL )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_ADDR24, instr->pm_addr.sym_ptr );
    else
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type16                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type16( ILNODE *instr )                              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 16 instruction, and then writing the instruction to the     *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/19/89       created                  -----             *
***********************************************************************/

void instr_type16( register ILNODE *instr )
{
    register long data;
    register short ireg;

    memset( this_is_it, 0, sizeof(PM_WORD) );

   
    if ( instr->g == G_DATA_MEMORY )
    {
         data = instr->dm_disp.value.int_val;
         ireg = instr->dm_ireg;
    }
    else
    { 
         data = PM_ADDRESS ( instr->pm_disp.value.int_val );
         ireg = instr->pm_ireg;
    }

    if( (IS_DAG1_REG(ireg) && IS_DAG1_REG(instr->ureg)) || 
        (IS_DAG2_REG(ireg) && IS_DAG2_REG(instr->ureg)) )
    {
        if( instr->d != D_READ )
            USER_ERROR( "Multiple accesses of the same DAG are illegal." );	
    }

    if( !Z3 && (instr->ureg == REG_PX && instr->g == G_DATA_MEMORY) )
        USER_ERROR( "Transfers between Data Memory and the PX register are illegal." );

    this_is_it[0] =  (char) (0xA0L | (instr->g << 4) | (DAG_REG(ireg) << 1) | instr->d);
    this_is_it[1] = (char) instr->ureg;
    this_is_it[2] = (char) DATA32_TOP8(data);
    this_is_it[3] = (char) DATA32_TOP_MIDDLE8(data);
    this_is_it[4] = (char) DATA32_MIDDLE8(data);
    this_is_it[5] = (char) DATA32_BOTTOM8(data);

    if( listing_flag )
        listing_output_instruction( this_is_it );

    if( (instr->g == G_DATA_MEMORY) && (instr->dm_disp.sym_ptr != (SYMBOL *) NULL) )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_ADDR32, instr->dm_disp.sym_ptr );
    else if( instr->pm_disp.sym_ptr != (SYMBOL *) NULL )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_ADDR24, instr->pm_disp.sym_ptr );
    else
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type17                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type17( ILNODE *instr )                              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 17 instruction, and then writing the instruction to the     *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/19/89       created                  -----             *
***********************************************************************/

void instr_type17( register ILNODE *instr )
{
    PM_WORD        buf;
    long           data;
    register short mreg;
    register short ireg;
    unsigned long  mantissa;
    int            exponent;

    memset( this_is_it, 0, sizeof(PM_WORD) );

    if( instr->immediate.expr_type == EXPR_TYPE_INT )
    {
        data = instr->immediate.value.int_val;	

        this_is_it[2] = (char) DATA32_TOP8(data);
        this_is_it[3] = (char) DATA32_TOP_MIDDLE8(data);
        this_is_it[4] = (char) DATA32_MIDDLE8(data);
        this_is_it[5] = (char) DATA32_BOTTOM8(data);	
    }
    else if( instr->immediate.expr_type == EXPR_TYPE_FLOAT && !(instr->immediate.sym_ptr) )
    {
        convert_to_ieee( instr->immediate.value.float_val, round_mode, 
                         SINGLE_PRECISION, &mantissa, &exponent );
        format_ieee( mantissa, exponent, buf, SINGLE_PRECISION );	

        this_is_it[2] = buf[0];
        this_is_it[3] = buf[1];
        this_is_it[4] = buf[2];
        this_is_it[5] = buf[3];
    }
    else
    {
        ASSEMBLER_ERROR( "Illegal expression type." );
        return;
    }

    if ( instr->g == G_DATA_MEMORY )
    {
         ireg = (short) DAG_REG( instr->dm_ireg );
         mreg = (short) DAG_REG( instr->dm_mreg );
    }
    else
    {
        ireg = (short) DAG_REG( instr->pm_ireg );
        mreg = (short) DAG_REG( instr->pm_mreg );
    }

    this_is_it[0] = (char) (0x90L | (ireg << 1) | ((mreg & 0x04L) >> 2));
    this_is_it[1] = (char) (((mreg & 0x03L) << 6) | (instr->g << 5));

    if( listing_flag )
        listing_output_instruction( this_is_it );

    if( instr->length != AFFIRMATIVE && instr->immediate.sym_ptr != (SYMBOL *) NULL )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_ADDR32, instr->immediate.sym_ptr );
    else
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type18                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type18( ILNODE *instr )                              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 18 instruction, and then writing the instruction to the     *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/19/89       created                  -----             *
***********************************************************************/

void instr_type18( register ILNODE *instr )
{
    DM_WORD        buf;
    long           data;
    unsigned long  mantissa;
    int            exponent;

    memset( this_is_it, 0, sizeof(PM_WORD) );

    if( instr->immediate.expr_type == EXPR_TYPE_INT )
    {
        data = instr->immediate.value.int_val;	
        this_is_it[2] = (char) DATA32_TOP8(data);
        this_is_it[3] = (char) DATA32_TOP_MIDDLE8(data);
        this_is_it[4] = (char) DATA32_MIDDLE8(data);
        this_is_it[5] = (char) DATA32_BOTTOM8(data);
    }
    else if( instr->immediate.expr_type == EXPR_TYPE_FLOAT && !(instr->immediate.sym_ptr) )
    {
        convert_to_ieee( instr->immediate.value.float_val, round_mode, 
                         SINGLE_PRECISION, &mantissa, &exponent );
        format_ieee( mantissa, exponent, buf, SINGLE_PRECISION );	

        this_is_it[2] = buf[0];
        this_is_it[3] = buf[1];
        this_is_it[4] = buf[2];
        this_is_it[5] = buf[3];
    }
    else
    {
        ASSEMBLER_ERROR( "Illegal expression type in instruction type 18." );
        return;
    }

    this_is_it[0] = (char) 0x0FL;
    this_is_it[1] = (char) instr->ureg;

    if( listing_flag )
        listing_output_instruction( this_is_it );

    if( instr->length != AFFIRMATIVE && instr->immediate.sym_ptr != (SYMBOL *) NULL )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_ADDR32, instr->immediate.sym_ptr );
    else
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type19                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type19( ILNODE *instr )                              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 19 instruction, and then writing the instruction to the     *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/19/89       created                  -----             *
***********************************************************************/

void instr_type19( register ILNODE *instr )
{

    memset( this_is_it, 0, sizeof(PM_WORD) );

    this_is_it[0] = (char) 0x05L;
    this_is_it[1] = (char) ((instr->cs << 6) | ((instr->cache_reg & 0x1FL) << 1) | (instr->s));
    this_is_it[2] = (char) ((instr->d << 7) | ((instr->ureg & 0xFEL) >> 1));
    this_is_it[3] = (char) (((instr->ureg & 0x01L) << 7));

    if( listing_flag )
        listing_output_instruction( this_is_it );

    WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type20                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type20( ILNODE *instr )                              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 20 instruction, and then writing the instruction to the     *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/19/89       created                  -----             *
***********************************************************************/

void instr_type20( register ILNODE *instr )
{
    register long data;
    memset( this_is_it, 0, sizeof(PM_WORD) );

   if( instr->immediate.expr_type == EXPR_TYPE_INT && !(instr->immediate.sym_ptr) )
       data = instr->immediate.value.int_val;
   else
   {
       USER_ERROR( "Illegal immediate value in bit instruction." );
       return;
   }

   this_is_it[0] = (char) 0x14L;
   this_is_it[1] = (char) (((instr->bop & 0x7L) << 5) | (instr->sreg & 0xFL));
   this_is_it[2] = (char) DATA32_TOP8(data);
   this_is_it[3] = (char) DATA32_TOP_MIDDLE8(data);
   this_is_it[4] = (char) DATA32_MIDDLE8(data);
   this_is_it[5] = (char) DATA32_BOTTOM8(data);

    if( listing_flag )
        listing_output_instruction( this_is_it );

   WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type21                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type21( ILNODE *instr )                              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 21 instruction, and then writing the instruction to the     *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/19/89       created                  -----             *
***********************************************************************/

void instr_type21( register ILNODE *instr )
{
    memset( this_is_it, 0, sizeof(PM_WORD) );

    if( instr->immediate.expr_type == EXPR_TYPE_INT && !(instr->immediate.sym_ptr) )
        ;
    else
    {
        USER_ERROR( "Illegal immediate value in trap instruction." );
        return;
    }
        
    this_is_it[0] = (char) 0x15L;
    this_is_it[1] = (char) (instr->cond << 1);
    this_is_it[4] = (char) (instr->immediate.value.int_val & 0x7L);

    if( listing_flag )
        listing_output_instruction( this_is_it );

   WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type22                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type22( ILNODE *instr )                              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 22 instruction, and then writing the instruction to the     *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/19/89       created                  -----             *
***********************************************************************/

void instr_type22( register ILNODE *instr )
{
    register short ireg;
    register long data;

    memset( this_is_it, 0, sizeof(PM_WORD) );

    if( instr->immediate.sym_ptr )
	data = instr->immediate.value.int_val;
    else if( instr->immediate.expr_type == EXPR_TYPE_INT )
	data = instr->immediate.value.int_val;

    ireg = (short) ((instr->g == G_DATA_MEMORY) ? DAG_REG( instr->dm_ireg ) : 
           DAG_REG( instr->pm_ireg ));
    this_is_it[0] = (char) 0x16L;
    this_is_it[1] = (char) ((instr->bit_rev << 7) | (instr->g << 6) | ireg);
    this_is_it[2] = (char) DATA32_TOP8(data);
    this_is_it[3] = (char) DATA32_TOP_MIDDLE8(data);
    this_is_it[4] = (char) DATA32_MIDDLE8(data);
    this_is_it[5] = (char) DATA32_BOTTOM8(data);

    if( listing_flag )
        listing_output_instruction( this_is_it );

    if( instr->immediate.sym_ptr != (SYMBOL *) NULL )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_ADDR32, instr->immediate.sym_ptr );
    else
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type23                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type23( ILNODE *instr )                              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 23 instruction, and then writing the instruction to the     *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/19/89       created                  -----             *
***********************************************************************/

void instr_type23( register ILNODE *instr )
{

    memset( this_is_it, 0, sizeof(PM_WORD) );

    this_is_it[0] = (char) 0x17L;
    this_is_it[1] = (char) ((instr->lpu << 7) | (instr->lpo << 6) | 
			    (instr->spu << 5) | (instr->spo << 4) |
			    (instr->ppu << 3) | (instr->ppo << 2));
    if( instr->s )
	this_is_it[1] |= 2;

    if( listing_flag )
        listing_output_instruction( this_is_it );

    WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type24                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type24( ILNODE *instr )                              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 24 instruction, and then writing the instruction to the     *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/19/89       created                  -----             *
***********************************************************************/

void instr_type24( register ILNODE *instr )
{

    instr;
    memset( this_is_it, 0, sizeof(PM_WORD) );

    this_is_it[5] = instr->immediate.value.int_val;

    if( listing_flag )
        listing_output_instruction( this_is_it );

    WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type25                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type25( ILNODE *instr )                              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 25 instruction, and then writing the instruction to the     *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/19/89       created                  -----             *
***********************************************************************/

void instr_type25( register ILNODE *instr )
{

    instr;
    memset( this_is_it, 0, sizeof(PM_WORD) );

    this_is_it[1] = (char) 0x80L;

    if( listing_flag )
        listing_output_instruction( this_is_it );

    WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      instr_type25a                                                   *
*                                                                      *
*   Synopsis                                                           *
*      void instr_type25a( ILNODE *instr )                             *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for putting together the pieces for *
*   a type 25a instruction, and then writing the instruction to the    *
*   intermediate file.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mystic  10/1/96       created                  -----             *
***********************************************************************/

void instr_type25a( register ILNODE *instr )
{

    instr;
    memset( this_is_it, 0, sizeof(PM_WORD) );

    this_is_it[1] = (char) 0xA0L;

    if( listing_flag )
        listing_output_instruction( this_is_it );

    WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}

void instr_type27( register ILNODE *instr )
{
	memset( this_is_it, 0, sizeof(PM_WORD) );

	this_is_it[0] = 0x19;

	if( listing_flag )
		listing_output_instruction( this_is_it );

	WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}


void instr_type28( register ILNODE *instr )
{
	memset( this_is_it, 0, sizeof(PM_WORD) );

	this_is_it[1] = 0xC0;

	if( listing_flag )
		listing_output_instruction( this_is_it );

	WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}

void instr_type29( register ILNODE *instr )
{
    register long address;

    memset( this_is_it, 0, sizeof(PM_WORD) );

    if( instr->address_type == ADDRESS_DIRECT)
    {
        address = PM_ADDRESS(instr->pm_addr.value.int_val);
        this_is_it[0] = 0x18L;
	this_is_it[1] = 0x04L;
    } else if ( instr->address_type == ADDRESS_PC_RELATIVE)
    {
        address = PM_ADDRESS( instr->pc_disp.value.int_val );
        this_is_it[0] = 0x18L;
	this_is_it[1] = 0x44L;
    } 
    this_is_it[3] = (char) PM_ADDRESS_TOP8(address);
    this_is_it[4] = (char) PM_ADDRESS_MIDDLE8(address);
    this_is_it[5] = (char) PM_ADDRESS_BOTTOM8(address);

    if( listing_flag )
        listing_output_instruction( this_is_it );
     
    if( ((instr->address_type == ADDRESS_DIRECT) || (instr->address_type == ADDRESS_PC_RELATIVE))
          && (instr->pm_addr.sym_ptr != (SYMBOL *) NULL) )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_ADDR24, instr->pm_addr.sym_ptr );
    else if( instr->pc_disp.sym_ptr != (SYMBOL *) NULL )
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, RELOC_PC_REL_LONG, 
                             instr->pc_disp.sym_ptr );
    else
        WRITE_INTERMEDIATE( this_is_it, PM_WORD_SIZE, NO_ACTION, NULL );
}





/***********************************************************************
*                                                                      *
*   Name                                                               *
*      last_minute_error_checking                                      *
*                                                                      *
*   Synopsis                                                           *
*      int last_minute_error_checking( ILNODE *instr )                 *
*                                                                      *
*   Description                                                        *
*      Do any last minute error checks.                                *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     7/31/89       created                  -----             *
***********************************************************************/

int last_minute_error_checking( register ILNODE *instr )
{
    if( instr->cond_kludge == AFFIRMATIVE )
        switch( (int) instr->instruction_type )
        {
	    case INSTRUCTION_TYPE2:
	    case INSTRUCTION_TYPE3:
	    case INSTRUCTION_TYPE4:
	    case INSTRUCTION_TYPE5:
	    case INSTRUCTION_TYPE6:
	    case INSTRUCTION_TYPE7:
	    case INSTRUCTION_TYPE8:
	    case INSTRUCTION_TYPE9:
	    case INSTRUCTION_TYPE10:
	    case INSTRUCTION_TYPE11:
	    case INSTRUCTION_TYPE12:
               if( instr->cond == COND_LCE )
               {
              USER_ERROR( "The loop counter expired (LCE) condition is illegal in an if statement." );
                   return( 1 );
	       }
        }
    return( 0 );
}
