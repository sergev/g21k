/* @(#)create_object.c  1.8 2/21/91 1 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "app.h"
#include "a_out.h"
#include "action.h"
#include "input_file.h"
#include "input_section.h"
#include "ldfcn.h"
#include "linker_aux.h"
#include "linker_sym.h"
#include "ar.h"
#include "list.h"
#include "error.h"
#include "read_object.h"
#include "syms.h"
#include "coff_io.h"
#include "util.h"
#include "tempio.h"

#include "error_fp.h"
#include "read_object_fp.h"
#include "syms_fp.h"
#include "list_fp.h"
#include "create_object.h"

char    *C_filename = NULL;
char    Arch_descrip_name[50];
struct  heap_list heap_base =
{
 /* was NULL, 0, 0L, 0L, NULL *EK*/
	0, 0, 0L, 0L, 0
};

extern  num_input_files;
extern  char *file_names[MAX_NUMBER_INPUT_FILES];
extern  char Temporary_filename[];

int     Init_word_size=PM_WORD_SIZE;
char    Stack_segment_name[] = "seg_stak";      /*Default stack segment*/
char    Init_segment_name[] = "seg_init";       /*Default initialization segment*/

unsigned long int segment_start(char *name);
unsigned long int segment_length(char *name);

#define C_STRING_TABLE_LENGTH sizeof(c_string_table) 
#define C_SYMBOL_COUNT 10

char c_string_table[] = {
"0000\
___lib_stack_space\0\
___lib_stack_length\0\
___lib_dmbank1\0___lib_dmbank2\0___lib_dmbank3\0___lib_dmwait\0\
___lib_pmbank1\0___lib_pmwait\0\
___lib_heap_space"
};

unsigned long C_init_list[] = 
{
	0L,                     /* ___lib_stack_space   */
	0L,                     /* ___lib_stack_length  */
	0x20000000L,            /* ___lib_dmbank1       */
	0x40000000L,            /* ___lib_dmbank2       */
	0x80000000L,            /* ___lib_dmbank3       */
	0x000f7bdeL,            /* ___lib_dmwait        */
	0x00800000L,            /* ___lib_pmbank1       */
	0x000003deL,            /* ___lib_pmwait        */
	0x00000000L,            /* ___inits             */
	0x00000000L             /* ___lib_heap_space    */
};

struct 
	{
		long x;
		long y;
		long n_value;
		short w;
		unsigned short b;
		char a;
		char c;
	}
c_symbol_list[] ={
/*      string or pointer       value           scnum   type    class   numaux          */      

	0L,     0x00000004L,    0x00000000L,    1,      0x0000, 2,      0,      /* stack start */
	0L,     0x00000017L,    0x00000001L,    1,      0x0000, 2,      0,      /* stack length */
	0L,     0x0000002BL,    0x00000002L,    1,      0x0000, 2,      0,      /* dmbank1 */
	0L,     0x0000003AL,    0x00000003L,    1,      0x0000, 2,      0,      /* dmbank2 */
	0L,     0x00000049L,    0x00000004L,    1,      0x0000, 2,      0,      /* dmbank3 */
	0L,     0x00000058L,    0x00000005L,    1,      0x0000, 2,      0,      /* dmwait */
	0L,     0x00000066L,    0x00000006L,    1,      0x0000, 2,      0,      /* pmbank1 */
	0L,     0x00000075L,    0x00000007L,    1,      0x0000, 2,      0,      /* pmwait */

	/* defines an entry for "___inits" */
#ifdef WORDS_BIGENDIAN  /*EK*/
	0x5f5f5f69L,0x6e697473L,0x00000008L,    1,      0x0000, 2,      0,      /* Inits */
#else
	0x695f5f5fL,0x7374696eL,0x00000008L,    1,      0x0000, 2,      0,      /* Inits */
#endif

	0L,     0x00000083L,    0x00000009L,    1,      0x0000, 2,      0,      /* Heap */
};

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      create_c_object                                                 *
*                                                                      *
*   Synopsis                                                           *
*      void create_c_object( char *symbol_name, long length,           *
*                            char *section_name)                       *
*                                                                      *
*   Description                                                        *
*       This routine creates a symbol that will be used by the C       *
*   runtime envirnment                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            * 
*     gas     5/6/91        created                  -----             * 
***********************************************************************/
        
