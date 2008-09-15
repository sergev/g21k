/* @(#)listing.c        2.1 1/13/94 2 */

#include <stdio.h>

#include "app.h"
#include "symbol.h"
#include "pass1.h"
#include "main.h"
#include "util.h"
#include "listing.h"
#include "release.h"

#define POS_ADDRESS    0    /* Print positions */
#define POS_OPCODE     8
#define POS_LINE       22
#define POS_SRC        28
#define POS_LOGO       0
#define POS_FILENAME   45
#define POS_TIME       45
#define POS_PAGE       70
#define LINES_PER_PAGE 66

#define TIME_STRING_LENGTH 26

static char buffer[BUFSIZ];                    /* output buffer */
static char page = 1;                          /* Page number */
static char lines = 0;                         /* Lines printed */
static char have_time = 0;                     /* Time flag */
static char time_string[TIME_STRING_LENGTH];   /* time string */
static void listing_output_source_line( void );
static void listing_output (void);

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      listing_banner
*                                                                      *
*   Description                                                        *
*      outputs a banner to the file                                    *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
***********************************************************************/

void listing_banner (void) {

/* Get the time if necessary */
   if (!have_time) {
     strcpy (time_string, get_local_time()); /*EK*/
     have_time = 1;
   }

/* Print banner */
   sprintf (buffer + POS_LOGO, "Analog Devices ADSP-210x0 Assembler");
   sprintf (buffer + POS_FILENAME, "%s", list_name);
   sprintf (buffer + POS_PAGE, "Page %d", page);
   listing_output ();
   sprintf (buffer + POS_LOGO, "Release %s, Version %s", get_release_number(), get_version_number());
   sprintf (buffer + POS_TIME, "%s", time_string);
   listing_output ();
   sprintf (buffer + POS_LOGO, "%s", get_copyright_notice());
   listing_output ();
   sprintf (buffer + POS_LOGO, "%s", get_license_notice() ); /*EK*/
   listing_output ();
   listing_output ();

}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      listing_new_page
*                                                                      *
*   Description                                                        *
*      makes a new page is the list file                               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
***********************************************************************/

void listing_new_page (void) {

/* Do nothing if no listing file */
   if (listing_file == NULL)
      return;

/* Update counters */
   page++;
   lines = 0;

/* Form feed */
   fprintf (listing_file, "\f");

/* Print banner */
   listing_banner ();

}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      listing_output
*                                                                      *
*   Description                                                        *
*      outputs the buffer to the file                                  *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
***********************************************************************/

static void listing_output (void) {

   short index;

/* Do nothing if no listing file */
   if (listing_file == NULL)
      return;

/* Remove any nulls or new lines in the buffer */
   for (index = 0; index < BUFSIZ; index++) {
     if ((buffer[index] == '\0') || (buffer[index] == '\n')) {
       buffer[index] = ' ';
     }
   }

/* Insert a carriage return and null after all relevent data */
   for (index = BUFSIZ - 3; index > 0; index--) {
     if (buffer[index] != ' ') {
       buffer[index + 1] = '\n';
       buffer[index + 2] = '\0';
       break;
     }
   }

/* Watch out for blank lines */
   if (index == 0) {
     buffer[index + 1] = '\n';
     buffer[index + 2] = '\0';
   }
      
/* Output the buffer to the file */
   fprintf (listing_file, "%s", buffer);

/* Clear the buffer */
   for (index = 0; index < BUFSIZ; index++) {
     buffer[index] = '\0';
   }

/* Increment line count */
   lines++;

/* See if we need a new page */
   if (lines >= LINES_PER_PAGE) {
     listing_new_page ();
   }

}

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      listing_output_instruction                                      *
*                                                                      *
*   Synopsis                                                           *
*      void listing_output_instruction( char *instruction )            *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for printing out the current offset *
*   and the opcode to the listing file.                                *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/25/89       created                  -----             *
***********************************************************************/

void listing_output_instruction( register unsigned char *instruction )
{

/* Write address and opcode to the buffer */
   sprintf( buffer + POS_ADDRESS, "%06lx", (pc->value & 0xffffff) );
   sprintf( buffer + POS_OPCODE,  "%02.2x%02.2x%02.2x%02.2x%02.2x%02.2x", instruction[0], instruction[1],
	   instruction[2], instruction[3], instruction[4], instruction[5] );

}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      listing_carriage_return()                                       *
*                                                                      *
*   Synopsis                                                           *
*      void listing_carriage_return( void )                            *
*                                                                      *
*   Description                                                        *
*      This routine is responsible for printing lines that have no     *
*   object code associated with them.                                  *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/25/89       created                  -----             *
***********************************************************************/

void listing_carriage_return( void )
{

/* Write the text to the buffer */
   listing_output_source_line();

/* Write out the buffer */
   listing_output ();

}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      listing_output_source_line                                      *
*                                                                      *
*   Synopsis                                                           *
*      void listing_output_source_line( void )                         *
*                                                                      *
*   Description                                                        *
*      This routine actually writes the assembly line to the listing   *
*   file.                                                              *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     4/25/89       created                  -----             *
***********************************************************************/

static void listing_output_source_line( void )
{
   short index;
   char read_buffer[BUFSIZ];

/* Print the line # */
   sprintf( buffer + POS_LINE, "%4d", yylineno - 1 );

/* IF (can get a line from the file) */
   if (fgets (read_buffer, BUFSIZ, duplicate_src_stream)) {

/* . Remove any newline characters */
     for (index = 0; index < BUFSIZ; index++) {
       if (read_buffer[index] == '\n') {
	 read_buffer[index] = '\0';
       }
     }

/* . Write it to the buffer */
     sprintf (buffer + POS_SRC, "%s", read_buffer);

/* ENDIF */
   }

}
