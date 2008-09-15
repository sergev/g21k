/* @(#)expression.c	2.3 1/13/94 2 */

#include "app.h"
#include "a_out.h"
#include "symbol.h"
#include "section_fp.h"
#include "codegen.h"
#include "expr.h"
#include "ilnode.h"
#include "pass1.h"
#include "action.h"
#include "error.h"
#include "ieee.h"
#include "main.h"
#include "tempio.h"
#include <string.h>

#include "make_ilnode_fp.h"

static char *error_table[] =
{
    "",
    "Illegal bitwise AND.",
    "Illegal bitwise OR.",
    "Illegal bitwise XOR."
    "Illegal shift right.",
    "Illegal shift left."
    "Illegal modulo division.",
    "",
};


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      expression_add                                                  *
*                                                                      *
*   Synopsis                                                           *
*      void expression_add( operand1, operand2, operand2 )             *
*      EXPR *operand1;                                                 *
*      EXPR *operand2;                                                 *
*      EXPR *result;                                                   *
*                                                                      *
*   Description                                                        *
*      result = operand1 + operand2;                                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/27/89       created                  -----             *
***********************************************************************/

void expression_add( register EXPR *operand1, register EXPR *operand2, 
                     register EXPR *result )
{
    short type_result;

    if( (operand1->sym_ptr == (SYMBOL *) NULL) || (operand2->sym_ptr == (SYMBOL *) NULL) )
    {
         type_result = expression_coerce( operand1, operand2 );
         result->sym_ptr = (operand1->sym_ptr == (SYMBOL *) NULL) ? operand2->sym_ptr :
                                                                    operand1->sym_ptr;

         if( type_result == EXPR_TYPE_FLOAT )
         {
             result->value.float_val = operand1->value.float_val + operand2->value.float_val;
             result->expr_type = EXPR_TYPE_FLOAT;
         }
         else if( type_result == EXPR_TYPE_INT )
         {
             result->value.int_val = operand1->value.int_val + operand2->value.int_val;
             result->expr_type = EXPR_TYPE_INT;
         }
         else
         {
             result->value.float_val = 0.0;
             result->expr_type = EXPR_TYPE_FLOAT;
             result->sym_ptr = (SYMBOL *) NULL;
         }
    }
    else
    {
        USER_ERROR( "Illegal addition. Cannot add two symbols." );
        il->error_occurred = ASM_ERROR;
        result->value.float_val = 0.0;
        result->expr_type = EXPR_TYPE_UNDEFINED;
        result->sym_ptr = (SYMBOL *) NULL;
    } 
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      expression_subtract                                             *
*                                                                      *
*   Synopsis                                                           *
*      void expression_add( operand1, operand2, result )               *
*      EXPR *operand1;                                                 *
*      EXPR *operand2;                                                 *
*      EXPR *result;                                                   *
*                                                                      *
*   Description                                                        *
*      result = operand1 - operand2;                                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/27/89       created                  -----             *
***********************************************************************/

void expression_subtract( register EXPR *operand1, register EXPR *operand2, 
                          register EXPR *result )
{
    short type_result;

    type_result = expression_coerce( operand1, operand2 );        

    if( type_result != -1 )
    {
        result->expr_type = type_result;

        if( type_result == EXPR_TYPE_FLOAT )
            result->value.float_val = operand1->value.float_val - operand2->value.float_val;
        else if( type_result == EXPR_TYPE_INT )
            result->value.int_val = operand1->value.int_val - operand2->value.int_val;

        if( !IS_SYM_PTR(operand2) )
        {
            result->sym_ptr = operand1->sym_ptr;
        }
        else if( operand1->expr_type != EXPR_TYPE_UNDEFINED )
        {
            result->sym_ptr = (SYMBOL *) NULL;

            /* Are we subtracting two symbols? */

            if( IS_SYM_PTR(operand1) )
                result->value.int_val += operand1->sym_ptr->value - operand2->sym_ptr->value;
        } 
        else
        {
            USER_ERROR( "Illegal subtraction." );
            il->error_occurred = ASM_ERROR;
            result->value.float_val = 0.0;
            result->expr_type = EXPR_TYPE_UNDEFINED;
            result->sym_ptr = (SYMBOL *) NULL;
        }            
    }
    else
    {
        result->value.float_val = 0.0;
        result->expr_type = EXPR_TYPE_UNDEFINED;
        result->sym_ptr = (SYMBOL *) NULL;
    } 
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      expression_multiply                                             *
*                                                                      *
*   Synopsis                                                           *
*      void expression_multiply( operand1, operand2, result )          *
*      EXPR *operand1;                                                 *
*      EXPR *operand2;                                                 *
*      EXPR *result;                                                   *
*                                                                      *
*   Description                                                        *
*      result = operand1 * operand2;                                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/27/89       created                  -----             *
***********************************************************************/

void expression_multiply( register EXPR *operand1, register EXPR *operand2,
                          register EXPR *result )
{
    short type_result;

    if( IS_SYM_PTR(operand1) || IS_SYM_PTR(operand2) )
    {
        USER_ERROR( "Illegal multiplication. Cannot multiply a symbol" );
        il->error_occurred = ASM_ERROR;
        result->value.float_val = 0.0;
        result->expr_type = EXPR_TYPE_UNDEFINED;
        result->sym_ptr = (SYMBOL *) NULL;
    }
    else if( (type_result = expression_coerce( operand1, operand2 )) != -1)
    {
        result->expr_type = type_result;

        if( type_result == EXPR_TYPE_FLOAT )
            result->value.float_val = operand1->value.float_val * operand2->value.float_val;
        else if( type_result == EXPR_TYPE_INT )
            result->value.int_val = operand1->value.int_val * operand2->value.int_val;
        
        result->sym_ptr = (SYMBOL *) NULL;
    }
    else
    {
        result->value.float_val = 0.0;
        result->expr_type = EXPR_TYPE_UNDEFINED;
        result->sym_ptr = (SYMBOL *) NULL;
    }
}                 


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      expression_divide                                               *
*                                                                      *
*   Synopsis                                                           *
*      void expression_divide( operand1, operand2, result )            *
*      EXPR *operand1;                                                 *
*      EXPR *operand2;                                                 *
*      EXPR *result;                                                   *
*                                                                      *
*   Description                                                        *
*      result = operand1 / operand2;                                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/27/89       created                  -----             *
***********************************************************************/

void expression_divide( register EXPR *operand1, register EXPR *operand2,
                          register EXPR *result )
{
    short type_result;

    if( IS_SYM_PTR(operand1) || IS_SYM_PTR(operand2) )
    {
        USER_ERROR( "Illegal division. Cannot divide with a symbol." );
        il->error_occurred = ASM_ERROR;
        result->value.float_val = 0.0;
        result->expr_type = EXPR_TYPE_UNDEFINED;
        result->sym_ptr = (SYMBOL *) NULL;
    }
    else if( (type_result = expression_coerce( operand1, operand2 )) != -1)
    {
        result->expr_type = type_result;

        if( type_result == EXPR_TYPE_FLOAT )
            result->value.float_val = operand1->value.float_val / operand2->value.float_val;
        else if( type_result == EXPR_TYPE_INT )
            result->value.int_val = operand1->value.int_val / operand2->value.int_val;
        
        result->sym_ptr = (SYMBOL *) NULL;
    }
    else
    {
        result->value.float_val = 0.0;
        result->expr_type = EXPR_TYPE_UNDEFINED;
        result->sym_ptr = (SYMBOL *) NULL;
    }
}                 


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      expression_general                                              *
*                                                                      *
*   Synopsis                                                           *
*      void expression_add( operand1, operand2, result, type )         *
*      EXPR *operand1;                                                 *
*      EXPR *operand2;                                                 *
*      EXPR *result;                                                   *
*                                                                      *
*   Description                                                        *
*      This routine performs the operation indicated by the variable   *
*   'type'.                                                            *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/27/89       created                  -----             *
***********************************************************************/

void expression_general( register EXPR *operand1, register EXPR *operand2,
                          register EXPR *result, long type )
{
    char *error_string;

    /* Make sure that neither operand is a symbol and also that both
     * operands are of type int.
     */

    if( IS_SYM_PTR(operand1) || IS_SYM_PTR(operand2) || 
        (!IS_INT(operand1) || !IS_INT(operand2)) )
    {
        error_string = error_table[type];
        USER_ERROR1( "%s", error_string );
        il->error_occurred = ASM_ERROR;
        result->value.float_val = 0.0;
        result->expr_type = EXPR_TYPE_UNDEFINED;
        result->sym_ptr = (SYMBOL *) NULL;
    }
    else
    {
        result->expr_type = EXPR_TYPE_INT;
        result->sym_ptr = (SYMBOL *) NULL;
        switch( (int) type )
        {
            case EXPR_OP_AND:
               result->value.int_val = operand1->value.int_val & operand2->value.int_val;
               break;

            case EXPR_OP_OR:
               result->value.int_val = operand1->value.int_val | operand2->value.int_val;
               break;

            case EXPR_OP_XOR:
               result->value.int_val = operand1->value.int_val ^ operand2->value.int_val;
               break;

            case EXPR_OP_SHIFT_RIGHT:
               result->value.int_val = operand1->value.int_val >> operand2->value.int_val;
               break;

            case EXPR_OP_SHIFT_LEFT:
               result->value.int_val = operand1->value.int_val << operand2->value.int_val;
               break;

            case EXPR_OP_MOD:
               result->value.int_val = operand1->value.int_val % operand2->value.int_val;
               break;
        }

    }
}                 


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      expression_coerce                                               *
*                                                                      *
*   Synopsis                                                           *
*      short expression_coerce( operand1, operand2 )                   *
*      EXPR *operand1;                                                 *
*      EXPR *operand2;                                                 *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for making sure that the two        *
*   operands are of the same type; it also does a little error         *
*   checking.                                                          *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/27/89       created                  -----             *
***********************************************************************/

short expression_coerce( register EXPR *operand1, register EXPR *operand2 )
{
    if( operand1->expr_type == operand2->expr_type )
        return( operand1->expr_type );

    if( IS_SYM_PTR(operand1) )
    {
        if( !IS_FLOAT(operand2) )
            ;
        else
        {
            USER_ERROR( "Cannot perform arithmetic on a symbol and a floating point constant" );
            il->error_occurred = ASM_ERROR;
            return( -1 );
        }
    }
    else if( IS_SYM_PTR(operand2) )
    {
        if( !IS_FLOAT(operand2) )
            ;
        else
        {
            USER_ERROR( "Cannot perform arithmetic on a symbol and a floating point constant" );
            il->error_occurred = ASM_ERROR;
            return( -1 );
        }
    }

    if( IS_FLOAT(operand1) && IS_INT(operand2) )
    {
        operand2->value.float_val = (double) operand2->value.int_val;
        return( operand2->expr_type = EXPR_TYPE_FLOAT );
    }
    else if( IS_FLOAT(operand2) && IS_INT(operand1) )
    {
        operand1->value.float_val = (double) operand1->value.int_val;
        return( operand1->expr_type = EXPR_TYPE_FLOAT );
    }
    else
    {
       ASSEMBLER_ERROR( "Expression coerce: Illegal types in expression" );
       il->error_occurred = ASM_ERROR;
       return( -1 );
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      process_initializers                                            *
*                                                                      *
*   Synopsis                                                           *
*      void process_initializers( EXPR *expr )                         *
*      EXPR *expr;                                                     *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for writing out initialized data.   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/11/89       created                  -----             *
***********************************************************************/

void expression_process_initializers( register EXPR *expr )
{
    register short action;
    unsigned long  mant;
    int            exp;
    unsigned char  data[PM_WORD_SIZE];
    unsigned short word_size;

    word_size = (unsigned short)(get_current_section_type() & SECTION_PM ? PM_WORD_SIZE : DM_WORD_SIZE);
    memset( data, 0, PM_WORD_SIZE );

    if( expr->expr_type == EXPR_TYPE_INT )
    {
        data[0] = (char) DATA32_TOP8( expr->value.int_val );
        data[1] = (char) DATA32_TOP_MIDDLE8( expr->value.int_val );
        data[2] = (char) DATA32_MIDDLE8( expr->value.int_val );
        data[3] = (char) DATA32_BOTTOM8( expr->value.int_val );

        if( expr->sym_ptr == (SYMBOL *) NULL )
            action = NO_ACTION;
        else
            action = RELOC_ADDR_VAR;

        WRITE_INTERMEDIATE( data, word_size, action, expr->sym_ptr );
    }
    else if( expr->expr_type == EXPR_TYPE_FLOAT )
    {
        convert_to_ieee( expr->value.float_val, round_mode, precision, &mant, &exp );
        format_ieee( mant, exp, data, precision );

        if( expr->sym_ptr == (SYMBOL *) NULL )
            WRITE_INTERMEDIATE( data, word_size, NO_ACTION, NULL );
        else
            USER_ERROR( "Illegal initialization." );
    }
    else if( expr->expr_type == EXPR_TYPE_HEX40 )
    {
        data[0] = expr->value.hex40_val[0];
        data[1] = expr->value.hex40_val[1];
        data[2] = expr->value.hex40_val[2];
        data[3] = expr->value.hex40_val[3];
        data[4] = expr->value.hex40_val[4];
	WRITE_INTERMEDIATE( data, word_size, NO_ACTION, NULL );
    }
    else
        USER_ERROR( "Illegal initialization." );
}    

void expression_hex_string_to_uns( char *string, void *buffer, int width )
{
    unsigned short length;
    short index;
    char *char_ptr;

   /* Avoid casts */
   char_ptr = buffer;

   for( index = 0; index < width; index++ )
        char_ptr[index] = 0;

   length = strlen (string);

   /* Point to least significant digit in the string */
   string += length - 1;

   for( index = width - 1; index >= 0; index-- )
   {
        if( length ) 
        {
            if( *string > '9' )
                char_ptr[index] = ((islower(*string) ? toupper(*string) : (*string)) - 'A') + 10;
            else
                char_ptr[index] = (*string - '0');
        }
        else 
            break;

        /* . Point to the next higher character in the string */
        string--;
        length--;

        if( length ) 
        {
            if( *string > '9' )
                char_ptr[index] += (((islower(*string) ? toupper(*string) : 
                                   (*string)) - 'A') + 10) << 4;
            else
                char_ptr[index] += (*string - '0') << 4;
        } 
        else
            break;

        /* . Point to the next higher character in the string */
        string--;
        length--;
    }
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      expression_init_to_zero                                         *
*                                                                      *
*   Synopsis                                                           *
*      void expression_init_to_zero( int length  )                     *
*                                                                      *
*   Description                                                        *
*      This routine initializes a variable to zero.                    *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/14/89       created                  -----             *
***********************************************************************/

void expression_init_to_zero( register long length, short word_size  )
{
    register long i;
    unsigned char data[PM_WORD_SIZE];
    
    memset( data, 0, PM_WORD_SIZE );
    for( i = 0; i < length; ++i )
         WRITE_INTERMEDIATE( data, word_size, NO_ACTION, NULL );
}
