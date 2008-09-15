/* @(#)read_object.c    2.8 7/13/93 2 */

#include <stdio.h>
#include <stddef.h>

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

#include "error_fp.h"
#include "read_object_fp.h"
#include "syms_fp.h"
#include "list_fp.h"

extern void create_c_object( void );

static long ar_symtab[TABLE_SIZE];          /* Archive symbol offset table */
static char *ar_symname_ptr[TABLE_SIZE];    /* Archive symbol string table */
static long sym_count = 0l;                 /* Number of symbols in archive */
static SYMENT    symbuf[MAXSYMS];

long   ar_size = 0l;
LIST   input_file_list;
long   max_section_size = 0;;
short  Pmstack = 0, Crts = 0, Dubs = 0, Regp = 0, g21k_flag = 0; 
short  First_c_file = TRUE, Input_file_types = 0;
char   First_c_file_name[MAX_FILENAME_LENGTH];



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      process_object_file                                             *
*                                                                      *
*   Synopsis                                                           *
*      void process_object_file( char *filename, FILE *fd,             *
*                                unsigned short file_type )            *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for processing a object file or a   *
*   library (archive) file.                                            *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     5/26/89       created                  -----             *
***********************************************************************/

void process_object_file(char *filename, FILE * fd, long file_type)
{
    AR_HDR                arheader;
    char                  found_one;
    register int          i;
    char                 *j;
    LDFILE                ldbuf;
    register LDFILE      *ldptr;
    AR_HDR                memberhdr;
    long                   string_table_size;
    char                 *string_table_ptr;
    long                  sym_ar_size;
    register LINKER_SYM  *symptr;

    /*
     * Process a NON-ARCHIVE file exactly ONCE; but, process an ARCHIVE file
     * each time it is given
     */

    if( (file_type != ARCHIVE_TYPE) && list_find_input_file(filename) )
	 return;

    ldptr = &ldbuf;
    IOPTR(ldptr) = fd;
    if( file_type == ARCHIVE_TYPE )
    {
	TYPE(ldptr) = (unsigned short) file_type;
	FILE_OFFSET(ldptr) = 0L;
	FSEEK(ldptr, 0L, 0);
    }
    else
    {
	TYPE(ldptr) = (unsigned short) file_type;
	FILE_OFFSET(ldptr) = 0L;
	FSEEK( ldptr, 0L, 0 );

	if( read_file_header( &HEADER(ldptr), IOPTR(ldptr) ) != 1 )
	    LINKER_ERROR1("Cannot read file header of file: %s", filename);
    }

    if( ISCOFF(TYPE(ldptr)) )
    {
	load_object_file(ldptr, filename, -1, NULL);
	fclose( IOPTR(ldptr) );
    }
    else if( TYPE(ldptr) == ARCHIVE_TYPE )
    {
	/*
	 * Read in the archive symbol table and search it for
	 * unresolved symbol references.  Keep passing over the
	 * symbol table, loading in archive members as you go until
	 * one complete pass is made with no additional loads of an
	 * archive member.
	 */

	if( fseek(IOPTR(ldptr), (long) SARMAG, 0) != OKFSEEK
	    || FREAD(&arheader, sizeof(struct ar_hdr), 1, ldptr) != 1
	    || strncmp(arheader.ar_fmag, ARFMAG, sizeof(arheader.ar_fmag))
	    || sscanf(arheader.ar_size, "%ld", &sym_ar_size) != 1 )
	{
	    LINKER_ERROR1( "Can't read archive header from %s", filename);
	}

	if( arheader.ar_name[0] != '/' )
	    LINKER_ERROR1( "Archive symbol directory is missing from archive %s\n", filename);

	/* figure out number of archive symbols */

	if( FREAD(&sym_count, sizeof(long), 1, ldptr) != 1 )
	    LINKER_ERROR1( "Can't read archive symbol directory number of symbols from archive %s",
			filename);

	if( sym_count <= 0 )
	    LINKER_ERROR1( "Archive symbol table is empty in archive %s\n", filename );

	if( sym_count > TABLE_SIZE )
	    LINKER_ERROR1( "Archive symbol directory in archive %s is too large", filename);

	/* Read in the archive symbol directory */

	if( (long) FREAD(ar_symtab, sizeof(long), sym_count, ldptr) != sym_count )
	    LINKER_ERROR1( "Can't read archive symbol directory of archive %s", filename );

	/* read in the symbol names string table */

	string_table_size = sym_ar_size - (sym_count + 1) * sizeof(long);
	string_table_ptr = my_malloc( (long) string_table_size );

	if( (FREAD(string_table_ptr, string_table_size, 1, ldptr) != 1) || 
	     string_table_ptr[string_table_size - 1] != '\0' )
	{
	    LINKER_ERROR1( "Can't read archive string table of archive %s", filename);
	}

	j = string_table_ptr;
	for( i = 0; i < (int) sym_count; i++ )
	{
	     if( j > (string_table_ptr + string_table_size - 1) )
		LINKER_ERROR1( "Too few symbol names in string table for archive %s", filename);

	    ar_symname_ptr[i] = j;
	    while( *++j != '\0' )
		   ;
	    j++;
	}

	/*
	 * Go thru the symbol table until you can't find any more
	 * unresolved symbols which are satisfied by this library
	 */

	found_one = 1;
	while( found_one && unresolved )
	{
	       found_one = 0;
	       for( i = 0; i < (int) sym_count; i++ )
	       {
		    symptr = findsym( ar_symname_ptr[i] );
		    if( symptr != NULL )                  
			if( OBJ_SYM_VALUE(&(symptr->obj_sym)) == 0 && 
			    OBJ_SYM_SECT(&(symptr->obj_sym)) == 0 )
			{
			    /*
			     * seek to the archive location
			     */

			    FILE_OFFSET(ldptr) = ar_symtab[i] + sizeof(struct ar_hdr);

			    /*
			     * read this member's archive header
			     */

			    if( FSEEK(ldptr, -(int) (sizeof(struct ar_hdr)), BEGINNING) != OKFSEEK ||
				FREAD(&memberhdr, sizeof(struct ar_hdr), 1, ldptr) != 1 ||
				strncmp(memberhdr.ar_fmag, ARFMAG, sizeof(memberhdr.ar_fmag)) )
			    {
				LINKER_ERROR1( "Can't read archive header of archive %s", filename);
			    }

			    if( sscanf(memberhdr.ar_size, "%ld", &ar_size) != 1 )
				LINKER_ERROR1( "Invalid archive size for file %s", filename);

			    /* read this member's object file header */

			    if( read_file_header( &HEADER(ldptr), 
				IOPTR(ldptr) ) != 1 )
			    {
				LINKER_ERROR1( "Can't read file header of archive %s", filename);                               
			    }


			    if( ISCOFF(HEADER(ldptr).f_magic) )
			    {
				/* Bind this member */

				load_object_file(ldptr, filename, -1, NULL);
				found_one = 1;
			    }
			}
	       }
	}

	fclose( IOPTR(ldptr) );
    }
    else
	LINKER_ERROR2( "Illegal magic number %x in file %s.\n", TYPE(ldptr), filename );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
       load_object_file                                                *
*                                                                      *
*   Synopsis                                                           *
*      load_object_file( LDFILE *fd, char *file_name, int file_index,  *
*                        SYMENT *symbuf )                             *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for reading in the file and section *
*   headers, the symbol table, and if one exists, ths string table.    *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     5/26/89       created                  -----             *
***********************************************************************/

void load_object_file( LDFILE *fd, char *file_name, int file_index, SYMENT *symbuf )
{
    char                *string_table_ptr;
    long                 string_table_size;
    register int         s;
    register INPUT_SECT *sec_ptr;
    register INPUT_FILE *file_ptr;
    ACTION              *p;

    file_ptr = (INPUT_FILE *) my_calloc((long)sizeof(INPUT_FILE));

    file_ptr->file_name = file_name;
    file_ptr->file_index = file_index;
    file_ptr->arch_offset = FILE_OFFSET(fd);
    file_ptr->sym_table_ptr = HEADER(fd).f_symptr;
    file_ptr->num_sections = (short) HEADER(fd).f_nscns;
    file_ptr->num_symbols  = HEADER(fd).f_nsyms;

    Input_file_types |=
      (HEADER(fd).f_flags & (F_ASM | F_ANSIC | F_DSPC | F_GCC));

    if((HEADER(fd).f_flags & F_DSPC)
       || (HEADER(fd).f_flags & F_ANSIC)
       || (HEADER(fd).f_flags & F_GCC))
      {
	if(First_c_file)
	  {
	    create_c_object();
	    Pmstack = HEADER(fd).f_flags & F_RTS;
	    Crts = HEADER(fd).f_flags & F_PCS;
	    Dubs = HEADER(fd).f_flags & F_DAD;
	    Regp = HEADER(fd).f_flags & F_REGPARMS;
	    g21k_flag = HEADER(fd).f_flags & F_GCC;
	    First_c_file = FALSE;
	    strcpy(First_c_file_name, file_name);
	  }
	
	if (Pmstack != (HEADER(fd).f_flags & F_RTS))
	  FATAL_ERROR2( "\"%s\" and \"%s\" compiled with different stack models\n",
		       First_c_file_name, file_name );
	
	if (Crts != (HEADER(fd).f_flags & F_PCS ))
	  FATAL_ERROR2( "\"%s\" and \"%s\" compiled with different call/return models\n",
		       First_c_file_name, file_name );
	
	if (Dubs != (HEADER(fd).f_flags & F_DAD ))
	  FATAL_ERROR2( "\"%s\" and \"%s\" compiled with different sizes for type double\n",
		       First_c_file_name, file_name );
	
	/* Hay sombody, permote my ass to FATAL someday when you have the balls! matt */
	if (Regp != (HEADER(fd).f_flags & F_REGPARMS ))
	  USER_WARN2( "\"%s\" and \"%s\" compiled with different parameter passing model\n",
		       First_c_file_name, file_name );

	/* Warn if different compilers are evidenced */
	if (g21k_flag != HEADER(fd).f_flags & F_GCC)
	  USER_WARN2( "\"%s\" and \"%s\" compiled by different compilers\n",
		       First_c_file_name, file_name );
      }

      
    if( FSEEK(fd, (long) (FILHSZ + HEADER(fd).f_opthdr), BEGINNING) != OKFSEEK )
	LINKER_ERROR1( "Failed to seek to section headers of file %s", file_name );

    for( s = 1; s <= (int) HEADER(fd).f_nscns; s++ )
    {
	sec_ptr = (INPUT_SECT *) my_calloc((long)sizeof(INPUT_SECT));

	if( read_section_header(&sec_ptr->sec_hdr, IOPTR(fd) ) != 1 )
	    LINKER_ERROR1( "Error reading section headers of file %s", file_name);

	if( !LEGAL_SECTION_NAME(sec_ptr->sec_hdr.s_name) )
	{
	    LINKER_ERROR2( "File %s has a section name which is a reserved ld identifier: %.8s",
			  file_name, sec_ptr->sec_hdr.s_name);
	    file_ptr->num_sections--;
	    continue;
	}

	if( (p=list_find_ach_section(sec_ptr->sec_hdr.s_name)) == (ACTION*)NULL )
	{
	    LINKER_ERROR2("Section %.8s mentioned in %s but not in architecture file", 
			  sec_ptr->sec_hdr.s_name, file_name);
	    file_ptr->num_sections--;
	    continue;
	}
	if (p->sect_type != sec_ptr->sec_hdr.s_flags)
	{
	    LINKER_ERROR2( "Section %.8s in File %s conflicts with architecture file",sec_ptr->sec_hdr.s_name, file_name);
	    file_ptr->num_sections--;
	    continue;
	}

	if( sec_ptr->sec_hdr.s_scnptr )
	    max_section_size = max(sec_ptr->sec_hdr.s_size, max_section_size);

	sec_ptr->section_num = (short) s;
	list_add( LIST_INPUT_SECTION, (LIST *) file_ptr, (char *) sec_ptr );
    }

    string_table_size = 0L;
    string_table_ptr = read_string_table( IOPTR(fd), file_ptr, &string_table_size );

    if( string_table_ptr && (string_table_size <= MAX_STRING_TABLE_SIZE) )
	file_ptr->strings = string_table_ptr;
    else
	file_ptr->strings = NULL;

    if( symbuf == NULL )
	if( FSEEK(fd, HEADER(fd).f_symptr, BEGINNING) != OKFSEEK )
	    LINKER_ERROR1( "Failed to seek to symbol table of file %s", file_name);

    file_ptr->local_syms = process_symbol_table( IOPTR(fd), HEADER(fd).f_nsyms, file_ptr, symbuf, 
						string_table_ptr );

    list_add( LIST_INPUT_FILE, &input_file_list, (char *) file_ptr );

    if( string_table_ptr && (string_table_size > MAX_STRING_TABLE_SIZE) )
      my_free( string_table_ptr );
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      read_string_table                                               *
*                                                                      *
*   Synopsis                                                           *
*      char *read_string_table( FILE *fd, INPUT_FILE *file_ptr,        *
*                               int *size )                            *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for reading in the string table of  *
*   a object file or a archive.                                        *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     5/26/89       created                  -----             *
***********************************************************************/

char *read_string_table( FILE *fd, register INPUT_FILE *file_ptr, register long *size )
{
    long symbol_table_end;
    char *string_table_ptr;
    long string_table_size;

    if( file_ptr->sym_table_ptr == 0 )
	return( NULL );

    symbol_table_end = file_ptr->sym_table_ptr + SYMESZ * file_ptr->num_symbols;

    if( file_ptr->arch_offset )
    {
	if( ar_size > symbol_table_end )
	{
	    if( fseek(fd, symbol_table_end + file_ptr->arch_offset, BEGINNING) != OKFSEEK ||
		fread( &string_table_size, sizeof(string_table_size), 1, fd) != 1 )
	    {
		LINKER_ERROR1( "Error reading string table of file %s", file_ptr->file_name );
	    }

	    *size = string_table_size;
	    if( *size > 4 )
	    {
		string_table_ptr = my_malloc( (long) *size );
#ifdef MSDOS
		if( fread(&string_table_ptr[4], (size_t) *size - 4, 1, fd) != 1 )
#else
		if( fread(&string_table_ptr[4], *size - 4, 1, fd) != 1 )
#endif
		    LINKER_ERROR1( "Error reading string table of file %s", file_ptr->file_name );
		else
		    return( string_table_ptr );
	    }
	    else
		return( NULL );
	}
	else
	    return( NULL );
    }
    else
    {
	/*
	 * Not a member of an archive so it must be a regular object file
	 */

	if( fseek(fd, symbol_table_end, BEGINNING) == OKFSEEK &&
	    fread( &string_table_size, sizeof(string_table_size), 1, fd) == 1 )
	{
	    *size = string_table_size;

	    if( *size > 4 )
	    {
		string_table_ptr = my_malloc( (long) *size );
#ifdef MSDOS
		if( fread(&string_table_ptr[4], (size_t) *size - 4, 1, fd) != 1 )
#else
		if( fread(&string_table_ptr[4], *size - 4, 1, fd) != 1 )
#endif
		    LINKER_ERROR1( "Error reading string table of file %s", file_ptr->file_name );
		else
		    return( string_table_ptr );
	    }
	    else
		return( NULL );
	}
	else
	    return( NULL );
    }
}

