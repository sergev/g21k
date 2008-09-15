/* @(#)relocate.c       2.2 2/13/95 2 */

#include <stdio.h>
#include <stddef.h>

#include "app.h"
#include "a_out.h"
#include "error.h"
#include "input_file.h"
#include "input_section.h"
#include "slotvec.h"
#include "relocate.h"
#include "process_args.h"
#include "util.h"
#include "coff_io.h"

#include "slotvec_fp.h"
#include "relocate_fp.h"
#include "error_fp.h"

static short calculate_address_size( short reloc_type );


/***********************************************************************
*                                                                      *
*   Name                                                               * 
*      relocate_section                                                *
*                                                                      *
*   Synopsis                                                           *
*      void relocate_section( FILE *input_file_fd,                     *
*                             INPUT_FILE *input_file_ptr,              *
*                             INPUT_SECT *input_sect_ptr,              *
*                             FILE *output_fd, FILE *reloc_fd,         *
*                             char *sect_buffer, long buffer_size )    *
*                                                                      *
*   Description                                                        *
*      This is what the linker is all about. We fix up the relocatable *
*   addresses within each section here.                                *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/21/89       created                  -----             *
***********************************************************************/

void relocate_section( FILE *input_file_fd, INPUT_FILE *input_file_ptr, 
		       INPUT_SECT *input_sect_ptr, FILE *output_fd, long offset, 
		       register char *sect_buffer, long buffer_size )
{
    short             address_size;
    short             buf_offset;
    long              byte_offset;
    long              bytes_processed;
    long              chunk_size;
    long              index;
    long              num_read;
    register long     reloc_offset;
    RELOC             reloc;
    long              section_size;
    register SLOTVEC *slot_vec_ptr;
    long              vaddr_diff;
    VALUE             value;
    long              file_offset;

    vaddr_diff = input_sect_ptr->vaddr - input_sect_ptr->sec_hdr.s_vaddr;
    section_size = input_sect_ptr->sec_hdr.s_size;
    chunk_size = min( section_size, buffer_size );

    if( rflag )
    {
	/* Save our current position in the object file and
	 * seek to the relocation entries
	 */

	file_offset = ftell( output_fd );
	fseek( output_fd, offset, 0 );
    }

    if( fread( sect_buffer, (size_t) chunk_size, 1, input_file_fd) != 1 )
    {
	FATAL_ERROR2( "Error reading section %s of file: %.8s.", input_sect_ptr->sec_hdr.s_name,
		       input_file_ptr->file_name );
    }

    section_size -= chunk_size;
    bytes_processed = 0L;

#ifdef DEBUG
    printf( "relocate_section: section = %.8s\n", input_sect_ptr->sec_hdr.s_name );
    fflush( stdout );
#endif

    fseek( input_file_fd, input_sect_ptr->sec_hdr.s_relptr + input_file_ptr->arch_offset, 0 );

    for( num_read = 1; num_read <= (long)input_sect_ptr->sec_hdr.s_nreloc; num_read++ )
    {
	 if( read_relocation_info( &reloc, input_file_fd) != 1 )
	 {
	     FATAL_ERROR2( "Error reading relocation entries of section %.8s in file: %s.",
			    input_sect_ptr->sec_hdr.s_name, input_file_ptr->file_name );
	 }

	 if( (slot_vec_ptr = slotvec_read(reloc.r_symndx)) == NULL )
	 {
	     FATAL_ERROR1( "Relocation entry found for non-relocatable symbol in file: %s",
			     input_file_ptr->file_name );
	     continue;
	 }

	 if( reloc.r_type == R_ABS )
	     continue;

	 reloc_offset = slot_vec_ptr->sv_new_vaddr - slot_vec_ptr->sv_vaddr;

	 if ((input_sect_ptr->sec_hdr.s_flags & SECTION_PM) == SECTION_PM)
	 {
	     byte_offset = ((reloc.r_vaddr - input_sect_ptr->sec_hdr.s_vaddr) *
			    PM_WORD_SIZE);
	     if( R_ADDR_VAR != reloc.r_type )
		  byte_offset += relocate_addr_offset( reloc.r_type );
	 }
	 else
	     byte_offset = (reloc.r_vaddr - input_sect_ptr->sec_hdr.s_vaddr) * DM_WORD_SIZE;

	 address_size = calculate_address_size( reloc.r_type );

	 /* If we haven't yet read in the portion of the instruction that
	  * contains the address we want to relocate, keep reading until we
	  * get it.
	  */

	 while( byte_offset > (bytes_processed + chunk_size) )
	 {
		bytes_processed += chunk_size;
		fwrite( sect_buffer, (size_t) chunk_size, 1, output_fd );
		chunk_size = min( section_size, buffer_size );
		fseek( input_file_fd, input_sect_ptr->sec_hdr.s_scnptr + 
		       input_file_ptr->arch_offset + bytes_processed, 0 );

		if( fread( sect_buffer, (size_t) chunk_size, 1, input_file_fd) != 1 )
		    FATAL_ERROR2( "Error reading section %.8s in file %s.",
				  input_sect_ptr->sec_hdr.s_name, input_file_ptr->file_name );

		section_size -= chunk_size;
		fseek( input_file_fd, input_sect_ptr->sec_hdr.s_relptr + 
		       input_file_ptr->arch_offset + (num_read * RELOC_SIZE), 0 );
	 }

	 /* We may have only read in part of the address. If so get the rest of it. */

	 if( (byte_offset + address_size > bytes_processed + chunk_size) && section_size )
	 {
	     bytes_processed += chunk_size - address_size;
	     fwrite( sect_buffer, (size_t) (chunk_size - (long) address_size), 1, output_fd );      
	     for( buf_offset = 0; buf_offset < address_size; ++buf_offset )
		  sect_buffer[buf_offset] = sect_buffer[chunk_size - (address_size - buf_offset)];
	     chunk_size = min( section_size, buffer_size - address_size );

	     fseek( input_file_fd, input_sect_ptr->sec_hdr.s_scnptr + 
		   input_file_ptr->arch_offset + bytes_processed + address_size, 0 );
		
	     if( fread( sect_buffer + address_size, (size_t) chunk_size, 1, input_file_fd) != 1 )
		 FATAL_ERROR2( "Error reading section %.8s in file %s.",
			      input_sect_ptr->sec_hdr.s_name, input_file_ptr->file_name );

	     section_size -= chunk_size;
	     chunk_size += address_size;
	     fseek( input_file_fd, input_sect_ptr->sec_hdr.s_relptr + 
		    input_file_ptr->arch_offset + (num_read * RELOC_SIZE), 0 );      
	 }

	 index = byte_offset - bytes_processed;
	 switch( reloc.r_type )
	 {
	       case R_ADDR24:
		  value.c[0] = 0;
		  value.c[1] = sect_buffer[index];
		  value.c[2] = sect_buffer[index + 1];
		  value.c[3] = sect_buffer[index + 2];
		  value.l = port_get_long( (unsigned char *)value.c );
		  value.l += reloc_offset;
		  break;
		
	       case R_PCRLONG:
		  value.c[0] = 0;
		  value.c[1] = sect_buffer[index];
		  value.c[2] = sect_buffer[index + 1];
		  value.c[3] = sect_buffer[index + 2];
		  value.l = port_get_long( (unsigned char *)value.c );
		  value.l += reloc_offset;
		  value.l -= ((reloc.r_vaddr - input_sect_ptr->sec_hdr.s_vaddr) +
			       input_sect_ptr->vaddr);
		  break;
		
	       case R_ADDR32:
	       case R_ADDR_VAR:
		  value.c[0] = sect_buffer[index];
		  value.c[1] = sect_buffer[index + 1];
		  value.c[2] = sect_buffer[index + 2];
		  value.c[3] = sect_buffer[index + 3];
		  value.l = port_get_long( (unsigned char *)value.c );
		  value.l += reloc_offset;
		  break;
	 }

	 /* Put the relocated address back in the buffer, to be written out */
	 if( reloc.r_type == R_ADDR24 || reloc.r_type == R_PCRLONG )
	 {
	     sect_buffer[index]     = (char) DATA32_TOP_MIDDLE8(value.l);
	     sect_buffer[index + 1] = (char) DATA32_MIDDLE8(value.l);
	     sect_buffer[index + 2] = (char) DATA32_BOTTOM8(value.l);
	 }
	 else
	     port_put_long( value.l, (unsigned char *)(sect_buffer + index) );
	     
	 if( rflag )
	 {
	     reloc.r_vaddr += vaddr_diff;
	     reloc.r_symndx = slot_vec_ptr->sv_new_symindex;
		
	     write_relocation_info( &reloc, output_fd );
	 }
     }

    /* If necessary, get back to our original position in the object file */

    if( rflag )
	fseek( output_fd, file_offset, 0 );

    while( section_size )
    {
	   bytes_processed += chunk_size;
	   fwrite( sect_buffer, (size_t) chunk_size, 1, output_fd );
	   chunk_size = min( section_size, buffer_size );
	   fseek( input_file_fd, input_file_ptr->arch_offset + input_sect_ptr->sec_hdr.s_scnptr +
		  bytes_processed, 0 );
	   if( fread(sect_buffer, (size_t) chunk_size, 1, input_file_fd) != 1 )
	   {
	      FATAL_ERROR2( "Error reading section %.8s in file: %s.", 
			    input_sect_ptr->sec_hdr.s_name, input_file_ptr->file_name );
	   }
	   section_size -= chunk_size;
    }
    fwrite( sect_buffer, (size_t) chunk_size, 1, output_fd );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      relocate_addr_offset                                            *
*                                                                      *
*   Synopsis                                                           *
*      int relocate_addr_offset( short reloc_type )                    *
*                                                                      *
*   Description                                                        *
*      This routine returns the byte offset of the address within a    *
*   instruction, based on the relocation type.                         *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/21/89       created                  -----             *
***********************************************************************/

int relocate_addr_offset( short reloc_type )
{
    switch( reloc_type )
    {
	    case R_ADDR24:
		return( 0x3 );

	    case R_ADDR32:
		return( 0x2 );

	    case R_ADDR_VAR:
		return( 0x2 );

	    case R_PCRSHORT:
		FATAL_ERROR( "PC relative short branches should be resolved in the assembler." );
		return( 0 );

	    case R_PCRLONG:
		return( 0x3 ); 

	    default:
		FATAL_ERROR( "Bad relocatable type. " );
		return( 0 );

    }
}

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      calcualte_address_size                                          *
*                                                                      *
*   Description                                                        *
*      This routine returns the size in bytes of an address given a    * 
*   relocation type.                                                   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/21/89       created                  -----             *
***********************************************************************/
	     
 static short calculate_address_size( short reloc_type )
{
    switch( reloc_type )
    {
	    case R_ADDR24:
	    case R_PCRLONG:
		return( 0x3 );

	    case R_ADDR32:
	    case R_ADDR_VAR:
		return( 0x4 );

	    case R_PCRSHORT:
		FATAL_ERROR( "PC relative short branches should be resolved in the assembler." );
		return( 0 );

    }
}
