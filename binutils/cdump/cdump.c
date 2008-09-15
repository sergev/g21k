#include "../config.h"

#include <stdio.h>

#ifdef MSDOS
#include <stdlib.h>
#include <stddef.h>
#endif

#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "a_out.h"
#include "cdump.h"
#include "coff_io.h"
// #include "release.h"


static int (*(action[]))(FILE *, FILHDR) =
{
  print_file_header,
  print_optional_header,
  print_section_header,
  print_section_data,
  print_relocation_information,
  print_line_number_information,
  print_symbol_table,
  print_string_table
};

static char *desc[] =
{
  "File Header",
  "Optional Header",
  "Section Header",
  "Section Data",
  "Relocation Information",
  "Line Number Information",
  "Symbol Table",
  "String Table"
};


struct err_msg
{
  short error_code;
  char *message;
  short severity;
} error_message[] =
  {
    { ERR_FOPEN, "could not open file", FATAL },
    { ERR_FSEEK, "could not fseek", FATAL },
    { ERR_FREAD, "could not fread", FATAL },
    { ERR_BAD_SWITCH, "bad switch - ignored", WARNING },
    { ERR_FILE_CORRUPTED, "file corrupted", FATAL },
    { ERR_OUT_OF_RANGE, "parameter out of range", WARNING },
    { ERR_IMPROPER_ALLIGNMENT, "data not on proper byte boundary", WARNING },
    { ERR_STRING_TABLE, "reading string table", WARNING },
    { ERR_USAGE, "improper usage", FATAL }
  };


static void skip ( int spaces);
static void print_raw_aux ( char *ae);
static int decode_aux(AUXENT *ae, SYMENT *sym);
static long port_get_long( register unsigned char *buffer );
static void banner (void);

void set_section_name (int, FILHDR *, FILE *);

#define SEC_NAME_SIZE 100

static char section_name[SEC_NAME_SIZE];
static FILE *outfp;



int main( int argc, char **argv)
{
    FILE                *fp;
    FILHDR       file_header;
    char                 c;
    short                i;
    short                j=0;
    unsigned long        flags = 0;
    char                 infile[200];
	
i=FILHSZ;
printf("SCNHSZ=%i\n",i);

i=AOUTSZ;
printf("AOUTSZ=%i\n",i);

i = SCNHSZ;
printf("SCNHSZ=%i\n",i);
	
i = RELOC_SIZE;
printf("RELOC_SIZE=%i\n",i);
	
i = LINESZ;
printf("LINESZ=%i\n",i);
	
i = SYMESZ;
printf("SYMESZ=%i\n",i);
	
i = AUXESZ;
printf("AUXESZ=%i\n",i);
	
    /*
     *  check the command line switches
     */
    outfp=stdout; /* initialize */
    fp=NULL;
    if (argc == 1) {
	banner();
	printf( "Usage: cdump executable_file or object_file [-o output_file]\n");
	exit(0);
    } else if( argc < 2 ) {
	printf( "Usage: cdump executable_file or object_file [-o output_file]\n");
	exit(0);
    }
	
    while(++j<argc)
    {
    /* always do dflorsty for now */
    flags = SECTION_DATA | FILE_HEADER | 
	    LINE_NUMBER | OPTIONAL_HEADER | RELOCATION | 
	    SECTION_HEADER | STRING_TABLE | SYMBOL_TABLE; 
	if( '-' == argv[j][0] )
	{
	    c = argv[j][1];
	    switch( c )
	    {
		  case 'o': 
		  case 'O': 
		      j++; /* get output file name */
		      if (j >= argc) {
			 printf( "Usage: cdump executable_file or object_file [-o output_file]\n");
			 exit(0);
		      }
		      if( NULL == ( outfp = fopen( argv[j], "w" ) ) )
			error( ERR_FOPEN, 1, argv[j] );
			
		  break;

		  case 'v':
		  case 'V':
		      banner();
		  break;

		  default:
		    printf( "Usage: cdump executable_file or object_file [-o output_file]\n");
		    exit(0);
		  break;
	    } /* switch */

	}
	else
	{
		
	    /*
	     *  open the object file and read the header
	     */
	    strcpy(infile,argv[j]);
	    if( NULL == ( fp = fopen( argv[j], "rb" ) ) )
		error( ERR_FOPEN, 1, argv[j] );
	    if( !read_file_header( &file_header, fp) )
		error( ERR_FILE_CORRUPTED, 2, "\nreading: ", desc[DESC_FILHDR]);
	    if( M_21000 != file_header.f_magic && M_2100 != file_header.f_magic)
		error( ERR_FILE_CORRUPTED, 1, "bad magic number" );
	}
     }
     if (fp == NULL) {
	printf( "Usage: cdump executable_file or object_file [-o output_file]\n");
	exit(0);
     }
		
   /*
    *  print file and each section that has been asked for
    */
	
   fprintf(outfp,"File: %s\n", infile);
   i = 0;
   while( flags )
   {
	if( flags & 0x1 )
	{
	    int ec;
		
	    fprintf( outfp,"%s\n", desc[i]);
	    if( (ec = action[i](fp, file_header) ) )
		error( ec, 1, desc[i] );
	    fprintf(outfp,"\n");
	}
	flags >>= 1;
	++i;
    }
		
		
    /*
     *  close the file and exit
     */
    fclose( fp);
    return 0;
}



