/* @(#)object.c 1.9 2/21/91 1 */

#include "app.h"
#include "a_out.h"
#include "error.h"
#include "pass2.h"
#include "symbol.h"
#include "list.h"
#include "codegen.h"
#include "action.h"
#include "str.h"

#include "internal_reloc.h"
#include "reloc_fp.h"
#include "section_fp.h"
#include "object_fp.h"
#include "coff_io.h"
#include "time.h"
#include "main.h"

static char buffer[BUFSIZ];
static FILHDR header;
static AOUTHDR Optional_header;


long num_syms = 0;
long num_glob_def_syms = 0;
long num_stat_def_syms = 0;
long sect_line_ptr[MAX_SECTIONS + 1];
int Section_flag = FALSE;

/* This function is provided because of the list_get_index() routine   */
/* that needs to be called in a strange way.  This function is only    */
/* used in the file, and it basically searches through the entire list */
/* even though the list was designed to be used for a particular types */
/* of searches.                                                        */
/* The reason that this is a seperate function is because the compiler */
/* was generating some bad code when you tried to AND together a bunch */
/* of list searches.                                  gas 1/28/93      */

static long get_any_list_index(char *symbol_name)
{
    long int symbol_index;

    if( (symbol_index = list_get_index(symbol_name, (short)C_EXT)) >= 0L )
	return symbol_index;
        
    if( (symbol_index = list_get_index(symbol_name, (short)C_STAT)) >= 0L )
	return symbol_index;

    if( (symbol_index = list_get_index(symbol_name, (short)C_ENTAG)) >= 0L )
	return symbol_index;

    if( (symbol_index = list_get_index(symbol_name, (short)C_UNTAG)) >= 0L )
	return symbol_index;

    if( (symbol_index = list_get_index(symbol_name, (short)C_STRTAG)) >= 0L )
	return symbol_index;

    return -1L;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      object_headers                                                  *
*                                                                      *
*   Synopsis                                                           *
*      void object_headers()                                           *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for writing the object header and   *
*   section headers to the object file.                                *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     2/28/89       created                  -----             *
***********************************************************************/

void object_headers( void )
{
    long address;
    register short i;
    long line;
    long line_ptr;
    long rel;
    long relocation_ptr;
    register SCNHDR *sec_hdr_ptr;
    long section_ptr;
    long size;
    long temp_ptr;

    temp_ptr = ftell( obj_fd );
    fseek( obj_fd, header_ptr, 0 );
    line = rel = size = 0;

    for( i = 1, sec_hdr_ptr = &section_header[1]; i <= (short) section_cnt; 
	 ++i,++sec_hdr_ptr )
    {
	 line += sec_hdr_ptr->s_nlnno;
	 rel  += sec_hdr_ptr->s_nreloc;
	 size += sec_hdr_ptr->s_size;
    }

    address = 0;
    section_ptr = FILHSZ + AOUTSZ + section_cnt * SCNHSZ;
    relocation_ptr = section_ptr + size;
    line_ptr = relocation_ptr + (long)((long)rel * (long)RELOC_SIZE);
    header.f_magic = M_21000;
    header.f_nscns = (unsigned short)section_cnt;
    header.f_symptr = line_ptr + line * LINESZ;
    header.f_nsyms = num_syms;
    time( &(header.f_timdat) );
    header.f_opthdr = AOUTSZ;
    header.f_flags = 0;

    if( Gcc_compiled )
	header.f_flags |= F_GCC;

    if( Ansi_c_code )
	header.f_flags |= F_ANSIC;

    if( Dsp_c_code )
	header.f_flags |= F_DSPC;

    if( !(Ansi_c_code | Dsp_c_code) )
	header.f_flags |= F_ASM;

    if( Pmstack )       /*C runtime stack located in PM*/
	header.f_flags |= F_RTS;

    if( Crts )          /*Return address stored on external C stack*/
	header.f_flags |= F_PCS;

    if( True_doubles )  /*Doubles are 64-bit quantities*/
	header.f_flags |= F_DAD;

    write_file_header( &header, obj_fd );

    Optional_header.magic = 0413;
    Optional_header.vstamp = 0;         /*Not Implemented yet*/
    Optional_header.tsize = 0;          /*Set by the linker*/
    Optional_header.dsize = 0;          /*Set by the linker*/
    Optional_header.bsize = 0;          /*Set by the linker*/
    Optional_header.entry = 0;          /*Set by the linker*/
    Optional_header.text_start = 0;     /*Set by the linker*/
    Optional_header.data_start = 0;     /*Set by the linker*/
        
    write_optional_header( &Optional_header, obj_fd );

    /* We have written the object file header, now lets calculate
     * info for the section headers and write them.
     */

    for( i = 1, sec_hdr_ptr = &section_header[1]; i <= (short) section_cnt; ++i, ++sec_hdr_ptr )
    {
	 sec_hdr_ptr->s_paddr = sec_hdr_ptr->s_vaddr = address;

	 if( sec_hdr_ptr->s_nreloc )
	     sec_hdr_ptr->s_relptr = relocation_ptr;

	 if( sec_hdr_ptr->s_nlnno )
	     sec_hdr_ptr->s_lnnoptr = sect_line_ptr[i] = line_ptr;

	 if( sec_hdr_ptr->s_size )
	     sec_hdr_ptr->s_scnptr = section_ptr;

	 section_ptr += sec_hdr_ptr->s_size;

	 if( sec_hdr_ptr->s_size )
	 {
	   address += sec_hdr_ptr->s_size
	     / ((sec_hdr_ptr->s_flags & SECTION_PM) == SECTION_PM
		? PM_WORD_SIZE : DM_WORD_SIZE);
	 }

	 relocation_ptr += (long)((long)sec_hdr_ptr->s_nreloc * (long)RELOC_SIZE);
	 line_ptr += sec_hdr_ptr->s_nlnno * LINESZ;

	 write_section_header( sec_hdr_ptr, obj_fd );
    }
    fseek( obj_fd, temp_ptr, 0 );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      copy_section                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void copy_section(file)                                         *
*      char *file;                                                     *
*                                                                      *
*   Description                                                        *
*      Copy a section from a temp file to the object file.             *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/2/89        created                  -----             *
***********************************************************************/

void copy_section( char *file )
{
    register short num;
    register FILE *fd;

    if( (fd = fopen(file, READ_BINARY)) == NULL )
	 FATAL_ERROR("Error opening temporary file");

    do
    {
	num = FREAD( buffer, sizeof(*buffer), BUFSIZ, fd );
	FWRITE( buffer, sizeof(*buffer), num, obj_fd );
    } while( num == BUFSIZ );

    fclose( fd );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      write_all_relocation_info                                       *
*                                                                      *
*   Synopsis                                                           *
*      void write_all_relocation_info(num_reloc)                       *
*      long num_reloc;                                                 *
*                                                                      *
*   Description                                                        *
*      Write the relocation information to the object file.            *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/2/89        created                  -----             *
***********************************************************************/

void write_all_relocation_info( long num_reloc )
{
  INTERNAL_RELOC    irel;
  RELOC             rel;
  register long     symbol_index;
  register char    *string;
  short             i;
      
      
  union 
  {
    long offsets[2];
    char dummy[8];
  } kluge;
  char              symname[SYMNMLEN+1];

      
  symname[SYMNMLEN] = '\0';
  while( num_reloc-- > 0L )
  {
      read_internal_relocation_info( &irel, rel_fd );
      strncpy( symname, irel.relname.r_name, SYMNMLEN);
	
	
      /* a null name means it's in the string table */
      if( ! ( *irel.relname.r_name ) )
      {
	      
	  /* When a string is stored in the string table, the first long word
	   * is 0, and the second long word contains the offset into the
	   * the string table.
	   */
	      
	  for( i = 0; i < 8; ++i )
	      kluge.dummy[i] = irel.relname.r_name[i]; 
	      
	  string = &string_table[kluge.offsets[1]];
	      
	  if( (symbol_index = get_any_list_index(string)) < 0L )
	  {
	      FATAL_ERROR1("Reference to relocatable symbol \"%s\"not in symbol table.", string);
	  }
	      
      }
      else
      {
	  if( (symbol_index = get_any_list_index(symname)) < 0L )
	  {
	      FATAL_ERROR1("Reference to relocatable symbol \"%s\"not in symbol table.", symname);
	  }
      }
	
      rel.r_vaddr = irel.r_address;
      rel.r_symndx = symbol_index;
      rel.r_type = (unsigned short)irel.r_type;
	
      write_relocation_info( &rel, obj_fd );
  }
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      make_one_up                                                     *
*                                                                      *
*   Synopsis                                                           *
*      void make_one_up(symbol, value, section, sclass, mem_type)      *
*      SYMBOL *symbol;                                                 *
*      long value;                                                     *
*      short section;                                                  *
*      short sclass;                                                   *
*      short mem_type;                                                 *
*                                                                      *
*   Description                                                        *
*      This routine makes up a symbol entry when no symbolic debugging *
*   info has been supplied. The reason we do this is because the symbol*
*   kept by the assembler is much different than the symbol table as   *
*   specified in the COFF spec. In general the output symbol table     *
*   needs much more info for things like symbolic debugging.           *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/2/89        created                  -----             *
***********************************************************************/

void make_one_up( register SYMBOL *symbol, long value, short section, short sclass,
		  long mem_type )
{
    register short i;
    SYMENT obj_sym;

    if( IN_STRING_TABLE(symbol) )
    {
	obj_sym.n_zeroes = 0L;
	obj_sym.n_offset = symbol->what.strtab_entry.offset;
    }
    else
    {
	for( i = 0; (i < SYMNMLEN) && (obj_sym._n._n_name[i] = symbol->what.name[i]); ++i )
	     ;

	for( ; i < SYMNMLEN; ++i )
	     obj_sym._n._n_name[i] = '\0';
    }

    if( strcmp(symbol->what.name, ".") )
    {
	obj_sym.n_value = value;
	obj_sym.n_scnum = section;
	obj_sym.n_type = 0;
	obj_sym.n_sclass = (char) sclass;
	obj_sym.n_numaux = 0;
	write_symbol( &obj_sym, sym_fd );
	list_insert_index( symbol, sclass, num_syms );
	++num_syms;
    }
mem_type;
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      make_symbol_entry                                               *
*                                                                      *
*   Synopsis                                                           *
*      void make_symbol_entry(symbol)                                  *
*      SYMBOL *symbol;                                                 *
*                                                                      *
*   Description                                                        *
*      This routine is passed as an argument to a routine to go through*
*   the assembler symbol table one element at a time, the routine then *
*   checks to see whether a object symbol entry exists (this would be) *
*   true only if debug info were produced by the compiler), and if not *
*   makes an entry for the symbol.                                     *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/8/89        created                  -----             *
***********************************************************************/

void make_symbol_entry( register SYMBOL *symbol )
{
    register char *string;
    register short type;

    if (symbol->Debug_symbol && !IS_EXTERN(symbol->type) && !IS_GLOBAL(symbol->type) )
	return;

    fflush( stdout );
    if( IN_STRING_TABLE(symbol) )
	string = &string_table[OFFSET(symbol)];
    else
	string = NAME(symbol);

    type = (short) symbol->type;


    if( ! ( IS_GLOBAL(type) || IS_STATIC(type) || symbol->num_relocs ))
      return;

    if( ( ! IS_GLOBAL(type) && ! IS_EXTERN(type) ) && IS_DEFINED(type) )
    {
      if( list_get_index(string, C_STAT) < 0 )
	make_one_up( symbol, symbol->value, (short) symbol->section_num, C_STAT,
		    (long) section_header[symbol->section_num].s_flags
		    & (SECTION_PM | SECTION_DM));
	return;
    }
    else if( ( IS_GLOBAL(type) || IS_EXTERN(type) ) && IS_DEFINED(type))
    {
      if( list_get_index(string, C_EXT) < 0 )
	make_one_up( symbol, symbol->value, (short) symbol->section_num, C_EXT,
		    (long ) section_header[symbol->section_num].s_flags 
		    & (SECTION_PM | SECTION_DM));
	return;
    }
    else if( ( IS_GLOBAL(type) || IS_EXTERN(type) ) && ! IS_DEFINED(type) )
    {
	if( list_get_index(string, C_EXT) < 0 )
	    make_one_up( symbol, symbol->value, (short)0, (short)C_EXT, (long)0 );
	return;
    }
}

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      dump_symbols                                                    *
*                                                                      *
*   Synopsis                                                           *
*      void dump_symbols()                                             *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for creating the object file symbol *
*   table.                                                             *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     3/8/89        created                  -----             *
***********************************************************************/

void dump_symbols( void )
{
    AUXENT auxent;
    register short i;
    CODE null_code;
    CODE null_code1;
    SYMENT  *obj_sym;
    register SYMBOL *symbol;
    char *symname;
    SYMENT  temp_obj_sym;
    register SCNHDR *sect_hdr_ptr;
    union
    {
	long l[3];
	char c[12];
    } kludge;
    char    terminated_symname[SYMNMLEN+1];
    short   write_flag;

    /* let's put the section names in the object symbol table */

    terminated_symname[SYMNMLEN] = '\0';
    memset( (char *) &null_code, 0, sizeof(CODE) );
    memset( (char *) &null_code1, 0, sizeof(CODE) );
    null_code1.code_value.value = C_STAT;

    Section_flag = TRUE;
    for( i = 1, sect_hdr_ptr = &section_header[1]; i <= (short) section_cnt; ++i, ++sect_hdr_ptr )
    {
	 strncpy( terminated_symname, sect_hdr_ptr->s_name, SYMNMLEN);
	 symbol = symbol_lookup( terminated_symname );

	 if( symbol == (SYMBOL *) NULL )
	     FATAL_ERROR("Section name not found in symbol table");

	 null_code.code_value.value = 0;
	 define_symbol( symbol, &null_code );
	 define_symbol_value( symbol, &null_code );
	 define_symbol_sclass( (SYMBOL *) NULL, &null_code1 );
	 define_section_auxent( sect_hdr_ptr->s_size, (long) sect_hdr_ptr->s_nreloc, 
			       (long)sect_hdr_ptr->s_nlnno );
	 end_symbol_define( symbol, &null_code );
    }
    Section_flag = FALSE;

    /* Now place the static symbols after sections and before symbols */

    for( i = 0; i < (short) num_stat_def_syms; i++ )
    {
	 read_symbol( &temp_obj_sym, stat_sym_fd );
     
	 obj_sym = &temp_obj_sym;
	 if( !IN_OBJ_STRING_TABLE(obj_sym) )
	 {
	     kludge.l[0] = OBJ_SYM_ZEROES(obj_sym);
	     kludge.l[1] = OBJ_SYM_OFFSET(obj_sym);
	     kludge.c[8] = '\0';
	     symname = kludge.c;
	 }
	 else
	     symname = &string_table[OBJ_SYM_OFFSET(obj_sym)];
     
	 symbol = symbol_lookup(symname);
	 if( symbol == NULL )
	     FATAL_ERROR("dump_symbol: Symbol not found in symbol table.");
     
	 write_flag = 1;
	     
	 write_symbol( &temp_obj_sym, sym_fd );
	 list_insert_index( symbol, OBJ_SYM_SCLASS(obj_sym), num_syms );
	 ++num_syms;
	     
	 if( OBJ_SYM_NUMAUX(obj_sym) > 0 )
	 {
	     read_aux_info( &auxent, stat_sym_fd, get_aux_class(symbol) );
     
	     if( write_flag )
	     {
	       write_aux_info( &auxent, sym_fd, get_aux_class(symbol) );
	       ++num_syms;
	       ++i;
	     }
	 }     
    }
     

    /* now let's go through the global defined symbols and write them to the symbol table
     * file, so that global symbols follow static symbols ala COFF spec.
     */

    for( i = 0; i < (short) num_glob_def_syms; i++ )
    {
	 read_symbol( &temp_obj_sym, glob_sym_fd );

	 obj_sym = &temp_obj_sym;    
	 if( !IN_OBJ_STRING_TABLE(obj_sym) )
	 {
	     kludge.l[0] = OBJ_SYM_ZEROES(obj_sym);
	     kludge.l[1] = OBJ_SYM_OFFSET(obj_sym);
	     kludge.c[8] = '\0';
	     symname = kludge.c;
	 }
	 else
	     symname = &string_table[OBJ_SYM_OFFSET(obj_sym)];

	 symbol = symbol_lookup(symname);
	 if( symbol == NULL )
	     FATAL_ERROR("dump_symbol: Symbol not found in symbol table.");

	 if( ! ( IS_EXTERN(symbol->type) && ! symbol->num_relocs ) )
	 {
	   write_flag = 1;
	     
	   write_symbol( &temp_obj_sym, sym_fd );
	   list_insert_index( symbol, OBJ_SYM_SCLASS(obj_sym), num_syms );
	   ++num_syms;
	 }
	 else
	   write_flag = 0;
	     
	 if( OBJ_SYM_NUMAUX(obj_sym) > 0 )
	 {
	     read_aux_info( &auxent, glob_sym_fd, get_aux_class(symbol) );

	     if( write_flag )
	     {
	       write_aux_info( &auxent, sym_fd, get_aux_class(symbol) );
	       ++num_syms;
	       ++i;
	     }
	 }
    }

    /* Go through the assembler symbol table one element at a time and
     * make sure that an entry appears in the object symbol table.
     * This will have to be done if no debug info is output by the compiler.
     */

    symbol_traverse( make_symbol_entry );
}

