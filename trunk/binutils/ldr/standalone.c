#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "app.h"
#include "util.h"
#include "achparse.h"
#include "a_out.h"
#include "coff_io.h"
#include "ldr.h"
#include "error.h"

#define PROCESSOR_ADSP_21020 1


struct init_info init_base_dm, init_base_pm;
struct init_info *init_ptr, *hld_dm_init, *hld_pm_init;

struct my_section_list my_section_base;

struct init_info init_bss_dm_base;
struct init_info init_bss_pm_base;

long int delta_symptr = 0;
long int end_of_data = 0;
long int delta_section_ptr = 0;
long int init_section_pointer;

int adjust, aux, reserved_section;

extern int      processor;
extern int      Ram_segment_count;
extern char     Ram_segment_list[125][9];
extern int      Width_segment_count;
extern char     Width_segment_list[125][9];
extern long     Width_segment_width[125];


char *rth_section_name = "seg_rth";
char *init_section_name = "seg_init";
char *code_section_name = "seg_pmco";

extern char     arch_descrip_file[];

#define SECT_BUF_SIZE 1024
#define MIN(a,b) (a<b?a:b)

int     sect_buffer[SECT_BUF_SIZE];

int     ram_section( char *section_name )
{
	int i;

	for(i=0; i<Ram_segment_count; i++)
		if (strncmp(section_name, Ram_segment_list[i], SYMNMLEN) == 0)
			return TRUE;
	return FALSE;
}
int     width_section( char *section_name )
{
	int i;

	for(i=0; i<Width_segment_count; i++)
		if (strncmp(section_name, Width_segment_list[i], SYMNMLEN) == 0)
			return Width_segment_width[i];
	return -1;
}

