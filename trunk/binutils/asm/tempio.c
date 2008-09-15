/* @(#)tempio.c	1.7 3/21/91 1 */

#include "app.h"
#include "error.h"
#include "symbol.h"
#include "codegen.h"
#include "action.h"
#include "pass1.h"
#include "a_out.h"
#include "str.h"
#include "tempio.h"
#include <string.h>

#include "section_fp.h"

char *temp_file[MAX_TEMPS + 1];
short num_open_files = 0;

#ifdef DEBUG
char *action_array[] =
{
    "NO ACTION",
    "DEFINE FILE SYMBOL",
    "DEFINE SYMBOL",
    "DEFINE SYMBOL VALUE",
    "DEFINE SYMBOL SCLASS",
    "DEFINE SYMBOL TAG",
    "DEFINE SYMBOL LINE",
    "DEFINE SYMBOL SIZE",
    "DEFINE SECTION AUXENT",
    "DEFINE DIM1",
    "DEFINE DIM2",
    "END SYMBOL DEFINE",
    "LINE NUMBER",
    "RELOCATE ADDRESS OF A VARIABLE",
    "RELOCATE ADDRESS 24",
    "RELOCATE ADDRESS 32",
    "RELOCATE PC RELATIVE SHORT",
    "RELOCATE PC RELATIVE LONG",
    "NEW STATEMENT",
    "",
};
#endif



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      temp_file_create                                                *
*                                                                      *
*   Synopsis                                                           *
*      FILE temp_file_create(type)                                     *
*      char *type;                                                     *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating and opening a temp.    *
*   file.                                                              *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/1/89        created                  -----             *
***********************************************************************/

