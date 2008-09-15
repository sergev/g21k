/* @(#)make_ilnode.c	2.2 1/13/94 2 */

#include "app.h"
#include "symbol.h"
#include "expr.h"
#include "lexical.h"
#include "opcodes.h"
#include "reg.h"
#include "ilnode.h"
#include "error.h"

#include "make_ilnode_fp.h"


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      make_alu                                                        *
*                                                                      *
*   Synopsis                                                           *
*      void make_alu( long xreg, long yreg, long dest_reg,             *
                      long opcode )                                    *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating the intermediate       *
*   language representation of a alu instruction.                      *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/12/89       created                  -----             *
***********************************************************************/

void make_alu( long xreg, long yreg, long dest_reg, long opcode )
{
    register ILNODE *il_copy;

    il_copy = &ilnode;

    switch( (int) opcode )
    {
        case  FIXED_ALU_ADD:
        case  FIXED_ALU_SUB:
        case  FIXED_ALU_ADD_DIV_BY_2:
        case  FIXED_ALU_ADD_X_WITH_CARRY:
        case  FIXED_ALU_ADD_X_Y_WITH_CARRY:
        case  FIXED_ALU_ADD_X_WITH_BORROW:
        case  FIXED_ALU_SUB_WITH_BORROW:
        case  FIXED_ALU_INCREMENT:
        case  FIXED_ALU_DECREMENT:
        case  FIXED_ALU_NEGATE:
        case  FIXED_ALU_ABS:
        case  FIXED_ALU_PASS:
        case  FIXED_ALU_AND:
        case  FIXED_ALU_OR:
        case  FIXED_ALU_XOR:
        case  FIXED_ALU_NOT:
        case  FIXED_ALU_MIN:
        case  FIXED_ALU_MAX:
        case  FIXED_ALU_CLIP:
        case  FIXED_ALU_COMP:

        case  FP_ALU_ADD:
        case  FP_ALU_SUB:
        case  FP_ALU_NEGATE:
        case  FP_ALU_COPYSIGN:
        case  FP_ALU_ADD_DIV_BY_2:
        case  FP_ALU_ABS:
        case  FP_ALU_ABS_X_PLUS_Y:
        case  FP_ALU_ABS_X_MINUS_Y:
        case  FP_ALU_MIN:
        case  FP_ALU_MAX:
        case  FP_ALU_CLIP:
        case  FP_ALU_LOGB:
        case  FP_ALU_SCALB:
        case  FP_ALU_MANT:
        case  FP_ALU_FIX_1OP:
        case  FP_ALU_FIX_2OP:
        case  FP_ALU_FLOAT_1OP:
        case  FP_ALU_FLOAT_2OP:
        case  FP_ALU_COMP:
        case  FP_ALU_RND:
        case  FP_ALU_PASS:
        case  FP_ALU_RECIPS:
        case  FP_ALU_RSQRTS:
      case FP_ALU_FPACK:
      case FP_ALU_FUNPACK:
      case FP_ALU_TRUNC:
      case FP_ALU_TRUNC_BY:
            if( il_copy->compute.compute_type == COMPUTE_NULL )
            {
                il_copy->compute.x1_reg = (short)xreg;
                il_copy->compute.y1_reg = (short)  yreg;
                il_copy->compute.result1_reg = (short)  dest_reg;
                il_copy->compute.computation_unit = ALU;
                il_copy->compute.opcode = opcode;
                il_copy->compute.compute_type = COMPUTE_TYPE1;    
            }
            else if( il_copy->compute.compute_type == COMPUTE_TYPE1 )
            {
                if( il_copy->compute.computation_unit == ALU )
                {
                    /* Check for dual add subtract */

                    if( (il_copy->compute.opcode == FIXED_ALU_ADD && opcode == FIXED_ALU_SUB) ||
                        (il_copy->compute.opcode == FP_ALU_ADD && opcode == FP_ALU_SUB) )
                    {
                         if( (il_copy->compute.x1_reg == (short) xreg) && 
                             (il_copy->compute.y1_reg == (short) yreg) )
                         {
                             il_copy->compute.x2_reg = (short)  xreg;
                             il_copy->compute.y2_reg = (short)  yreg;
                             il_copy->compute.result2_reg = (short)  dest_reg;
                             il_copy->compute.opcode = (opcode == FP_ALU_SUB || opcode == 
                                                        FP_ALU_ADD) ? FLOAT_DUAL_ADDSUB : 
                                                        FIXED_DUAL_ADDSUB;
                             il_copy->compute.compute_type = COMPUTE_TYPE2;
                             il_copy->compute.computation_unit = ALU;

                             if( il_copy->compute.result1_reg == (short) dest_reg )
                                 USER_WARN( "Destination register in dual add/subtract is the same.");
                         }
                         else
                            USER_ERROR( "Illegal dual add-subtract." );
                    }
                    else
                       USER_ERROR( "Illegal alu instruction." );
                }
                else if( il_copy->compute.computation_unit == MULTIPLIER )
                {
                    if( check_multifunction_type(opcode) )
                    {
                        if( check_multifunction_registers( (int) il_copy->compute.x1_reg,
                            (int) il_copy->compute.y1_reg, (int) xreg, (int) yreg, (int) opcode) )
                        {
                            il_copy->compute.x2_reg = (short)  xreg;
                            il_copy->compute.y2_reg = (short)  yreg;
                            il_copy->compute.result2_reg = (short)  dest_reg;
                            il_copy->compute.opcode = get_multifunction_opcode( 
                                                      il_copy->compute.opcode,
                                                      opcode, COMPUTE_TYPE3 );
                            il_copy->compute.compute_type = COMPUTE_TYPE3;
                            il_copy->compute.computation_unit = DONT_CARE;			    
			}

                    }
                    else
                        USER_ERROR( "Illegal multifunction instruction." );
                }
                else
                    USER_ERROR( "Illegal multifunction instruction." );
            }    
            else if( il_copy->compute.compute_type == COMPUTE_TYPE3 )
            {
                     if( (il_copy->compute.x2_reg == (short) xreg) && 
                         (il_copy->compute.y2_reg == (short) yreg))
                     {
                          if( check_multifunction_registers( (int) il_copy->compute.x1_reg,
                              (int) il_copy->compute.y1_reg, (int) xreg, (int) yreg, (int) opcode) )
                          {
                              il_copy->compute.x3_reg = (short)  xreg;
                              il_copy->compute.y3_reg = (short)  yreg;
                              il_copy->compute.result3_reg = (short)  dest_reg;
                              il_copy->compute.opcode = get_multifunction_opcode(
                                                        il_copy->compute.opcode,
                                                        opcode, COMPUTE_TYPE4 );

                              il_copy->compute.compute_type = COMPUTE_TYPE4;
                              il_copy->compute.computation_unit = DONT_CARE;
                              if( il_copy->compute.result2_reg == (short) dest_reg )
                                  USER_WARN( "Destination register in dual add/subtract is the same.");			      
			  }

                     }
                     else
                         USER_ERROR( "Illegal registers for dual add/subtract in multifunction instruction." ); 
            }
            else
                USER_ERROR( "Illegal compute instruction." );
            break;

        default:
            ASSEMBLER_ERROR( "Illegal alu opcode." );
            break;
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      make_shifter                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void make_shifter( long x_operand, long y_operand, long dest,   *
*                         long opcode, long shift_by )                 *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating the intermediate       *
*   language representation of a shifter instruction.                  *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/12/89       created                  -----             *
***********************************************************************/

void make_shifter( long x_operand, long y_operand, long dest, register long opcode, long shift_by )
{
     register ILNODE *il_copy;

     il_copy = &ilnode;

     if( SHIFTER_21040_INST(opcode) )
     {
	 FATAL_ERROR( "Opcodes for the 21040 shifter instructions have not been determinded." );
     }

     switch ( (int) opcode )
     {

/*
        case  SHIFTER_FMERG:
        case  SHIFTER_PACK:
        case  SHIFTER_UNPACK:
        case  SHIFTER_UNPACK_NO_UP:
        case  SHIFTER_UNPACK_SIGN_EXT:
        case  SHIFTER_UNPACK_NO_SIGN:
        case  SHIFTER_NOFZ:
        case  SHIFTER_NOFO:
           if( !IS_21040 )  
           {
               USER_ERROR( "Illegal shift instruction, only available on 21040." );
               ilnode.error_occurred = ASM_ERROR;
               break;
	   }
 */
        case  SHIFTER_LSHIFT:
        case  SHIFTER_OR_LSHIFT:
        case  SHIFTER_ASHIFT:
        case  SHIFTER_OR_ASHIFT:
        case  SHIFTER_ROT:
        case  SHIFTER_BCLR:
        case  SHIFTER_BSET:
        case  SHIFTER_BTGL:
        case  SHIFTER_BTST:
        case  SHIFTER_FDEP:
        case  SHIFTER_FDEP_SIGN_EXT:
        case  SHIFTER_OR_FDEP:
        case  SHIFTER_OR_FDEP_SIGN_EXT:
        case  SHIFTER_FEXT:
        case  SHIFTER_FEXT_SIGN_EXT:
        case  SHIFTER_EXP:
        case  SHIFTER_EXP_EXTENDED:
        case  SHIFTER_LEFTZ:
        case  SHIFTER_LEFTO:
           il_copy->compute.x1_reg = (short) x_operand;

           if( shift_by == SHIFT_IMMEDIATE )
           {
               il_copy->shift_immediate = y_operand;
               il_copy->compute.y1_reg = NULL_OPERAND;
               il_copy->shift_type = shift_by;
           }
           else
           {
               il_copy->compute.y1_reg = (short)  y_operand;
               il_copy->shift_type = shift_by;
           }

           il_copy->compute.result1_reg = (short)  dest;
           il_copy->compute.compute_type = COMPUTE_TYPE1;
           il_copy->compute.computation_unit = SHIFTER;
           il_copy->compute.opcode = opcode;
           break;

        default:
            ASSEMBLER_ERROR( "Illegal shifter opcode." );
            break;
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      make_function_unit                                              *
*                                                                      *
*   Synopsis                                                           *
*      void make_function_unit( long x_operand, long y_operand,        *
*                               long dest, long modifier, long opcode )*
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating the intermediate       *
*   language representation of a function unit instruction.            *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/12/89       created                  -----             *
***********************************************************************/

void make_function_unit( long x_operand, long y_operand, long dest, long modifier, 
                         register long opcode )
{
    register ILNODE *il_copy;

    il_copy = &ilnode;

    if( !IS_21040 )
    {
        USER_ERROR( "Function unit instructions are only available on the 21040." );
        il_copy->error_occurred = ASM_ERROR;
        return;
    }

    if( dest != NULL_TRANSFER )
        modifier |= FUNCTION_REGISTER_TRANSFER;

    switch( (int) opcode )
    {
            case FUNCTION_TRANSFER:
            case FUNCTION_DIVIDE:
            case FUNCTION_1_DIV_N:
            case FUNCTION_MOD:
            case FUNCTION_SQR:
            case FUNCTION_SQR_1_DIV_N:
            case FUNCTION_EXP2:
            case FUNCTION_LOG2:
            case FUNCTION_SIN:
            case FUNCTION_COS:
            case FUNCTION_ACT:
            case FUNCTION_STEP:
               il_copy->compute.x1_reg = (short)  x_operand;
               il_copy->compute.y1_reg = (short)  y_operand;

               if( dest != NULL_TRANSFER )
                   il_copy->compute.result1_reg = (short)  dest;
               else
                   il_copy->compute.result1_reg = NULL_OPERAND;

               il_copy->compute.opcode = opcode | modifier;
               il_copy->compute.computation_unit = FUNCTION_UNIT;
               il_copy->compute.compute_type = COMPUTE_TYPE1;
               break;

            default:
               ASSEMBLER_ERROR( "Illegal function unit opcode." );
               break;
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      mult_opcode                                                     *
*                                                                      *
*   Synopsis                                                           *
*      long mult_opcode( long type, long modifier )                    *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for determining the multiplier      *
*   opcode, based on the instruction modifier.                         *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/12/89       created                  -----             *
***********************************************************************/

long mult_opcode( register long type, register long modifier )
{
    switch( (int) type )
    {
            case MULTIPLIER_FIXED_MULT:
               switch( (int) modifier )
               {
                       case SSI:
                          return( MULTIPLIER_FIXED_MULT_SSI );
                       case SUI:
                          return( MULTIPLIER_FIXED_MULT_SUI );
                       case USI:
                          return( MULTIPLIER_FIXED_MULT_USI );
                       case UUI:
                          return( MULTIPLIER_FIXED_MULT_UUI );
                       case SSF:
                       case DEFAULT_MULT_MODIFIER:
                          return( MULTIPLIER_FIXED_MULT_SSF );
                       case SUF:
                          return( MULTIPLIER_FIXED_MULT_SUF );
                       case USF:
                          return( MULTIPLIER_FIXED_MULT_USF );
                       case UUF:
                          return( MULTIPLIER_FIXED_MULT_UUF );
                       case SSIR:
                          return( MULTIPLIER_FIXED_MULT_SSIR );
                       case SUIR:
                          return( MULTIPLIER_FIXED_MULT_SUIR );
                       case USIR:
                          return( MULTIPLIER_FIXED_MULT_USIR );
                       case UUIR:
                          return( MULTIPLIER_FIXED_MULT_UUIR );
                       case SSFR:
                          return( MULTIPLIER_FIXED_MULT_SSFR );
                       case SUFR:
                          return( MULTIPLIER_FIXED_MULT_SUFR );
                       case USFR:
                          return( MULTIPLIER_FIXED_MULT_USFR );
                       case UUFR:
                          return( MULTIPLIER_FIXED_MULT_UUFR );
                       default:
                          ASSEMBLER_ERROR( "Illegal multiplier modifier." );
                          return( 0L );
               }

            case MULTIPLIER_MULT_ACCUM_ADD:
               switch( (int) modifier )
               {
                       case SSI:
                          return( MULTIPLIER_MULT_ACCUM_ADD_SSI );
                       case SUI:
                          return( MULTIPLIER_MULT_ACCUM_ADD_SUI );
                       case USI:
                          return( MULTIPLIER_MULT_ACCUM_ADD_USI );
                       case UUI:
                          return( MULTIPLIER_MULT_ACCUM_ADD_UUI );
                       case SSF:
                       case DEFAULT_MULT_MODIFIER:
                          return( MULTIPLIER_MULT_ACCUM_ADD_SSF );
                       case SUF:
                          return( MULTIPLIER_MULT_ACCUM_ADD_SUF );
                       case USF:
                          return( MULTIPLIER_MULT_ACCUM_ADD_USF );
                       case UUF:
                          return( MULTIPLIER_MULT_ACCUM_ADD_UUF );
                       case SSIR:
                          return( MULTIPLIER_MULT_ACCUM_ADD_SSIR );
                       case SUIR:
                          return( MULTIPLIER_MULT_ACCUM_ADD_SUIR );
                       case USIR:
                          return( MULTIPLIER_MULT_ACCUM_ADD_USIR );
                       case UUIR:
                          return( MULTIPLIER_MULT_ACCUM_ADD_UUIR );
                       case SSFR:
                          return( MULTIPLIER_MULT_ACCUM_ADD_SSFR );
                       case SUFR:
                          return( MULTIPLIER_MULT_ACCUM_ADD_SUFR );
                       case USFR:
                          return( MULTIPLIER_MULT_ACCUM_ADD_USFR );
                       case UUFR:
                          return( MULTIPLIER_MULT_ACCUM_ADD_UUFR );
                       default:
                          ASSEMBLER_ERROR( "Illegal multiplier modifier." );
                          return( 0L );
               }

            case MULTIPLIER_MULT_ACCUM_SUB:
               switch( (int) modifier )
               {
                       case SSI:
                          return( MULTIPLIER_MULT_ACCUM_SUB_SSI );
                       case SUI:
                          return( MULTIPLIER_MULT_ACCUM_SUB_SUI );
                       case USI:
                          return( MULTIPLIER_MULT_ACCUM_SUB_USI );
                       case UUI:
                          return( MULTIPLIER_MULT_ACCUM_SUB_UUI );
                       case SSF:
                       case DEFAULT_MULT_MODIFIER:
                          return( MULTIPLIER_MULT_ACCUM_SUB_SSF );
                       case SUF:
                          return( MULTIPLIER_MULT_ACCUM_SUB_SUF );
                       case USF:
                          return( MULTIPLIER_MULT_ACCUM_SUB_USF );
                       case UUF:
                          return( MULTIPLIER_MULT_ACCUM_SUB_UUF );
                       case SSIR:
                          return( MULTIPLIER_MULT_ACCUM_SUB_SSIR );
                       case SUIR:
                          return( MULTIPLIER_MULT_ACCUM_SUB_SUIR );
                       case USIR:
                          return( MULTIPLIER_MULT_ACCUM_SUB_USIR );
                       case UUIR:
                          return( MULTIPLIER_MULT_ACCUM_SUB_UUIR );
                       case SSFR:
                          return( MULTIPLIER_MULT_ACCUM_SUB_SSFR );
                       case SUFR:
                          return( MULTIPLIER_MULT_ACCUM_SUB_SUFR );
                       case USFR:
                          return( MULTIPLIER_MULT_ACCUM_SUB_USFR );
                       case UUFR:
                          return( MULTIPLIER_MULT_ACCUM_SUB_UUFR );
                       default:
                          ASSEMBLER_ERROR( "Illegal multiplier modifier." );
                          return( 0L );
               }

            case MULTIPLIER_SATURATE:
               switch( (int) modifier )
               {
                       case SI:
                          return( MULTIPLIER_SATURATE_SI );
                       case UI:
                          return( MULTIPLIER_SATURATE_UI );
                       case SF:
                       case DEFAULT_MULT_MODIFIER:
                          return( MULTIPLIER_SATURATE_SF );
                       case UF:
                          return( MULTIPLIER_SATURATE_UF );
                       default:
                          ASSEMBLER_ERROR( "Illegal multiplier modifier." );
                          return( 0L );
               }                         

           case MULTIPLIER_RND:
              switch( (int) modifier )
              {
                      case DEFAULT_MULT_MODIFIER:
		      case SF:
                         return( MULTIPLIER_RND_SF );

                      case UF:
                         return( MULTIPLIER_RND_UF );
	      }

           default:
              ASSEMBLER_ERROR( "Illegal multiplier type." );
              return( 0L );
    }           
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      make_multiplier                                                 *
*                                                                      *
*   Synopsis                                                           *
*      long make_multiplier( long x_operand, long y_operand, long dest *
*                            , long opcode, long modifier )            *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating the intermediate       *
*   language representation of a multiplier instruction.               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/12/89       created                  -----             *
***********************************************************************/

void make_multiplier( long x_operand, long y_operand, long dest, long opcode, long modifier )
{
    register ILNODE *il_copy;
  
    il_copy = &ilnode;

    switch( (int) opcode )
    {
            case MULTIPLIER_FIXED_MULT_SSI:
            case MULTIPLIER_FIXED_MULT_SUI:
            case MULTIPLIER_FIXED_MULT_USI:
            case MULTIPLIER_FIXED_MULT_UUI:
            case MULTIPLIER_FIXED_MULT_SSF:
            case MULTIPLIER_FIXED_MULT_SUF:
            case MULTIPLIER_FIXED_MULT_USF:
            case MULTIPLIER_FIXED_MULT_UUF:
            case MULTIPLIER_FIXED_MULT_SSIR:
            case MULTIPLIER_FIXED_MULT_SUIR:
            case MULTIPLIER_FIXED_MULT_USIR:
            case MULTIPLIER_FIXED_MULT_UUIR:
            case MULTIPLIER_FIXED_MULT_SSFR:
            case MULTIPLIER_FIXED_MULT_SUFR:
            case MULTIPLIER_FIXED_MULT_USFR:
            case MULTIPLIER_FIXED_MULT_UUFR:

            case MULTIPLIER_MULT_ACCUM_ADD_SSI:
            case MULTIPLIER_MULT_ACCUM_ADD_SUI:
            case MULTIPLIER_MULT_ACCUM_ADD_USI:
            case MULTIPLIER_MULT_ACCUM_ADD_UUI:
            case MULTIPLIER_MULT_ACCUM_ADD_SSF:
            case MULTIPLIER_MULT_ACCUM_ADD_SUF:
            case MULTIPLIER_MULT_ACCUM_ADD_USF:
            case MULTIPLIER_MULT_ACCUM_ADD_UUF:
            case MULTIPLIER_MULT_ACCUM_ADD_SSIR:
            case MULTIPLIER_MULT_ACCUM_ADD_SUIR:
            case MULTIPLIER_MULT_ACCUM_ADD_USIR:
            case MULTIPLIER_MULT_ACCUM_ADD_UUIR:
            case MULTIPLIER_MULT_ACCUM_ADD_SSFR:
            case MULTIPLIER_MULT_ACCUM_ADD_SUFR:
            case MULTIPLIER_MULT_ACCUM_ADD_USFR:
            case MULTIPLIER_MULT_ACCUM_ADD_UUFR:

            case MULTIPLIER_MULT_ACCUM_SUB_SSI:
            case MULTIPLIER_MULT_ACCUM_SUB_SUI:
            case MULTIPLIER_MULT_ACCUM_SUB_USI:
            case MULTIPLIER_MULT_ACCUM_SUB_UUI:
            case MULTIPLIER_MULT_ACCUM_SUB_SSF:
            case MULTIPLIER_MULT_ACCUM_SUB_SUF:
            case MULTIPLIER_MULT_ACCUM_SUB_USF:
            case MULTIPLIER_MULT_ACCUM_SUB_UUF:
            case MULTIPLIER_MULT_ACCUM_SUB_SSIR:
            case MULTIPLIER_MULT_ACCUM_SUB_SUIR:
            case MULTIPLIER_MULT_ACCUM_SUB_USIR:
            case MULTIPLIER_MULT_ACCUM_SUB_UUIR:
            case MULTIPLIER_MULT_ACCUM_SUB_SSFR:
            case MULTIPLIER_MULT_ACCUM_SUB_SUFR:
            case MULTIPLIER_MULT_ACCUM_SUB_USFR:
            case MULTIPLIER_MULT_ACCUM_SUB_UUFR:

            case MULTIPLIER_SATURATE_SI:
            case MULTIPLIER_SATURATE_UI:
            case MULTIPLIER_SATURATE_SF:
            case MULTIPLIER_SATURATE_UF:

            case MULTIPLIER_RND_SF:
            case MULTIPLIER_RND_UF:

            case MULTIPLIER_CLR_MRF:
            case MULTIPLIER_CLR_MRB:

            case MULTIPLIER_FP_MULT:
               if( il_copy->compute.compute_type == COMPUTE_NULL )
               {
                   il_copy->compute.x1_reg = (short) x_operand;
                   il_copy->compute.y1_reg = (short) y_operand;
                   il_copy->compute.result1_reg = (short) dest;
                   il_copy->compute.computation_unit = MULTIPLIER;
                   il_copy->compute.opcode = opcode | modifier;
                   il_copy->compute.compute_type = COMPUTE_TYPE1;
               }
               else
               {
                   USER_ERROR( "Illegal instruction." );
                   il_copy->compute.x1_reg = NULL_OPERAND;
                   il_copy->compute.y1_reg = NULL_OPERAND;
                   il_copy->compute.result1_reg = NULL_OPERAND;
                   il_copy->compute.computation_unit = DONT_CARE;
                   il_copy->compute.opcode = -1;
                   il_copy->compute.compute_type = COMPUTE_NULL;
               }
               break;

            default:
               ASSEMBLER_ERROR( "Illegal multiplier opcode." );
               break;
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      make_transfer                                                   *
*                                                                      *
*   Synopsis                                                           *
*      void make_transfer( long data_reg, long cu_reg, long direction )*
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating an intermediate        *
*   language representation of transfer between a data file register   *
*   and a computation unit register.                                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/12/89       created                  -----             *
***********************************************************************/

void make_transfer( long data_reg, long cu_reg, long direction )
{
    register ILNODE *il_copy;

    il_copy = &ilnode;
    il_copy->compute.cu_reg = (short) cu_reg;
    il_copy->compute.data_reg =(short)  data_reg;
    il_copy->compute.t = (unsigned int) direction;
    il_copy->compute.compute_type = COMPUTE_TYPE5;
    il_copy->compute.computation_unit = DONT_CARE;
    il_copy->compute.opcode = DREG_CU_TRANSFER;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      check_multifunction_type                                        *
*                                                                      *
*   Synopsis                                                           *
*      int check_multifunction_type( long alu_opcode )                 *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for ensuring that only certain      *
*   combinations of multifunction instruction are permissable.         *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/9/89        created                  -----             *
***********************************************************************/

int check_multifunction_type( register long alu_opcode )
{
    register long multiplier_opcode;

    multiplier_opcode = ilnode.compute.opcode;
    switch( (int) (multiplier_opcode & AB_MASK) )
    {
            case MULTIPLIER_FIXED_MULT_SSFR:
               if( DESTINATION_IS_REG_FILE(multiplier_opcode) )
               {
                   switch( (int) alu_opcode )
                   {
                           case FIXED_ALU_ADD:
                           case FIXED_ALU_SUB:
                           case FIXED_ALU_ADD_DIV_BY_2:
                              return( 1 );

                           default:
                              return( 0 );
                   }
               }
               else
                  return( 0 );
         
            case MULTIPLIER_MULT_ACCUM_ADD_SSF:
            case MULTIPLIER_MULT_ACCUM_SUB_SSF:
               if( DESTINATION_IS_MRF_OR_MRB(multiplier_opcode) && SELECT_IS_MRF(multiplier_opcode) )
               {
                   switch( (int) alu_opcode )
                   {
                           case FIXED_ALU_ADD:
                           case FIXED_ALU_SUB:
                           case FIXED_ALU_ADD_DIV_BY_2:
                              return( 1 );

                           default:
                              return( 0 );
                   }
               }
               else
                  return( 0 );

            case MULTIPLIER_MULT_ACCUM_ADD_SSFR:
            case MULTIPLIER_MULT_ACCUM_SUB_SSFR:
               if( DESTINATION_IS_REG_FILE(multiplier_opcode) && SELECT_IS_MRF(multiplier_opcode) )
               {
                   switch( (int) alu_opcode )
                   {
                           case FIXED_ALU_ADD:
                           case FIXED_ALU_SUB:
                           case FIXED_ALU_ADD_DIV_BY_2:
                              return( 1 );

                           default:
                              return( 0 );
                   }
               }
               else
                  return( 0 );

            case MULTIPLIER_FP_MULT:
               switch( (int) alu_opcode )
               {
                       case FP_ALU_ADD:
                       case FP_ALU_SUB:
                       case FP_ALU_FLOAT_2OP:
                       case FP_ALU_FIX_2OP:
                       case FP_ALU_ADD_DIV_BY_2:
                       case FP_ALU_ABS:
                       case FP_ALU_MAX:
                       case FP_ALU_MIN:
                          return( 1 );

                       default:
                          return( 0 );
               }

            default:
               return( 0 );
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      get_multifunction_opcode                                        *
*                                                                      *
*   Synopsis                                                           *
*      long get_multifunction_opcode( long multiplier_opcode,          *
*                                     long alu_opcode,                 *
*                                     long compute_type )              *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for returning the opcode for a      *
*   multifunction instruction.                                         *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/9/89        created                  -----             *
***********************************************************************/

long get_multifunction_opcode( register long multiplier_opcode, long alu_opcode, long compute_type )
{
    switch( (int) compute_type )
    {
            case COMPUTE_TYPE3:
               switch( (int) (multiplier_opcode & AB_MASK) )
               {
                       case MULTIPLIER_FIXED_MULT_SSFR:
                          switch( (int) alu_opcode )
                          {
                                  case FIXED_ALU_ADD:
                                     return( MULTI_FIXED_MULT_WITH_ADD );

                                  case FIXED_ALU_SUB:
                                     return( MULTI_FIXED_MULT_WITH_SUB );

                                  case FIXED_ALU_ADD_DIV_BY_2:
                                     return( MULTI_FIXED_MULT_WITH_AVG );

                                  default:
                                     USER_ERROR( "Illegal multifunction instruction." );
                                     ilnode.error_occurred = ASM_ERROR;
                                     return( 0 );
                          }

                       case MULTIPLIER_MULT_ACCUM_ADD_SSF:
                          switch( (int) alu_opcode )
                          {
                                  case FIXED_ALU_ADD:
                                     return( MULTI_MRF_MULACC_ADD_WITH_ADD );

                                  case FIXED_ALU_SUB:
                                     return( MULTI_MRF_MULACC_ADD_WITH_SUB );

                                  case FIXED_ALU_ADD_DIV_BY_2:
                                     return( MULTI_MRF_MULACC_ADD_WITH_AVG );

                                  default:
                                     USER_ERROR( "Illegal multifunction instruction." );
                                     ilnode.error_occurred = ASM_ERROR;
                                     return( 0 );
                          }

                       case MULTIPLIER_MULT_ACCUM_ADD_SSFR:
                          switch( (int) alu_opcode )
                          {
                                  case FIXED_ALU_ADD:
                                     return( MULTI_MULACC_ADD_WITH_ADD );

                                  case FIXED_ALU_SUB:
                                     return( MULTI_MULACC_ADD_WITH_SUB );

                                  case FIXED_ALU_ADD_DIV_BY_2:
                                     return( MULTI_MULACC_ADD_WITH_AVG );

                                  default:
                                     USER_ERROR( "Illegal multifunction instruction." );
                                     ilnode.error_occurred = ASM_ERROR;
                                     return( 0 );
                          }

                       case MULTIPLIER_MULT_ACCUM_SUB_SSF:
                          switch( (int) alu_opcode )
                          {
                                  case FIXED_ALU_ADD:
                                     return( MULTI_MRF_MULACC_SUB_WITH_ADD );

                                  case FIXED_ALU_SUB:
                                     return( MULTI_MRF_MULACC_SUB_WITH_SUB );

                                  case FIXED_ALU_ADD_DIV_BY_2:
                                     return( MULTI_MRF_MULACC_SUB_WITH_AVG );

                                  default:
                                     USER_ERROR( "Illegal multifunction instruction." );
                                     ilnode.error_occurred = ASM_ERROR;
                                     return( 0 );
                          }

                       case MULTIPLIER_MULT_ACCUM_SUB_SSFR:
                          switch( (int) alu_opcode )
                          {
                                  case FIXED_ALU_ADD:
                                     return( MULTI_MULACC_SUB_WITH_ADD );

                                  case FIXED_ALU_SUB:
                                     return( MULTI_MULACC_SUB_WITH_SUB );

                                  case FIXED_ALU_ADD_DIV_BY_2:
                                     return( MULTI_MULACC_SUB_WITH_AVG );

                                  default:
                                     USER_ERROR( "Illegal multifunction instruction." );
                                     ilnode.error_occurred = ASM_ERROR;
                                     return( 0 );
                          }

                       case MULTIPLIER_FP_MULT:
                          switch( (int) alu_opcode )
                          {
                                  case FP_ALU_ADD:
                                     return( MULTI_FP_MULT_WITH_FP_ADD );

                                  case FP_ALU_SUB:
                                     return( MULTI_FP_MULT_WITH_FP_SUB );

                                  case FP_ALU_FLOAT_2OP:
                                     return( MULTI_FP_MULT_WITH_FP_FLOAT );

                                  case FP_ALU_FIX_2OP:
                                     return( MULTI_FP_MULT_WITH_FP_FIX );

                                  case FP_ALU_ADD_DIV_BY_2:
                                     return( MULTI_FP_MULT_WITH_FP_AVG );

                                  case FP_ALU_ABS:
                                     return( MULTI_FP_MULT_WITH_FP_ABS );

                                  case FP_ALU_MAX:
                                     return ( MULTI_FP_MULT_WITH_FP_MAX );

                                  case FP_ALU_MIN:
                                     return( MULTI_FP_MULT_WITH_FP_MIN );

                                  default:
                                     USER_ERROR( "Illegal multifunction instruction." );
                                     ilnode.error_occurred = ASM_ERROR;
                                     return( 0 );
                          }
               }


            case COMPUTE_TYPE4:
               switch( (int) multiplier_opcode )
               {     
                       case MULTI_FIXED_MULT_WITH_ADD:
                          if( alu_opcode == FIXED_ALU_SUB )
                              return( MULTI_MULT_WITH_DUAL_ADD_SUB );
                          else
                          {
                              USER_ERROR( "Illegal multifunction instruction." );
                              ilnode.error_occurred = ASM_ERROR;
                              return( 0 );
                          }

                       case MULTI_FP_MULT_WITH_FP_ADD:
                          if( alu_opcode == FP_ALU_SUB )
                              return( MULTI_FP_MULT_WITH_DUAL_ADD_SUB );
                          else
                          {
                              USER_ERROR( "Illegal multifunction instruction." );
                              ilnode.error_occurred = ASM_ERROR;
                              return( 0 );
                          }
               }

            default:
               ASSEMBLER_ERROR( "Illegal multifunction opcode." );
               ilnode.error_occurred = ASM_ERROR;
               return( 0 );
    }
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      check_multifunction_registers                                   *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for chaecking the inputs for the    *
*   multiplier and the alu in multifunction instruction.               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     10/16/89      created                  -----             *
***********************************************************************/

int check_multifunction_registers( int mult_x, int mult_y, int alu_x, int alu_y, int opcode )
{
    int error = FALSE;

    if( !(mult_x >= REG_R0 && mult_x <= REG_R3) )
    {
        USER_ERROR( "Illegal multiplier x operand, only registers (r0 - r3) allowed." );
        ilnode.error_occurred = ASM_ERROR;
        error = 1;	
    }

    if( !(mult_y >= REG_R4 && mult_y <= REG_R7) )
    {
        USER_ERROR( "Illegal multiplier y operand, only registers (r4 - r7) allowed." );
        ilnode.error_occurred = ASM_ERROR;
        error = 1;	
    }

    if( !(alu_x >= REG_R8 && alu_x <= REG_R11) )
    {
        USER_ERROR( "Illegal alu x operand, only registers (r8 - r11) allowed." );
        ilnode.error_occurred = ASM_ERROR;
        error = 1;	
    }


    if( opcode != FP_ALU_ABS )                    /* ABS instruction has no y-operand */  
        if( !(alu_y >= REG_R12 && alu_y <= REG_R15) )
        {
            USER_ERROR( "Illegal alu y operand, only registers (r12 - r15) allowed." );
            ilnode.error_occurred = ASM_ERROR;
            error = 1;	
        }

    return( (error == FALSE) ? 1 : 0 );
}
