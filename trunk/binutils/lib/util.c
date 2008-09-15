/* @(#) util.c 2.30@(#) 8/31/95 */

#include <stdio.h>

#ifndef sun
#include <stddef.h>
#endif /* !sun */

#include <malloc.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h> /*EK*/

#ifdef MSDOS
#include <time.h>
#else
#include <sys/time.h>
#endif

#if defined(MSDOS) && !defined(WIN31) && !defined(__GNUC__)
#include <stdlib.h>
void            *D16HugeAlloc (long total_size);        /* Rational */

#endif

#ifdef sun
#include <strings.h>
#endif

#ifndef MSDOS /*EK*/
#define huge
#endif

#include "app.h"
#include "util.h"

#ifdef CSCAPE /*EK*/
#include "flt_pt.h"
#endif

#define FATAL 1
#ifndef SUCCESS
#define SUCCESS 0
#endif

#define BITSPERBYTE  8

#ifndef MAX_FILENAME_LENGTH
#define MAX_FILENAME_LENGTH 64
#endif


/* Redirect exit and stdout for CSCAPE */

#if (defined (SIM) || defined (EMU)) && defined CSCAPE /*EK*/
#include <cscape.h>
#include <scancode.h>
#include "gui_mgr.h"
#include "gui_tty.h"
#define fprintf gui_tty_output
#define stdout gui_debug_win
#endif

/* Redirect exit and stdout for Windows */

#if defined WIN31 && (defined (ICE) || defined (SIM))
#include <windows.h>
#include <stdarg.h>
#define exit my_exit
#define fprintf bugprint
static void 
bugprint (FILE * stream, char *fmt,...)
{
  va_list                 argptr;
  char                    buffer[BUFSIZ];

  va_start (argptr, fmt);
  vsprintf (buffer, fmt, argptr);
  strcat (buffer, "\n");
  OutputDebugString ((LPCSTR) buffer);
  va_end (argptr);
}

#endif


/* Halloc Kludge for DOS mallocs */

#if defined (MSDOS) && !defined(__GNUC__) && !defined (WIN31)
#define USING_HALLOC
#endif

#if defined (TESTHEAD)
#undef USING_HALLOC
#endif

#ifdef USING_HALLOC
#define MALLOC_CUTOFF_SIZE (65535)
typedef struct halloc_entry
{
  void                   *buffer_address;
  struct halloc_entry    *next;
}                       HALLOC_ENTRY;
HALLOC_ENTRY           *halloc_list = NULL;

#endif

/***********************************************************************/

/********************************************************************
	
  Name:           my_malloc
	
  Function:       malloc with a little more control
	
  Return Value:   address of memory malloced
	
  ********************************************************************/

void                   *
my_malloc (long size)
{
  void                   *buffer;

#ifdef USING_HALLOC
  HALLOC_ENTRY           *entry;

#endif

  if (!size)
    return NULL;

#ifdef USING_HALLOC
  /* . IF (mallocing more than malloc allows) */
  if (size > MALLOC_CUTOFF_SIZE)
    {
      /* . . Allocate an halloc entry */
      entry = malloc ((long) sizeof (HALLOC_ENTRY));

      if (size > 131072L)
	{                       /* if size > 128k, must be a power of 2 */
	  if (size > 2097152)
	    {
	      fprintf (stdout, "Cannot malloc enough memory 2mb max\n");
	      exit (FATAL);
	    }
	  if (size > 1048576)
	    size = 2097152;
	  else if (size > 524288)
	    size = 1048576;
	  else if (size > 262144)
	    size = 524288;
	  else
	    size = 262144;
	}

      /* . . See if we can halloc this much memory */

#if defined (RATIONAL)
      entry->buffer_address = D16HugeAlloc (size);
#else
      entry->buffer_address = halloc ((size >> 1) + 1, 2);
#endif

      if (entry->buffer_address == NULL)
	{
	  fprintf (stdout, "Cannot malloc enough memory\n");
	  exit (FATAL);
	}

      /* . . Add this entry into our halloc list */
      entry->next = halloc_list;
      halloc_list = entry;

      /* . . Return the address */
      return (entry->buffer_address);

      /* . ENDIF */
    }

  /* ENDIF */
#endif

  /* Malloc the memory */

#if defined(WIN31) && defined(__BORLANDC__)
  if ((buffer = farmalloc (size)) == NULL)
#else
    if ((buffer = malloc ((size_t) size)) == NULL)
#endif

      {
	fprintf (stdout, "Cannot malloc enough memory\n");
	exit (FATAL);
      }

  /* Return the address */
  return (buffer);
}




