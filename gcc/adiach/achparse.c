/* $Id: achparse.c,v 2.20 1996/06/27 20:37:01 lehotsky Exp $ */

#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#ifdef __sun__
#include <memory.h>
#endif
#ifdef MSDOS
#include <stddef.h>
#include <malloc.h>
#endif
#include <stdlib.h>
#include "app.h"
#include "util.h"
#include "achparse.h"
static short ach_check_segment(ACH_TOKEN processor,unsigned long start,
                        unsigned long end, unsigned short type, unsigned long width);

typedef enum {
  ACH_STATE_NULL,
  ACH_STATE_SYSTEM,
  ACH_STATE_PROCESSOR,
  ACH_STATE_SEGMENT,
  ACH_STATE_REGISTER,
  ACH_STATE_COMPILER,
  ACH_STATE_BANK,
  ACH_STATE_ENDSYS
  } ACH_STATES;

static struct keywords {
  char *name;
  ACH_TOKEN token;
} keyword_table[] = {
  ".SYSTEM",    ACH_SYSTEM,
  ".PROCESSOR", ACH_PROCESSOR,
  ".SEGMENT",   ACH_SEGMENT,
  ".OVERLAY",   ACH_OVERLAY,
  ".BANK",	ACH_BANK,
  ".REGISTER",  ACH_REGISTER,
  ".COMPILER",  ACH_COMPILER,
  ".ENDSYS",    ACH_ENDSYS,
  "ADSP21010", ACH_ADSP21010,
  "ADSP21020", ACH_ADSP21020,
  "ADSP210Z3", ACH_ADSP210Z3,
  "ADSP21060", ACH_ADSP210Z3,
  "ADSP210Z4", ACH_ADSP210Z4,
  "ADSP21061", ACH_ADSP21061,
  "ADSP21062", ACH_ADSP210Z4,
  "BEGIN",     ACH_BEGIN,
  "END",       ACH_END,
  "CDEFAULT",  ACH_CDEFAULT,
  "CSTRINGS",  ACH_CSTRINGS,
  "CGLOBALS",  ACH_CGLOBALS,
  "CSWITCH",   ACH_CSWITCH,
  "CSTACK",    ACH_CSTACK,
  "CHEAP",     ACH_CHEAP,
  "CDATA",     ACH_CDATA,
  "CCODE",     ACH_CCODE,
  "CINIT",     ACH_CINIT,
  "UNINIT",    ACH_UNINIT,
  "PM",        ACH_PM,
  "DM",        ACH_DM,
  "RAM",       ACH_RAM,
  "ROM",       ACH_ROM,
  "PORT",      ACH_PORT,
  "RESERVED",  ACH_RESERVED_REG,
  "SCRATCH",   ACH_SCRATCH_REG,
  "CIRC",      ACH_CIRC_REG,
  "PM0",       ACH_PM0,
  "PM1",       ACH_PM1,
  "DM0",       ACH_DM0,
  "DM1",       ACH_DM1,
  "DM2",       ACH_DM2,
  "DM3",       ACH_DM3,
#ifdef ADSPZ3
  "DMU",        ACH_DMU,
#endif /* ADSPZ3 */
  "PGSIZE",    ACH_PGSIZE,
  "STORE",     ACH_STORE,
  "SWAP",      ACH_SWAP,
  "WTSTATES",  ACH_WTSTATES,
  "DRAM",      ACH_DRAM,
  "SRAM",      ACH_SRAM,
  "WTMODE",    ACH_WTMODE,
  "PGWAIT",    ACH_PGWAIT,
  "PGMODE",    ACH_PGMODE,
  "NORMAL",    ACH_NORMAL,
  "PARITYCK",  ACH_PARITY,
  "DMAPARITYCK",ACH_DMAPARITY,
  "INTERNAL",  ACH_INTERNAL,
  "EXTERNAL",  ACH_EXTERNAL,
  "EITHER",    ACH_EITHER,
  "BOTH",      ACH_BOTH,
  "PCRTS",     ACH_PCRTS,
  "RTRTS",     ACH_RTRTS,
  "DOUBLES_ARE_FLOATS", 	ACH_DAF,
  "DOUBLES_ARE_DOUBLES",  	ACH_DAD,
  "PARAMETER_PASSING_OFF",  	ACH_PPY,
  "PARAMETER_PASSING_ON",	ACH_PPN,
  "SREV_X",    ACH_SREV_X,
  "SREV_0",    ACH_SREV_X,
  "WIDTH",     ACH_WIDTH,
  "ID",	       ACH_PROCESS_ID,
#ifdef ADSPZ3
  "LONG48",    ACH_LONG48,
#endif /* ADSPZ3 */
  "EMOVERLAY", ACH_EMOVERLAY
};
ACH_TOKEN Processor_type=ACH_ADSP21020;
static unsigned long Processor_id;
static unsigned long block_0_start[100]; 
static unsigned long block_0_end[100]; 
static unsigned long block_0_line[100];
static unsigned short block_0_count=0;
static unsigned long block_0_last48[8]={0L,0L,0L,0L,0L,0L,0L,0L};
static unsigned long block_0_first32[8]={0x7fffffffL,0x7fffffffL,0x7fffffffL,0x7fffffffL,
                                         0x7fffffffL,0x7fffffffL,0x7fffffffL,0x7fffffffL};
static short block_0_first_line[8]={-1,-1,-1,-1,-1,-1,-1,-1};
static short block_0_last_line[8]={-1,-1,-1,-1,-1,-1,-1,-1};

static unsigned long block_1_start[100]; 
static unsigned long block_1_end[100]; 
static unsigned long block_1_line[100];
static unsigned short block_1_count=0;
static unsigned long block_1_last48[8]={0L,0L,0L,0L,0L,0L,0L,0L};
static unsigned long block_1_first32[8]={0x7fffffffL,0x7fffffffL,0x7fffffffL,0x7fffffffL,
                                         0x7fffffffL,0x7fffffffL,0x7fffffffL,0x7fffffffL};
static short block_1_first_line[8]={-1,-1,-1,-1,-1,-1,-1,-1};
static short block_1_last_line[8]={-1,-1,-1,-1,-1,-1,-1,-1};

/*
 * NOTE: SREV_0 and SREV_X are the same ... 
 * for backward compatibility 
 */

static int ach_process_system (void);
static int ach_process_endsys (void);
static int ach_process_processor (void);
static int ach_process_segment (void);
static int ach_process_overlay (void);
static int ach_process_bank (void);
static int ach_process_register (void);
static int ach_process_compiler (void);
static short get_address (unsigned long *address);
static ACH_TOKEN token_lookup (char *token);

static int line_number;                 /* line number being parsed */
static char err_string [BUFSIZ];        /* error message buffer */
static jmp_buf error_stack;             /* longjmp stack */
static char *delimiters = "\t ;=/";     /* token delimiters */

/********************************************************************

Name:           ach_parse

Function:       entry point into ach file parser

Return Value:   TRUE - errors encountered, FALSE - no errors

********************************************************************/

