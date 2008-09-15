#include <stdio.h>
#include <stddef.h>
#include "aux.h"
#include "line.h"
#include "obj_sym.h"
#include "reloc.h"
#include "object.h"
#include "section.h"
#ifdef sun
#include <strings.h>
#else
#include <string.h>
#endif
#ifdef SIM
#include "app.h"
#endif
#include "util.h"
#include "port.h"



/* WARNING: If any of the header files that make up the COFF object file format
 * are changed, these routines may have to be changed. In particular if a
 * particluar item is declared as a short, and then changed to a long the change
 * would have to be reflected here in the apapropriate routine. Even more
 * important is if any changes take place in a union, that change will have
 * to be reflected here.
 */


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      port_write_object_header                                        *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*      Write the object file header a byte at a time, so the object    *
*   file format will be compatible across all platforms.               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     11/17/89       created                  -----            *
***********************************************************************/

void port_write_object_header( register OBJ_HEADER *header, FILE *fd )
{
    char short_buff[sizeof(short)];
    char long_buff[sizeof(long)];

    port_put_long( header->magic, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );

    port_put_long( header->num_sections, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );

    port_put_long( header->time, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );

    port_put_long( header->symbol_ptr, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );

    port_put_long( header->num_symbols, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );

    port_put_short( (short) header->opt_hdr, short_buff );
    fwrite( (char *) short_buff, sizeof(short), 1, fd );

    port_put_short( (short) header->flags, short_buff );
    fwrite( (char *) short_buff, sizeof(short), 1, fd );
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      port_write_section_header                                       *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*      Write a section header a byte at a time, so the object          *
*   file format will be compatible across all platforms.               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     11/17/89       created                  -----            *
***********************************************************************/

void port_write_section_header( register SEC_HEADER *header, FILE *fd )
{
    char short_buff[sizeof(short)];
    char long_buff[sizeof(long)];

    fwrite( (char *) header->sec_name, MAX_SECTION_NAME_SIZE, 1, fd );

    port_put_long( header->paddr, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );

    port_put_long( header->vaddr, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );

    port_put_long( header->data_ptr, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );

    port_put_long( header->data_size, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );

    port_put_long( header->reloc_ptr, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );

    port_put_short( header->num_reloc, short_buff );
    fwrite( (char *) short_buff, sizeof(short), 1, fd );

    port_put_long( header->line_ptr, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );

    port_put_short( header->num_line, short_buff );
    fwrite( (char *) short_buff, sizeof(short), 1, fd );

    port_put_long( header->flags, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      port_write_symbol                                               *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*      Write a symbol table member a byte at a time, so the object     *
*   file format will be compatible across all platforms.               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     11/17/89       created                  -----            *
***********************************************************************/

void port_write_symbol( register OBJ_SYM *ptr, FILE *fd )
{
    char short_buff[sizeof(short)];
    char long_buff[sizeof(long)];

    if( !IN_OBJ_STRING_TABLE(ptr) )
	fwrite( (char *) ptr->what.name, SYMBOL_NAME_LENGTH, 1, fd );
    else
    {
	port_put_long( OBJ_SYM_ZEROES(ptr), long_buff );
	fwrite( (char *) long_buff, sizeof(long), 1, fd );

	port_put_long( OBJ_SYM_OFFSET(ptr), long_buff );
	fwrite( (char *) long_buff, sizeof(long), 1, fd );
    }

    port_put_long( ptr->value, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );

    port_put_short( ptr->section_num, short_buff );
    fwrite( (char *) short_buff, sizeof(short), 1, fd );

    port_put_long( ptr->type, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );

    fwrite( (char *) &(ptr->sclass), sizeof(char), 1, fd );

    fwrite( (char *) &(ptr->num_aux), sizeof(char), 1, fd );

    port_put_short( ptr->mem_type, short_buff );
    fwrite( (char *) short_buff, sizeof(short), 1, fd );
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      port_write_relocation_info                                      *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*      Write a relocation record a byte at a time, so the object       *
*   file format will be compatible across all platforms.               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     11/17/89       created                  -----            *
***********************************************************************/

void port_write_relocation_info( register RELOC *ptr, FILE *fd )
{
    char short_buff[sizeof(short)];
    char long_buff[sizeof(long)];

    port_put_long( ptr->r_address, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );

    port_put_long( ptr->relname.r_symindex, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );

    port_put_short( ptr->r_type, short_buff );
    fwrite( (char *) short_buff, sizeof(short), 1, fd );
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      port_write_aux_info                                             *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*      Write a auxillary entry a byte at a time, so the object         *
*   file format will be compatible across all platforms.               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     11/17/89       created                  -----            *
***********************************************************************/

void port_write_aux_info( register AUXENT *ptr, FILE *fd, int aux_class )
{
    char short_buff[sizeof(short)];
    char long_buff[sizeof(long)];

    if( aux_class == AUX_CLASS_SYM )
    {
	port_put_long( ptr->sym.tag_index, long_buff );
	fwrite( (char *) long_buff, sizeof(long), 1, fd );

	port_put_long( ptr->sym.misc.func_size, long_buff );
	fwrite( (char *) long_buff, sizeof(long), 1, fd );

	port_put_long( ptr->sym.fcnary.fcn.line_ptr, long_buff );
	fwrite( (char *) long_buff, sizeof(long), 1, fd );

	port_put_long( ptr->sym.fcnary.fcn.end_index, long_buff );
	fwrite( (char *) long_buff, sizeof(long), 1, fd );

	port_put_long( ptr->sym.tvndx, long_buff );
	fwrite( (char *) long_buff, sizeof(long), 1, fd );

	port_put_short( ptr->sym.dummy, short_buff );
	fwrite( (char *) short_buff, sizeof(short), 1, fd );
    }
    else if( aux_class == AUX_CLASS_FILE_NAME )
    {
	fwrite( ptr->file.fname, sizeof(ptr->file.fname), 1, fd );

	memset( ptr->file.file_pad, 0, sizeof(ptr->file.file_pad) );
	fwrite( ptr->file.file_pad, sizeof(ptr->file.file_pad), 1, fd );                
    }
    else if( aux_class == AUX_CLASS_SECTION )
    {
	port_put_long( ptr->scn.scnlen, long_buff );
	fwrite( (char *) long_buff, sizeof(long), 1, fd );

	port_put_short( ptr->scn.nreloc, short_buff );
	fwrite( (char *) short_buff, sizeof(short), 1, fd );

	port_put_short( ptr->scn.nlinno, short_buff );
	fwrite( (char *) short_buff, sizeof(short), 1, fd );

	memset( ptr->scn.scn_pad, 0, sizeof(ptr->scn.scn_pad) );
	fwrite( ptr->scn.scn_pad, sizeof(ptr->scn.scn_pad), 1, fd );    
    }
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      port_write_line_info                                            *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*      Write a line entry a byte at a time, so the object file format  *
*   will be compatible across all platforms.                           *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     11/17/89       created                  -----            *
***********************************************************************/

void port_write_line_info( register LINE *ptr, FILE *fd )
{
    char short_buff[sizeof(short)];
    char long_buff[sizeof(long)];

    port_put_short( ptr->line_number, short_buff );
    fwrite( (char *) short_buff, sizeof(short), 1, fd );

    port_put_long( ptr->which.sym_index, long_buff );
    fwrite( (char *) long_buff, sizeof(long), 1, fd );
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      port_read_object_header                                         *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*      Read the object file header a byte at a time, so the object     *
*   file format will be compatible across all platforms.               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     11/17/89       created                  -----            *
***********************************************************************/

int port_read_object_header( register OBJ_HEADER *header, FILE *fd )
{
    char short_buff[sizeof(short)];
    char long_buff[sizeof(long)];

    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	header->magic = port_get_long( long_buff );
    else
	return( 0 );    

    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	header->num_sections = port_get_long( long_buff );
    else
	return( 0 );

    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	header->time = port_get_long( long_buff );
    else
	return( 0 );

    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	header->symbol_ptr = port_get_long( long_buff );
    else
	return( 0 );
  
    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	header->num_symbols = port_get_long( long_buff );
    else
	return( 0 );

    if( fread( (char *) short_buff, sizeof(short), 1, fd ) == 1 )
	header->opt_hdr = port_get_short( short_buff );
    else
	return( 0 );

    if( fread( (char *) short_buff, sizeof(short), 1, fd ) == 1 )
	header->flags = port_get_short( short_buff );
    else
	return( 0 );

    return( 1 );
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      port_read_section_header                                        *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*      Read a section header a byte at a time, so the object           *
*   file format will be compatible across all platforms.               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     11/17/89       created                  -----            *
***********************************************************************/

int port_read_section_header( register SEC_HEADER *header, FILE *fd )
{
    char short_buff[sizeof(short)];
    char long_buff[sizeof(long)];

    if( fread( (char *) header->sec_name, MAX_SECTION_NAME_SIZE, 1, fd ) == 1 )
	;
    else
	return( 0 );

    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	header->paddr = port_get_long( long_buff );
    else
	return( 0 );

    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	header->vaddr = port_get_long( long_buff );
    else
	return( 0 );

    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	header->data_ptr = port_get_long( long_buff );
    else
	return( 0 );

    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	header->data_size = port_get_long( long_buff );
    else
	return( 0 );

    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	header->reloc_ptr = port_get_long( long_buff );
    else
	return( 0 );

    if( fread( (char *) short_buff, sizeof(short), 1, fd ) == 1 )
	header->num_reloc = port_get_short( short_buff );
    else
	return( 0 );

    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	header->line_ptr = port_get_long( long_buff );
    else
	return( 0 );

    if( fread( (char *) short_buff, sizeof(short), 1, fd ) == 1 )
	header->num_line = port_get_short( short_buff );
    else
	return( 0 );

    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	header->flags = port_get_long( long_buff );
    else
	return( 0 );

    return( 1 );
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      port_read_symbol                                                *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*      Read a symbol table entry a byte at a time, so the object       *
*   file format will be compatible across all platforms.               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     11/17/89       created                  -----            *
*     dkl     5/8/90         Now get offset in                         *
*                            machine independent manner                *
***********************************************************************/

int port_read_symbol( register OBJ_SYM *ptr, FILE *fd )
{
    char short_buff[sizeof(short)];
    char long_buff[sizeof(long)];
    long temp_long;

    if( fread( (char *) ptr->what.name, SYMBOL_NAME_LENGTH, 1, fd ) == 1 )
	;
    else
	return( 0 );

    if (IN_OBJ_STRING_TABLE (ptr)) {
      temp_long = OBJ_SYM_OFFSET (ptr);
      ptr->what.strtab_entry.offset = port_get_long ((char *)&temp_long);
    }

    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	ptr->value = port_get_long( long_buff );
    else
	return( 0 );

    if( fread( (char *) short_buff, sizeof(short), 1, fd ) == 1 )
	ptr->section_num = port_get_short( short_buff );
    else
	return( 0 );

    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	ptr->type = port_get_long( long_buff );
    else
	return( 0 );

    if( fread( (char *) &(ptr->sclass), sizeof(char), 1, fd ) == 1 )
	;
    else
	return( 0 );

    if( fread( (char *) &(ptr->num_aux), sizeof(char), 1, fd ) == 1 )
	;
    else
	return( 0 );

    if( fread( (char *) short_buff, sizeof(short), 1, fd ) == 1 )
	ptr->mem_type = port_get_short( short_buff );
    else
	return( 0 );

    return( 1 );
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      port_read_relocation_info                                       *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*      Read relocation entry a byte at a time, so the object           *
*   file format will be compatible across all platforms.               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     11/17/89       created                  -----            *
***********************************************************************/

int port_read_relocation_info( register RELOC *ptr, FILE *fd )
{
    char short_buff[sizeof(short)];
    char long_buff[sizeof(long)];

    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	ptr->r_address = port_get_long( long_buff );
    else
	return( 0 );

    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	ptr->relname.r_symindex = port_get_long( long_buff );
    else
	return( 0 );

    if( fread( (char *) short_buff, sizeof(short), 1, fd ) == 1 )
	ptr->r_type = port_get_short( short_buff );
    else
	return( 0 );

    return( 1 );
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      port_read_aux_info                                              *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*      Read a auxillary entry a byte at a time, so the object          *
*   file format will be compatible across all platforms.               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     11/17/89       created                  -----            *
***********************************************************************/

int port_read_aux_info( register AUXENT *ptr, FILE *fd, int aux_class )
{
    char short_buff[sizeof(short)];
    char long_buff[sizeof(long)];

    if( aux_class == AUX_CLASS_SYM )
    {
	if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	    ptr->sym.tag_index = port_get_long( long_buff );
	else
	    return( 0 );

	if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	    ptr->sym.misc.func_size = port_get_long( long_buff );
	else
	    return( 0 );

	if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	    ptr->sym.fcnary.fcn.line_ptr = port_get_long( long_buff );
	else
	    return( 0 );

	if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	    ptr->sym.fcnary.fcn.end_index = port_get_long( long_buff );
	else
	    return( 0 );

	if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	    ptr->sym.tvndx = port_get_long( long_buff );
	else
	    return( 0 );

	if( fread( (char *) short_buff, sizeof(short), 1, fd ) == 1 )
	    ptr->sym.dummy = port_get_short( short_buff );
	else
	    return( 0 );
    }
    else if( aux_class == AUX_CLASS_FILE_NAME )
    {
	if( fread( ptr->file.fname, sizeof(ptr->file.fname), 1, fd) == 1 )
	    ;
	else
	    return( 0 );

	if( fread( ptr->file.file_pad, sizeof(ptr->file.file_pad), 1, fd) == 1) 
	    memset( ptr->file.file_pad, 0, sizeof(ptr->file.file_pad ) );
	else
	    return( 0 );
    }
    else if( aux_class == AUX_CLASS_SECTION )
    {
	if( fread( long_buff, sizeof(long), 1, fd ) == 1 )
	    ptr->scn.scnlen = port_get_long( long_buff );
	else
	    return( 0 );

	if( fread( short_buff, sizeof(short), 1, fd ) == 1 )
	    ptr->scn.nreloc = port_get_short( short_buff );
	else
	    return( 0 );

	if( fread( short_buff, sizeof(short), 1, fd ) == 1 )
	    ptr->scn.nlinno = port_get_short( short_buff );
	else
	    return( 0 );

	if( fread( ptr->scn.scn_pad, sizeof(ptr->scn.scn_pad), 1, fd) == 1)
	    memset( ptr->scn.scn_pad, 0, sizeof(ptr->scn.scn_pad) );
	else
	    return( 0 );
    }
    else
	return( 0 );

    return( 1 );
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      port_read_line_info                                             *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*      Read a line number entry a byte at a time, so the object        *
*   file format will be compatible across all platforms.               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     11/17/89       created                  -----            *
***********************************************************************/

int port_read_line_info( register  LINE *ptr, FILE *fd )
{
    char short_buff[sizeof(short)];
    char long_buff[sizeof(long)];

    if( fread( (char *) short_buff, sizeof(short), 1, fd ) == 1 )
	ptr->line_number = port_get_short( short_buff );
    else
	return( 0 );

    if( fread( (char *) long_buff, sizeof(long), 1, fd ) == 1 )
	ptr->which.sym_index = port_get_long( long_buff );
    else
	return( 0 );

    return( 1 );
}