/********************************************************************
	
  Name:           my_strdup
	
  Function:       dup a string
	
  Return Value:   new string pointer
	
  ********************************************************************/
void                   *
my_strdup (void *p)
{
  return strdup (p);
}





/********************************************************************
	
  Name:           my_free
	
  Function:       free with a little more control
	
  Return Value:   none
	
  ********************************************************************/

void 
my_free (void *buffer)
{

#ifdef USING_HALLOC
  HALLOC_ENTRY           *entry;
  HALLOC_ENTRY           *prev_entry;

#endif

  /* Be sure we're really freeing something */
  if (!buffer)
    return;

#ifdef USING_HALLOC
  /* . IF (this buffer was halloced) */
  for (prev_entry = NULL, entry = halloc_list;
       entry;
       prev_entry = entry, entry = entry->next)
    {
      if (entry->buffer_address == buffer)
	{
	  /* . . Remove it */
	  if (prev_entry)
	    prev_entry->next = entry->next;
	  else
	    halloc_list = entry->next;

	  /* . . Free the buffer and the halloc entry */
	  hfree (entry->buffer_address);
	  free (entry);
	  return;
	}
    }
#endif

  /* Free the buffer */

#if defined(WIN31) && defined(__BORLANDC__)
  farfree (buffer);
#else
  free (buffer);
#endif

  /* Return */
}



/***********************************************************************
 *                                                                      *
 *   Name                                                               *
 *      my_calloc                                                       *
 *                                                                      *
 *   Synopsis                                                           *
 *      char *my_calloc( long nbytes )                                  *
 *                                                                      *
 *   Description                                                        *
 *      This routine allocates a chunk of zeroed memory.                *
 *                                                                      *
 *   Revision History                                                   *
 *                                                                      *
 *     name    date          description              number            *
 *     mkc     6/13/89       created                  -----             *
 ***********************************************************************/

void                   *
my_calloc (long size)
{

  unsigned char          *buffer;
  unsigned char          *ptr;

  /* Malloc the memory */
  buffer = my_malloc (size);

  /* Zero it out */
  for (ptr = buffer; size; size--, ptr++)
    {
      *ptr = 0;
    }

  /* Return */
  return (buffer);
}


/***********************************************************************
 *                                                                      *
 *   Name                                                               *
 *      my_realloc                                                      *
 *                                                                      *
 *   Synopsis                                                           *
 *      char *my_realloc( void *buffer, long size)                      *
 *                                                                      *
 *   Description                                                        *
 *      This routine re-allocs a buffer                                 *
 *                                                                      *
 *   Revision History                                                   *
 *                                                                      *
 *     name    date          description              number            *
 ***********************************************************************/

void                   *
my_realloc (void *buffer, long size)
{
  char                   *new_buffer;

#ifdef USING_HALLOC
  HALLOC_ENTRY           *h_entry;

#endif

  if (!buffer)
    return NULL;

#ifdef USING_HALLOC
  if (size > MALLOC_CUTOFF_SIZE)
    {
      fprintf (stdout, "Realloc size too large - greater than 64K\n");
      exit (FATAL);
    }

  /* If entry is in halloc list, it is too big */
  for (h_entry = halloc_list; h_entry; h_entry = h_entry->next)
    if (buffer == h_entry->buffer_address)
      {
	fprintf (stdout, "Realloc target too large - found on halloc list\n");
	exit (FATAL);
      }
#endif

  /* Call realloc */

#if defined(WIN31) && defined(__BORLANDC__)
  if ((new_buffer = farrealloc (buffer, (unsigned long) size)) == NULL)
#else
    if ((new_buffer = realloc (buffer, (unsigned) size)) == NULL)
#endif

      {
	fprintf (stdout, "Cannot realloc enough memory\n");
	exit (FATAL);
      }

  return (new_buffer);
}






/***********************************************************************
 *                                                                      *
 *   Name                                                               *
 *      strtoupper                                                      *
 *                                                                      *
 *   Synopsis                                                           *
 *      void strtoupper(str)                                            *
 *      char *str;                                                      *
 *                                                                      *
 *   Description                                                        *
 *      This routine converts a string to uppercase.                    *
 *                                                                      *
 *   Revision History                                                   *
 *                                                                      *
 *     name    date          description              number            *
 *     mkc     3/16/89       created                  -----             *
 ***********************************************************************/