int ach_parse (char *filename) {

  char line [BUFSIZ];
  FILE *file_ptr;
  char *token;
  int state;
  short i;

/* Initialize */
   state = ACH_STATE_NULL;

/* Open the file */
   if (!(file_ptr = fopen (filename, READ_TEXT))) {
     sprintf (err_string, "Can't open architecture file %s", filename);
     ach_error (err_string);
     return (TRUE);
   }

/* reset check data */
   memset(block_0_end,0,100);
   memset(block_0_line,0,100);
   memset(block_1_end,0,100);
   memset(block_1_line,0,100);
   block_0_count=0;
   block_1_count=0;
   for (i=0;i<8;i++) {
       block_0_last48[i]=0L;
       block_1_last48[i]=0L;
       block_0_first32[i]=0x7ffffffL;
       block_1_first32[i]=0x7ffffffL;
       block_0_first_line[i]=-1;
       block_1_first_line[i]=-1;
       block_0_last_line[i]=-1;
       block_1_last_line[i]=-1;
   }

/* Set up error handling */
   if (setjmp (error_stack) != 0) {
     fclose (file_ptr);
     ach_error (err_string);
     return (TRUE);
   }

/* WHILE (more lines) */
   line_number = 0;
   while (fgets (line, BUFSIZ, file_ptr)) {

/* . Update line number */
     line_number++;

/* . Remove new lines */
     remove_new_lines (line);

/* . Break into tokens and skip blank lines */
     token = strtok (line, delimiters);
     if (!token) {
       continue;
     }

     if( (*token == '!') || (*token == '#') ) {
       continue;
     }

     strtoupper (token);

/* . CASEOF (State) */
     switch (state) {

/* . CASE (NULL) */
     case ACH_STATE_NULL:

/* . . Accept a SYSTEM directive only */
       switch (token_lookup (token)) {
       case ACH_SYSTEM:
         state = ach_process_system ();
         break;
       default:
	 sprintf (err_string, "Expecting .SYSTEM directive at line %d of architecture file.", line_number);
         longjmp (error_stack, -1);
       }
       break;

/* . CASE (SYSTEM) */
     case ACH_STATE_SYSTEM:

/* . . Accept PROCESSOR, REGISTER, SEGMENT, BANK, COMPILER or ENDSYS */
       switch (token_lookup (token)) {
       case ACH_PROCESSOR:
	 state = ach_process_processor ();
         break;
       case ACH_SEGMENT:
	 state = ach_process_segment ();
         break;
       case ACH_REGISTER:
	state = ach_process_register ();
	break;
       case ACH_BANK:
	state = ach_process_bank ();
	break;
       case ACH_COMPILER:
	 state = ach_process_compiler();
	 break;
       case ACH_ENDSYS:
	 state = ach_process_endsys ();
         break;
       default:
	 sprintf (err_string, "Expecting .PROCESSOR, .SEGMENT, .BANK, .REGISTER .COMPILER or .ENDSYS directive at line %d of architecture file.", line_number);
	 longjmp (error_stack, -1);
       }
       break;

/* . CASE (PROCESSOR) */
     case ACH_STATE_PROCESSOR:

/* . . Accept SEGMENT, BANK, REGISTER, COMPILER or ENDSYS directive */
       switch (token_lookup (token)) {
       case ACH_SEGMENT:
	 state = ach_process_segment ();
         break;
       case ACH_REGISTER:
	 state = ach_process_register ();
	 break;
       case ACH_BANK:
	 state = ach_process_bank();
	 break;
       case ACH_COMPILER:
    	 state = ach_process_compiler();
	 break;
       case ACH_ENDSYS:
	 state = ach_process_endsys ();
         break;
       default:
	 sprintf (err_string, "Expecting .SEGMENT, .BANK, .REGISTER, .COMPILER or .ENDSYS directive at line %d of architecture file.", line_number);
	 longjmp (error_stack, -1);
       }
       break;

/* . CASE (SEGMENT) */
/* . CASE (REGISTER) */
/* . CASE (BANK) */
/* . CASE (COMPILER) */
     case ACH_STATE_BANK:
     case ACH_STATE_REGISTER:
     case ACH_STATE_SEGMENT:
     case ACH_STATE_COMPILER:

/* . . Accept SEGMENT, BANK, REGISTER, COMPILER or ENDSYS directive */
       switch (token_lookup (token)) {
       case ACH_SEGMENT:
	 state = ach_process_segment ();
         break;
       case ACH_OVERLAY:
	 state = ach_process_overlay ();
         break;
       case ACH_REGISTER:
	 state = ach_process_register ();
	 break;
       case ACH_BANK:
	 state = ach_process_bank();
	 break;
       case ACH_COMPILER:
         state = ach_process_compiler();
	 break;
       case ACH_ENDSYS:
	 state = ach_process_endsys ();
         break;
       default:
	 sprintf (err_string, "Expecting .SEGMENT .BANK, .REGISTER, .COMPILER or .ENDSYS directive at line %d of architecture file.", line_number);
	 longjmp (error_stack, -1);
       }
       break;

/* . CASE (ENDSYS) */
     case ACH_STATE_ENDSYS:

/* . . Accept nothing else */
       sprintf (err_string, "Unexpected data following .ENDSYS directive at line %d of architecture file.", line_number);
       longjmp (error_stack, -1);

/* . ENDCASE */
     }

/* ENDWHILE */
   }

/* Make sure we got everything */
   if (state != ACH_STATE_ENDSYS) {
     sprintf (err_string, "Missing .ENDSYS directive at line %d of architecture file.", line_number);
     longjmp (error_stack, -1);
   }

/* Close the file */
   fclose (file_ptr);

/* Return */
   return (FALSE);
   }




/********************************************************************

Name:           ach_process_system

Function:       processes system directives

Return Value:   new state

********************************************************************/

static int ach_process_system (void) {

   char *token;

/* Get the system name */
   token = strtok (NULL, delimiters);
   ach_system (token);

/* Return */
   return (ACH_STATE_SYSTEM);
   }


/********************************************************************

Name:           ach_process_endsys

Function:       processes endsys directives

Return Value:   new state

********************************************************************/

static int ach_process_endsys (void) {

/* Call application function */
   ach_endsys ();

/* Return */
   return (ACH_STATE_ENDSYS);
   }


/********************************************************************

Name:           ach_process_processor 

Function:       processes processor directives

Return Value:   new state

********************************************************************/

static int ach_process_processor (void) {

   ACH_TOKEN token_id;
   char *token;

/* Get the processor name */
   token = strtok (NULL, delimiters);
   if ((token_id = token_lookup (token)) != ACH_BADTOKEN) {
     ach_processor (token_id);
     Processor_type=token_id;
     token = strtok (NULL, delimiters);
     if (token == (char *)NULL)
	Processor_id = 1L;
     else if ((token_lookup (token)) == ACH_PROCESS_ID) {
	if (!get_address(&Processor_id)) {
     		sprintf (err_string, "Missing or invalid processor id at line %d of architecture file.", line_number);
     		longjmp (error_stack, -1);
	}
        if ((Processor_id < 1L) || (Processor_id > 6L)) {
     		sprintf (err_string, "Processor id must be 1-6 at line %d of architecture file.", line_number);
     		longjmp (error_stack, -1);
	}
     } else {
     	sprintf (err_string, "Missing or invalid processor id at line %d of architecture file.", line_number);
     	longjmp (error_stack, -1);
     }
   } else {
     sprintf (err_string, "Missing or invalid processor type at line %d of architecture file.", line_number);
     longjmp (error_stack, -1);
   }

/* Return */
   return (ACH_STATE_PROCESSOR);
   }


/********************************************************************

Name:           ach_process_segment

Function:       processes segment directives

Return Value:   new state

********************************************************************/