void create_c_object( void )
{
    FILHDR                file_header;
    FILHDR               *c_file_header = &file_header;
    AOUTHDR               opt_header;
    AOUTHDR              *c_opt_header = &opt_header;
    SCNHDR                section_header;
    SCNHDR               *c_section_header = &section_header;
    SYMENT                obj_sym;
    SYMENT               *c_obj_sym_ptr = &obj_sym;
    AUXENT                obj_aux;
    AUXENT               *c_obj_aux_ptr = &obj_aux;
    FILE                 *c_object_fd;
    struct heap_list     *heap;
    unsigned char         long_buff[PM_WORD_SIZE];
    register int          i, j;
    register long         first_address;
    long                  heap_length;
    long                  section_length;
    long                  stack_only_length;
    long                  long_name;
    long tmp_long;

/* First test for proper bank settings */
    if( C_init_list[DMBANK1_OFFSET] >= C_init_list[DMBANK2_OFFSET] )
	FATAL_ERROR2( "DMBANK1 (=%ld) must be less than DMBANK2 (=%ld).\n", C_init_list[DMBANK1_OFFSET], C_init_list[DMBANK2_OFFSET]);

    if( C_init_list[DMBANK2_OFFSET] >= C_init_list[DMBANK3_OFFSET] )
	FATAL_ERROR2( "DMBANK2 (=%ld) must be less than DMBANK3 (=%ld).\n", C_init_list[DMBANK2_OFFSET], C_init_list[DMBANK3_OFFSET]);

/* Create temporary file for C support */
    if( (c_object_fd = temp_file_create(WRITE_BINARY)) == NULL)
	FATAL_ERROR( "Error opening temporary file.");

/* Add temporary file to file name list */
    C_filename = (char *)my_calloc( (long)strlen(Temporary_filename) + 5);

    strcpy(C_filename, Temporary_filename);
    file_names[num_input_files++] = C_filename;

/* Set start and length of stack */
    C_init_list[STACK_OFFSET] = segment_start(Stack_segment_name);
    C_init_list[STACK_LENGTH_OFFSET] = segment_length(Stack_segment_name);

/* Determine the number of defined heap spaces */
   i=0;
   for(heap=heap_base.next; heap != NULL; heap=heap->next)
	i++;
   section_length = C_SYMBOL_COUNT + i * 5;
	        
/* Generate and write file and optional headers */
    c_file_header->f_magic = M_21000;
    c_file_header->f_nscns = 1;
    c_file_header->f_timdat = 0L;
    c_file_header->f_symptr = (FILHSZ + AOUTSZ + SCNHSZ + Init_word_size * section_length);
    c_file_header->f_nsyms = (long)(2 + C_SYMBOL_COUNT);
    c_file_header->f_opthdr = AOUTSZ;
    c_file_header->f_flags = 0;

    if( write_file_header(c_file_header, c_object_fd) == 0 )
	FATAL_ERROR( "Error writing to temporary file." );

    if( write_optional_header(c_opt_header, c_object_fd) == 0 )
	FATAL_ERROR( "Error writing to temporary file." );

/* Generate and write out section header */
    for( i=0; i<SYMNMLEN; i++ )
	c_section_header->s_name[i] = Init_segment_name[i];

    c_section_header->s_paddr = 0L;
    c_section_header->s_vaddr = 0L;
    c_section_header->s_size = Init_word_size * section_length;
    c_section_header->s_scnptr = FILHSZ + AOUTSZ + SCNHSZ;
    c_section_header->s_relptr = 0L;
    c_section_header->s_lnnoptr = 0L;
    c_section_header->s_nreloc = 0;
    c_section_header->s_nlnno = 0;
    c_section_header->s_flags = (Init_word_size==DM_WORD_SIZE)? SECTION_DM:SECTION_PM;

    if( write_section_header(c_section_header, c_object_fd) == 0 )
	FATAL_ERROR( "Error writing to temporary file." );

/* No Relocation entries */

/* No line number entries */

/* Generate data section */
    for( i=0; i<C_SYMBOL_COUNT - 1; i++)
    {
	port_put_long( C_init_list[i], long_buff );
	fwrite( long_buff, Init_word_size, 1, c_object_fd );
    }
	
/* Generate heap1 data */
   for(heap=heap_base.next; heap != NULL; heap=heap->next)
   {
	long_name = 0;
	for(i=0; i<4; i++)
	{
		long_name = (long_name << 8) | heap->name[i];
	}
	port_put_long( long_name, long_buff);
	fwrite( long_buff, sizeof(long), 1, c_object_fd );

	long_name = 0;
	for(i=4; i<8; i++)
	{
		long_name = (long_name << 8) | heap->name[i];
	}
	port_put_long( long_name, long_buff);
	fwrite( long_buff, sizeof(long), 1, c_object_fd );

	port_put_long( (long)heap->location, long_buff);
	fwrite( long_buff, sizeof(long), 1, c_object_fd );
	port_put_long( 0L, long_buff );
	fwrite(long_buff, Init_word_size, 1, c_object_fd );
	heap->address = segment_start(heap->name);
	port_put_long( heap->address, long_buff );
	fwrite( long_buff, Init_word_size, 1, c_object_fd );
	heap->length = segment_length(heap->name);
	port_put_long( heap->length, long_buff );
	fwrite(long_buff, Init_word_size, 1, c_object_fd );
   }
   port_put_long( 0L, long_buff );
   fwrite(long_buff, Init_word_size, 1, c_object_fd );

/* Generate and write out first symbol (section and it's aux) */
    for( i=0; i<SYMNMLEN; i++ )
	c_obj_sym_ptr->_n._n_name[i] = Init_segment_name[i];

    c_obj_sym_ptr->n_value = 0L;
    c_obj_sym_ptr->n_scnum = 1;
    c_obj_sym_ptr->n_type = 0;
    c_obj_sym_ptr->n_sclass = 3;
    c_obj_sym_ptr->n_numaux = 1;

    c_obj_aux_ptr->x_scn.x_scnlen = Init_word_size * section_length;
    c_obj_aux_ptr->x_scn.x_nreloc = 0;
    c_obj_aux_ptr->x_scn.x_nlinno = 0;

    if( write_symbol(c_obj_sym_ptr, c_object_fd) == 0 )
	FATAL_ERROR( "Error writing to temporary file." );

    if( write_aux_info(c_obj_aux_ptr, c_object_fd, 0) == 0 )
	FATAL_ERROR( "Error writing to temporary file." );

/* Generate the rest of the symbol table */
    for( i=0; i<C_SYMBOL_COUNT; i++)
    {
	memcpy(c_obj_sym_ptr, &c_symbol_list[i], SYMESZ);       /* Take symbol from list */
	if( write_symbol(c_obj_sym_ptr, c_object_fd) == 0 )
		FATAL_ERROR( "Error writing to temporary file." );
    }
    tmp_long = (long)C_STRING_TABLE_LENGTH;
    fwrite( &tmp_long, sizeof(long), 1, c_object_fd );

/* Write out the string table */
#ifdef MSDOS
    fwrite(&c_string_table[4],sizeof(char),(size_t)(C_STRING_TABLE_LENGTH-4),c_object_fd );
#else
    fwrite(&c_string_table[4],sizeof(char), (C_STRING_TABLE_LENGTH - 4), c_object_fd );
#endif
    fflush(c_object_fd);
    fclose(c_object_fd);
stack_only_length;
j;
first_address;
heap_length;
}

