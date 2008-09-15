
#include "../config.h"

#include <stdio.h>
#include "a_out.h"
#include "coff_io.h"

/***********************************************************************

  Name:         write_file_header

  Description:  writes a FILHDR structure to a file

  Return Value: 0 on error, 1 for success

  **********************************************************************/
int write_file_header( FILHDR *file_header, FILE *fp)
{
  if( 1 == fwrite( (unsigned char *)file_header, FILHSZ, 1, fp) )
    return(1);
  else
    return( 0);
}



/***********************************************************************

  Name:         write_optional_header

  Description:  writes a FILHDR structure to a file

  Return Value: 0 on error, 1 for success

  **********************************************************************/
int write_optional_header( AOUTHDR *optional_header, FILE *fp)
{
  if( 1 ==fwrite((unsigned char *)optional_header, AOUTSZ, 1, fp) )
    return(1);
  else
    return( 0);
}



/***********************************************************************

  Name:         write_section_header

  Description:  writes a SCNHDR structure to a file

  Return Value: 0 on error, 1 for success

  **********************************************************************/
int write_section_header( SCNHDR *section_header, FILE *fp )
{
  if( 1 == fwrite( (unsigned char *)section_header, SCNHSZ, 1, fp) )
    return(1);
  else
    return( 0);
}



/***********************************************************************

  Name:         write_symbol

  Description:  write a symbol table entry

  Return Value: 0 on error, 1 for success

  **********************************************************************/
int write_symbol( SYMENT *symbol_ptr, FILE *fp )
{
  if( 1 == fwrite( (unsigned char *)symbol_ptr, SYMESZ, 1, fp))
    return(1);
  else
    return( 0);
}




/***********************************************************************

  Name:         write_relocation_info

  Description:  write a relocation structure to a file

  Return Value: 0 on error, 1 for success

  **********************************************************************/
int write_relocation_info( RELOC *relocation_ptr, FILE *fp )
{
  if( 1 == fwrite( (unsigned char *)relocation_ptr, RELOC_SIZE, 1, fp) )
    return(1);
  else
    return( 0);
}




/***********************************************************************

  Name:         write_aux_info

  Description:  write an auxiliary information structure to a file

  Return Value: 0 on error, 1 for success

  **********************************************************************/
int write_aux_info( AUXENT *auxiliary_ptr, FILE *fp, int aux_class )
{
  aux_class = 0; /* to shut the compiler up */
   
  if( 1 == fwrite( (unsigned char *)auxiliary_ptr, AUXESZ, 1, fp) )
    return(1);
  else
    return( 0);
}



/***********************************************************************

  Name:         write_line_info

  Description:  write a LINENO structure to a file

  Return Value: 0 on error, 1 for success

  **********************************************************************/
int write_line_info( LINENO *line_ptr, FILE *fp )
{
  if( 1 == fwrite( (unsigned char *)line_ptr, LINESZ, 1, fp))
    return(1);
  else
    return( 0);
}


/***********************************************************************

  Name:         read_file_header

  Description:  reads a FILHDR structure to a file

  Return Value: 0 on error, 1 for success

  **********************************************************************/
int read_file_header( FILHDR *file_header, FILE *fp)
{
  if( 1 == fread( (unsigned char *)file_header, FILHSZ, 1, fp) )
    return(1);
  else
    return( 0);
}



/***********************************************************************

  Name:         read_optional_header

  Description:  reads a FILHDR structure to a file

  Return Value: 0 on error, 1 for success

  **********************************************************************/
int read_optional_header( AOUTHDR *optional_header, FILE *fp)
{
  if( 1 == fread((unsigned char *)optional_header, AOUTSZ, 1, fp) )
    return(1);
  else
    return( 0);
}



/***********************************************************************

  Name:         read_section_header

  Description:  reads a SCNHDR structure to a file

  Return Value: 0 on error, 1 for success

  **********************************************************************/
int read_section_header( SCNHDR *section_header, FILE *fp )
{
  if( 1 == fread( (unsigned char *)section_header, SCNHSZ, 1, fp) )
    return(1);
  else
    return( 0);
}



/***********************************************************************

  Name:         read_symbol

  Description:  read a symbol table entry

  Return Value: 0 on error, 1 for success

  **********************************************************************/
int read_symbol( SYMENT *symbol_ptr, FILE *fp )
{
  if( 1 == fread( (unsigned char *)symbol_ptr, SYMESZ, 1, fp))
    return(1);
  else
    return( 0);
}




/***********************************************************************

  Name:         read_relocation_info

  Description:  read a relocation structure to a file

  Return Value: 0 on error, 1 for success

  **********************************************************************/
int read_relocation_info( RELOC *relocation_ptr, FILE *fp )
{
  if( 1 == fread( (unsigned char *)relocation_ptr, RELOC_SIZE, 1, fp) )
    return(1);
  else
    return( 0);
}




/***********************************************************************

  Name:         read_aux_info

  Description:  read an auxiliary information structure to a file

  Return Value: 0 on error, 1 for success

  **********************************************************************/
int read_aux_info( AUXENT *auxiliary_ptr, FILE *fp, int aux_class )
{
  aux_class = 0; /* to shut the compiler up */
  if( 1 == fread( (unsigned char *)auxiliary_ptr, AUXESZ, 1, fp) )
    return(1);
  else
    return( 0);
}



/***********************************************************************

  Name:         read_line_info

  Description:  read a LINENO structure to a file

  Return Value: 0 on error, 1 for success

  **********************************************************************/
int read_line_info( LINENO *line_ptr, FILE *fp )
{
  if( 1 == fread( (unsigned char *)line_ptr, LINESZ, 1, fp))
    return(1);
  else
    return( 0);
}