static int ach_process_segment (void) {

  unsigned long begin;
  unsigned long end;
  unsigned long stack_length, heap_length;
  unsigned long width;
  unsigned short access;
  unsigned short type;
  char *label;
  char *dup_token;
  ACH_TOKEN token_value;
  char *token;
  unsigned short have_begin;
  unsigned short have_end;
  unsigned short have_access;
  unsigned short have_type;
  unsigned short have_cstack, have_cheap;
  unsigned short have_cdefault, have_cstrings, have_cglobals;
  unsigned short have_ccode, have_cdata;
  unsigned short have_cswitch;
  unsigned short have_cinit;
  unsigned short have_uninit;


/* Initialize */
   label = NULL;
   have_begin = FALSE;
   have_end = FALSE;
   have_type = FALSE;
   have_access = FALSE;
   have_cstack = FALSE;
   have_cheap = FALSE;
   have_cdefault = FALSE;
   have_cstrings = FALSE;
   have_cglobals = FALSE;
   have_cswitch = FALSE;
   have_ccode = FALSE;
   have_cdata = FALSE;
   have_cinit = FALSE;
   have_uninit = FALSE;
   width = 0L;

/* WHILE (more tokens in the line) */
   while (token = strtok (NULL, delimiters)) {

/* . IF (we can make a duplicate copy of the token) */
     if (dup_token = strdup (token)) {

/* . . Convert to upper case */
       strtoupper (dup_token);

/* . . IF (token is a descriptor) */
       if ((token_value = token_lookup (dup_token)) != ACH_BADTOKEN) {

/* . . . CASEOF (TOKEN) */
	 switch (token_value) {

/* . . . CASE (BEGIN) */
	 case ACH_BEGIN:

/* . . . . Get address */
	   if (get_address (&begin)) {
	     have_begin = TRUE;
	   }
	   break;

/* . . . CASE (END) */
	 case ACH_END:

/* . . . . Get address */
	   if (get_address (&end)) {
	     have_end = TRUE;
	   }
	   break;

/* . . . CASE (WIDTH) */
         case ACH_WIDTH:

/* . . . . Get width */
           get_address (&width);
           break;

/* . . . CASE (memory type) */
	 case ACH_DM:
           if (width == 0L)
		width =32L; 
/* . . . . Save it */
	   type = token_value;
	   have_type = TRUE;
	   break;
	 case ACH_PM:
           if (width == 0L)
		width =48L; 
/* . . . . Save it */
	   type = token_value;
	   have_type = TRUE;
	   break;

/* . . . CASE (access) */
	 case ACH_ROM:
	 case ACH_RAM:
	 case ACH_PORT:

/* . . . . Save it */
	   access = token_value;
	   have_access = TRUE;
	   break;
         
/* . . . CASE (uninitialized segment) */
         case ACH_UNINIT:
	   have_uninit = TRUE;
	   break;
#ifdef ADSPZ3
         case ACH_LONG48:
                width=48L;
                break;                          /*indicate 48bit word for Z3 int
ernal mem*/

#endif /* ADSPZ3 */

/* . . . CASE (c support) */
  	 case ACH_CSTACK:
	   have_cstack = TRUE;
	   stack_length = atol(token + strlen(token) + 2);
	   if (stack_length != 0)
		strtok(NULL, delimiters);
	   break;

	 case ACH_CHEAP:
	   have_cheap = TRUE;
	   heap_length = atol(token + strlen(token)+2);
	   if (heap_length != 0)
		strtok(NULL, delimiters);
	   break;
 
	 case ACH_CSTRINGS:
           have_cstrings = TRUE;
  	   break;

	 case ACH_CGLOBALS:
           have_cglobals = TRUE;
	   break;

         case ACH_CSWITCH:
           have_cswitch = TRUE;
           break;

	 case ACH_CDEFAULT:
	   have_cdefault = TRUE;
	   break;

         case ACH_CCODE:
	   have_ccode = TRUE;
	   break;

	 case ACH_CDATA:
	   have_cdata = TRUE;
	   break;
         
         case ACH_CINIT:
	   have_cinit = TRUE;
	   break;

/* . . . CASEELSE */
	 default :

/* . . . . What is this */
           sprintf (err_string, "Unknown descriptor '%s' following .SEGMENT at line %d of architecture file.", token, line_number);
	   longjmp (error_stack, -1);

/* . . . ENDCASE */
	 }

/* . . ELSE */
       } else {

/* . . . Assume it's a label */
	 label = token;

/* . . ENDIF */
       }

/* . . Free the duplicate value of the token */
       free (dup_token);

/* . ENDIF */
     }

/* ENDWHILE */
   }

/* Make sure we have everything needed */
   if (have_begin && have_end && have_access && have_type) {
     if (!ach_check_segment(Processor_type,begin,end,type,width)) {
        longjmp (error_stack, -1);
     }
     ach_segment (begin, end, type, access, label, have_uninit, width);
     if (have_cstack) {
	if (stack_length == 0) 
		stack_length = end - begin + 1;
	ach_cstack(stack_length, type, access, label);
	}

     if (have_cheap) {
	if (heap_length == 0)
		heap_length = end - begin + 1;
	ach_cheap(heap_length, type, access, label);
	}
     if (have_ccode)
        ach_cdefaults(ACH_CCODE, type, label);
     if (have_cdata)
        ach_cdefaults(ACH_CDATA, type, label);
     if (have_cstrings)
	ach_cdefaults(ACH_CSTRINGS, type, label);
     if (have_cglobals)
	ach_cdefaults(ACH_CGLOBALS, type, label);
     if (have_cswitch)
        ach_cdefaults(ACH_CSWITCH, type, label);
     if (have_cdefault)
	ach_cdefaults(ACH_CDEFAULT, type, label);
     if (have_cinit)
	ach_cdefaults(ACH_CINIT, type, label);
   } else {
     sprintf (err_string, "Bad .SEGMENT directive at line %d of architecture file.", line_number);
     longjmp (error_stack, -1);
   }

/* Return */
   return (ACH_STATE_SEGMENT);
   }



/********************************************************************

Name:           ach_process_bank

Function:       processes bank directives

Return Value:   new state

********************************************************************/

