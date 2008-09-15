/* @(#)pass2.c	1.10 3/21/91 1 */

#include <stdio.h>
#include <stddef.h>

#include "app.h"
#include "error.h"
#include "symbol.h"
#include "codegen.h"
#include "action.h"
#include "a_out.h"
#include "tempio.h"
#include "main.h"
#include "update.h"
#include "pass2.h"
#include "str.h"
#include "obj_sym_misc.h"
#include "util.h"

#include "internal_reloc.h"
#include "reloc_fp.h"
#include "section_fp.h"
#include "object_fp.h"

/* Indexes into the array of temporary files */

static short glob_sym_temp_index = 0;
static short stat_sym_temp_index = 0;
static short line_temp_index = 0;
static short rel_temp_index = 0;
static short sym_temp_index = 0;

long header_ptr = 0L;
FILE *glob_sym_fd = (FILE *) NULL;
FILE *stat_sym_fd = (FILE *) NULL;
FILE *line_fd = (FILE *) NULL;
FILE *obj_fd = (FILE *) NULL;
FILE *rel_fd = (FILE *) NULL;
FILE *sym_fd = (FILE *) NULL;


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      pass2                                                           *
*                                                                      *
*   Synopsis                                                           *
*      short pass2(void)                                               *
*                                                                      *
*   Description                                                        *
*      This routine is the master code generation routine for the      *
*   second pass.                                                       *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/14/89       created                  -----             *
***********************************************************************/

short pass2( void )
{
    short int                i;
    unsigned short           reloc;
    register SCNHDR         *sec_hdr_ptr;
    register SEC_DATA       *sec_data_ptr;
    register unsigned long  size;
    SYMBOL                  *sym;
    long                     where_we_were;
    char                     section_name[SYMNMLEN + 1];

    pass = 2;
    size = 0;
    section_name[SYMNMLEN] = '\0';

    for( i = 1, sec_hdr_ptr = &section_header[1]; i <= (short) section_cnt; 
         ++i, ++sec_hdr_ptr )
    {
         strncpy( section_name, sec_hdr_ptr->s_name, SYMNMLEN);
         sym = symbol_lookup( section_name );
	 if( NULL == sym )
          FATAL_ERROR("Couldn't find section header in symbol table - pass2.c:pass2()");
	   
         sym->value = 0L;

         if( i > 1 )
             update_symbol_table( 0L, (long) size, (long) i );

         size  += (sec_hdr_ptr->s_size
                   / ((sec_hdr_ptr->s_flags & SECTION_PM) == SECTION_PM
                      ? PM_WORD_SIZE : DM_WORD_SIZE));
    }         

    if( (obj_fd = fopen( obj_name, UPDATE_BINARY )) == NULL )
    {
          FATAL_ERROR("Error opening object file");
    }

    header_ptr = ftell( obj_fd );

    /* Seek past the object file and section headers so we can write the
     * the raw data for each section.
     */

    fseek( obj_fd, (long)(header_ptr + FILHSZ + AOUTSZ + section_cnt *
                          SCNHSZ), 0 );

    glob_sym_fd = temp_file_create( WRITE_BINARY );
    glob_sym_temp_index = num_open_files - 1;

    stat_sym_fd = temp_file_create( WRITE_BINARY );
    stat_sym_temp_index = num_open_files - 1;

    sym_fd      = temp_file_create( WRITE_BINARY );
    sym_temp_index = num_open_files - 1;

    rel_fd      = temp_file_create( WRITE_BINARY );
    rel_temp_index = num_open_files - 1;

    line_fd     = temp_file_create( WRITE_BINARY );
    line_temp_index = num_open_files - 1;

    reloc = 0;
    size = 0;
    for( i = 1, sec_hdr_ptr = &section_header[1], sec_data_ptr =
         &section_data[1]; i <= (short) section_cnt; ++i, ++sec_hdr_ptr, ++sec_data_ptr )
    {
         num_line = 0;
         num_reloc = 0;

         /* Code generation time */

         code_process( temp_file[sec_data_ptr->temp_file_index], (long) size, i );

         sec_hdr_ptr->s_nlnno = (unsigned short) num_line;
         sec_hdr_ptr->s_nreloc = (unsigned short) num_reloc;
         reloc += sec_hdr_ptr->s_nreloc;
         size  += (sec_hdr_ptr->s_size
                   / ((sec_hdr_ptr->s_flags & SECTION_PM) == SECTION_PM
                      ? PM_WORD_SIZE : DM_WORD_SIZE));
    }

    fixup_symbol_table( sym_fd );
    flush_files();

    /* Write the finished table to the symbol table file */

    if( (glob_sym_fd = fopen(temp_file[glob_sym_temp_index], READ_BINARY )) == NULL )
         FATAL_ERROR("Error opening global symbol temp file");

    if( (stat_sym_fd = fopen(temp_file[stat_sym_temp_index], READ_BINARY )) == NULL )
         FATAL_ERROR("Error opening static symbol temp file");

    dump_symbols();
    fclose( glob_sym_fd );
    fclose( stat_sym_fd );

    fflush( sym_fd );
    fclose( sym_fd );

    /* Write the object file header and the section headers */

    where_we_were = ftell( obj_fd );
    fseek( obj_fd, 0L, 0 );
    object_headers();

    fseek( obj_fd, where_we_were, 0 );

    /* Write the relocation info to the object file */

    if( (rel_fd = fopen(temp_file[rel_temp_index], READ_BINARY)) == NULL )
         FATAL_ERROR("Error opening temp relocation file");
    write_all_relocation_info( (long) reloc );
    fclose( rel_fd );

    if( !check_if_errors() )
    {
        /* Append the line number entries, symbol table, and string table
         * to the end of the object file.
         */

        copy_section( temp_file[line_temp_index] );
        copy_section( temp_file[sym_temp_index] );

	write_string_table();

        fflush( obj_fd );
        fclose( obj_fd );

        delete_temp_files();
        return( 0 );
    }
    else
    {
        asm_exit( FATAL );
    }
}
    

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      write_string_table                                              *
*                                                                      *
*   Synopsis                                                           *
*      void write_string_table(void)                                   *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for writing the string table to the *
*   object code file.                                                  *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/14/89       created                  -----             *
***********************************************************************/

void write_string_table( void )
{
    long t = 0;

    if( string_table_index > 4 )
      t = string_table_index;

    fseek( obj_fd, 0L, 2 );
    FWRITE( &t, sizeof(long), 1, obj_fd );
#ifdef MSDOS
    FWRITE( &string_table[4], sizeof(char), (size_t) (string_table_index - 4), obj_fd );
#else
    FWRITE( &string_table[4], sizeof(char), (string_table_index - 4), obj_fd );
#endif
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      flush_files                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void flush_files()                                              *
*                                                                      *
*   Description                                                        *
*      This routine flushes the line, relocation, and global symbol    *
*   to their respective files.                                         *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/14/89       created                  -----             *
***********************************************************************/

void flush_files( void )
{
    fflush( line_fd );
    fclose( line_fd );

    fflush( rel_fd );
    fclose( rel_fd );

    fflush( glob_sym_fd );
    fclose( glob_sym_fd );

    fflush( stat_sym_fd );
    fclose( stat_sym_fd );
}