void 
strtoupper (register char *str)
{
  while (*str != '\0')
    {
      if (islower (*str))
	*str = toupper (*str);
      str++;
    }
}



/***********************************************************************
 *                                                                      *
 *   Name                                                               *
 *      port_get_long                                                   *
 *                                                                      *
 *   Synopsis                                                           *
 *      long port_get_long( char *buffer )                              *
 *                                                                      *
 *   Description                                                        *
 *      The intent here is to provide a meanss to make the value of     *
 *   bytes in an io-buffer correspond to the value of a long in memory  *
 *   while doing the io a 'long' at a time. Files written and read in   *
 *   this way are machine-independent.                                  *
 *                                                                      *
 *   Revision History                                                   *
 *                                                                      *
 *     name    date          description              number            *
 *     mkc     6/23/89       created                  -----             *
 ***********************************************************************/

long 
port_get_long (register unsigned char  * buffer)
{
  register long           w = 0;
  register int            i = BITSPERBYTE * sizeof (long);

  while ((i -= BITSPERBYTE) >= 0)
    w |= (long) ((unsigned char) *buffer++) << i;

  return (w);
}



/***********************************************************************
 *                                                                      *
 *   Name                                                               *
 *      port_put_long                                                   *
 *                                                                      *
 *   Synopsis                                                           *
 *      void port_put_long( long w, char *buffer )                      *
 *                                                                      *
 *   Description                                                        *
 *      Write out a long in a portable manner.                          *
 *                                                                      *
 *   Revision History                                                   *
 *                                                                      *
 *     name    date          description              number            *
 *     mkc     7/17/89       created                  -----             *
 ***********************************************************************/

void 
port_put_long (register long w, register unsigned char * buffer)
{
  register int            i = BITSPERBYTE * sizeof (long);

  while ((i -= BITSPERBYTE) >= 0)
    *buffer++ = (char) (w >> i);
}




/***********************************************************************
 *                                                                      *
 *   Name                                                               *
 *      port_get_short                                                  *
 *                                                                      *
 *   Synopsis                                                           *
 *      short port_get_short( char *buffer )                            *
 *                                                                      *
 *   Description                                                        *
 *      The intent here is to provide a means to make the value of      *
 *   bytes in an io-buffer correspond to the value of a short in memory *
 *   while doing the io a 'short' at a time. Files written and read in  *
 *   this way are machine-independent.                                  *
 *                                                                      *
 *   Revision History                                                   *
 *                                                                      *
 *     name    date          description              number            *
 *     mkc     6/23/89       created                  -----             *
 ***********************************************************************/

short 
port_get_short (register unsigned char * buffer)
{
  register short          w = 0;
  register int            i = BITSPERBYTE * sizeof (short);

  while ((i -= BITSPERBYTE) >= 0)
    w |= (short) ((unsigned char) *buffer++) << i;

  return (w);
}



/***********************************************************************
 *                                                                      *
 *   Name                                                               *
 *      port_put_short                                                  *
 *                                                                      *
 *   Synopsis                                                           *
 *      void port_put_short( short w, char *buffer )                    *
 *                                                                      *
 *   Description                                                        *
 *      Write out a short in a portable manner.                         *
 *                                                                      *
 *   Revision History                                                   *
 *                                                                      *
 *     name    date          description              number            *
 *     mkc     7/17/89       created                  -----             *
 ***********************************************************************/

void 
port_put_short (register short w, register unsigned char * buffer)
{
  register int            i = BITSPERBYTE * sizeof (short);

  while ((i -= BITSPERBYTE) >= 0)
    *buffer++ = (char) (w >> i);
}


/********************************************************************
	
  Name:           append_ext
	
  Function:       Appends the given extension to the filename if necessary
  NOTE: this routine assumes filename is big enough to
  contain the added extension characters
	
  Return Value:   none
	
  ********************************************************************/