static int ach_process_bank (void) {

  char *dup_token;
  ACH_TOKEN token_value;
  char *token;

  unsigned short which_bank;
  int bank_index = -1;

  /* Assume we have none */
  unsigned short have_which_bank = FALSE;
  unsigned short have_pagesize   = FALSE;
  unsigned short have_waitstates = FALSE;
  unsigned short have_begin      = FALSE;
  unsigned short have_wtmode     = FALSE;
  unsigned short have_pgwait     = FALSE;
  unsigned short have_type       = FALSE;

  /* Attributes & defaults for this specific instance of ".BANK" */
  unsigned long  pagesize   = 256L;
  unsigned long  waitstates = 7L;
  unsigned long  begin      = -1L;
  unsigned short wtmode     = ACH_BOTH;
  unsigned short type       = 0;
  
  /* Keep track of all bank directives statically in support
     of multiple assignments to the same bank at different times. */
  static struct
    {
      unsigned long  pagesize;
      unsigned long  waitstates;
      unsigned long  begin;
      unsigned short wtmode;
      unsigned short type;
    } banks[6] =
      {
	{256L, 7L, -1L, ACH_BOTH, 0},
	{256L, 7L, -1L, ACH_BOTH, 0},
	{256L, 7L, -1L, ACH_BOTH, 0},
	{256L, 7L, -1L, ACH_BOTH, 0},
	{256L, 7L, -1L, ACH_BOTH, 0},
	{256L, 7L, -1L, ACH_BOTH, 0}
      };

/* WHILE (more tokens in the line) */
   while (token = strtok (NULL, delimiters)) {

/* . IF (we can make a duplicate copy of the token) */
     if (dup_token = strdup (token)) {

/* . . Convert to upper case */
       strtoupper (dup_token);

       token_value = token_lookup (dup_token);

/* . . . CASEOF (TOKEN) */
	 switch (token_value) {

/* . . . CASE (BEGIN) */
	 case ACH_BEGIN:

/* . . . . Get address */
	   if (get_address (&begin)) {
	     have_begin = TRUE;
	   }
	   break;

/* . . . CASE (memory type) */
#ifndef ADSPZ3
	 case ACH_PM0:
	 case ACH_PM1:
#endif /* ADSPZ3 */
	 case ACH_DM0:
	 case ACH_DM1:
	 case ACH_DM2:
	 case ACH_DM3:
#ifdef ADSPZ3
	 case ACH_DMU:
#endif /* ADSPZ3 */

/* . . . . Save it */
	   which_bank = token_value;
	   bank_index = token_value - ACH_PM0;
	   have_which_bank = TRUE;
	   break;

/* . . . CASE (page wait) */
	 case ACH_PGWAIT:
	  	have_pgwait = TRUE;
		break;

/* . . . CASE (access) */
	 case ACH_DRAM:
         case ACH_SRAM:

/* . . . . Save it */
	   type = token_value;
	   have_type = TRUE;
	   break;

/* . . . CASE (page size) */
	 case ACH_PGSIZE:

/* . . . . Get page size */
	   if (get_address (&pagesize)) {
	     switch ((unsigned int)pagesize) {
	     case 256:
	     case 512:
	     case 1024:
	     case 2048:
	     case 4096:
	     case 8192:
	     case 16384:
	     case 32768:
	       have_pagesize = TRUE;
	       break;
	     default:
	       sprintf(err_string, "Invalid page size specified at line %d of architecture file.", line_number);
	       longjmp(error_stack, -1);
	       break;
	     }
	   }
	   break;
	
/* . . . CASE (wait states) */
	 case ACH_WTSTATES:

/* . . . . Get number of wait states */
	   if (get_address (&waitstates))
	     {
	       if( waitstates > 7)
		 {
		   sprintf(err_string, "Invalid number of waitstates specified at line %d of architecture file.", line_number);
		   longjmp(error_stack, -1);
		 }
	       have_waitstates = TRUE;
	     }
	   break;

/* . . . CASE (wait mode) */
	 case ACH_WTMODE:
	   token = strtok(NULL, delimiters);
	   strtoupper(token);
	   switch( token_value = token_lookup(token)) {
		case ACH_INTERNAL:
		case ACH_EXTERNAL:
		case ACH_BOTH:
		case ACH_EITHER:
		  wtmode = token_value;
		  have_wtmode = TRUE;
		  break;

		default:
		  sprintf(err_string, "Unknown descriptor '%s' following WTMODE at line %d of architecture file.", token, line_number);
		  longjmp(error_stack, -1);
	 	  break;
		}
	   break;

/* . . . CASEELSE */
	 default :

/* . . . . What is this */
           sprintf (err_string, "Unknown descriptor '%s' following .BANK at line %d of architecture file.", token, line_number);
	   longjmp (error_stack, -1);

/* . . . ENDCASE */
	 }

/* . . Free the duplicate value of the token */
       free (dup_token);

/* . ENDIF */
     }

/* ENDWHILE */
   }


/* Make sure we have everything needed */
   if (have_which_bank && bank_index > -1 && bank_index < 6
       && (have_begin || have_pagesize || have_waitstates || have_type || have_wtmode))
     {
       /* Only update those attributes we have been given */
       if (have_pagesize)   banks[bank_index].pagesize = pagesize;
       if (have_waitstates) banks[bank_index].waitstates = waitstates;
       if (have_begin)      banks[bank_index].begin = begin;
       if (have_wtmode)     banks[bank_index].wtmode = wtmode;
       if (have_type)       banks[bank_index].type = type;

       /* Always distatch from the static array */
       ach_bank (banks[bank_index].begin,
		 banks[bank_index].pagesize,
		 banks[bank_index].waitstates,
		 banks[bank_index].type,
		 banks[bank_index].wtmode,
		 which_bank,
		 have_pgwait);
     }
   else
     {
       sprintf (err_string,
		"Bad .BANK directive at line %d of architecture file.", line_number);
       longjmp (error_stack, -1);
     }

/* Return */
   return (ACH_STATE_BANK);
   }



/********************************************************************
 
Name:           ach_process_compiler
 
Function:       processes compiler directives
 
Return Value:   new state
 
********************************************************************/
static int ach_process_compiler (void) 
{
  
  short	map_doubles_to_float, jjb, param_passing;
  short	chip_rev;
  char 	*token, *dup_token;
  ACH_TOKEN	token_value;
  
  /*	INITIALIZE */
  map_doubles_to_float = 0;
  jjb = 0;
  param_passing = 0;
  chip_rev = 0;
  
  /* WHILE (more tokens in the line) */
  while (token = strtok (NULL, delimiters)) {
    
    /* . IF (we can make a duplicate copy of the token) */
    if (dup_token = strdup (token)) {
      
      /* . . Convert to upper case */
      strtoupper (dup_token);
      
      /* . . Lookup token in list */
      token_value = token_lookup (dup_token);
      
      /* . . CASEOF (TOKEN) */
      switch (token_value) 
	{
	  /* . . . CASE (DOUBLES_ARE_DOUBLES) */
	case ACH_DAD:
	  sprintf(err_string,"DOUBLES_ARE_DOUBLES is now the only supported option, line %d",
		  line_number);
	  longjmp(error_stack, -1);
	  map_doubles_to_float = -1;
	  break;
	  
	  /* . . . CASE (DOUBLES_ARE_FLOATS) */
	case ACH_DAF:
	  sprintf(err_string,"DOUBLES_ARE_FLOATS is no longer supported, line %d",line_number);
	  longjmp(error_stack, -1);
	  map_doubles_to_float = 1;
	  break;
	  
	  /* . . . CASE (PCRTS) */
	case ACH_PCRTS:
	  sprintf(err_string,"PCRTS is no longer supported, line %d",line_number);
	  longjmp(error_stack, -1);
	  jjb = -1;
	  break;
	  
	  /* . . . CASE (RTRTS) */
	case ACH_RTRTS:
	  sprintf(err_string,"RTRTS is now the only supported option, line %d",line_number);
	  longjmp(error_stack, -1);
	  jjb = 1;
	  break;
	  
	  /* . . . CASE (PARAMETER_PASSING_ON) */
	case ACH_PPY:
	  sprintf(err_string, 
		  "Parameter passing on the stack is no longer supported, line %d",
		  line_number);
	  longjmp(error_stack, -1);
	  param_passing = 1;
	  break;
	  
	  /* . . . CASE (PARAMETER_PASSING_OFF) */
	case ACH_PPN:
	  sprintf(err_string,
		  "Parameter passing in registers is now the only supported option, line %d",
		  line_number);
	  longjmp(error_stack, -1);
	  param_passing = -1;
	  break;
	  
	  /* . . . CASE (X_GRADE CHIP) */
	  /* . . . also called REV 0 */
	case ACH_SREV_X:
	  chip_rev = 1;
	  break;
	  
	  /* . . . CASEELSE */
	  default :
	    
	    /* . . . . What is this */
	    sprintf (err_string, 
		     "Unknown descriptor '%s' following .COMPILER at line %d of architecture file.", 
		     token, line_number);
	  longjmp (error_stack, -1);
	  
	  /* . . . ENDCASE */
	}
      
      /* . . Free the duplicate value of the token */
      free (dup_token);
      
      /* . ENDIF */
    }
    
    /* ENDWHILE */
  }
  
  /* Report to tool */
  ach_compiler(map_doubles_to_float, jjb, param_passing, chip_rev);
  
  /* Return */
  return (ACH_STATE_COMPILER);
}

/********************************************************************

Name:           ach_process_register

Function:       processes register directives

Return Value:   new state

********************************************************************/