FILE *temp_file_create( char *type )
{
    FILE *desc;

    temp_file[num_open_files] = tempnam("",TEMP_PREFIX);
    desc = fopen( temp_file[num_open_files], type );

    if( desc != NULL )
    {
        ++num_open_files;
        return( desc );
    }
    else
        FATAL_ERROR("Could not open temporary file");
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      temp_file_write                                                 *
*                                                                      *
*   Synopsis                                                           *
*      void temp_file_write(ptr, num_bytes, action, symbol)            *
*      short *ptr;                                                     *
*      short  num_bytes;                                               *
*      unsigned short action;                                          *
*      SYMBOL *symbol;                                                 *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for writing the temporary section   *
*   information to the file indicated by the current pc. This routine  *
*   is made slightly more complex because we have to write 40-bit data *
*   and 48-bit instructions and also write 32-bit integers.            *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     2/27/89       created                  -----             *
***********************************************************************/

void temp_file_write( char *ptr, short num_bytes, unsigned short action,
		      SYMBOL *symbol )
{
    long packed_word;
    register SEC_DATA *sect_data_ptr;
    long value;

    if( num_bytes == 0 )
        value = *((long *) ptr);

    sect_data_ptr = &section_data[pc->section_num];

    if( (section_cnt <= 0 || in_section == FALSE) && (num_bytes) )
        USER_ERROR( "Code cannot be generated without being in a section" );

    if( action != NEW_STATEMENT )
        sect_data_ptr->code_gen += num_bytes;
    else
    {
        if( sect_data_ptr->code_gen )
            sect_data_ptr->code_gen = 0;
        else
            return;
    }
  
    if( ((sect_data_ptr->buf_cnt + num_bytes) >= BUFSIZ) || 
         (sect_data_ptr->buf_cnt + sizeof(long) >= BUFSIZ) )
    {
#ifdef MSDOS
         FWRITE( sect_data_ptr->buf, sizeof(char), (size_t) sect_data_ptr->buf_cnt,
                 sect_data_ptr->file_desc );
#else
         FWRITE( sect_data_ptr->buf, sizeof(char), sect_data_ptr->buf_cnt,
                 sect_data_ptr->file_desc );
#endif
         sect_data_ptr->buf_cnt = 0;
    }

    packed_word = PACK_ACTION( action ) | PACK_NUM_BITS( num_bytes );

    if( symbol )
    {
        packed_word |= SYMBOL_BIT;

        if( (num_bytes == 0) && (value == 0) )
        {
             packed_word |= VALUE0;
             memcpy( &sect_data_ptr->buf[sect_data_ptr->buf_cnt], (char *) &packed_word, 
                     sizeof(long) );
             sect_data_ptr->buf_cnt += sizeof(long);

             if( (sect_data_ptr->buf_cnt + sizeof(symbol)) >= BUFSIZ )
             {
#ifdef MSDOS
                  FWRITE( sect_data_ptr->buf, sizeof(char), (size_t) sect_data_ptr->buf_cnt,
                          sect_data_ptr->file_desc );
#else
                  FWRITE( sect_data_ptr->buf, sizeof(char), sect_data_ptr->buf_cnt,
                          sect_data_ptr->file_desc );
#endif
                  sect_data_ptr->buf_cnt = 0;
             }

             memcpy( &sect_data_ptr->buf[sect_data_ptr->buf_cnt], &symbol, sizeof(symbol) );
             sect_data_ptr->buf_cnt += sizeof(symbol);
        }
        else if( (num_bytes == 0) && (value != 0) )
        {
             packed_word |= VALUE32;
             memcpy( &sect_data_ptr->buf[sect_data_ptr->buf_cnt] , (char *) &packed_word,
                     sizeof(long) );
             sect_data_ptr->buf_cnt += sizeof(long);

             if( (sect_data_ptr->buf_cnt + sizeof(symbol) + sizeof(long)) >= BUFSIZ )
             {
#ifdef MSDOS
                  FWRITE( sect_data_ptr->buf, sizeof(char),( size_t) sect_data_ptr->buf_cnt,
                          sect_data_ptr->file_desc );
#else
                  FWRITE( sect_data_ptr->buf, sizeof(char), sect_data_ptr->buf_cnt,
                          sect_data_ptr->file_desc );
#endif
                  sect_data_ptr->buf_cnt = 0;
             }

             memcpy( &sect_data_ptr->buf[sect_data_ptr->buf_cnt], &symbol, sizeof(symbol) );
             sect_data_ptr->buf_cnt += sizeof(symbol);

             memcpy( &sect_data_ptr->buf[sect_data_ptr->buf_cnt] , (char *) &value,
                     sizeof(long) );
             sect_data_ptr->buf_cnt += sizeof(long);
        }
        else
        {
            memcpy( &sect_data_ptr->buf[sect_data_ptr->buf_cnt] , (char *) &packed_word,
                    sizeof(long) );
            sect_data_ptr->buf_cnt += sizeof(long);
            if( (sect_data_ptr->buf_cnt + num_bytes + sizeof(symbol)) >= BUFSIZ )
            {
#ifdef MSDOS
                 FWRITE( sect_data_ptr->buf, sizeof(char), (size_t) sect_data_ptr->buf_cnt,
                         sect_data_ptr->file_desc );
#else
                 FWRITE( sect_data_ptr->buf, sizeof(char), sect_data_ptr->buf_cnt,
                         sect_data_ptr->file_desc );
#endif
                 sect_data_ptr->buf_cnt = 0;
            }

            memcpy( &sect_data_ptr->buf[sect_data_ptr->buf_cnt], &symbol, sizeof(symbol) );
            sect_data_ptr->buf_cnt += sizeof(symbol);

            memcpy( &sect_data_ptr->buf[sect_data_ptr->buf_cnt], ptr, num_bytes );
            sect_data_ptr->buf_cnt += num_bytes;
        }
   
    }
    else if( (num_bytes == 0) && (WILL_FIT_IN_16_BITS(value)) )
    {
        packed_word |= VALUE16 | PACK16(value);
        memcpy( &sect_data_ptr->buf[sect_data_ptr->buf_cnt] , (char *) &packed_word,
                sizeof(long) );
        sect_data_ptr->buf_cnt += sizeof(long);
    }
    else if( (num_bytes == 0) && (value != 0) )
    {
        packed_word |= VALUE32;
        memcpy( &sect_data_ptr->buf[sect_data_ptr->buf_cnt] , (char *) &packed_word, 
                sizeof(long) );
        sect_data_ptr->buf_cnt += sizeof(long);

        if( (sect_data_ptr->buf_cnt + sizeof(long)) >= BUFSIZ )
        {
#ifdef MSDOS
             FWRITE( sect_data_ptr->buf, sizeof(char), (size_t) sect_data_ptr->buf_cnt,
                     sect_data_ptr->file_desc );
#else
             FWRITE( sect_data_ptr->buf, sizeof(char), sect_data_ptr->buf_cnt,
                     sect_data_ptr->file_desc );
#endif
             sect_data_ptr->buf_cnt = 0;
        }

         memcpy( &sect_data_ptr->buf[sect_data_ptr->buf_cnt] , (char *) &value, sizeof(long) );
         sect_data_ptr->buf_cnt += sizeof(long);
    }
    else
    {
        memcpy( &sect_data_ptr->buf[sect_data_ptr->buf_cnt] , (char *) &packed_word,
                sizeof(long) );
        sect_data_ptr->buf_cnt += sizeof(long);
        if( (sect_data_ptr->buf_cnt + num_bytes) >= BUFSIZ )
        {
#ifdef MSDOS
             FWRITE( sect_data_ptr->buf, sizeof(char), (size_t) sect_data_ptr->buf_cnt,
                     sect_data_ptr->file_desc );
#else
             FWRITE( sect_data_ptr->buf, sizeof(char), sect_data_ptr->buf_cnt,
                     sect_data_ptr->file_desc );
#endif
             sect_data_ptr->buf_cnt = 0;
        }

        memcpy( &sect_data_ptr->buf[sect_data_ptr->buf_cnt], ptr, num_bytes );
        sect_data_ptr->buf_cnt += num_bytes;
    }


    if( num_bytes )
        ++newpc;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      flush_temp_files                                                *
*                                                                      *
*   Synopsis                                                           *
*      void flush_temp_files()                                         *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for flushing the individual section *
*   info to the section temp file so the next pass will have all the   *
*   info it needs. If we don't do this there will be stray info in the *
*   buffers.                                                           *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     2/27/89       created                  -----             *
***********************************************************************/

void flush_temp_files( void )
{
    register int i;
    register SEC_DATA *sec_data_ptr;

    for( i = 1, sec_data_ptr = &section_data[1]; i <= (int) section_cnt; ++i, ++sec_data_ptr )
    {
         if( !sec_data_ptr->is_open )
             sec_data_ptr->file_desc = fopen( temp_file[sec_data_ptr->temp_file_index], 
                                              APPEND_BINARY );
#ifdef MSDOS
         FWRITE( sec_data_ptr->buf, sizeof(char), (size_t) sec_data_ptr->buf_cnt,
                 sec_data_ptr->file_desc );
#else
         FWRITE( sec_data_ptr->buf, sizeof(char), sec_data_ptr->buf_cnt,
                 sec_data_ptr->file_desc );
#endif
         sec_data_ptr->buf_cnt = 0;

         fflush( sec_data_ptr->file_desc );
         fclose( sec_data_ptr->file_desc );
    }
}