void 
append_ext (char *filename, char *ext)
{

  char                   *slash;
  char                   *name_only;

  /* Skip over all directory characters */
  name_only = filename;
  slash = strrchr (filename, FILENAME_SEPARATOR);
  if (slash > name_only)
    {
      name_only = slash + 1;
    }

  /* IF (there is no "." in the actual filename) */
  if (!strchr (name_only, '.'))
    {

      /* . Append the extension (insure we have only one ".") */
      if (!strchr (ext, '.'))
	{
	  strncat (filename, ".", MAX_FILENAME_LENGTH - strlen (filename));
	}
      strncat (filename, ext, MAX_FILENAME_LENGTH - strlen (filename));

      /* ENDIF */
    }

  /* Return */
}


/********************************************************************
	
  Name:           strip_ext
	
  Function:       Strips the extension, if any from the filename
	
  Return Value:   none
	
  ********************************************************************/
void 
strip_ext (char *filename)
{

  char                   *slash;
  char                   *name_only;
  char                   *dot;

  /* Skip over all directory characters */
  name_only = filename;
  slash = strrchr (filename, FILENAME_SEPARATOR);
  if (slash > name_only)
    {
      name_only = slash + 1;
    }

  /* Terminate the string immediately before the ".", if it exists */
  if (dot = strchr (name_only, '.'))
    {
      *dot = '\0';
    }

  /* Return */
}



/********************************************************************
	
  Name:           strip_path
	
  Function:       Strips the pathname, if any from the filename
	
  Return Value:   none
	
  ********************************************************************/
void 
strip_path (char *filename)
{

  char                   *slash;
  char                   *name_only;

  /* Skip over all directory characters */
  name_only = filename;
  slash = strrchr (filename, FILENAME_SEPARATOR);
  if (slash > name_only)
    {
      name_only = slash + 1;
    }

  /* Slide the filename into the first part of the string */
  if (filename != name_only)
    {
      strcpy (filename, name_only);
    }

  /* Return */
}



/********************************************************************
	
  Name:           remove_new_lines
	
  Function:       Removes new line characters, <cr>, <lf>
  Allows unix/dos compatibility
	
  Return Value:   none
	
  ********************************************************************/

void 
remove_new_lines (char *string)
{

  /* Do it */
  while (*string)
    {
      if ((*string == '\n') || (*string == '\r'))
	{
	  *string = '\0';
	}
      string++;
    }

  /* Return */
}





/********************************************************************
	
  Name:           uns_to_hex_string
	
  Function:       Converts an UNS4X number to a hex string
	
  Return Value:   none
	
  ********************************************************************/

void 
uns_to_hex_string (void *number, char *string, size_t width)
{

  unsigned char           digit;
  unsigned char           byte;
  unsigned char           index;
  unsigned char          *char_ptr;

  /* Avoid casts */
  char_ptr = number;

  /* Convert to hex string */
  for (index = 0, byte = 0;
       byte < (unsigned char) width;
       byte++)
    {
      digit = (char_ptr[byte] & 0xf0) >> 4;
      if (digit > 9)
	{
	  string[index++] = (unsigned char) (digit - 10 + 'a');
	}
      else
	{
	  string[index++] = (unsigned char) (digit + '0');
	}
      digit = (char_ptr[byte] & 0x0f);
      if (digit > 9)
	{
	  string[index++] = (unsigned char) (digit - 10 + 'a');
	}
      else
	{
	  string[index++] = (unsigned char) (digit + '0');
	}
    }
  string[index] = '\0';

  /* Return */
}



/********************************************************************
	
  Name:           hex_string_to_uns
	
  Function:       Converts a hex string to an UNS4X
	
  Return Value:   none
	
  ********************************************************************/

void 
hex_string_to_uns (char *string, void *buffer, size_t width)
{

  unsigned short          length;
  short                   index;
  char                   *char_ptr;

  /* Avoid casts */
  char_ptr = buffer;

  /* Clear the buffer */
  for (index = 0; index < width; index++)
    {
      char_ptr[index] = 0;
    }

  /* Get the length of the string */
  length = strlen (string);

  /* Point to least significant digit in the string */
  string += length - 1;

  /* FOR (each byte in the UNS4X structure) */
  for (index = width - 1; index >= 0; index--)
    {

      /* . IF (we're still in the string) */
      if (length)
	{

	  /* . . Convert the ascii character to binary */
	  if (*string > '9')
	    {
	      char_ptr[index] = (toupper (*string) - 'A') + 10;
	    }
	  else
	    {
	      char_ptr[index] = (*string - '0');
	    }

	  /* . ELSE */
	}
      else
	{

	  /* . . Pop out of the loop */
	  break;

	  /* . ENDIF */
	}

      /* . Point to the next higher character in the string */
      string--;
      length--;

      /* . IF (we're still in the string) */
      if (length)
	{

	  /* . . Convert the ascii character to binary and sum it */
	  if (*string > '9')
	    {
	      char_ptr[index] += ((toupper (*string) - 'A') + 10) << 4;
	    }
	  else
	    {
	      char_ptr[index] += (*string - '0') << 4;
	    }

	  /* . ELSE */
	}
      else
	{

	  /* . . Pop out of the loop */
	  break;

	  /* . ENDIF */
	}

      /* . Point to the next higher character in the string */
      string--;
      length--;

      /* ENDFOR */
    }

  /* Return */
}


