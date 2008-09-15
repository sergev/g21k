#include "../config.h"

#include <stdio.h>
#ifdef MSDOS
#include <stdlib.h>
#include <stddef.h>
#endif
#include <string.h>
#include "a_out.h"
#include "dump.h"
#include "coff_io.h"

static int (*(action[]))(FILE *, FILHDR) =
{
  print_file_header,
  print_optional_header,
  print_section_header,
  print_section_data,
  print_relocation_information,
  print_line_number_information,
  print_auxiliary_entry,
  print_string_table,
  print_symbol_table
};

static char *desc[] =
{
  "file header",
  "optional header",
  "section header",
  "section data",
  "relocation information",
  "line number information",
  "auxiliary entry",
  "string table",
  "symbol table"
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
    { ERR_USAGE, "improper usage", FATAL }
  };



int main( int argc, char **argv)
{
  unsigned long flags = 0;
  char          c, tmp[10];
  short         i, j = --argc;
  FILE         *fp;
  FILHDR        file_header;


  /*
   *  check the command line switches
   */
  if( ! argc )
    error( ERR_USAGE, 1, "coffdump [-adflorsty] object_file");

  while(--j)
  {
    if( '-' != argv[j][0] )
      continue;
    i = 1;
    while( c = argv[j][i++] )
      switch( c )
      {
      case 'a': flags |= AUX_ENTRY; break;
      case 'd': flags |= SECTION_DATA; break;
      case 'f': flags |= FILE_HEADER; break;
      case 'l': flags |= LINE_NUMBER; break;
      case 'o': flags |= OPTIONAL_HEADER; break;
      case 'r': flags |= RELOCATION; break;
      case 's': flags |= SECTION_HEADER; break;
      case 't': flags |= STRING_TABLE; break;
      case 'y': flags |= SYMBOL_TABLE; break;
      case 'x': flags |= AUX_ENTRY              /*EK*/
		       | SECTION_DATA
		       | FILE_HEADER
		       | LINE_NUMBER
		       | OPTIONAL_HEADER
		       | RELOCATION
		       | SECTION_HEADER
		       | STRING_TABLE
		       | SYMBOL_TABLE; break;
      default:
	tmp[0] = c;
	tmp[1] = 0;
	error( ERR_BAD_SWITCH, 1, tmp);
	break;
      } /* switch */
  }
	
	
  /*
   *  open the object file and read the header
   */
  if( NULL == ( fp = fopen( argv[argc], "rb" ) ) )
    error( ERR_FOPEN, 1, argv[argc] );
  if( !read_file_header( &file_header, fp) )
    error( ERR_FILE_CORRUPTED, 2, "\nreading: ", desc[DESC_FILHDR]);
  if( 21020 != file_header.f_magic ) /*EK* was 21000 */
    error( ERR_FILE_CORRUPTED, 1, "bad magic number" );
	
  /*
   *  print file and each section that has been asked for
   */

  printf("file: %s\n", argv[argc]);
  i = 0;
  while( flags )
  {
    if( flags & 0x1 )
    {
      printf( "%s\n", desc[i]);
      if( (j = action[i](fp, file_header) ) )
	error( j, 1, desc[i] );
      printf("\n");
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



int get_section(SCNHDR *section_header, FILE *fp, short section_number, FILHDR file_header)
{
  if( section_number < 1 || section_number > file_header.f_nscns )
    return( ERR_OUT_OF_RANGE);
  if( fseek( fp, (long)(FILHSZ + file_header.f_opthdr + ( ( section_number - 1 ) * SCNHSZ ) ), SEEK_SET) )
    return( ERR_FSEEK); 
  if( !read_section_header( section_header, fp) )
    return( ERR_FREAD);
  return( 0);

}




	
int print_file_header(FILE *fp, FILHDR file_header)
{
  static char title[] = "f_magic    f_nscns    f_timedat   f_symptr    f_nsyms     f_opthdr  f_flags\n";
  static char control[] = "0x%04hX     0x%04hX     0x%08lX  0x%08lX  0x%08lX  0x%04hX    0x%04hX\n";

  printf( title);
  printf( control, file_header.f_magic, file_header.f_nscns, file_header.f_timdat, file_header.f_symptr, file_header.f_nsyms, file_header.f_opthdr, file_header.f_flags);
  return(0);
	
}



int print_optional_header(FILE *fp, FILHDR file_header)
{
  AOUTHDR opt_header;
  static char title[] = "magic  vstamp tsize      dsize      bsize      entry      text_start data_start\n";
  static char control[] = "0x%04hX 0x%04hX 0x%08lX 0x%08lX 0x%08lX 0x%08lX 0x%08lX 0x%08lX\n";
	
  if( 0 == file_header.f_opthdr )
  {
    printf("no optional header\n");
    return(0);
  }

  if( fseek( fp, (long)(FILHSZ), SEEK_SET) )
    return( ERR_FSEEK);
  if( !read_optional_header( &opt_header, fp))
    return( ERR_FREAD);

  printf( title);
  printf( control, opt_header.magic, opt_header.vstamp, opt_header.tsize, opt_header.dsize, opt_header.bsize, opt_header.entry, opt_header.text_start, opt_header.data_start);  

  return(0);
}



int print_section_header(FILE *fp, FILHDR file_header)
{
  short         i;
  int           j;
  char          tmp[SYMNMLEN + 1];
  SCNHDR        section_header;
  static char title[] = "s_name   s_paddr    s_vaddr    s_size     s_scnptr   s_relptr   s_lnnoptr\n         s_nreloc   s_nlnno    s_flags\n";
  static char control[] = "%-8s 0x%08lX 0x%08lX 0x%08lX 0x%08lX 0x%08lX 0x%08lX\n         0x%04hX     0x%04hX     0x%08lX\n";

  tmp[SYMNMLEN] = '\0';
	
  printf( title);
  for( i = 1 ; i <= file_header.f_nscns ; ++i )
  {
    if( j = get_section( &section_header, fp, i, file_header) )
      return( j);
    strncpy( tmp, section_header.s_name, SYMNMLEN);
    printf( control, tmp, section_header.s_paddr, section_header.s_vaddr, section_header.s_size, section_header.s_scnptr, section_header.s_relptr, section_header.s_lnnoptr, section_header.s_nreloc, section_header.s_nlnno, section_header.s_flags);
    printf( "\n");
  }

  return(0);
}





int print_section_data(FILE *fp, FILHDR file_header)
{
  short         i, line, word_len, line_mod;
  int           j;
  long          l;
  unsigned char buf[6];
  SCNHDR        section_header;
  char          tmp[SYMNMLEN + 1];
	
  tmp[SYMNMLEN] = '\0';
  for( i = 1 ; i <= file_header.f_nscns ; ++i )
  {
    if( j = get_section( &section_header, fp, i, file_header) )
      return( j);
    if( fseek( fp, section_header.s_scnptr, SEEK_SET) )
      return( ERR_FSEEK);

    /*EK* was: if( SECTION_DMDATA == section_header.s_flags )*/

    if( SECTION_PM != (section_header.s_flags & SECTION_PM)) /*EK*/
    {
      word_len = 5;
      line_mod = DM_WORDS_PER_LINE;
    }
    else
    {
      word_len = 6;
      line_mod = PM_WORDS_PER_LINE;
    }

    strncpy( tmp, section_header.s_name, SYMNMLEN);
    printf( "section: %s\n", tmp);
    for( l = 0, line = line_mod ; l < section_header.s_size ; l += word_len )
    {
      if( word_len != fread( buf, 1, word_len, fp) )
	return( ERR_FREAD);
	
      printf("%02X%02X%02X%02X%02X", buf[0], buf[1], buf[2], buf[3], buf[4]);
      if( 6 == word_len )
	printf("%02X ", buf[5]);
      else
	printf(" ");
      if( ! --line )
      {
	printf( "\n");
	line = line_mod;
      } /* if 'line' */
    } /* for 'l' */
    if( line != line_mod )
      printf("\n");
    printf("\n");

    if( section_header.s_size != l )
      return( ERR_IMPROPER_ALLIGNMENT);
	
  } /* for 'i' */

  return( 0);
}




int print_relocation_information(FILE *fp, FILHDR file_header)
{
  fp = NULL;
  file_header.f_magic =0;
  return(0);
}
int print_line_number_information(FILE *fp, FILHDR file_header)
{
  fp = NULL;
  file_header.f_magic =0;
  return(0);
}
int print_auxiliary_entry(FILE *fp, FILHDR file_header)
{
  fp = NULL;
  file_header.f_magic =0;
  return(0);
}
int print_string_table(FILE *fp, FILHDR file_header)
{
  fp = NULL;
  file_header.f_magic =0;
  return(0);
}
int print_symbol_table(FILE *fp, FILHDR file_header)
{
  fp = NULL;
  file_header.f_magic =0;
  return(0);
}
	



void error( int error_code, int numargs, ... )
{
  char *argp;
  short i;

  printf( "%s: %s ... ", (error_message[error_code].severity == WARNING ? "warning" : "fatal error" ), error_message[error_code].message);
	
  /*EK* BUG: this highly *NON*portable... */
  argp = ((char *)&numargs) + sizeof(int);
  for( i = 0 ; i < numargs ; ++i ) {
    printf( "%s", *((char **)(argp)));
    argp += sizeof(char *);
  }
  printf("\n");

  if( FATAL == error_message[error_code].severity )
    exit(error_code);
}
