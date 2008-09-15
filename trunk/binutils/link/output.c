/* @(#)output.c 2.12 6/6/95 2 */

#include <stdio.h>
#include <stddef.h>

#include "app.h"
#include "a_out.h"
#include "global_syms.h"
#include "address.h"
#include "allocate.h"
#include "ar.h"
#include "error.h"
#include "input_file.h"
#include "input_section.h"
#include "ldfcn.h"
#include "action.h"
#include "list.h"
#include "list_fp.h"
#include "linker_aux.h"
#include "linker_sym.h"
#include "output_section.h"
#include "mem_map_symbol.h"
#include "mem_map.h"
#include "process_section.h"
#include "read_object.h"
#include "slotvec.h"
#include "process_args.h"
#include "update.h"
#include "coff_io.h"
#include "tempio.h"
#include "util.h"
#include <time.h>

#include "output_fp.h"
#include "slotvec_fp.h"
#include "error_fp.h"
#include "relocate_fp.h"
#include "read_object_fp.h"
#include "syms_fp.h"
#include "memory_map_util_fp.h"

int output_get_aux_class( SYMENT *sym );

MEM_MAP         *mem_map;
MEM_MAP         *base_mem_map;
static short    error_level = 0;
static long     num_output_syms = 0l;
static long     lastfile = -1L;         /* symtab index of previous .file entry */
static long     nextfile = -1L;         /* symtab index of next .file entry */
static char     undefined_message = '\0';  /* has the undefined message been printed? */

static long     string_buf_length = 0;
static char    *last_string_ptr = NULL;
static int      sym_temp_index = 0;
static int      string_temp_index = 0;
long            string_offset = 4L;


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      output_object                                                   *
*                                                                      *
*   Synopsis                                                           *
*      void output_object( void )                                      *
*                                                                      *
*   Description                                                        *
*      This is the master routine for creating the final executable.   *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/13/89       created                  -----             *
***********************************************************************/

