/* @(#)update.c 2.4 2/13/95 2 */

#include "app.h"
#include "address.h"
#include "allocate.h"
#include "a_out.h"
#include "linker_aux.h"
#include "linker_sym.h"
#include "list.h"
#include "memory.h"
#include "memory_fp.h"
#include "input_file.h"
#include "input_section.h"
#include "process_args.h"
#include "process_section.h"
#include "output_section.h"
#include "read_object.h"
#include "syms.h"

#include "syms_fp.h"
#include "update_fp.h"

long symbol_table_origin = 0l;
long total_output_symbols = 0l;


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      update_linker_information                                       *
*                                                                      *
*   Synopsis                                                           *
*      void update_linker_information( void )                          *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for updating linker information     *
*   that is needed in creating the final executable.                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/13/89       created                  -----             *
***********************************************************************/

void update_linker_information( void )
{
    update_section_information();
    update_symbol_information();
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      void update_section_information                                 *
*                                                                      *
*   Synopsis                                                           *
*      void update_section_information( void )                         *
*                                                                      *
*   Description                                                        *
*      This routine updates various pieces if section information in   *
*   preparation for generating the output file.                        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/13/89       created                  -----             *
***********************************************************************/

void update_section_information( void )
{
    register ADDRESS     *addr_ptr;
	     short        data;
	     long         file_offset;
	     long         line_ptr;
	     long         num_reloc;
    register OUTPUT_SECT *out_sect_ptr;
    register INPUT_SECT  *input_sect_ptr;
	     long         section_num;
	
    num_reloc = section_num = 0L;
    file_offset = (long) FILHSZ + SCNHSZ * num_output_sects;

    for( addr_ptr = (ADDRESS *) avail_list.head; addr_ptr; addr_ptr = addr_ptr->next )
    {
	 if( addr_ptr->addr_type == ADDRESS_ALLOCATED )
	 {
	     out_sect_ptr = addr_ptr->sect_ptr;
	     out_sect_ptr->section_num = (short) ++section_num;
		
	     data = 0;
	     for( input_sect_ptr = out_sect_ptr->in_sect_head; input_sect_ptr; 
		  input_sect_ptr = input_sect_ptr->inc_next )
	     {
		      
		  input_sect_ptr->paddr
		    = out_sect_ptr->sec_hdr.s_paddr + 
		      input_sect_ptr->data_disp / 
			((input_sect_ptr->sec_hdr.s_flags & SECTION_PM) == SECTION_PM
			 ? PM_WORD_SIZE : DM_WORD_SIZE);
		  input_sect_ptr->vaddr = input_sect_ptr->paddr;
		  if( input_sect_ptr->sec_hdr.s_scnptr > 0 ) 
		    data = 1;
	     }

	     if( segment_uninit(out_sect_ptr->sec_hdr.s_name) )
	     {
		 out_sect_ptr->sec_hdr.s_scnptr = 0L;
		 out_sect_ptr->sec_hdr.s_size = 0L;
		 data = 0;
	     }

	     if( data )
	     {
		 out_sect_ptr->sec_hdr.s_scnptr = file_offset;
		 file_offset += out_sect_ptr->sec_hdr.s_size;
		 if( rflag )
		     num_reloc += (long) out_sect_ptr->sec_hdr.s_nreloc;
	     }
	     else
		out_sect_ptr->sec_hdr.s_scnptr = 0L;
	 }
    }

    line_ptr = file_offset + num_reloc * (long) RELOC_SIZE;
    for( addr_ptr = (ADDRESS *) avail_list.head; addr_ptr; addr_ptr = addr_ptr->next )
    {
	 if( addr_ptr->addr_type == ADDRESS_ALLOCATED )
	 {
	     out_sect_ptr = addr_ptr->sect_ptr;
	     if( rflag && out_sect_ptr->sec_hdr.s_nreloc != 0 )
	     {
		 out_sect_ptr->sec_hdr.s_relptr = file_offset;
		 file_offset += ((long) out_sect_ptr->sec_hdr.s_nreloc * RELOC_SIZE);
	     }

	     if( !sflag && out_sect_ptr->sec_hdr.s_nlnno != 0 )
	     {
		 out_sect_ptr->sec_hdr.s_lnnoptr = line_ptr;
		 line_ptr += ((long) out_sect_ptr->sec_hdr.s_nlnno * (long) LINESZ);
	     }
	 }
    }
    symbol_table_origin = line_ptr;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      update_symbol_information                                       *
*                                                                      *
*   Synopsis                                                           *
*      void update_symbol_information( void )                          *
*                                                                      *
*   Description                                                        *
*      This routine updates various pieces of symbol information in    *
*   preparation for generating the final ouput file.                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/13/89       created                  -----             *
***********************************************************************/

void update_symbol_information( void )
{
    register long        i;
    long                 index;
    INPUT_FILE          *in_file_ptr;
    register INPUT_SECT *in_sect_ptr;
    register LINKER_SYM *sym_ptr;
        
    in_file_ptr = (INPUT_FILE *) input_file_list.head;
    index = 0L;

    while( in_file_ptr )
    {
	   in_file_ptr->symbol_index = index;
	   index += in_file_ptr->local_syms;
	   in_file_ptr = in_file_ptr->infile_next;
    }

    i = 1;
    while( i <= total_symbols )
    {
	   sym_ptr = getsym( i++ );

	   /* Is the symbol defined in some section ? */

	   if( sym_ptr->obj_sym.n_scnum > 0 )
	   {
#ifdef DEBUG
	       printf( "update: Symbol name = %s\n", sym_ptr->obj_sym._n._n_name );
	       fflush( stdout );
#endif
	       in_sect_ptr = sym_ptr->input_sect;
	       sym_ptr->obj_sym.n_scnum = in_sect_ptr->out_sec->section_num;
	       sym_ptr->new_val = sym_ptr->obj_sym.n_value + in_sect_ptr->paddr -
				  in_sect_ptr->sec_hdr.s_paddr;
	       sym_ptr->new_virtual = sym_ptr->new_val - in_sect_ptr->paddr + in_sect_ptr->vaddr;

	       if( sym_ptr->local_flag )
		   sym_ptr->sym_index += in_sect_ptr->file_ptr->symbol_index;
	       else
	       {
		   sym_ptr->sym_index = index++;
		   index += sym_ptr->obj_sym.n_numaux;
	       }
	   }
	   else
	   {
	       if( (IN_OBJ_STRING_TABLE(&(sym_ptr->obj_sym))) || 
		   (strncmp(sym_ptr->obj_sym._n._n_name, ".",1) != 0) )
	       {
		   sym_ptr->sym_index = index++;
	       }

	       sym_ptr->new_val = sym_ptr->obj_sym.n_value;
	       sym_ptr->new_virtual = sym_ptr->obj_sym.n_value;
	       index += sym_ptr->obj_sym.n_numaux;
	   }
    }
    total_output_symbols = index;
}