int get_section(SCNHDR *section_header,
		FILE *fp,
		short section_number,
		FILHDR file_header)
{
    if( section_number < 1 || section_number > file_header.f_nscns )
	return( ERR_OUT_OF_RANGE);

    if( fseek(fp, (long)(FILHSZ + file_header.f_opthdr + (( section_number - 1 ) * SCNHSZ )), SEEK_SET))
	return( ERR_FSEEK); 

    if( !read_section_header( section_header, fp) )
	return( ERR_FREAD);

  return( 0);

}



	
int print_file_header(FILE *fp, FILHDR file_header)
{
  static char title[] =
    "f_magic    f_nscns    f_timedat   f_symptr    f_nsyms     f_opthdr  f_flags\n";
  static char control[] =
    "0x%04hX     0x%04hX     0x%08lX  0x%08lX  0x%08lX  0x%04hX    0x%04hX\n";
fp;

  fprintf( outfp,title);
  fprintf( outfp,control,
	 file_header.f_magic,
	 file_header.f_nscns,
	 file_header.f_timdat,
	 file_header.f_symptr,
	 file_header.f_nsyms,
	 file_header.f_opthdr,
	 file_header.f_flags);
  fprintf( outfp,"   f_timedat in English is: %s", ctime(&file_header.f_timdat));
  return(0);
	
}



int print_optional_header(FILE *fp, FILHDR file_header)
{
  AOUTHDR opt_header;
  static char title[] =
    "magic  vstamp tsize      dsize      bsize      entry      text_start data_start\n";
  static char control[] =
    "0x%04hX 0x%04hX 0x%08lX 0x%08lX 0x%08lX 0x%08lX 0x%08lX 0x%08lX\n";
	
  if( 0 == file_header.f_opthdr )
  {
    fprintf(outfp,"no optional header\n");
    return(0);
  }

  if( fseek( fp, (long)(FILHSZ), SEEK_SET) )
    return( ERR_FSEEK);
  if( !read_optional_header( &opt_header, fp))
    return( ERR_FREAD);

  fprintf( outfp,title);
  fprintf( outfp,control,
	 opt_header.magic,
	 opt_header.vstamp,
	 opt_header.tsize,
	 opt_header.dsize,
	 opt_header.bsize,
	 opt_header.entry,
	 opt_header.text_start,
	 opt_header.data_start);

 {
	int i;
	char string[32];


	i = file_header.f_opthdr - AOUTSZ;
	for( ; i>0; i-=32){
		fread(string, sizeof(char), 32, fp);
		fprintf(outfp,"String 1 %s\n",string);
	}
  }
  return(0);
}