void output_object( void )
{
    char                 *buf;
    FILE                 *output_fd;
    FILE                 *string_fd;
    FILE                 *symbol_fd;

    buf = my_calloc( (long)BUFSIZ );

    if( (output_fd = fopen(output_file_name, WRITE_BINARY)) == NULL)
	 FATAL_ERROR( "Error opening final object file." );

	if( (symbol_fd = temp_file_create(WRITE_BINARY)) == NULL )
	     FATAL_ERROR( "Error opening symbol table file" );
	sym_temp_index = num_open_files - 1;

	if( (string_fd = temp_file_create(WRITE_BINARY)) == NULL )
	     FATAL_ERROR( "Error opening string table file" );
	string_temp_index = num_open_files - 1;

    fseek( output_fd, (long) FILHSZ, 0 );
    output_section_headers( output_fd );
    slotvec_allocate( max_relocatable_syms );
    output_process_input_files( output_fd, symbol_fd, string_fd, buf );

    if( error_level > 1 )
	FATAL_ERROR( "Undefined symbols encountered." );

    if( aflag && error_level )
	USER_WARN1( "Output file %s not executable", output_file_name );

    fflush( output_fd );
    output_global_symbols( symbol_fd, string_fd );

    if( error_level > 1 )
	FATAL_ERROR( "Undefined symbols encountered." );

    output_finish_up( symbol_fd, output_fd, string_fd );
    my_free( buf );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      output_section_headers                                          *
*                                                                      *
*   Synopsis                                                           *
*     void output_section_headers( FILE *output_fd )                   *   
*                                                                      *
*   Description                                                        *
*      This routine is responsible for writing the section headers to  *
*   the final executable.                                              *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/13/89       created                  -----             *
***********************************************************************/

void output_section_headers( FILE *output_fd )
{
    register ADDRESS *addr_ptr;

    for( addr_ptr = (ADDRESS *) avail_list.head; addr_ptr; addr_ptr = addr_ptr->next )
    {
	if (addr_ptr->addr_type == ADDRESS_ALLOCATED)
	{
	    if( sflag || xflag )
		addr_ptr->sect_ptr->sec_hdr.s_nlnno = 0;

	    write_section_header( &(addr_ptr->sect_ptr->sec_hdr), output_fd );
	}
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      output_process_input_files                                      *
*                                                                      *
*   Synopsis                                                           *
*      void output_process_input_files( FILE *output_fd,               *
*      FILE *symbol_fd, FILE *string_fd, FILE *reloc_fd, char *buf )   *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for processing every input file by  *
*   processing the local symbols, relocating and writing the section   *
*   info and updating the line info for each section.                  *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/13/89       created                  -----             *
***********************************************************************/

void output_process_input_files( register FILE *output_fd, FILE *symbol_fd, 
				 FILE *string_fd, char *buf )
{
    long                 buffer_size;
    register FILE       *input_file_fd;
    INPUT_FILE          *input_file_ptr;
    register INPUT_SECT *input_sect_ptr;
    OUTPUT_SECT         *output_sect_ptr;
    INPUT_FILE          *prev_input_file_ptr;
    long                 offset;
    char                *sect_buffer;

    offset = 0L;
	
#ifdef MSDOS
    sect_buffer = buf;
    buffer_size = BUFSIZ;
#else
    if( sect_buffer = my_malloc( (long)max_section_size) )
	buffer_size = max_section_size;
#endif

    input_file_fd = NULL;
    prev_input_file_ptr = NULL;
    input_file_ptr = (INPUT_FILE *) input_file_list.head;

    /* Make a dummy header, to get the ball rolling */
    if( mflag )
	base_mem_map = mem_map = mem_map_init( "", 0 );

    while( input_file_ptr != NULL )
    {
	   if( input_file_fd == NULL )
	   {     
	       if( (input_file_fd = fopen(input_file_ptr->file_name, READ_BINARY)) == NULL )
		    FATAL_ERROR1("Error opening %s", input_file_ptr->file_name);
	   }

#ifdef DEBUG
	   printf( "output_process_input_files: %s\n", input_file_ptr->file_name );
	   fflush( stdout );
#endif
	   if( mflag)
	   {
	       mem_map->next = mem_map_init( input_file_ptr->file_name, 
					     (int) input_file_ptr->num_symbols );
	       mem_map = mem_map->next;
	   }

	   num_output_syms += output_local_symbols( symbol_fd, string_fd, input_file_fd, 
						    input_file_ptr );

	   for( input_sect_ptr = input_file_ptr->head; input_sect_ptr; 
		input_sect_ptr = input_sect_ptr->next )
	   {

		output_sect_ptr = input_sect_ptr->out_sec;
		offset = output_sect_ptr->sec_hdr.s_scnptr + input_sect_ptr->data_disp;

		if( (input_sect_ptr->sec_hdr.s_size == 0L) || 
		    (output_sect_ptr->sec_hdr.s_scnptr == 0L) )
		{
		     continue;
		}

		if( fseek(input_file_fd, input_sect_ptr->sec_hdr.s_scnptr + 
			  input_file_ptr->arch_offset, BEGINNING) != OKFSEEK )
		{
		    LINKER_ERROR2( "Seek to %s section %.8s failed", input_file_ptr->file_name, 
			       input_sect_ptr->sec_hdr.s_name );
		}

#ifdef DEBUG
		printf( "output_process_input_files: section = %.8s\n", 
			input_sect_ptr->sec_hdr.s_name);
		fflush( stdout );
#endif 
		fseek(output_fd, offset, 0);
		if( input_sect_ptr->sec_hdr.s_nreloc == 0 )
		{
		     output_section( input_file_fd, input_file_ptr, input_sect_ptr, output_sect_ptr, 
				     output_fd, sect_buffer, buffer_size );
		}
		else
		{
		     if( rflag )
			 offset = output_sect_ptr->sec_hdr.s_relptr + input_sect_ptr->reloc_disp;

		     relocate_section( input_file_fd, input_file_ptr, input_sect_ptr, output_fd, 
				       offset, sect_buffer, buffer_size );
		}

		if( !sflag && !xflag && input_sect_ptr->sec_hdr.s_nlnno > 0 )
		{
		    output_line_numbers( input_sect_ptr, output_sect_ptr, input_file_fd,
					 symbol_fd, output_fd, input_file_ptr );
		}

	   }


	   prev_input_file_ptr = input_file_ptr;
	   input_file_ptr = input_file_ptr->infile_next;

	   /* see if next input file is from same archive */

	   if( (prev_input_file_ptr->arch_offset > 0L) && (input_file_ptr != NULL) )
	   {
		if( strcmp(prev_input_file_ptr->file_name, input_file_ptr->file_name) == 0 
		    && prev_input_file_ptr->arch_offset < input_file_ptr->arch_offset )
		{
		    continue;
		}
	   }

	   fclose( input_file_fd );
	   input_file_fd = NULL;
       }
#ifndef MSDOS
    my_free( sect_buffer );
#endif

}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      output_local_symbols                                            *
*                                                                      *
*   Synopsis                                                           *
*      long output_local_symbols( FILE *symbol_fd, FILE *string_fd,    *
*      FILE *input_file_fd, INPUT_FILE *input_file_ptr )               *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for processing the local symbol     *
*   table of every file by relocating and writing the symbols and      *
*   building the 'slot vector' so we can easily get at the relocation  *
*   info when the time comes.                                          *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/13/89       created                  -----             *
***********************************************************************/

long output_local_symbols( FILE *symbol_fd, FILE *string_fd, FILE *input_file_fd, 
			    INPUT_FILE *input_file_ptr )
{
    register LINKER_AUX *auxp;
    AUXENT              *aux;
    AUXENT               aux_buf;
    register long        count;
    int                  i;    
    long                 index;
    struct ar_hdr        memberhdr;
    long                *new_address;
    int                 *new_section_num;
    unsigned             siz_section_num; /*EK* for guard */
    long                 num_symbols_stripped;
    long                *old_address;
    int                  reloc_tag;       /* Needs tag index relocated */
    int                  reloc_end;       /* Needs end index relocated */
    char                *save_ptr;
    int                  secnum;
    long                 slot_value1;
    long                 slot_value2;
    char                *string_table_ptr;
    long                 string_table_size;
    char                 strip_flag;
    SYMENT               sym_buf;
    register LINKER_SYM *sym_ptr;
    register SYMENT     *sym;
    char                *symname;
    char                 local_symname[SYMNMLEN+1];
    char                 tmp_numaux;
    INPUT_SECT          *input_section_ptr;

    sv_high_slot = 0;
    siz_section_num = input_file_ptr->num_sections + 1;
    num_symbols_stripped = 0;
    index = input_file_ptr->symbol_index;

    new_section_num = (int *) my_malloc( (long)(siz_section_num * sizeof(int)) );
    old_address =    (long *) my_malloc( (long)(siz_section_num * sizeof(long)) ); /*EK*/
    new_address =    (long *) my_malloc( (long)(siz_section_num * sizeof(long)) ); /*EK*/
    output_init_arrays( input_file_ptr->head, new_section_num, old_address, new_address );

    if( input_file_ptr->arch_offset )
    {
	if( fseek(input_file_fd, input_file_ptr->arch_offset - (int) (sizeof(struct ar_hdr)),
	    BEGINNING) != OKFSEEK || 
	    fread(&memberhdr, sizeof(struct ar_hdr), 1, input_file_fd) != 1 ||
	    strncmp(memberhdr.ar_fmag, ARFMAG, sizeof(memberhdr.ar_fmag)) )
	{
	    LINKER_ERROR1( "Failed to read archive header of file %s", input_file_ptr->file_name );
	}

	if( sscanf(memberhdr.ar_size, "%ld", &ar_size) != 1 )
	    LINKER_ERROR1( "Invalid archive size for file %s", input_file_ptr->file_name );
    }

    string_table_size = 0L;
    if( input_file_ptr->strings )
	string_table_ptr = input_file_ptr->strings;
    else
	string_table_ptr = read_string_table( input_file_fd, input_file_ptr, &string_table_size);

    if( input_file_ptr->sym_table_ptr <= 0L ||
	fseek(input_file_fd, input_file_ptr->sym_table_ptr + input_file_ptr->arch_offset, BEGINNING)
	      != OKFSEEK )
    {
	my_free( new_section_num );
	my_free( old_address );
	my_free( new_address );
	return( 0 );
    }

    sym = &sym_buf;
    aux = &aux_buf;
    for( count = 0; count < input_file_ptr->num_symbols; count++ )
    {
	reloc_tag = 0;
	reloc_end = 0;

	if( read_symbol(sym, input_file_fd) != 1 )
	    LINKER_ERROR1( "Failed to read symbol table of file %s", input_file_ptr->file_name );

	if( IN_OBJ_STRING_TABLE(sym) )
	{
	    if( string_table_ptr == NULL )
		LINKER_ERROR1( "String table does not exist in file %s", input_file_ptr->file_name );
	    sym->_n._n_p._n_nptr = string_table_ptr + OBJ_SYM_OFFSET( sym );
#ifdef DEBUG
	    printf( "output_local_symbols: string in table = %s\n",  sym->_n._n_p._n_nptr);
	    fflush( stdout );
#endif        
	}

	if( 0L == OBJ_SYM_ZEROES(sym) )
	  symname = sym->_n._n_p._n_nptr;
	else 
	{
	  strncpy( local_symname, sym->_n._n_name, SYMNMLEN);
	  symname = local_symname;
	  local_symname[SYMNMLEN] = '\0';
	}

	switch( sym->n_sclass )
	{
		case C_EXT:
		case C_EXTDEF:
		case C_CRTL:
		   if( NULL == (sym_ptr = findsym( symname )))
		     LINKER_ERROR1( "Symbol not found: %s", symname);
		   secnum = sym->n_scnum;
		   slot_value1 = sym->n_value;
		   slot_value2 = sym_ptr->new_virtual;

		   if( sym_ptr->local_flag && (secnum > 0) )
		   {
		       slotvec_create( count, slot_value1, slot_value2, sym_ptr->sym_index,
				       sym_ptr->obj_sym.n_scnum, 0 );

		       if( !sflag )
		       {
			   slot_value1 = sym_ptr->obj_sym.n_value;
			   sym_ptr->obj_sym.n_value = sym_ptr->new_val;

			   if( IN_OBJ_STRING_TABLE(&sym_ptr->obj_sym) )
			   {
			       save_ptr = sym_ptr->obj_sym._n._n_p._n_nptr;
			       output_write_string( string_fd, &sym_ptr->obj_sym, 
						   sym_ptr->name_length );
			   }

			   write_symbol( &(sym_ptr->obj_sym), symbol_fd );
			   if( IN_OBJ_STRING_TABLE(&sym_ptr->obj_sym) )
			       sym_ptr->obj_sym._n._n_p._n_nptr = save_ptr;

			   sym_ptr->obj_sym.n_value = slot_value1;
			   if( sym_ptr->obj_sym.n_numaux > 0 )
			   {
			       auxp = getaux(sym_ptr->aux_id);
			       if( auxp->aux.x_sym.x_tagndx > 0 )
			       {
				   if( xflag )
				       auxp->aux.x_sym.x_tagndx = 0;
				   else
				       auxp->aux.x_sym.x_tagndx += input_file_ptr->symbol_index -
								  num_symbols_stripped;
			       }

			       if( auxp->aux.x_sym.x_fcnary.x_fcn.x_endndx > 0 )
			       {
				   if( xflag )
				       auxp->aux.x_sym.x_fcnary.x_fcn.x_endndx = index + 2;
				   else
				       auxp->aux.x_sym.x_fcnary.x_fcn.x_endndx +=
				       input_file_ptr->symbol_index - num_symbols_stripped;
			       }
			       write_aux_info( &(auxp->aux), symbol_fd,
						    output_get_aux_class( &(sym_ptr->obj_sym)) );
			   }
		       }
		       if( sym->n_numaux > 0 )
		       {
			   if( fseek(input_file_fd, (long) (sym->n_numaux * AUXESZ), 1) 
			       != OKFSEEK)
			   {
			       LINKER_ERROR1( "Failed to skip a aux entry in file: %s", 
					      input_file_ptr->file_name);
			   }
		       }
		       index += 1 + sym->n_numaux;
		       count += sym->n_numaux;
		   }
		   else
		   {
		       if( (sym_ptr->obj_sym.n_scnum == 0) && aflag )
		       {
			    slotvec_create( count, slot_value1, slot_value2, sym_ptr->sym_index,
					    sym_ptr->obj_sym.n_scnum, SV_ERR );
			    if( !undefined_message )
			    {
				fprintf(stderr, "Undefined\t\t\tFirst referenced\n");
				fprintf(stderr, " Symbol  \t\t\t    in file\n");
				undefined_message = 1;
				error_level = (rflag) ? 1 : 2;
			    }
			    if( !sym_ptr->undefined_msg )
			    {
				fprintf(stderr, "%-35s %s", symname, input_file_ptr->file_name);
				fprintf(stderr, "\n");
				sym_ptr->undefined_msg = 1;
			    }
		       }
		       else
		       {
#ifdef DEBUG
			   printf( "output_local_symbols: %s\n", sym_ptr->obj_sym._n._n_name );
			   fflush( stdout );
#endif
			   if( mflag )
			   {
			       input_section_ptr = list_find_input_section( sym->n_scnum, input_file_ptr);
			       mem_map_add_symbol(sym, sym_ptr->new_val,
						  ((secnum == 0) ? sym_ptr->input_sect : NULL), mem_map,
						  sym_ptr->input_sect->sec_hdr.s_flags & (SECTION_PM | SECTION_DM));
			   }
				
			   slotvec_create( count, slot_value1, slot_value2, sym_ptr->sym_index,
				     sym_ptr->obj_sym.n_scnum,  0 );
		       }


		       for( i = 0; i < sym->n_numaux; count++, i++ )
		       {
			    if( read_aux_info(aux, input_file_fd, 
						   output_get_aux_class(sym)) != 1 )
			    {
				LINKER_ERROR1( "Failed to read auxillary entries of file %s",
					       input_file_ptr->file_name);
			    }

			   if( !sflag && (secnum > 0 || (secnum == 0 && sym->n_value > 0)) )
			   {
			       auxp = getaux(sym_ptr->aux_id);
			       if( (auxp->aux.x_sym.x_tagndx > 0) &&
				   (auxp->aux.x_sym.x_tagndx == aux->x_sym.x_tagndx))
			       {
				    if( xflag )
					auxp->aux.x_sym.x_tagndx = 0;
				    else
					auxp->aux.x_sym.x_tagndx += input_file_ptr->symbol_index;
			       }
			   }
		       }
		   }
		   break;

		case C_STAT:
		case C_HIDDEN:
#ifdef DEBUG
		   printf( "output_local_symbols: %s\n", sym_ptr->obj_sym._n._n_name );
		   fflush( stdout );
#endif
		   if( sym->n_scnum <= 0 )
		       goto wrtsym;

		   if ( sym->n_scnum >= siz_section_num )
			FATAL_ERROR1( "Bad section number from object file, class %d C_STAT/C_HIDDEN.",
							sym->n_sclass ); /*EK*/

		   sym->n_value += (new_address[sym->n_scnum] - old_address[sym->n_scnum]);
		   slot_value1 = old_address[sym->n_scnum];
		   slot_value2 = new_address[sym->n_scnum];

		   if( mflag )
		   {
		       input_section_ptr = list_find_input_section( sym->n_scnum, input_file_ptr);
		       mem_map_add_symbol( sym, sym->n_value, NULL, mem_map,
					  input_section_ptr->sec_hdr.s_flags & (SECTION_PM | SECTION_DM));
		   }

		   slotvec_create( count, slot_value1, slot_value2, index, 
				   new_section_num[sym->n_scnum], 0 );

		   if( IS_FCN(sym->n_type) )
		       reloc_end = 1;

	       case C_FIELD:
	       case C_REGPARM:
		  if( (OBJ_SYM_BASIC_TYPE(sym->n_type) == T_ENUM) || 
		      (OBJ_SYM_BASIC_TYPE(sym->n_type) == T_UNION) ||
		      (OBJ_SYM_BASIC_TYPE(sym->n_type) == T_STRUCT) )
		  {
		       reloc_tag = 1;
		  }
		  goto wrtsym;
	
	       case C_FCN:
	       case C_BLOCK:
	       case C_PROLOG:
		  reloc_end = 1;

	       case C_LABEL:
		  if ( sym->n_scnum >= siz_section_num )
		      FATAL_ERROR1( "Bad section number from object file, class %d C_FCN/C_BLOCK/C_PROLOG/C_LABEL.",
							sym->n_sclass ); /*EK*/

		  sym->n_value += (new_address[sym->n_scnum] - old_address[sym->n_scnum]);
wrtsym:
		  if( sym->n_sclass != C_STAT && sym->n_sclass != C_FILE && xflag )
		  {
		      strip_flag = 1;
		      if( !sflag )
		      {
			  num_symbols_stripped++;
		      }
		  }
		  else
		      strip_flag = 0;

		  if( !sflag && !strip_flag )
		  {
		      /*EK* BUG: I really don't know whether this is a correct fix!? */
		      /*EK* was: if( sym->n_scnum > 0 ) */
		      if( sym->n_scnum > 0 && sym->n_sclass != C_NULL
					   && sym->n_sclass != C_AUTO )
		      {
			  if ( sym->n_scnum >= siz_section_num )
			      FATAL_ERROR1( "Bad section number from object file, class %d.",
							sym->n_sclass ); /*EK*/

			  sym->n_scnum = new_section_num[sym->n_scnum];
		      }

		      if( IN_OBJ_STRING_TABLE(sym) )
			  output_write_string(string_fd, sym, strlen(sym->_n._n_p._n_nptr) + 1);

		      if( 1 )
			  write_symbol( sym, symbol_fd );
		      index++;
		  }

		  tmp_numaux = sym->n_numaux;
		  for( i = 0; i < tmp_numaux; i++ )
		  {
		       if( read_aux_info( aux, input_file_fd, 
					       output_get_aux_class(sym) ) != 1 )
		       {
			   LINKER_ERROR1( "Failed to read aux entries of file %s",
					  input_file_ptr->file_name);
		       }

		       if( !sflag && strip_flag )
			   num_symbols_stripped++;

		       if( !sflag && !strip_flag )
		       {
			   if( reloc_tag && aux->x_sym.x_tagndx > 0 )
			   {
			       if( xflag )
				   aux->x_sym.x_tagndx = 0;
			       else
				   aux->x_sym.x_tagndx += input_file_ptr->symbol_index -
							  num_symbols_stripped;
			   }

			   if( reloc_end && aux->x_sym.x_fcnary.x_fcn.x_endndx > 0 )
			   {
			       if( xflag )
				   aux->x_sym.x_fcnary.x_fcn.x_endndx += index + 1;
			       else
				   aux->x_sym.x_fcnary.x_fcn.x_endndx += input_file_ptr->symbol_index -
								    num_symbols_stripped;
			   }
			   write_aux_info( aux, symbol_fd, 
						output_get_aux_class( sym ));
			   index++;
		       }
		       count++;
		  }
		  break;

	       case C_STRTAG:
	       case C_UNTAG:
	       case C_ENTAG:
		  reloc_end = 1;

	       case C_ARG:
	       case C_AUTO:
	       case C_EOS:
	       case C_MOS:
	       case C_MOU:
	       case C_REG:
	       case C_TPDEF:
		  reloc_tag = 1;
		  goto wrtsym;

	       case C_FILE:
		  if( !sflag && (lastfile != -1L) && (nextfile != index) )
		      output_update_file_entry(symbol_fd, index);
		  if( input_file_ptr->infile_next )
		  {
		      nextfile = input_file_ptr->infile_next->symbol_index;
		      sym->n_value = nextfile;
		  }
		  lastfile = index;

	       default:
		  goto wrtsym;
	}
    }

    my_free( new_section_num );
    my_free( old_address );
    my_free( new_address );

    if( last_string_ptr )
    {
#ifdef MSDOS
	fwrite( last_string_ptr, (size_t) string_buf_length, 1, string_fd );
#else
	fwrite( last_string_ptr, string_buf_length, 1, string_fd );
#endif
	last_string_ptr = NULL;
	string_buf_length = 0;
    }

    if( string_table_ptr && (string_table_size > MAX_STRING_TABLE_SIZE) )
      my_free( string_table_ptr );

    return( index - input_file_ptr->symbol_index );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      output_get_aux_class                                            *
*                                                                      *
*   Description                                                        *
*      Get the auxillary class associated with this symbol.            *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     5/7/90        created                  -----             *
***********************************************************************/

int output_get_aux_class( SYMENT *sym )
{
    if( sym->n_sclass == C_FILE )
	return( AUX_CLASS_FILE_NAME );
    else if( sym->n_sclass == C_STAT && OBJ_SYM_BASIC_TYPE(OBJ_SYM_TYPE(sym)) )
	return( AUX_CLASS_SECTION );
    else
	return( AUX_CLASS_SYM );
	
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      output_global_symbols                                           *
*                                                                      *
*   Synopsis                                                           *
*      void output_global_symbols( FILE *symbol_fd, FILE *string_fd )  *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for writing the global symbols at   *
*  the  end of the output symbol table.                                *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/13/89       created                  -----             *
***********************************************************************/

void output_global_symbols( FILE *symbol_fd, FILE *string_fd )
{
    LINKER_AUX           *aux_ptr;
    register long         index;
    char                 *save_ptr;
    register LINKER_SYM  *sym_ptr;

    index = 1;
    while( index <= total_symbols )
    {
	   sym_ptr = getsym(index++);
	   sym_ptr->obj_sym.n_value = sym_ptr->new_val;

	   if( (sym_ptr->local_flag == 0) && !sflag )
	   {
		/*
		 * Set last .file entry to point to first global
		 * symbol entry
		 */

		if( lastfile != -1L )
		{
		    output_update_file_entry(symbol_fd, num_output_syms );
		    lastfile = -1L;
		}

		if( (sym_ptr->obj_sym.n_scnum == 0) && aflag && (sym_ptr->undefined_msg == 0) )
		{
		     if( !undefined_message )
		     {
			 USER_ERROR( "Undefined symbol" );
			 undefined_message = 1;
			 error_level = rflag ? 1 : 2;
		     }

		     if( sym_ptr->obj_sym.n_zeroes == 0L )
			 USER_ERROR1( "%s\n", sym_ptr->obj_sym._n._n_p._n_nptr);
		     else
			 fprintf(stderr, "%.8s\n", sym_ptr->obj_sym._n._n_name);
		     sym_ptr->undefined_msg = 1;
		}

		/* Is the name in the string table or stored in the symbol ? */

		if( sym_ptr->obj_sym.n_zeroes == 0L )
		{
		    save_ptr = sym_ptr->obj_sym._n._n_p._n_nptr;
		    output_write_string( string_fd, &sym_ptr->obj_sym, sym_ptr->name_length );
		}

		write_symbol( &(sym_ptr->obj_sym), symbol_fd);

		if( sym_ptr->obj_sym.n_zeroes == 0L )
		    sym_ptr->obj_sym._n._n_p._n_nptr= save_ptr;

		num_output_syms++;
		aux_ptr = NULL;
		while( (aux_ptr = find_aux_entry(sym_ptr, aux_ptr, 0)) != NULL )
		{
			if( (OBJ_SYM_BASIC_TYPE(sym_ptr->obj_sym.n_type) == T_STRUCT || 
			     OBJ_SYM_BASIC_TYPE(sym_ptr->obj_sym.n_type) == T_UNION ||
			     OBJ_SYM_BASIC_TYPE(sym_ptr->obj_sym.n_type) == T_ENUM ||
			     sym_ptr->obj_sym.n_sclass == C_EOS) && aux_ptr->aux.x_sym.x_tagndx > 0 )
			{
			     if( xflag )
				 aux_ptr->aux.x_sym.x_tagndx = 0;
			}

			write_aux_info( &(aux_ptr->aux), symbol_fd, 
					     output_get_aux_class( &(sym_ptr->obj_sym)) );
			num_output_syms++;
		}
	   }
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      output_line_numbers                                             *
*                                                                      *
*   Synopsis                                                           *
*      void output_line_numbers( INPUT_SECT *input_sect_ptr,           *
*      OUTPUT_SECT *output_sect_ptr, FILE *input_file_fd,              *
*      FILE *symbol_fd, FILE *output_fd, INPUT_FILE *input_file_ptr )  *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for updating the line number info   *
*   and writing it to the output file.                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/13/89       created                  -----             *
***********************************************************************/

void output_line_numbers( INPUT_SECT *input_sect_ptr, OUTPUT_SECT *output_sect_ptr,
			   FILE *input_file_fd, FILE *symbol_fd, FILE *output_fd, 
			   INPUT_FILE *input_file_ptr )
{
    long                      addr_diff;
    AUXENT                    aux;
    register  unsigned short  i;
    register  LINENO           *line;
    LINENO                      line_buf;
    long                      line_ptr;
    register long             offset;
    long                      old_offset;
    SLOTVEC                  *slot_vec_ptr;

    line = &line_buf;
    addr_diff = input_sect_ptr->paddr - input_sect_ptr->sec_hdr.s_paddr;

    if( fseek(input_file_fd, input_sect_ptr->sec_hdr.s_lnnoptr + input_file_ptr->arch_offset, 
	BEGINNING) != OKFSEEK )
    {
	FATAL_ERROR2( "Seek to %s section %.8s lnno failed", input_file_ptr->file_name, 
		     input_sect_ptr->sec_hdr.s_name);
    }

    old_offset = ftell(symbol_fd);
    offset = output_sect_ptr->sec_hdr.s_lnnoptr + input_sect_ptr->line_disp;
    fseek(output_fd, offset, 0);
    line_ptr = offset;

    for( i = 0; i < input_sect_ptr->sec_hdr.s_nlnno; i++ )
    {
	 if( read_line_info( line, input_file_fd ) != 1 )
	 {
	     LINKER_ERROR2( "Failed to read line numbers of section %.8s of file %s",
			    input_sect_ptr->sec_hdr.s_name, input_file_ptr->file_name);
	 }

	 if( line->l_lnno == 0 )
	 {
	     if( (slot_vec_ptr = slotvec_read(line->l_addr.l_symndx)) == NULL )
	     {
		  FATAL_ERROR4( "Line number entry (%ld %d) found for non-relocatable \
				symbol: section %.8s, file %s", line->l_addr.l_symndx,
				line->l_lnno, input_sect_ptr->sec_hdr.s_name, 
				input_file_ptr->file_name);
		  continue;
	     }

	     line->l_addr.l_symndx = slot_vec_ptr->sv_new_symindex;

/* This was very strange.  The code seemed to work on the sun fine, but did not work */
/* on the PC if the calculation for offset was done in one line.  I'm sure that it   */
/* had something to do with an int/long conversion, but everything was long!  Oh well*/
/* after two full days I don't care why it works, just so long as it does.  GAS      */
	     offset = 1 + line->l_addr.l_symndx;
	     offset *= SYMESZ;
	     offset += ((char *) &aux.x_sym.x_fcnary.x_fcn.x_lnnoptr - (char *) &aux);
	     fseek(symbol_fd, offset, 0);
	     fwrite( &line_ptr, sizeof(long), 1, symbol_fd);
	 }
	 else
	    line->l_addr.l_paddr += addr_diff;

	 write_line_info( line, output_fd );
	 line_ptr += LINESZ;
    }
    fseek(symbol_fd, old_offset, 0);
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      output_update_file_entry                                        *
*                                                                      *
*   Synopsis                                                           *
*      void output_update_file_entry( FILE *symbol_fd, long sym_index )*
*                                                                      *
*   Description                                                        *
*      This routine is responsible for chaining '.file' symbol table   *
*   entries in a list. It does this by writing the symbol table index  *
*   of the next '.file' entry in the value field of the previous       *
*   '.file' entry.                                                     *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/13/89       created                  -----             *
***********************************************************************/

void output_update_file_entry( FILE *symbol_fd, long sym_index )
{
    register long  here;   /* where we are now */
    register long  there;  /* where to write update value */

    here = symbol_table_origin + sym_index * (long) SYMESZ;
    there = symbol_table_origin + (lastfile * (long) SYMESZ) + (long) SYMNMLEN;
    fseek( symbol_fd, there - here, 1 );
    fwrite( &sym_index, sizeof(long), 1, symbol_fd );
    fseek( symbol_fd, here - there - sizeof(long), 1 );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      void output_write_string                                        *
*                                                                      *
*   Synopsis                                                           *
*      void output_write_string( FILE *string_fd, SYMENT *sym,        *
*                                int string_length )                   *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for writing a string to the output  *
*   string table.                                                      *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/13/89       created                  -----             *
***********************************************************************/

void output_write_string( FILE *string_fd, register SYMENT *sym, register int string_length )
{
    if( (last_string_ptr + string_buf_length) == sym->_n._n_p._n_nptr)
	 string_buf_length += string_length;
    else
    {
#ifdef MSDOS
	 if( last_string_ptr && fwrite(last_string_ptr, (size_t) string_buf_length, 1, string_fd) != 1 )
#else
	 if( last_string_ptr && fwrite(last_string_ptr, string_buf_length, 1, string_fd) != 1 )
#endif
	 {
	     LINKER_ERROR2( "Error writing symbol name %s in string table for file %s",
			    last_string_ptr, output_file_name);
	 }
	 last_string_ptr = sym->_n._n_p._n_nptr;
	 string_buf_length = string_length;
    }

    OBJ_SYM_OFFSET(sym) = string_offset;
    string_offset += string_length;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      output_section                                                  *
*                                                                      *
*   Synopsis                                                           *
*      void output_section( FILE *input_file_fd,                       *
*                           INFILE *input_file_ptr,                    *
*                           INSECT *input_sect_ptr,                    *
*                           OUTSECT *output_sect_ptr, FILE *output_fd, *
*                           char *sect_buffer, long buffer_size )      *
*                                                                      *
*   Description                                                        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/13/89       created                  -----             *
***********************************************************************/

void output_section( register FILE *input_file_fd, INPUT_FILE *input_file_ptr, 
		     INPUT_SECT *input_sect_ptr,
		     OUTPUT_SECT *output_sect_ptr, register FILE *output_fd, char *sect_buffer,
		     long buffer_size )
{
    register long  more;
    register long  num_bytes;

    output_sect_ptr;
    more = input_sect_ptr->sec_hdr.s_size;
    while( more )
    {
	   num_bytes = min(buffer_size, more);

	   if( fread(sect_buffer, (int) num_bytes, 1, input_file_fd) != 1 || 
	       fwrite(sect_buffer, (int) num_bytes, 1, output_fd) != 1 )
	   {
	       LINKER_ERROR2( "Cannot copy section %.8s of file %s", 
			      input_sect_ptr->sec_hdr.s_name, input_file_ptr->file_name );
	   }
	   more -= num_bytes;
    }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      output_init_arrays                                              *
*                                                                      *
*   Synopsis                                                           *
*      void output_init_arrays( INPUT_SECT *input_sect_ptr,            *
*      int *new_section_num, long *old_address, long *new_address )    *
*                                                                      *
*   Description                                                        *
*      This routine sets up some section info in a manner that is more *
*   easily dealt with.                                                 *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/13/89       created                  -----             *
***********************************************************************/

void output_init_arrays( INPUT_SECT *input_sect_ptr, int *new_section_num, 
			 long *old_address, long *new_address )
{
    register int i;

    for( i = 1; input_sect_ptr; input_sect_ptr = input_sect_ptr->next, i++ )
    {
	 new_section_num[i] = input_sect_ptr->out_sec->section_num;
	 old_address[i] = input_sect_ptr->sec_hdr.s_vaddr;
	 new_address[i] = input_sect_ptr->vaddr;
	
    }
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      output_file_header                                              *
*                                                                      *
*   Synopsis                                                           *
*      void output_file_header( OBJ_HDR *output_file_header,           *
*                               FILE *output_fd )                      *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for calculating the info for, and   *
*   writing the COFF file header.                                      *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/13/89       created                  -----             *
***********************************************************************/

void output_file_header( FILHDR *file_header, register FILE *output_fd )
{
    file_header->f_magic = M_21000;
    file_header->f_flags = 0;

    if( sflag )
    {
	file_header->f_symptr = 0L;
	file_header->f_nsyms = 0L;
	file_header->f_flags |= (OBJ_LINE | OBJ_LSYMS);
    }
    else if( xflag )
    {
	file_header->f_symptr = symbol_table_origin;
	file_header->f_nsyms = num_output_syms;
	file_header->f_flags |= (OBJ_LINE | OBJ_LSYMS);
    }
    else
    {
	file_header->f_symptr = symbol_table_origin;
	file_header->f_nsyms = num_output_syms;
    }

    if (!rflag)
	file_header->f_flags |= OBJ_RELOC;

    if( !check_if_errors() && aflag )
	file_header->f_flags |= OBJ_EXEC;

    if( !check_if_errors() )
	file_header->f_flags |= Input_file_types;

    /* Roll in the stack model, calling protocol, floating-point representation
       and whether register parameters are being used */
    file_header->f_flags |= Pmstack;
    file_header->f_flags |= Crts;
    file_header->f_flags |= Dubs;
    file_header->f_flags |= Regp;
	

    file_header->f_nscns = (short)num_output_sects;
    file_header->f_timdat = (long) time( 0 );
    file_header->f_opthdr = 0L;
    fseek( output_fd, 0L, 0 );
    write_file_header( file_header, output_fd );
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      output_finish_up                                                *
*                                                                      *
*   Synopsis                                                           *
*      void output_finish_up( FILE *symbol_fd, FILE *output_fd,        *
*                             FILE *string_fd )                        *
*                                                                      *
*   Description                                                        *
*      This routine writes the size eof the string table, outputs the  *
*   file header, and closes a bunch of files.                          *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/13/89       created                  -----             *
***********************************************************************/

void output_finish_up( FILE *symbol_fd, FILE *output_fd, FILE *string_fd )
{
    FILHDR file_header;
    long       string_table_origin;

    if( last_string_ptr )
#ifdef MSDOS
	fwrite(last_string_ptr, (size_t) string_buf_length, 1, string_fd);
#else
	fwrite(last_string_ptr, string_buf_length, 1, string_fd);
#endif

     if( fwrite( &string_offset, sizeof(long), 1, symbol_fd) != 1 )
	 FATAL_ERROR( "Failed to write size of string table");  

    output_file_header( &file_header, output_fd );

    /* Unless the strip flag has been specified, copy the string table and
     * the symbol table temp files to the final object file.
     */
	
    if( !sflag )
    {
	fseek( output_fd, symbol_table_origin, 0);
	fflush( symbol_fd );

	if( ferror(symbol_fd) )
	    LINKER_ERROR1( "I/O error on temporary symbol file %s", temp_file[sym_temp_index]  );

	fclose( symbol_fd );
	copy_section( temp_file[sym_temp_index], output_fd );
	
	string_table_origin = symbol_table_origin + SYMESZ * total_output_symbols + 4;
	fseek( output_fd, string_table_origin, 0);
	fflush( string_fd );

	if( ferror(string_fd) )
	    LINKER_ERROR1( "I/O error on string table file %s", temp_file[string_temp_index] );

	fclose( string_fd );
	copy_section( temp_file[string_temp_index], output_fd );
    }

    if( ferror(output_fd) )
	LINKER_ERROR1( "I/O error on output file %s", output_file_name );

    fclose( output_fd );
}