static int ach_process_register (void) {

    char *token, *dag_string, *dup_token;
    void (*(ach_register_function))(char *token);
    short circular_def = FALSE;
    ACH_TOKEN token_value;

    token = strtok(NULL, delimiters);
    if (token == NULL) {
           sprintf (err_string, "Expecting /CIRC, /SCRATCH or /RESERVED following .REGISTER directive at line %d.", line_number);
	   longjmp (error_stack, -1);
    }

	strtoupper(token);
	token_value = token_lookup(token);
	if (token_value == ACH_SCRATCH_REG)
	  {
	    sprintf(err_string, "Optional scratch registers no longer supported -- line %d.", 
		    line_number);
	    longjmp(error_stack, -1);
	    ach_register_function = ach_scratch_register;
	  }
	else 
	  if (token_value == ACH_RESERVED_REG)
	    ach_register_function = ach_reserved_register;
	else 
	if (token_value == ACH_CIRC_REG) 
	  {
	    ach_register_function = ach_reserved_register;
	    circular_def = TRUE;
	  }
	else {
           sprintf (err_string, "Expecting /CIRC, /SCRATCH or /RESERVED following .REGISTER directive at line %d.", line_number);
	   longjmp (error_stack, -1);
	}

    while (token = strtok(NULL, "= \t,;")) {
	strtoupper(token);
	switch (*token) {
	    case 'R':
	    case 'F':
	    case 'B':
	    case 'I':
	    case 'M':
	    case 'L':
		if (( *(token+1) < '0' ) || ( *(token+1) > '9')) 
		{ sprintf(err_string, "Unknown register used in .REGISTER directive at line %d.", line_number);
		  longjmp(error_stack, -1);
		}

		if ((*(token+2)) && ((*(token+2) < '0') || (*(token+2) >'5')))
		{ sprintf(err_string, "Unknown register used in .REGISTER directive at line %d.", line_number);
		  longjmp(error_stack, -1);
		}

		ach_register_function(token);
		break;
	    
	    case 'D':
		if (strncmp(token, "DAG", 3) !=0 )
		{ sprintf(err_string, "Unknown register used in .REGISTER directive at line %d.", line_number);
		  longjmp(error_stack, -1);
		}

		if (( *(token+3) < '0' ) || ( *(token+3) > '9')) 
		{ sprintf(err_string, "Unknown register used in .REGISTER directive at line %d.", line_number);
		  longjmp(error_stack, -1);
		}

		if ((*(token+4)) && ((*(token+4) < '0') || (*(token+4) >'5')))
		{ sprintf(err_string, "Unknown register used in .REGISTER directive at line %d.", line_number);
		  longjmp(error_stack, -1);
		}

		ach_register_function(token);
		if (circular_def == TRUE) {
			dag_string=strdup(token);
			if ((token = strtok (NULL, " \t;")) != NULL) {
     				if ((dup_token = strdup (token)) != NULL) {
       					strtoupper (dup_token);
       					if ((token_value = token_lookup (dup_token)) == ACH_BADTOKEN) {
						ach_circular_register(dag_string, token);
						free(dag_string);
						free(dup_token);
						return(ACH_STATE_REGISTER);
					}
				}	
			}
			sprintf(err_string, "Variable name expected after DAG in .REGISTER directive at line %d.", \
				line_number);
		}
		break;
	    default:
		sprintf(err_string, "Unknown register used in .REGISTER directive at line %d.", line_number);
		longjmp(error_stack, -1);
		break;
	}
    }
    return(ACH_STATE_REGISTER);
}

/********************************************************************

Name:           get_address

Function:       gets an address from the token list

Return Value:   FALSE - no address found, ANYTHING ELSE - address found

********************************************************************/

static short get_address (unsigned long *address) {

   char *token;

/* Get the address */
   token = strtok (NULL, delimiters);

/* Convert to hex if necessary */
   if ((*token == '0') && ((*(token + 1) == 'x') || (*(token + 1) == 'X'))) {
     token += 2;
     return (sscanf (token, "%lx", address));
   } else {
     return (sscanf (token, "%lu", address));
   }

/* Return */
   return (FALSE);
   }



/********************************************************************

Name:           token_lookup

Function:       looks up tokens from keyword table 

Return Value:   token id

********************************************************************/

static ACH_TOKEN token_lookup (char *token) {

   int index;

/* See if the token is in the table */
   for (index = 0; index < sizeof (keyword_table)/sizeof (struct keywords); index++) {
     if (!strcmp (token, keyword_table[index].name)) {
       return (keyword_table[index].token);
     }
   }
   return (ACH_BADTOKEN);
 }


/********************************************************************
 
Name:           ach_find_stack
 
Function:       entry point for a fast stack locator
 
Return Value:   ACH_DM for dm stack (default), ACH_PM for pm stack 
 
********************************************************************/
 