int print_section_header(FILE *fp, FILHDR file_header)
{
  short         i;
  int           j;
  SCNHDR        section_header;
  static char title[] = "s_name                                    s_paddr    s_vaddr    s_size\ns_scnptr   s_relptr   s_lnnoptr  s_nreloc s_nlnno    s_flags\n";
  static char control[] = "%-40s  0x%08lX 0x%08lX 0x%08lX\n0x%08lX 0x%08lX 0x%08lX 0x%04hX   0x%04hX     0x%08lX\n";



  fprintf( outfp,title);
  for( i = 1 ; i <= file_header.f_nscns ; ++i )
  {
      set_section_name (i, &file_header, fp);

      if( j = get_section( &section_header, fp, i, file_header) )
	  return( j);

      fprintf( outfp,control, section_name,
	     section_header.s_paddr,
	     section_header.s_vaddr,
	     section_header.s_size,
	     section_header.s_scnptr,
	     section_header.s_relptr,
	     section_header.s_lnnoptr,
	     section_header.s_nreloc,
	     section_header.s_nlnno,
	     section_header.s_flags);
      fprintf( outfp,"\n");
  }

  return(0);
}


void
set_section_name (int secnum, FILHDR *file_header, FILE *fp)
{
    SCNHDR section_header;

    get_section (&section_header, fp, secnum, *file_header);

    strncpy (section_name, section_header.s_name, SYMNMLEN);
    section_name[SYMNMLEN] = '\0';
}
	