/********************************************************************
	
  Name:           add_40
	
  Function:       Adds two 40 bit numbers  c = a + b
	
  Return Value:   none
	
  ********************************************************************/

void 
add_40 (UNS40 * a, UNS40 * b, UNS40 * c)
{

  register unsigned short carry;

  carry = a->byte[4] + b->byte[4];
  c->byte[4] = (unsigned char) carry;
  carry = carry >> 8;
  carry = a->byte[3] + b->byte[3] + carry;
  c->byte[3] = (unsigned char) carry;
  carry = carry >> 8;
  carry = a->byte[2] + b->byte[2] + carry;
  c->byte[2] = (unsigned char) carry;
  carry = carry >> 8;
  carry = a->byte[1] + b->byte[1] + carry;
  c->byte[1] = (unsigned char) carry;
  carry = carry >> 8;
  carry = a->byte[0] + b->byte[0] + carry;
  c->byte[0] = (unsigned char) carry;
}



/********************************************************************
	
  Name:           add_48
	
  Function:       Adds two 48 bit numbers  c = a + b
	
  Return Value:   none
	
  ********************************************************************/

void 
add_48 (UNS48 * a, UNS48 * b, UNS48 * c)
{

  register unsigned short carry;

  carry = a->byte[5] + b->byte[5];
  c->byte[5] = (unsigned char) carry;
  carry = carry >> 8;
  carry = a->byte[4] + b->byte[4] + carry;
  c->byte[4] = (unsigned char) carry;
  carry = carry >> 8;
  carry = a->byte[3] + b->byte[3] + carry;
  c->byte[3] = (unsigned char) carry;
  carry = carry >> 8;
  carry = a->byte[2] + b->byte[2] + carry;
  c->byte[2] = (unsigned char) carry;
  carry = carry >> 8;
  carry = a->byte[1] + b->byte[1] + carry;
  c->byte[1] = (unsigned char) carry;
  carry = carry >> 8;
  carry = a->byte[0] + b->byte[0] + carry;
  c->byte[0] = (unsigned char) carry;
}



/********************************************************************
	
  Name:           align
	
  Function:       Aligns first value to second value
	
  Return Value:   none
	
  ********************************************************************/