void copy_block(long int remaining)
{
	long int num_bytes;

/*      Copy blocks from input to output at the current file pointer */ 
	while( remaining )
	{
		num_bytes = MIN((long)SECT_BUF_SIZE, remaining);
		if( fread(sect_buffer, (int) num_bytes, 1, input_fd) != 1 )
			FATAL_ERROR( "Unable to copy section data from input file" );
		if( fwrite(sect_buffer, (int) num_bytes, 1, output_fd) != 1 )
			FATAL_ERROR( "Unable to write section data\n" );
		remaining -= num_bytes;
    }
}

    
void make_standalone(void)
{

	FILHDR  file_header, *file_hdr = &file_header;
	AOUTHDR opt_header, *opt_hdr = &opt_header;
	SCNHDR  section_header, *section_hdr = &section_header;
	SYMENT  symbol_struct, *symbol = &symbol_struct;
	struct  my_section_list *my_section, *free_old;
	int     i, section_word_size, line_number_total;        
	int     section_count;
	unsigned char   long_buff[PM_WORD_SIZE];
	long int l;
	long int symbol_address;
	long int init_block_address;
	long int inits_ptr;
	long int init_section_address;
	long int init_section_size;
	long int string_table_length;
	long int next_section_ptr;

	my_section = &my_section_base;

	hld_dm_init = &init_base_dm;
	hld_pm_init = &init_base_pm;

	if( ach_parse(arch_descrip_file) )
		FATAL_ERROR1("Unable to parse %s\n",arch_descrip_file);

/*      Seek to the beginning of linker output file */
	fseek(output_fd, 0L, 0);                
	fseek(input_fd, 0L, 0);

/*      Read the file and optional header */
	if( !read_file_header(file_hdr, input_fd) )
		FATAL_ERROR( "Unable to read file header" );
	if( file_hdr->f_opthdr != 0 )
		if( !read_optional_header(opt_hdr, input_fd) )
			FATAL_ERROR( "Unable to read optional header" );

/*      Scan each section to determine if it is RAM and needs initialization */
	line_number_total = 0;
	section_count = 0;
	for( i=0; i<file_header.f_nscns; i++)
	{
		section_count++;
		my_section->next = 
		(struct my_section_list *)my_calloc( (long)sizeof(struct my_section_list) );
		my_section=my_section->next;
		if( !read_section_header(&my_section->old_hdr, input_fd) )
			FATAL_ERROR( "Unable to read section header" );
		line_number_total += my_section->old_hdr.s_nlnno;
		memcpy(&my_section->new_hdr, &my_section->old_hdr, SCNHSZ);
		if( strncmp(my_section->old_hdr.s_name, code_section_name, SYMNMLEN)== 0 ||
				strncmp(my_section->old_hdr.s_name,  rth_section_name, SYMNMLEN)== 0 )
			reserved_section = 1;
		else 
			reserved_section = 0;
		if( strncmp( my_section->old_hdr.s_name, init_section_name, SYMNMLEN) == 0 )
		{
			init_section_address = my_section->old_hdr.s_paddr;
			init_section_size = my_section->old_hdr.s_size;
		}
		else 
			if( ram_section(my_section->old_hdr.s_name) )
			{
				if ((my_section->old_hdr.s_flags & SECTION_PM) == SECTION_PM)
					init_ptr=hld_pm_init;
				else
					init_ptr=hld_dm_init;

				section_word_size = 
					(my_section->old_hdr.s_flags & SECTION_PM) == SECTION_PM ? \
					PM_WORD_SIZE : DM_WORD_SIZE;

				/* adjust for 32 bit internal pm memory */
				if ((processor != PROCESSOR_ADSP_21020) && 
				    (my_section->old_hdr.s_paddr <0x400000) &&
				    ((my_section->old_hdr.s_flags & SECTION_PM) == SECTION_PM) &&
				    ((width_section(my_section->old_hdr.s_name) == 32) ||
				     (width_section(my_section->old_hdr.s_name) == 16))) 
					init_ptr=hld_dm_init;

				if( !reserved_section )
				  create_init_list( my_section, section_word_size);
				if((my_section->old_hdr.s_flags & SECTION_PM) == SECTION_PM) {
				    /* adjust for 32 bit internal pm memory */
				    if ((processor != PROCESSOR_ADSP_21020) && 
					(my_section->old_hdr.s_paddr <0x400000) &&
					((my_section->old_hdr.s_flags & SECTION_PM) == SECTION_PM) &&
					((width_section(my_section->old_hdr.s_name) == 32) ||
					 (width_section(my_section->old_hdr.s_name) == 16))) 
					    hld_dm_init=init_ptr;
					else
					    hld_pm_init = init_ptr;
				} else
					hld_dm_init = init_ptr;
			}
		my_section->new_hdr.s_flags
		  = my_section->new_hdr.s_flags & (SECTION_MASK);
		my_section->old_hdr.s_flags
		  = my_section->old_hdr.s_flags & (SECTION_MASK);
	}
/*      Remove redundancies from list, seperate zero initializations */
	clean_list();

/*      Write out the file and optional header information */
/*      Compute new symbol pointer based on change in section sizes */
	file_hdr->f_symptr += delta_symptr;
	if( !write_file_header(file_hdr, output_fd) )
		FATAL_ERROR( "Unable to write file header" );
	if( file_hdr->f_opthdr != 0 )
		if( !write_optional_header(opt_hdr,  output_fd) )
			FATAL_ERROR( "Unable to write optional header" );

/*      Write out section headers */
	my_section = my_section_base.next;
	delta_section_ptr = 0L;
	next_section_ptr = 0L;
	for(i=0; i<file_hdr->f_nscns; i++)
	{

/*      .       Check for initialization segment, if so, compute section info */
		if( strncmp(my_section->old_hdr.s_name, init_section_name, SYMNMLEN) == 0 )
			output_init_section(my_section, next_section_ptr);

/*      .       If section contains data, adjust pointer values */
		if( my_section->new_hdr.s_scnptr != 0L )
		{
			my_section->new_hdr.s_scnptr += delta_section_ptr;
		}

/*      .       If section contains linenumbers, adjust pointer values */
		if( my_section->new_hdr.s_lnnoptr )
			my_section->new_hdr.s_lnnoptr += delta_symptr;

		delta_section_ptr += my_section->delta_section_ptr;
	
		if( strncmp(my_section->old_hdr.s_name, init_section_name, SYMNMLEN) == 0 )
			init_section_pointer = my_section->new_hdr.s_scnptr;
		if( !write_section_header(&my_section->new_hdr, output_fd) )
			FATAL_ERROR( "Unable to write section header to output file\n" );

/*      .       Compute next data section pointer (for output_init_section) */
		if( my_section->new_hdr.s_scnptr != 0L )
			next_section_ptr = my_section->new_hdr.s_scnptr + my_section->new_hdr.s_size;
		my_section = my_section->next;
	}

/*      Write out section data */
	my_section = my_section_base.next;
	for(i=0; i<file_hdr->f_nscns; i++)
	{

		fseek(input_fd, my_section->old_hdr.s_scnptr, 0);
		if( my_section->new_hdr.s_size )
			copy_block(my_section->old_hdr.s_size);
/*      .       Check for initialization segment, if so write out computed data */
		if( strncmp(my_section->old_hdr.s_name, init_section_name, SYMNMLEN) == 0 )
			output_init_data();
/*      .       Compute end of data sections for symbol copy */
		if( my_section->old_hdr.s_size )
			end_of_data = my_section->old_hdr.s_scnptr + my_section->old_hdr.s_size;
		free_old = my_section;
		my_section = my_section->next;
		my_free(free_old);
	}

/*      Write out linenumber table */
	fseek(input_fd, end_of_data, 0);
	copy_block( (long)line_number_total * LINESZ );

/*      Write out symbol table to output file */
	adjust = 0;
	aux = 0;
	for( l=0; l<file_hdr->f_nsyms; l++)
	  {
	    if( !read_symbol(symbol, input_fd) )
	      FATAL_ERROR( "Unable to read symbol" );
	    if( OBJ_SYM_ZEROES(symbol) != 0L )
	      if( strncmp(symbol->_n._n_name, "___inits", SYMNMLEN) == 0)
		symbol_address = symbol->n_value;
	    if( aux && adjust )
	      AUX_LINEPTR( *(AUXENT *)symbol ) += delta_symptr;
	    if( !write_symbol(symbol, output_fd) )
	      FATAL_ERROR( "Unable to write symbol" );
	    if( (!aux) && (symbol->n_numaux == 1) )
	      aux = 1;
	    else
	      aux = 0;
	    if( (!adjust) && (symbol->n_type & 0x30) == 0x20 )
	      adjust = 1;
	    else 
	      adjust = 0;
	  }
	
/*      Write out length of string table */
	fread(&string_table_length, sizeof(long), 1, input_fd);
	fwrite(&string_table_length, sizeof(long), 1, output_fd);

	if( string_table_length > 4 )
		copy_block( string_table_length - 4);

/*      Change ___inits symbol to poin to beginning of init_block!*/
	init_block_address = init_section_size / PM_WORD_SIZE;
	init_block_address += init_section_address;
	inits_ptr = init_section_pointer;
	inits_ptr += (symbol_address - init_section_address) * PM_WORD_SIZE;
	fseek(output_fd, inits_ptr, 0);
	port_put_long( init_block_address, long_buff);
	fwrite( long_buff, PM_WORD_SIZE, 1, output_fd);

/*      Flush and close files */
	fflush(input_fd);
	fflush(output_fd);
}