int print_section_data(FILE *fp, FILHDR file_header)
{
  short         i, line, word_len, line_mod;
  int           j;
  long          l;
  unsigned char buf[6];
  SCNHDR        section_header;



  for( i = 1 ; i <= file_header.f_nscns ; ++i )
  {
      set_section_name (i, &file_header, fp);
      fprintf( outfp,"section: '%s'\n", section_name);

      if( j = get_section( &section_header, fp, i, file_header) )
	  return( j);

      if( fseek( fp, section_header.s_scnptr, SEEK_SET) )
	  return( ERR_FSEEK);


      if( SECTION_PM == (section_header.s_flags & SECTION_PM))
	{
	  word_len = 6;
	  line_mod = PM_WORDS_PER_LINE;
	}
      else
	{
	  word_len = 5;
	  line_mod = DM_WORDS_PER_LINE;
	}
	
	
      for( l = 0, line = line_mod ; l < section_header.s_size ; l += word_len )
      {
	  if( word_len != fread( buf, 1, word_len, fp) )
	      return( ERR_FREAD);

	  switch (word_len)
	  {
	    case 6:
	      fprintf(outfp,"%02X%02X%02X%02X%02X%02X ", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	      break;

	    case 5:
	      fprintf(outfp,"%02X%02X%02X%02X%02X ", buf[0], buf[1], buf[2], buf[3], buf[4]);
	      break;

	    default:
	      fprintf (stdout, "internal error: invalid word length");
	      return (ERR_IMPROPER_ALLIGNMENT);
	      break;
	  }
		
	  if( ! --line )
	  {
	      fprintf( outfp,"\n");
	      line = line_mod;
	  } /* if 'line' */
      } /* for 'l' */
      if( line != line_mod )
	  fprintf(outfp,"\n");
      fprintf(outfp,"\n");
	
      if( section_header.s_size != l )
	  return( ERR_IMPROPER_ALLIGNMENT);
	
  } /* for 'i' */
	
  return( 0);
}




int print_relocation_information(FILE *fp, FILHDR file_header)
{
  SCNHDR        section_header;
  RELOC         reloc_entry;
  short         s, sx;
  int           i;
  static char   title[] =   "     r_vaddr r_symndx r_type\n";
  static char   control[] = "    %8ld %8ld   %4d\n";

  for( s = 1 ; s <= file_header.f_nscns ; ++s )
  {
      set_section_name (s, &file_header, fp);

    if( i = get_section( &section_header, fp, s, file_header) )
      return( i);

    if( fseek( fp, section_header.s_relptr, SEEK_SET) )
      return( ERR_FSEEK);

    fprintf(outfp,"section '%s' has %d relocation entr%s\n",
	   section_name,
	   section_header.s_nreloc,
	   ( 1 == section_header.s_nreloc ? "y" : "ies"));
    if( 0 != section_header.s_nreloc )
      fprintf( outfp,title );
    for( sx = 0 ; sx < section_header.s_nreloc ; ++sx )
    {
      if( ! read_relocation_info( &reloc_entry, fp ) )
	return( ERR_FREAD);
      fprintf( outfp,control,
	     reloc_entry.r_vaddr,                    /*EK*/
	     reloc_entry.r_symndx, /*EK*/
	     reloc_entry.r_type);
    }
  }
	
  return(0);

}

int print_line_number_information(FILE *fp, FILHDR file_header)
{
  static char   title[] = "l_addr.l_symndx l_addr.l_paddr l_lnno\n";

  short         s, sx;
  int           i;
  LINENO        line_num_entry;
  SCNHDR        section_header;

  for( s = 1 ; s <= file_header.f_nscns ; ++s )
  {
      set_section_name (s, &file_header, fp);

    if( i = get_section( &section_header, fp, s, file_header) )
      return( i);

    if( fseek( fp, section_header.s_lnnoptr, SEEK_SET) )
      return( ERR_FSEEK);

    fprintf(outfp,"section '%s' has %d line number entr%s\n",
	   section_name,
	   section_header.s_nlnno,
	   ( 1 == section_header.s_nlnno ? "y" : "ies"));

    if( 0 != section_header.s_nlnno )
      fprintf( outfp,title );
    for( sx = 0 ; sx < section_header.s_nlnno ; ++sx )
    {
      if( ! read_line_info( &line_num_entry, fp ) )
	return( ERR_FREAD);
      if( 0 == line_num_entry.l_lnno )
	fprintf( outfp,"   %12ld                ", line_num_entry.l_addr.l_symndx);
      else
	fprintf( outfp,"                      0x%06lx ", line_num_entry.l_addr.l_paddr);
      fprintf( outfp,"     %d\n", line_num_entry.l_lnno);
    }
  }
	

  return(0);
}

int print_symbol_table(FILE *fp, FILHDR file_header)
{
  AUXENT        aux;
  SYMENT        symbol;
  long          symctr;
  static char   *symbol_union_control[] =
    { "%8ld %8.8s               ", "%8ld %8ld   0x%08lx  " };
  static char   *symbol_union_title[] =
    { "   index   _n_name            ", "   index _n_zeroes   _n_offset" };
	
  static char   symbol_control[] = "0x%08lx %7d 0x%04x %8d %8d\n";
  static char   symbol_title[] = "     n_value n_scnum n_type n_sclass n_numaux\n";
	
  if( fseek( fp, file_header.f_symptr, SEEK_SET ) )
    return( ERR_FSEEK);
	
  fprintf( outfp,"%s%s\n%s%s\n",
	 symbol_union_title[0],
	 symbol_title,
	 symbol_union_title[1],
	 symbol_title);
	
  for( symctr = 0 ; symctr < file_header.f_nsyms ; symctr++ )
    {
      if( !read_symbol( &symbol, fp) )
	return(ERR_FREAD);
      if( 0L == symbol.n_zeroes ) /* character overlaying not supported */
	fprintf( outfp,symbol_union_control[1], symctr, symbol.n_zeroes, symbol.n_offset);
      else
	fprintf( outfp,symbol_union_control[0], symctr, symbol.n_name);
	
      fprintf( outfp,symbol_control,
	     symbol.n_value,
	     symbol.n_scnum,
	     symbol.n_type,
	     symbol.n_sclass,
	     symbol.n_numaux );
	
      if( symbol.n_numaux)
	{
	  if ( !read_aux_info( &aux, fp, 0))
	    return( ERR_FREAD);

	  ++symctr;
	  fprintf(outfp,"%8ld ", symctr);

	  /* print the raw aux entry in hex */
	  print_raw_aux ( (char *) &aux);

	  /* decode and print the aux entry */
	  if (decode_aux ( &aux, &symbol))
	    fprintf(outfp,"Unexpected aux entry; could not decode.");
	  fputc('\n',outfp);
	}
    }
  return(0);
}


int print_string_table(FILE *fp, FILHDR file_header)
{
  int           c, buf_ct = 0;
  long          string_table_size, offset, begin_offset;
  char          string_name[BUFSIZ];

  if( fseek( fp, file_header.f_symptr + file_header.f_nsyms * SYMESZ, SEEK_SET) )
    return( ERR_FSEEK);
  if( 1 != fread( &string_table_size, sizeof(long), 1, fp) )
    return( ERR_FREAD);
	
  if ( string_table_size > sizeof(long) )
    {
      fprintf(outfp,"size of string table: 0x%08lx\n", string_table_size);
      fprintf(outfp,"offset\t\tstring\n");

      begin_offset = offset = sizeof(long);
      while( (EOF != ( c = getc(fp)))  && (offset++ < string_table_size) )
	{
	  string_name[buf_ct] = (char)c;
	  buf_ct += ( buf_ct != BUFSIZ );
	  if( !c )
	    {
	      fprintf(outfp,"0x%08lx\t%s\n", begin_offset, string_name );
	      begin_offset = offset;
	      buf_ct = 0;
	    }
	}

      if( offset < string_table_size )
	error( ERR_STRING_TABLE, 1, "table to short");
      if( offset > string_table_size )
	error( ERR_STRING_TABLE, 1, "table to long");
    }

  else fprintf(outfp,"no string table present.");
	
  return(0);
}


static
void skip ( int spaces)
{
  int i; for (i = 1; i <= spaces; i++) fprintf(outfp," ");
}


static
void print_raw_aux ( char *ae)
{
  int i;
  fprintf(outfp,"     aux = 0x");
  for (i = 1; i <= AUXESZ; i++, *ae++)
    {
      fprintf(outfp,"%1x", (*ae >> 4) & 0xf);
      fprintf(outfp,"%1x", (*ae & 0xf));
    }
  fprintf(outfp,"\n");
}
	

static
int decode_aux(AUXENT *ae, SYMENT *sym)
{
  int space = 20;

  if (X_FILENAME(sym))
    {
      skip (space);
      fprintf(outfp,"(filename)\n");
      skip (space);
      fprintf(outfp,"x_fname=\"%s\"", ae->x_file.x_fname);
    }

  else if (X_SECTION(sym))
    {
      skip (space);
      fprintf(outfp,"(section)\n");
      skip (space);
      fprintf(outfp,"x_scnlen=0x%lx, x_nreloc=%hu, x_nlinno=%hu",
	     ae->x_scn.x_scnlen,
	     ae->x_scn.x_nreloc,
	     ae->x_scn.x_nlinno);
    }

  else if (X_TAGNAME(sym))
    {
      skip (space);
      fprintf(outfp,"(tag name)\n");
      skip (space);
      fprintf(outfp,"x_size=0x%hx, x_endndx=%ld",
	     ae->x_sym.x_misc.x_lnsz.x_size,
	     ae->x_sym.x_fcnary.x_fcn.x_endndx);
    }

  else if (X_EOS(sym))
    {
      skip (space);
      fprintf(outfp,"(end of structure)\n");
      skip (space);
      fprintf(outfp,"x_tagndx=%ld, x_size=0x%hx",
	     ae->x_sym.x_tagndx,
	     ae->x_sym.x_misc.x_lnsz.x_size);
    }

  else if (X_FUNCNAME(sym))
    {
      skip (space);
      fprintf(outfp,"(function name)\n");
      skip (space);
      fprintf(outfp,"x_tagndx=%ld, x_fsize=0x%lx, x_lnnoptr=0x%lx, x_endndx=%ld",
	     ae->x_sym.x_tagndx,
	     ae->x_sym.x_misc.x_fsize,
	     ae->x_sym.x_fcnary.x_fcn.x_lnnoptr,
	     ae->x_sym.x_fcnary.x_fcn.x_endndx);
    }

  else if (X_ARRAY(sym))
    {
      skip (space);
      fprintf(outfp,"(array)\n");
      skip (space);
      fprintf(outfp,"x_tagndx=%ld, x_lnno=%hu, x_size=0x%hx\n",
	     ae->x_sym.x_tagndx,
	     ae->x_sym.x_misc.x_lnsz.x_lnno,
	     ae->x_sym.x_misc.x_lnsz.x_size);
      skip (space);
      fprintf(outfp,"x_dimen[0][1][2][3]=%hu, %hu, %hu, %hu",
	     ae->x_sym.x_fcnary.x_ary.x_dimen[0],
	     ae->x_sym.x_fcnary.x_ary.x_dimen[1],
	     ae->x_sym.x_fcnary.x_ary.x_dimen[2],
	     ae->x_sym.x_fcnary.x_ary.x_dimen[3]);
    }

  else if (X_BLOCK(sym))
    {
      skip (space);
      if (strcmp (OBJ_SYM_NAME(sym), ".bb") == 0)
	fprintf(outfp,"(begin block)\n");
      else
	fprintf(outfp,"(end block)\n");
      skip (space);
      fprintf(outfp,"x_lnno=%hu", ae->x_sym.x_misc.x_lnsz.x_lnno);
      if (strcmp (OBJ_SYM_NAME(sym), ".bb") == 0)
	fprintf(outfp,", x_endndx=%ld", ae->x_sym.x_fcnary.x_fcn.x_endndx);
    }

  else if (X_FUNCTION(sym))
    {
      skip (space);
      if (strcmp (OBJ_SYM_NAME(sym), ".bf") == 0)
	fprintf(outfp,"(begin function)\n");
      else
	fprintf(outfp,"(end function)\n");
      skip (space);
      fprintf(outfp,"x_lnno=%hu", ae->x_sym.x_misc.x_lnsz.x_lnno);
      if (strcmp (OBJ_SYM_NAME(sym), ".bf") == 0)
	fprintf(outfp,", x_endndx=%ld", ae->x_sym.x_fcnary.x_fcn.x_endndx);
    }

  else if (X_SUENAME(sym))
    {
      skip (space);
      fprintf(outfp,"(struct/union/enum name)\n");
      skip (space);
      fprintf(outfp,"x_tagndx=%ld, x_size=0x%hx",
	     ae->x_sym.x_tagndx,
	     ae->x_sym.x_misc.x_lnsz.x_size);
    }

  else if (X_BITFLDEL(sym))
    {
      skip (space);
      fprintf(outfp,"(bitfield element)\n");
      skip (space);
      fprintf(outfp,"x_tagndx=%ld, n_value(bit_pos)=%ld, x_size(bit_width)=%hd",
	     ae->x_sym.x_tagndx,
	     sym->n_value,
	     ae->x_sym.x_misc.x_lnsz.x_size);
    }


  else return (1);

  return (0);
}




void error( int error_code, int numargs, ... )
{
  va_list argp;
  short i;

  fprintf( outfp,"%s: %s ... ",
	 (error_message[error_code].severity == WARNING ? "warning" : "fatal error" ),
	 error_message[error_code].message);
	
  va_start( argp, numargs);

  for( i = 0 ; i < numargs ; ++i )
  {
      fprintf( outfp,"%s", va_arg(argp, char *));
  }
  fprintf(outfp,"\n");

  if( FATAL == error_message[error_code].severity )
    exit(error_code);
}



static long port_get_long( register unsigned char *buffer )
{
    register long w = 0;
    register int i = 8 * sizeof(long);

    while( (i -= 8) >= 0 )
	    w |= (long) ((unsigned char) *buffer++) << i;

    return( w );
}



static void banner (void) {

	printf( "\nAnalog Devices ADSP-210xx Coff Dumper \n");
	printf( "Release %s \n", get_release_number());
	printf( "%s\n", get_copyright_notice() );
	printf( "%s\n", get_warranty_notice() ); /*EK*/
	printf( "%s\n\n", get_license_notice() ); /*EK*/

	  }