void 
align (unsigned short width_1, void *b1, unsigned short width_2, void *b2)
{

  unsigned short          count;
  unsigned long          *b1_32;
  UNS40                  *b1_40;
  UNS48                  *b1_48;
  unsigned long          *b2_32;
  UNS40                  *b2_40;
  UNS48                  *b2_48;

  /* CASEOF (source width) */
  switch (width_1)
    {

      /* CASE (32 bit) */
    case 32:

      /* . Avoid casts */
      b1_32 = b1;

      /* . CASEOF (dest width) */
      switch (width_2)
	{

	  /* . CASE (32 bit) */
	case 32:

	  /* . . Straight copy */
	  b2_32 = b2;
	  *b2_32 = *b1_32;
	  break;

	  /* . CASE (40 bit) */
	case 40:

	  /* . . Left justify 32 bits */
	  b2_40 = b2;
	  b2_40->byte[4] = 0;
	  b2_40->byte[3] = (unsigned char) (*b1_32 & 0x000000ff);
	  b2_40->byte[2] = (unsigned char) ((*b1_32 & 0x0000ff00) >> 8);
	  b2_40->byte[1] = (unsigned char) ((*b1_32 & 0x00ff0000) >> 16);
	  b2_40->byte[0] = (unsigned char) ((*b1_32 & 0xff000000) >> 24);
	  break;

	  /* . CASE (48 bit) */
	case 48:

	  /* . . Left justify 32 bits */
	  b2_48 = b2;
	  b2_48->byte[5] = 0;
	  b2_48->byte[4] = 0;
	  b2_48->byte[3] = (unsigned char) (*b1_32 & 0x000000ff);
	  b2_48->byte[2] = (unsigned char) ((*b1_32 & 0x0000ff00) >> 8);
	  b2_48->byte[1] = (unsigned char) ((*b1_32 & 0x00ff0000) >> 16);
	  b2_48->byte[0] = (unsigned char) ((*b1_32 & 0xff000000) >> 24);
	  break;

	  /* . ENDCASE */
	}
      break;

      /* CASE (40 bit) */
    case 40:

      /* . Avoid casts */
      b1_40 = b1;

      /* . CASEOF (dest width) */
      switch (width_2)
	{

	  /* . CASE (32 bit) */
	case 32:

	  /* . . Kill off the LSB */
	  b2_32 = b2;
	  *b2_32 = ((unsigned long) b1_40->byte[0] << 24) +
	    ((unsigned long) b1_40->byte[1] << 16) +
	      ((unsigned long) b1_40->byte[2] << 8) +
		(unsigned long) b1_40->byte[3];
	  break;

	  /* . CASE (40 bit) */
	case 40:

	  /* . . Straight copy */
	  b2_40 = b2;
	  *b2_40 = *b1_40;
	  break;

	  /* . CASE (48 bit) */
	case 48:

	  /* . . Left justify */
	  b2_48 = b2;
	  for (count = 0; count < 5; count++)
	    {
	      b2_48->byte[count] = b1_40->byte[count];
	    }
	  b2_48->byte[5] = 0;
	  break;

	  /* . ENDCASE */
	}
      break;

      /* CASE (48 bit) */
    case 48:

      /* . Avoid casts */
      b1_48 = b1;

      /* . CASEOF (dest width) */
      switch (width_2)
	{

	  /* . CASE (32 bit) */
	case 32:

	  /* . . Kill off the LSBs */
	  b2_32 = b2;
	  *b2_32 = ((unsigned long) b1_48->byte[0] << 24) +
	    ((unsigned long) b1_48->byte[1] << 16) +
	      ((unsigned long) b1_48->byte[2] << 8) +
		(unsigned long) b1_48->byte[3];
	  break;

	  /* . CASE (40 bit) */
	case 40:

	  /* . . Kill off the LSB */
	  b2_40 = b2;
	  for (count = 0; count < 5; count++)
	    {
	      b2_40->byte[count] = b1_48->byte[count];
	    }
	  break;

	  /* . CASE (48 bit) */
	case 48:

	  /* . . Straight copy */
	  b2_48 = b2;
	  *b2_48 = *b1_48;
	  break;

	  /* . ENDCASE */
	}
      break;

      /* ENDCASE */
    }

  /* Return */
}


#if defined (SIM) || defined (CBUG) || defined (ICE)
/********************************************************************
	
  Name:           get_mem_from_file
	
  Function:       Reads a memory value from a file
	
  Return Value:   TRUE - value read, FALSE - not read
	
  ********************************************************************/

