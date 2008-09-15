/* @(#)codegen.c	1.8 3/21/91 1 */

#include "app.h"
#include "error.h"
#include "symbol.h"
#include "a_out.h"
#include "codegen.h"
#include "action.h"
#include "tempio.h"
#include "obj_sym_misc.h"
#include "pass1.h"
#include "pass2.h"

#include "section_fp.h"

static PM_WORD instr_null = {0,0,0,0,0,0};

#ifdef DEBUG
extern char *action_array[];
#endif


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      code_gen                                                        *
*                                                                      *
*   Synopsis                                                           *
*      void code_gen( code, fd )                                       *
*      CODE *code;                                                     *
*      FILE *fd;                                                       *
*                                                                      *
*   Description                                                        *
*     Write the instruction or data to the object code file.           *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/8/89        created                  -----             *
***********************************************************************/

void code_gen( register CODE *code, FILE *fd )
{
    if( code->code_size )
    {
        ++newpc;
        if( code->code_size == PM_WORD_SIZE )
            FWRITE( code->code_value.instr_value, PM_WORD_SIZE, 1, fd );
        else
            FWRITE( code->code_value.data_value, DM_WORD_SIZE, 1, fd );     
    }
}        




/***********************************************************************
*                                                                      *
*   Name                                                               *
*      code_process                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void code_process(fd, start, section)                           *
*      FILE *fd;                                                       *
*      long start;                                                     *
*      short section;                                                  *
*                                                                      *
*   Description                                                        *
*      Process a intermediate code template and then generate any code *
*   if necessary.                                                      *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/8/89        created                  -----             *
***********************************************************************/

void code_process( char *file, long  start, short section )
{
    CODE code;
    FILE *fd;
    long packed_word;
    register short read_result;
    register SYMBOL *symbol;
    SYMBOL	    *intermediate_symbol;

    if( (fd = fopen(file, READ_BINARY)) == NULL )
         FATAL_ERROR("Error opening temporary file");

    pc = symbol_lookup( "." );
    pc->value = newpc = start;
    pc->type = section_data[section].sec_type;

#ifdef DEBUG
    printf("\n\ncode process:\n");
    printf("section = %d\n", section);
#endif
    while( 1 )
    {
           if( FREAD( &packed_word, sizeof(long), 1, fd ) != 1 )
               break;

           code.code_action = (unsigned short) UNPACK_ACTION( packed_word );
           code.code_size   = (unsigned short) UNPACK_NUM_BITS( packed_word );
#ifdef DEBUG
          printf("\n\n");
          printf("section = %s\n", section_header[section].sec_name);
          printf("action = %s\n", action_array[code.code_action]);
          printf("number of bytes = %d\n", code.code_size);
#endif
           if( SYMBOL_PRESENT(packed_word) )
           {
               read_result = FREAD( &intermediate_symbol, sizeof(intermediate_symbol), 1, fd );
               if( read_result != 1)
                   FATAL_ERROR("Error in temporary file format");
           }
           else
               intermediate_symbol = NULL;

           if( IS_ZERO(packed_word) )
           {
#ifdef DEBUG
               printf("Value is zero.");
#endif
               memcpy( (char *) code.code_value.instr_value, (char *) instr_null,
                       PM_WORD_SIZE );
           }
           else if( VALUE_IS_16_BITS(packed_word) )
           {
               code.code_value.value = PACK16( packed_word );
#ifdef DEBUG
               printf("Value fits in 16 bits and = %d\n", code.code_value.value);
#endif               
           }
           else if( VALUE_IS_32_BITS(packed_word) )
           {
               read_result = FREAD( &(code.code_value.value), sizeof(long), 1, fd );

               if( read_result != 1 )
                   FATAL_ERROR("Error in temporary file format");
#ifdef DEBUG
               printf("Value is 32 bits and = %d\n", code.code_value.value);
#endif             
           }
           else
           {
               if( code.code_size == PM_WORD_SIZE )
                   read_result = FREAD( (code.code_value.instr_value), PM_WORD_SIZE, 1, fd );
               else
                   read_result = FREAD( (code.code_value.data_value), DM_WORD_SIZE, 1, fd );     

               if( read_result != 1 )
                   FATAL_ERROR("Error in temporary file format");
#ifdef DEBUG
               if( code.code_size == PM_WORD_SIZE )
                   printf("instruction = %02x%02x%02x%02x%02x%02x\n", 
                          code.code_value.instr_value[0],
                          code.code_value.instr_value[1], code.code_value.instr_value[2],
                          code.code_value.instr_value[3], code.code_value.instr_value[4],
                          code.code_value.instr_value[5] );
               else
                 printf("data = %02x%02x%02x%02x\n", 
                          code.code_value.data_value[0],
                          code.code_value.data_value[1], code.code_value.data_value[2],
                          code.code_value.data_value[3], code.code_value.data_value[4] );
#endif

           }

           if( code.code_action != NO_ACTION )
           {
               if( IS_VALID_ACTION(code.code_action) )
               {
                   if( intermediate_symbol )
                       symbol = intermediate_symbol;
                   else
                       symbol = (SYMBOL *) NULL;

                   (*(array_ptr_functions[code.code_action]))( symbol, &code );
               }
               else
                   FATAL_ERROR("Invalid action routine");
           }

           code_gen( &code, obj_fd );
    }

    if( stack_top > 0 )
        FATAL_ERROR("Symbol table is unbalanced");
    fclose( fd );
}