int ach_find_stack (char *filename) {
 
  char line [BUFSIZ];
  FILE *file_ptr;
  char *char_ptr;
  char *token;
  int token_number;
  int stack_location;
  int stack_found;

    
/* Initialize */
   stack_found = FALSE;
 
/* Open the file */
   if (!(file_ptr = fopen (filename, READ_TEXT))) {
     sprintf (err_string, "Can't open architecture file %s", filename);
     ach_error (err_string);
     return (ACH_DM);
   } 
 
/* WHILE (more lines) */
   while (fgets (line, BUFSIZ, file_ptr)) {
 
/* . DOS/UNIX compatibility */
     for (char_ptr = line; *char_ptr; char_ptr++) {
       if (*char_ptr == '\n') {
         *char_ptr = '\0';
       }
     }   
       
/* . Break into tokens and skip blank lines */
     token = strtok (line, delimiters);
     if (!token) {
       continue;
     }
     strtoupper (token);
 
/* . CASEOF (State) */
     switch (token_lookup(token)) {
 
/* . CASE (segment) */
     case ACH_SEGMENT:
/* . . Scan through segment, check for location and possible cstack */
	while( token = strtok(NULL, delimiters)) {
		strtoupper(token);
		switch( token_number = token_lookup(token)) {
			case ACH_PM:
			case ACH_DM:
				stack_location = token_number;
				break;

			case ACH_CSTACK:
				stack_found = TRUE;
				break;
			
			default:
				break;
			}
		}
     default:
	break;
     }
   if (stack_found) {
      fclose(file_ptr);
      return(stack_location);
      }
   }

  fclose(file_ptr);
  return(ACH_DM);		/*default location*/
}
/********************************************************************

Name:           ach_check_segment

Function:       checks memory alignment for Z series parts 

Return Value:   TRUE=ok,FALSE = bad 

********************************************************************/
short ach_check_segment(ACH_TOKEN processor,unsigned long start,
                        unsigned long end, unsigned short type, unsigned long width)
{
    short block;
    short short_word;
    short unsigned i;
    unsigned long column_height;
    unsigned long max_pm_block;
    unsigned long n,start_data32;
    unsigned short m;
    short first_line,last_line;
    unsigned long real_start,real_end;
    unsigned long alias_start,alias_end;
    short mem_space,loop,block_mem;

    short_word = FALSE;
/* make sure that this a Z series part */

    if ((processor != ACH_ADSP210Z3) && (processor !=  ACH_ADSP210Z4) &&
        (processor != ACH_ADSP21061))
	return(TRUE); /* memory check for Z parts only */

/* check for bogus width */
    if ((width !=0L) && (width !=16L) && (width !=32L) && (width !=40L) &&
        (width !=48L)) {
        sprintf (err_string,"Legal values for width are 16|32|40|48 only at line %d in architecture file\n",line_number);
	return(FALSE);
    }

/* see if we are in internal memory */
    if (start >=EXTERNAL_MEMORY_START) {
	if ((type == ACH_PM) && (end >EXT_PM_END)) {
            sprintf (err_string,"%lx is max pm address at line %d in architecture file\n",(long)EXT_PM_END,line_number);
	    return(FALSE);
	} else 
	    return(TRUE); /* memory check for internal memory */
    }
/* adjust the memory space if in multiprocessor */

    /* keep track of real address for memory conflicts */
    real_start = start;
    real_end = end;
    mem_space = 0; /* local */
    if ((start >= MULTISPACE_ID1_START) && (start <= MULTISPACE_ID1_END)) {
	start -=MULTISPACE_ID1_START;
	end -=MULTISPACE_ID1_START;
        mem_space = 1;
	if (Processor_id == 1) {
	    real_start=start;
	    real_end=end;
	}
    } else if ((start >= MULTISPACE_ID2_START) && (start <= MULTISPACE_ID2_END)) {
	start -=MULTISPACE_ID2_START;
	end -=MULTISPACE_ID2_START;
        mem_space = 2;
	if (Processor_id == 2) {
	    real_start=start;
	    real_end=end;
	}
    } else if ((start >= MULTISPACE_ID3_START) && (start <= MULTISPACE_ID3_END)) {
	start -=MULTISPACE_ID3_START;
	end -=MULTISPACE_ID3_START;
        mem_space = 3;
	if (Processor_id == 3) {
	    real_start=start;
	    real_end=end;
	}
    } else if ((start >= MULTISPACE_ID4_START) && (start <= MULTISPACE_ID4_END)) {
	start -=MULTISPACE_ID4_START;
	end -=MULTISPACE_ID4_START;
        mem_space = 4;
	if (Processor_id == 4) {
	    real_start=start;
	    real_end=end;
	}
    } else if ((start >= MULTISPACE_ID5_START) && (start <= MULTISPACE_ID5_END)) {
	start -=MULTISPACE_ID5_START;
	end -=MULTISPACE_ID5_START;
        mem_space = 5;
	if (Processor_id == 5) {
	    real_start=start;
	    real_end=end;
	}
    } else if ((start >= MULTISPACE_ID6_START) && (start <= MULTISPACE_ID6_END)) {
	start -=MULTISPACE_ID6_START;
	end -=MULTISPACE_ID6_START;
        mem_space = 6;
	if (Processor_id == 6) {
	    real_start=start;
	    real_end=end;
	}
    }
/* also adjust memory if in broadcast area */
    if ((start >= MULTISPACE_ALL_START) && (start <= MULTISPACE_ALL_END)) {
	    start -=MULTISPACE_ALL_START;
	    end -=MULTISPACE_ALL_START;
            mem_space = 7;
	    real_start = start;
	    real_end = end;
    }
/* weed out illegal addresses */
    if (start <BLOCK0_START_060) {
        sprintf (err_string,"Reserved or IOP register area at line %d in architecture file\n",line_number);
	 return(FALSE);
    }


/* Now figure out which block we are in based on processor type */
    if (processor == ACH_ADSP210Z3) {
        column_height = COLUMN_HEIGHT_060;
        max_pm_block = MAX_PM_BLOCK_060;
	if (start >= BLOCK1_SHORT_START_060) {
           if (end > BLOCK1_SHORT_END_060) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width != 16L)) {
              sprintf (err_string,"Width must be 16 for short word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
           short_word = TRUE;
	   start -= (long)BLOCK1_SHORT_START_060; /* get relative address */
	   end -= (long)BLOCK1_SHORT_START_060;
        } else if (start >= BLOCK0_SHORT_START_060) {
           if (end > BLOCK0_SHORT_END_060) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
	   }
           if ((width > 0L) && (width != 16L)) {
              sprintf (err_string,"Width must be 16 for short word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 0;
           short_word = TRUE;
	   start -= (long)BLOCK0_SHORT_START_060; /* get relative address */
	   end -= (long)BLOCK0_SHORT_START_060;
        } else if (start >= BLOCK1_START_060) {
           if (end > BLOCK1_END_060) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
	   }
           if ((width > 0L) && (width == 16L)) {
              sprintf (err_string,"Width cannot be 16 for normal word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
	   start -= (long)BLOCK1_START_060; /* get relative address */
	   end -= (long)BLOCK1_START_060;
        } else  { /* block 0 normal */
           if (end > BLOCK0_END_060) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
	   }
           if ((width > 0L) && (width == 16L)) {
              sprintf (err_string,"Width cannot be 16 for normal word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 0;
	   start -= (long)BLOCK0_START_060; /* get relative address */
	   end -= (long)BLOCK0_START_060;
        }
     } else if (processor == ACH_ADSP210Z4) { /* 21062, 2 Mbit */
        column_height = COLUMN_HEIGHT_062;
        max_pm_block = MAX_PM_BLOCK_062;
	if (start >= ALIAS_SHORT_START2_062) {
           if (end > ALIAS_SHORT_END2_062) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width != 16L)) {
              sprintf (err_string,"Width must be 16 for short word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
           short_word = TRUE;
	   start -= (long)ALIAS_SHORT_START2_062; /* get relative address */
	   end -= (long)ALIAS_SHORT_START2_062;
        } else if (start >= ALIAS_SHORT_START1_062) {
           if (end > ALIAS_SHORT_END1_062) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width != 16L)) {
              sprintf (err_string,"Width must be 16 for short word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
           short_word = TRUE;
	   start -= (long)ALIAS_SHORT_START1_062; /* get relative address */
	   end -= (long)ALIAS_SHORT_START1_062;
        } else if (start >= BLOCK1_SHORT_START_062) {
           if (end > BLOCK1_SHORT_END_062) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width != 16L)) {
              sprintf (err_string,"Width must be 16 for short word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
           short_word = TRUE;
	   start -= (long)BLOCK1_SHORT_START_062; /* get relative address */
	   end -= (long)BLOCK1_SHORT_START_062;
        } else if (start >= BLOCK0_SHORT_START_062) {
           if (end > BLOCK0_SHORT_END_062) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width != 16L)) {
              sprintf (err_string,"Width must be 16 for short word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 0;
           short_word = TRUE;
	   start -= (long)BLOCK0_SHORT_START_062; /* get relative address */
	   end -= (long)BLOCK0_SHORT_START_062;
        } else if (start >= ALIAS_START2_062) {
           if (end > ALIAS_END2_062) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width == 16L)) {
              sprintf (err_string,"Width cannot be 16 for normal word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
	   start -= (long)ALIAS_START2_062; /* get relative address */
	   end -= (long)ALIAS_START2_062;
        } else if (start >= ALIAS_START1_062) {
           if (end > ALIAS_END1_062) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width == 16L)) {
              sprintf (err_string,"Width cannot be 16 for normal word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
	   start -= (long)ALIAS_START1_062; /* get relative address */
	   end -= (long)ALIAS_START1_062;
        } else if (start >= BLOCK1_START_062) {
           if (end > BLOCK1_END_062) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width == 16L)) {
              sprintf (err_string,"Width cannot be 16 for normal word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
	   start -= (long)BLOCK1_START_062; /* get relative address */
	   end -= (long)BLOCK1_START_062;
        } else { /* must be block 0 */
           if (end > BLOCK0_END_062) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width == 16L)) {
              sprintf (err_string,"Width cannot be 16 for normal word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 0;
	   start -= (long)BLOCK0_START_062; /* get relative address */
	   end -= (long)BLOCK0_START_062;
        }
    } else if (processor == ACH_ADSP21061) { /* 1 Mbit part */
        column_height = COLUMN_HEIGHT_061;
        max_pm_block = MAX_PM_BLOCK_061;
	if (start >= ALIAS_SHORT_START6_061) {
           if (end > ALIAS_SHORT_END6_061) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width != 16L)) {
              sprintf (err_string,"Width must be 16 for short word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
           short_word = TRUE;
	   start -= (long)ALIAS_SHORT_START6_061; /* get relative address */
	   end -= (long)ALIAS_SHORT_START6_061;
	} else if (start >= ALIAS_SHORT_START5_061) {
           if (end > ALIAS_SHORT_END5_061) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width != 16L)) {
              sprintf (err_string,"Width must be 16 for short word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
           short_word = TRUE;
	   start -= (long)ALIAS_SHORT_START5_061; /* get relative address */
	   end -= (long)ALIAS_SHORT_START5_061;
	} else if (start >= ALIAS_SHORT_START4_061) {
           if (end > ALIAS_SHORT_END4_061) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width != 16L)) {
              sprintf (err_string,"Width must be 16 for short word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
           short_word = TRUE;
	   start -= (long)ALIAS_SHORT_START4_061; /* get relative address */
	   end -= (long)ALIAS_SHORT_START4_061;
	} else if (start >= ALIAS_SHORT_START3_061) {
           if (end > ALIAS_SHORT_END3_061) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width != 16L)) {
              sprintf (err_string,"Width must be 16 for short word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
           short_word = TRUE;
	   start -= (long)ALIAS_SHORT_START3_061; /* get relative address */
	   end -= (long)ALIAS_SHORT_START3_061;
	} else if (start >= ALIAS_SHORT_START2_061) {
           if (end > ALIAS_SHORT_END2_061) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width != 16L)) {
              sprintf (err_string,"Width must be 16 for short word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
           short_word = TRUE;
	   start -= (long)ALIAS_SHORT_START2_061; /* get relative address */
	   end -= (long)ALIAS_SHORT_START2_061;
	} else if (start >= ALIAS_SHORT_START1_061) {
           if (end > ALIAS_SHORT_END1_061) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width != 16L)) {
              sprintf (err_string,"Width must be 16 for short word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
           short_word = TRUE;
	   start -= (long)ALIAS_SHORT_START1_061; /* get relative address */
	   end -= (long)ALIAS_SHORT_START1_061;
	} else if (start >= BLOCK1_SHORT_START_061) {
           if (end > BLOCK1_SHORT_END_061) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width != 16L)) {
              sprintf (err_string,"Width must be 16 for short word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
           short_word = TRUE;
	   start -= (long)BLOCK1_SHORT_START_061; /* get relative address */
	   end -= (long)BLOCK1_SHORT_START_061;
	} else if (start >= BLOCK0_SHORT_START_061) {
           if (end > BLOCK0_SHORT_END_061) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width != 16L)) {
              sprintf (err_string,"Width must be 16 for short word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 0;
           short_word = TRUE;
	   start -= (long)BLOCK0_SHORT_START_061; /* get relative address */
	   end -= (long)BLOCK0_SHORT_START_061;
	} else if (start >= ALIAS_START6_061) {
           if (end > ALIAS_END6_061) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width == 16L)) {
              sprintf (err_string,"Width cannot be 16 for normal word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
	   start -= (long)ALIAS_START6_061; /* get relative address */
	   end -= (long)ALIAS_START6_061;
	} else if (start >= ALIAS_START5_061) {
           if (end > ALIAS_END5_061) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width == 16L)) {
              sprintf (err_string,"Width cannot be 16 for normal word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
	   start -= (long)ALIAS_START5_061; /* get relative address */
	   end -= (long)ALIAS_START5_061;
	} else if (start >= ALIAS_START4_061) {
           if (end > ALIAS_END4_061) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width == 16L)) {
              sprintf (err_string,"Width cannot be 16 for normal word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
	   start -= (long)ALIAS_START4_061; /* get relative address */
	   end -= (long)ALIAS_START4_061;
	} else if (start >= ALIAS_START3_061) {
           if (end > ALIAS_END3_061) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width == 16L)) {
              sprintf (err_string,"Width cannot be 16 for normal word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
	   start -= (long)ALIAS_START3_061; /* get relative address */
	   end -= (long)ALIAS_START3_061;
	} else if (start >= ALIAS_START2_061) {
           if (end > ALIAS_END2_061) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width == 16L)) {
              sprintf (err_string,"Width cannot be 16 for normal word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
	   start -= (long)ALIAS_START2_061; /* get relative address */
	   end -= (long)ALIAS_START2_061;
	} else if (start >= ALIAS_START1_061) {
           if (end > ALIAS_END1_061) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width == 16L)) {
              sprintf (err_string,"Width cannot be 16 for normal word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
	   start -= (long)ALIAS_START1_061; /* get relative address */
	   end -= (long)ALIAS_START1_061;
	} else if (start >= BLOCK1_START_061) {
           if (end > BLOCK1_END_061) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width == 16L)) {
              sprintf (err_string,"Width cannot be 16 for normal word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 1;
	   start -= (long)BLOCK1_START_061; /* get relative address */
	   end -= (long)BLOCK1_START_061;
	} else { /* must be block 0 */
           if (end > BLOCK0_END_061) {
              sprintf (err_string,"Segment goes over block boundary at line %d in architecture file\n",line_number);
	      return(FALSE);
           }
           if ((width > 0L) && (width == 16L)) {
              sprintf (err_string,"Width cannot be 16 for normal word data area -- line %d in architecture file \n",line_number);
	      return(FALSE);
           }
	   block = 0;
	   start -= (long)BLOCK0_START_061; /* get relative address */
	   end -= (long)BLOCK0_START_061;
	}
     }
     /* now check for memory conflicts between segments */
     /* use real addresses for conflicts (only diff from start and end when multi-processsor */
     /* also total the number of pm (48 bit) words per block */
     if (short_word) {
	real_start /=2;
	real_end /=2;
     }
     if (block == 0) {
         for (i=0;i<block_0_count;i++) {
		if (((real_start >= block_0_start[i]) && (real_start <= block_0_end[i])) ||
                    ((real_start < block_0_start[i]) && (real_end >= block_0_start[i]))) {
		    sprintf (err_string,"Memory conflict: line %d conflicts with line %d in architecture file\n",
                            line_number,block_0_line[i]);
		    return(FALSE);
		}
	}
        block_0_start[block_0_count] = real_start;
        block_0_end[block_0_count] = real_end;
        block_0_line[block_0_count] = line_number;
        block_0_count++;
        /* maintain highest 48 bit access and lowest 32 access */
        if ((width ==40L) || (width ==48L)) {
	    if (end > block_0_last48[mem_space]) {
		block_0_last48[mem_space] = end;
                block_0_last_line[mem_space] = line_number;
            }
        } else { /* 16,32 data */
	    if (start < block_0_first32[mem_space]) {
		block_0_first32[mem_space] = start;
                block_0_first_line[mem_space] = line_number;
            }
        }
     } else { /* block 1 */
         for (i=0;i<block_1_count;i++) {
		if (((real_start >= block_1_start[i]) && (real_start <= block_1_end[i])) ||
                    ((real_start < block_1_start[i]) && (real_end >= block_1_start[i]))) {
		    sprintf (err_string,"Memory conflict: line %d conflicts with line %d in architecture file\n",
                            line_number,block_1_line[i]);
		    return(FALSE);
		}
	}
        /* now check for alias conflicts */
        if (short_word) {
	    alias_start = start/2;
	    alias_end = end/2;
	} else {
	    alias_start = start;
	    alias_end = end;
        }
        alias_start += BLOCK1_START_062;
        alias_end += BLOCK1_START_062;
	if (Processor_type == ACH_ADSP210Z4) { /* 21062, 2Mbit */
	    for (i=0;i<block_1_count;i++) {
		if (((alias_start >= block_1_start[i]) && 
                    (alias_start <= block_1_end[i])) ||
                    ((alias_start < block_1_start[i]) && 
                    (alias_end+BLOCK1_START_062 >= block_1_start[i]))) {
		    sprintf (err_string,"Memory conflict: line %d conflicts with line %d in architecture file\n",
                            line_number,block_1_line[i]);
		    return(FALSE);
		}
             }
        } else if (Processor_type == ACH_ADSP21061) {
            if (short_word) {
	        alias_start = start/2;
	        alias_end = end/2;
	    } else {
	        alias_start = start;
		alias_end = end;
	    }
            alias_start += BLOCK1_START_061;
            alias_end += BLOCK1_START_061;
	    for (i=0;i<block_1_count;i++) {
                if (((alias_start >= block_1_start[i]) &&
                    (alias_start <= block_1_end[i])) ||
                    ((alias_start < block_1_start[i]) &&
                    (alias_end >= block_1_start[i]))) {
                    sprintf (err_string,"Memory conflict: line %d conflicts with line %d in architecture file\n",
                            line_number,block_1_line[i]);
                    return(FALSE);
                }
             }
        }
        block_1_start[block_1_count] = real_start;
        block_1_end[block_1_count] = real_end;
        block_1_line[block_1_count] = line_number;
        block_1_count++;
        /* maintain highest 48 bit access and lowest 32 access */
        if ((width ==40L) || (width ==48L)) {
	    if (end > block_1_last48[mem_space]) {
		block_1_last48[mem_space] = end;
                block_1_last_line[mem_space] = line_number;
            }
        } else { /* 16,32 data */
	    if (start < block_1_first32[mem_space]) {
		block_1_first32[mem_space] = start;
                block_1_first_line[mem_space] = line_number;
            }
        }
     }
     /* check address of 48 or 40 bit words per block */
     if (((width ==40L) || (width ==48L)) && (end >= max_pm_block)) {
        sprintf (err_string,"Max number of 40 or 48 bit words in block is %ld, line %d in architecture file\n",max_pm_block,line_number);
        return(FALSE);
     }
     /*********************************************************************/
     /* now check to see if there is a 48 -- 32 bit memory space conflict */
     /*                                                                   */
     /* (n-1)/CH          start contig          range non-contig          */
     /* ----------        ------------          ----------------          */
     /*                                                                   */
     /*     0             B+CH+m+1              (B+n) to (CH-1)           */
     /*     1             B+2CH+m+1                    -                  */
     /*     2             B+4CH+m+1             (B+3ch+n) to 4CH-1        */
     /*     3             B+5CH+m+1                    -                  */
     /*     4             B+7CH+m+1             (B+6ch+n) to 7CH-1        */
     /*                                                                   */
     /*                                                                   */
     /* key: m=(n-1) mod CH, CH = column height = 8k 060,4k 062,2k 061    */
     /*      B = block base, n = number of words                          */
     /*                                                                   */
     /* note that our calculations do NOT use the block base, since it has*/
     /* already been chopped out of our addresses                         */
     /*********************************************************************/

     /* n, number of words is block_?_last48+1                            */
     
/* loop -- once for current block, once each for common area versus a block */
  block_mem=0;
  for (loop=0;loop<7;loop++) {
   if (loop == 0) {
     /* current block */
     if (block == 0) { 
        n = block_0_last48[mem_space]+1;
        start_data32 = block_0_first32[mem_space];
        first_line = block_0_first_line[mem_space];
        last_line = block_0_last_line[mem_space];
     } else {
        n = block_1_last48[mem_space]+1;
        start_data32 = block_1_first32[mem_space];
        first_line = block_1_first_line[mem_space];
        last_line = block_1_last_line[mem_space];
     }
    } else {
     if (block == 0) {
        n = block_0_last48[7]+1;
        start_data32 = block_0_first32[7];
        first_line = block_0_first_line[7];
        last_line = block_0_last_line[7];
	if (block_0_last48[block_mem] > block_0_last48[7]) {
	    n = block_0_last48[block_mem];
	    last_line = block_0_last_line[block_mem];
	}
	if (block_0_first_line[block_mem] < block_0_first_line[7]) {
	    start_data32 = block_0_first32[block_mem];
	    first_line = block_0_first_line[block_mem];
	}
     } else {
        n = block_1_last48[7]+1;
        start_data32 = block_1_first32[7];
        first_line = block_1_first_line[7];
        last_line = block_1_last_line[7];
	if (block_1_last48[block_mem] > block_0_last48[7]) {
	    n = block_1_last48[block_mem];
	    last_line = block_1_last_line[block_mem];
	}
	if (block_1_first_line[block_mem] < block_0_first_line[7]) {
	    start_data32 = block_1_first32[block_mem];
	    first_line = block_1_first_line[block_mem];
	}
     }
     block_mem++;
    }



     m = (short)((n-1)%column_height);

    
    if ((first_line != -1) && (last_line != -1)) { /* only check if poss conflict */

     /* now use formulas to see if there is a conflict between 40 or 48 and 32 bit data */
     switch((short)((n-1)/column_height)) {

     case 0:
	if ((start_data32 < column_height+m+1) &&
            ((start_data32 < (m+1)) || (start_data32 > column_height-1))) {
	    sprintf(err_string,"Memory conflict: 40,48 bit - 32 bit violation lines %d and %d in architecture file\n",last_line,first_line);
	    return(FALSE);
        }
	/* repeat the same logic with the current start if a dm 32 bit access */
	if ((width ==40L) || (width ==48L))
	   break;
	if ((start < column_height+m+1) &&
            ((start < (m+1)) || (start > column_height-1))) {
	    sprintf(err_string,"Memory conflict: 40,48 bit - 32 bit violation lines %d and %d in architecture file\n",last_line,line_number);
	    return(FALSE);
        }
        break;
     case 1:
	if (start_data32 < 2*column_height+m+1)  {
	    sprintf(err_string,"Memory conflict: 40,48 bit - 32 bit violation lines %d and %d in architecture file\n",last_line,first_line);
	    return(FALSE);
        }
	/* repeat the same logic with the current start if a dm 32 bit access */
	if ((width ==40L) || (width ==48L))
	   break;
	if (start < 2*column_height+m+1)  {
	    sprintf(err_string,"Memory conflict: 40,48 bit - 32 bit violation lines %d and %d in architecture file\n",last_line,line_number);
	    return(FALSE);
        }
        break;
     case 2:
	if ((start_data32 < 4*column_height+m+1) &&
            ((start_data32 < 3*column_height+(m+1)) || (start_data32 > 4*column_height-1))) {
	    sprintf(err_string,"Memory conflict: 40,48 bit - 32 bit violation lines %d and %d in architecture file\n",last_line,first_line);
	    return(FALSE);
        }
	/* repeat the same logic with the current start if a dm 32 bit access */
	if ((width ==40L) || (width ==48L))
	   break;
	if ((start < 4*column_height+m+1) &&
            ((start < 3*column_height+(m+1)) || (start > 4*column_height-1))) {
	    sprintf(err_string,"Memory conflict: 40,48 bit - 32 bit violation lines %d and %d in architecture file\n",last_line,line_number);
	    return(FALSE);
        }
        break;
     case 3:
	if (start_data32 < 5*column_height+m+1)  {
	    sprintf(err_string,"Memory conflict: 40,48 bit - 32 bit violation lines %d and %d in architecture file\n",last_line,first_line);
	    return(FALSE);
        }
	/* repeat the same logic with the current start if a dm 32 bit access */
	if ((width ==40L) || (width ==48L))
	   break;
	if (start < 5*column_height+m+1)  {
	    sprintf(err_string,"Memory conflict: 40,48 bit - 32 bit violation lines %d and %d in architecture file\n",last_line,line_number);
	    return(FALSE);
        }
        break;
     case 4:
	if ((start_data32 < 7*column_height+m+1) &&
            ((start_data32 < 6*column_height+(m+1)) || (start_data32 > 7*column_height-1))) {
	    sprintf(err_string,"Memory conflict: 40,48 bit - 32 bit violation lines %d and %d in architecture file\n",last_line,first_line);
	    return(FALSE);
        }
	/* repeat the same logic with the current start if a dm 32 bit access */
	if ((width ==40L) || (width ==48L))
	   break;
	if ((start < 7*column_height+m+1) &&
            ((start < 6*column_height+(m+1)) || (start > 7*column_height-1))) {
	    sprintf(err_string,"Memory conflict: 40,48 bit - 32 bit violation lines %d and %d in architecture file\n",last_line,line_number);
	    return(FALSE);
        }
        break;
     }
    }
  }
return(TRUE); /* all is well */
}


	   

/********************************************************************

Name:           ach_process_overlay

Function:       processes overlay directives

Return Value:   new state

********************************************************************/

static int ach_process_overlay (void) 
{    
    char *dup_token;
    ACH_TOKEN token_value;
    char *token;
    char *store_seg = NULL, *swap_seg = NULL;
    char *label = NULL;
    
    while (token = strtok (NULL, delimiters)) 
    {
	if (dup_token = strdup (token)) 
	{
	    strtoupper (dup_token);
	    if ((token_value = token_lookup (dup_token)) != ACH_BADTOKEN) 
	    {
		switch (token_value) 
		{
		  case ACH_STORE:
		    token = strtok (NULL, delimiters);
		    store_seg = token;
		    break;

		  case ACH_SWAP:
		    token = strtok (NULL, delimiters);
		    swap_seg = token;
		    break;
		    
		  default:
		    fprintf (stderr, "Warning: unknown descriptor \"%s\"", token);
		    fprintf (stderr, " in OVERLAY segment at line %d", line_number);
		    fprintf (stderr, " ignored\n");
		    break;
		}
	    }
	    else
		label = token;
	} 
	free (dup_token);
    }

    if (store_seg && swap_seg && label)
	ach_overlay(store_seg, swap_seg, label);
    else
	fprintf (stderr, "Warning: bad OVERLAY directive %d ignored\n", line_number);

    return (ACH_STATE_SEGMENT);
}