unsigned short 
get_mem_from_file (FILE * file_ptr,
		   unsigned short autowrap,
		   unsigned short format,
		   size_t width,
		   void *buffer)
{

  static char             line[BUFSIZ];
  char                    myline[BUFSIZ];
  double                  d;
  unsigned long           mant;
  static char            *delimiters = "\t\n\r ;=/";
  char                   *token;
  unsigned short          eof;

  /* Get a line that contains data from the file (ignore blank line and
   * comments are #) */
  eof = FALSE;
  do
    {
      if (!fgets (myline, BUFSIZ, file_ptr))
	{
	  if (feof (file_ptr))
	    {
	      if (eof == TRUE)
		{
		  return (FALSE);
		}
	      eof = TRUE;
	      if (autowrap)
		{
		  fseek (file_ptr, 0L, 0);
		  continue;
		}
	      else
		{
		  return (FALSE);
		}
	    }
	}
      if ((myline[0] == '0') && ((myline[1] == 'x') ||
				 (myline[1] == 'X')))
	{
	  /* hex input */
	  format = FORMAT_HEX;
	  strcpy (line, &myline[2]);
	}
      else
	strcpy (line, myline);

      if (strchr (line, '#'))
	{
	  *strchr (line, '#') = '\0';
	}
      token = strtok (line, delimiters);
      if (!token)
	{
	  continue;
	}
      if (strcmp (token, "#"))
	{
	  break;
	}
      else
	{
	  continue;
	}
    } while (1);

  /* Convert the string into the proper type (10 digits = 40, 12 = 48 else
   * 32) */
  switch (format)
    {
    case FORMAT_HEX:
      /* transform 16-bit short words to look like 32-bit normal words */
      if (width == 16)
	{
	  /* strip any leading chars beyond bottom-most 4  */
	  while (strlen (line) > 4)
	    {
	      strcpy (myline, &line[1]);
	      strcpy (line, myline);
	    }
	  width = 32;
	}

      /* add explicit leading zeros to fill out field */
      while (strlen (line) < width/4)
	{
	  strcpy (myline, "0");
	  strcat (myline, line);
	  strcpy (line, myline);
	}

      /* convert as needed */
      hex_string_to_uns (line, buffer, width/8);
      break;

    case FORMAT_FIX:
      sscanf (line, "%ld", &mant);
      switch (width)
	{
	case 16:
	  /* mask out top */
	  mant &= 0xffffL;
	  /* and fall through to 32-bit case */
	case 32:
	  port_put_long (mant, buffer);
	  break;
	default:
	  align (32, &mant, width, buffer);
	  break;
	}
      break;

    case FORMAT_FLOAT:
      sscanf (line, "%lg", &d);
      dbl_to_uns (d, buffer);
      break;
    }

  /* Return */
  return (TRUE);
}

#endif



/* This routine is used to copy a path-specified file name input arguments to
 * a tool.  The routine creates a new string, copies the argument into that
 * string, exchanging whatevery directory slashes are used for those that the
 * system likes. */

char                   *
make_my_slash (char *instring, int directory)
{
  char                   *ptr;
  char                   *base;

  base = (char *) my_malloc ((long) (strlen (instring) + 2));

  for (ptr = base; *instring; instring++)
    if (*instring == '/' || *instring == '\\')
      *ptr++ = (char) FILENAME_SEPARATOR;
    else
      *ptr++ = *instring;

  if (directory && (*(ptr - 1) != '/' && *(ptr - 1) != '\\'))
    *ptr++ = FILENAME_SEPARATOR;

  *ptr = 0;

  return base;
}

short
is_extension (char *filename)
{

  char                   *is_this_root;

  /* first get rid of directory stuff */

#if defined (MSDOS)
  if (is_this_root = strrchr (filename, (short) FILENAME_SEPARATOR))
#else
    if (is_this_root = rindex (filename, FILENAME_SEPARATOR))
#endif

      is_this_root += 1;        /* go beyond slash */
    else
      is_this_root = filename;

  /* now use to find . */

#if defined (MSDOS)
  if (is_this_root = strrchr (filename, '.'))
#else
    if (is_this_root = rindex (filename, '.'))
#endif

      return ((short) TRUE);
    else
      return ((short) FALSE);
}



/********************************************************************
	
  Name:           get_local_time
	
  Function:       Get local time as a string
	
  Return Value:   Static string
	
  *EK* Implemented to isolate MSDOS/Unix differences

  ********************************************************************/


char *
get_local_time ( void )
{

#ifdef MSDOS
  time_t time_buffer;
#else
#if 1 /*EK* BUG: */
  time_t timet;
#else
  struct timeval  timep;
  struct timezone timezp;
#endif
#endif


#ifdef MSDOS
     time (&time_buffer); 
     return ctime(&time_buffer);
#else 
#if 1 /*EK* BUG: */
    time(&timet);
    return ctime(&timet);
#else
    return ctime(&timep.tv_sec);
#endif
#endif
}



/********************************************************************
	
  Name:           get_adi_dsp
	
  Function:       Get ADI_DSP environment variable, or set reasonable
		  default
	
  Return Value:   Static string
	
  *EK* Implemented to increase sanity

  ********************************************************************/


char *
get_adi_dsp ( void )
{
    char *p;

    if( (p = getenv("ADI_DSP")) != NULL )
    {
	if ( ! *p ) return NULL; /* setting it to "" behaves as if it is
				    not set with no default. I'm not really
				    sure who would need this feature, though */
	return p;
    }

#ifdef ADI_DSP
    return ADI_DSP;     /* reasonable default */
#else
    return NULL;
#endif
}
