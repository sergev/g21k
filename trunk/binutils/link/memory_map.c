/* @(#)memory_map.c     1.12 5/13/91 1 */

#include <stdio.h>

#include "app.h"
#include "a_out.h"
#include "error.h"
#include "address.h"
#include "allocate.h"
#include "memory.h"
#include "input_file.h"
#include "input_section.h"
#include "list.h"
#include "util.h"
#include "linker_sym.h"
#include "linker_aux.h"
#include "output_section.h"
#include "mem_map_symbol.h"
#include "mem_map.h"
#include "release.h"
#include "error_fp.h"
#include "main.h"

#include "memory_map_fp.h"
#include "syms_fp.h"

extern FILE    *map_file_fd;
extern MEM_MAP *base_mem_map;
#ifdef sun
extern void qsort( char *base, unsigned count, int size, int (*compar)() );
#endif


extern char *C_filename;
extern char Arch_descrip_name[];

#define TIME_STRING_LENGTH 26
#define LINES_PER_PAGE 66
#define POS_LOGO       0
#define POS_FILENAME   45
#define POS_TIME       45
#define POS_PAGE       70
#define POS_XREF_TITLE 0
#define POS_XREF_SYMBOL 0
#define POS_XREF_MEM_TYPE 35 
#define POS_XREF_ADDRESS_PM  43
#define POS_XREF_ADDRESS_DM  41
#define POS_XREF_SCLASS   51
#define POS_ACH_TITLE 0
#define POS_ACH_SYMBOL 0
#define POS_ACH_START 12
#define POS_ACH_END 22
#define POS_ACH_LENGTH 32
#define POS_ACH_MEM_TYPE 44
#define POS_ACH_ATTR 65
#define POS_ACH_WIDTH 78
#define POS_USAGE_TITLE 0
#define POS_USAGE_MEM_TYPE 44
#define POS_USAGE_SEGMENT 0
#define POS_USAGE_ATTR 65
#define POS_USAGE_START 12
#define POS_USAGE_END 22
#define POS_USAGE_LENGTH 32
#define POS_SUM_TITLE 0
#define POS_SUM_MEM_TYPE 0
#define POS_SUM_ATTR 16
#define POS_SUM_TOTAL 27


#define MAX_DM "4294967296"
static char buffer[BUFSIZ];                    /* output buffer */
static char page = 1;                          /* Page number */
static char lines = 0;                         /* Lines printed */
static char have_time = 0;                     /* Time flag */
static char time_string[TIME_STRING_LENGTH];   /* time string */

static void map_banner (void);
static void map_new_page (void);
static void map_output_line (void);
       
/***********************************************************************
*                                                                      *
*   Name                                                               *
*      map_banner
*                                                                      *
*   Description                                                        *
*      outputs a banner to the file                                    *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
***********************************************************************/

static void map_banner (void) {

/* Get the time if necessary */
   if (!have_time) {
     strcpy (time_string, get_local_time()); /*EK*/
     have_time = 1;
   }

/* Print banner */
   sprintf (buffer + POS_LOGO, "Analog Devices ADSP-210x0 Linker");
   sprintf (buffer + POS_FILENAME, "%s", map_file);
   sprintf (buffer + POS_PAGE, "Page %d", page);
   map_output_line ();
   sprintf (buffer + POS_LOGO, "Release %s, Version %s", get_release_number(), get_version_number());
   sprintf (buffer + POS_TIME, "%s", time_string);
   map_output_line ();
   sprintf (buffer + POS_LOGO, "%s", get_copyright_notice());
   map_output_line ();
   sprintf (buffer + POS_LOGO, "%s", get_license_notice()); /*EK*/
   map_output_line ();
   map_output_line ();

}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      map_new_page
*                                                                      *
*   Description                                                        *
*      makes a new page is the list file                               *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
***********************************************************************/

static void map_new_page (void) {

/* Update counters */
   page++;
   lines = 0;

/* Form feed */
   fprintf (map_file_fd, "\f");

/* Print banner */
   map_banner ();

}

/***********************************************************************
*                                                                      *
*   Name                                                               *
*      map_output_line
*                                                                      *
*   Description                                                        *
*      outputs the buffer to the file                                  *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
***********************************************************************/

static void map_output_line (void) {

   short index;

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
   fprintf (map_file_fd, "%s", buffer);

/* Clear the buffer */
   for (index = 0; index < BUFSIZ; index++) {
     buffer[index] = '\0';
   }

/* Increment line count */
   lines++;

/* See if we need a new page */
   if (lines >= LINES_PER_PAGE) {
     map_new_page ();
   }

}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      memory_map                                                      *
*                                                                      *
*   Synopsis                                                           *
*      void memory_map( void )                                         *      
*                                                                      *
*   Description                                                        *
*      Output a memory map.                                            *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/27/89       created                  -----             *
***********************************************************************/

void memory_map( void)
{

/* Output a banner */
   map_banner ();

/* Output architecture description */
   output_system_memory();

/* Output next section */
   output_linker_memory();

/* Output cross reference section */
   output_xref();

}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      output_system_memory                                            *
*                                                                      *
*   Synopsis                                                           *
*      void output_system_memory( void )                               *
*                                                                      *
*   Description                                                        *
*      Show em' what they gave us in the architecture description file.*
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/27/89       created                  -----             *
***********************************************************************/

void output_system_memory( void )
{
    register MEMORY  *mem_ptr;

    if( memory_list.head != NULL)
    {
	sprintf (buffer + POS_ACH_TITLE, "Architecture Description: %s", Arch_descrip_name);
	map_output_line ();
	map_output_line ();

	sprintf (buffer + POS_ACH_SYMBOL, "Segment");
	sprintf (buffer + POS_ACH_START, "Start");
	sprintf (buffer + POS_ACH_END, "End");
	sprintf (buffer + POS_ACH_LENGTH, "Length");
	sprintf (buffer + POS_ACH_MEM_TYPE, "Memory Type");
	sprintf (buffer + POS_ACH_ATTR, "Attribute");
	sprintf (buffer + POS_ACH_WIDTH, "Width");
	map_output_line ();
	map_output_line ();
	for( mem_ptr = (MEMORY *) memory_list.head; mem_ptr; mem_ptr = mem_ptr->mem_next)
	{
		
	    sprintf (buffer + POS_ACH_SYMBOL, "%s", mem_ptr->sec_name);
	    switch( mem_ptr->mem_type)
	    {
	      case TYPE_PM:
		sprintf (buffer + POS_ACH_START, "%6.6lx", mem_ptr->start_address);
		sprintf (buffer + POS_ACH_END, "%6.6lx", (mem_ptr->start_address + mem_ptr->mem_length) - 1);
		sprintf (buffer + POS_ACH_LENGTH, "%lu", mem_ptr->mem_length);
		sprintf (buffer + POS_ACH_MEM_TYPE, "Program Memory");
		break;
		
	      case TYPE_DM:
		sprintf (buffer + POS_ACH_START, "%8.8lx", mem_ptr->start_address);
		sprintf (buffer + POS_ACH_END, "%8.8lx", (mem_ptr->start_address + mem_ptr->mem_length) - 1);
		if (mem_ptr->mem_length) {
		  sprintf (buffer + POS_ACH_LENGTH, "%lu", mem_ptr->mem_length);
		} else {
		  sprintf (buffer + POS_ACH_LENGTH, MAX_DM);
		}
		sprintf (buffer + POS_ACH_MEM_TYPE, "Data Memory");
		break;
	    }
		
	    switch( mem_ptr->mem_attr)
	    {
	      case ATTR_ROM:
		sprintf (buffer + POS_ACH_ATTR, "ROM");
		break;
		
	      case ATTR_RAM:
		sprintf (buffer + POS_ACH_ATTR, "RAM");
		break;
		
	      case ATTR_PORT:
		sprintf (buffer + POS_ACH_ATTR, "PORT");
		break;
	    }

#ifdef ADSPZ3
	    /* width */
	    sprintf (buffer + POS_ACH_WIDTH, "%lu", mem_ptr->width);
#endif
	    map_output_line ();
	}
      map_output_line ();
      map_output_line ();
      }    
}


/***********************************************************************
*                                                                      *
*   Name                                                               *
*      output_linker_memory( void )                                    *
*                                                                      *
*   Synopsis                                                           *
*      void output_linker_memory( void )                               *
*                                                                      *
*   Description                                                        *
*      Show em' what the linker put together.                          *
*      Static memory usage                                             *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     6/27/89       created                  -----             *
***********************************************************************/

void output_linker_memory( void )
{
    register ADDRESS     *addr_ptr;
    long                   dm_ram_size;
    long                   dm_rom_size;
    long                   dm_port_size;
    register OUTPUT_SECT *output_sect_ptr;
    long                   pm_ram_size;
    long                   pm_rom_size;
    long                   pm_port_size;
	
    /* Let's do data memory first */

    dm_ram_size = dm_rom_size = pm_ram_size = pm_rom_size = 0;
    pm_port_size = dm_port_size = 0;

    sprintf (buffer + POS_USAGE_TITLE, "Memory Usage (Actual):" );
    map_output_line ();
    map_output_line ();
    sprintf (buffer + POS_USAGE_MEM_TYPE, "Memory Type");
    sprintf (buffer + POS_USAGE_SEGMENT, "Segment");
    sprintf (buffer + POS_USAGE_ATTR, "Attribute");
    sprintf (buffer + POS_USAGE_START, "Start");
    sprintf (buffer + POS_USAGE_END, "End");
    sprintf (buffer + POS_USAGE_LENGTH, "Length");
    map_output_line ();
    map_output_line ();

    /* Program Memory Map */

    for( addr_ptr = (ADDRESS *) avail_list.head; addr_ptr; addr_ptr = addr_ptr->next )
    {
	 if( addr_ptr->addr_type == ADDRESS_ALLOCATED)
	 {
	     output_sect_ptr = addr_ptr->sect_ptr;
	     if ((output_sect_ptr->sec_hdr.s_flags & SECTION_PM) == SECTION_PM)
	     {
		 sprintf (buffer + POS_USAGE_MEM_TYPE, "Program Memory");
		 if (addr_ptr->size) {
		   sprintf (buffer + POS_USAGE_START, "%6.6lx", addr_ptr->addr);
		   sprintf (buffer + POS_USAGE_END, "%6.6lx",  (addr_ptr->addr + addr_ptr->size) - 1);
		 } else {
		   sprintf (buffer + POS_USAGE_START, "******");
		   sprintf (buffer + POS_USAGE_END, "******");
		 }
		 sprintf (buffer + POS_USAGE_LENGTH, "%lu", addr_ptr->size);

		 switch( addr_ptr->mem_ptr->mem_attr )
		 {
			 case (short) ATTR_ROM:
			    sprintf (buffer + POS_USAGE_ATTR, "ROM");
			    pm_rom_size += addr_ptr->size;
			    break;

			 case (short) ATTR_RAM:
			    sprintf (buffer + POS_USAGE_ATTR, "RAM");
			    pm_ram_size += addr_ptr->size;
			    break;

			 case (short) ATTR_PORT:
			    sprintf (buffer + POS_USAGE_ATTR, "PORT");
			    pm_port_size += addr_ptr->size;
			    break;
		 }
		 sprintf (buffer + POS_USAGE_SEGMENT, "%.8s", output_sect_ptr->sec_hdr.s_name );
		 map_output_line ();
	     }
	 }
    }
    for( addr_ptr = (ADDRESS *) avail_list.head; addr_ptr; addr_ptr = addr_ptr->next )
    {
	 if( addr_ptr->addr_type == ADDRESS_ALLOCATED)
	 {
	     output_sect_ptr = addr_ptr->sect_ptr;
	     if ((output_sect_ptr->sec_hdr.s_flags & SECTION_DM) == SECTION_DM)
	     {
	       sprintf (buffer + POS_USAGE_MEM_TYPE, "Data Memory");
	       if (addr_ptr->size) {
		 sprintf (buffer + POS_USAGE_START, "%8.8lx", addr_ptr->addr);
		 sprintf (buffer + POS_USAGE_END, "%8.8lx",  (addr_ptr->addr + addr_ptr->size) - 1);
	       } else {
		 sprintf (buffer + POS_USAGE_START, "********");
		 sprintf (buffer + POS_USAGE_END, "********");
	       }
	       sprintf (buffer + POS_USAGE_LENGTH, "%lu", addr_ptr->size);

		 switch( addr_ptr->mem_ptr->mem_attr )
		 {
			 case (short) ATTR_ROM:
			    sprintf (buffer + POS_USAGE_ATTR, "ROM");
			    dm_rom_size += addr_ptr->size;
			    break;

			 case (short) ATTR_RAM:
			    sprintf (buffer + POS_USAGE_ATTR, "RAM");
			    dm_ram_size += addr_ptr->size;
			    break;

			 case (short) ATTR_PORT:
			    sprintf (buffer + POS_USAGE_ATTR, "PORT");
			    dm_port_size += addr_ptr->size;
			    break;
		 }
		 sprintf (buffer + POS_USAGE_SEGMENT, "%.8s", output_sect_ptr->sec_hdr.s_name);
		 map_output_line ();
	     }

	 }
    }   
    map_output_line ();
    map_output_line ();

    sprintf (buffer + POS_SUM_TITLE, "Memory Usage Summaries:");
    map_output_line ();
    map_output_line ();
    sprintf (buffer + POS_SUM_MEM_TYPE, "Memory Type");
    sprintf (buffer + POS_SUM_ATTR, "Attribute");
    sprintf (buffer + POS_SUM_TOTAL, "Total");
    map_output_line ();
    map_output_line ();
    sprintf (buffer + POS_SUM_MEM_TYPE, "Program Memory");
    sprintf (buffer + POS_SUM_ATTR, "ROM");
    sprintf (buffer + POS_SUM_TOTAL, "%lu", pm_rom_size);
    map_output_line ();
    sprintf (buffer + POS_SUM_MEM_TYPE, "Program Memory");
    sprintf (buffer + POS_SUM_ATTR, "RAM");
    sprintf (buffer + POS_SUM_TOTAL, "%lu", pm_ram_size);
    map_output_line ();
    sprintf (buffer + POS_SUM_MEM_TYPE, "Program Memory");
    sprintf (buffer + POS_SUM_ATTR, "PORT");
    sprintf (buffer + POS_SUM_TOTAL, "%lu", pm_port_size);
    map_output_line ();
    sprintf (buffer + POS_SUM_MEM_TYPE, "Data Memory");
    sprintf (buffer + POS_SUM_ATTR, "ROM");
    sprintf (buffer + POS_SUM_TOTAL, "%lu", dm_rom_size);
    map_output_line ();
    sprintf (buffer + POS_SUM_MEM_TYPE, "Data Memory");
    sprintf (buffer + POS_SUM_ATTR, "RAM");
    sprintf (buffer + POS_SUM_TOTAL, "%lu", dm_ram_size);
    map_output_line ();
    sprintf (buffer + POS_SUM_MEM_TYPE, "Data Memory");
    sprintf (buffer + POS_SUM_ATTR, "PORT");
    sprintf (buffer + POS_SUM_TOTAL, "%lu", dm_port_size);
    map_output_line ();
    map_output_line ();
    map_output_line ();
}



/***********************************************************************
*                                                                      *
*   Name                                                               *
*      output_xref                                                     *
*                                                                      *
*   Synopsis                                                           *
*                                                                      *
*   Description                                                        *
*      Generate a cross-reference on a file by file basis.             *
*                                                                      *
*   Revision History                                                   *
*                                                                      *
*     name    date          description              number            *
*     mkc     11/27889       created                  -----            *
***********************************************************************/

void output_xref( void )
{
   register int             i;
   LINKER_SYM              *global_sym;
   register MEM_MAP        *mem_map_ptr;
   register MEM_MAP_SYMBOL *symbol_ptr;


   mem_map_ptr = base_mem_map->next;

   while( mem_map_ptr )
   {
     sprintf( buffer + POS_XREF_TITLE, "Cross Reference for file -> %s:", mem_map_ptr->filename);
     map_output_line ();
     map_output_line ();
	
     sprintf (buffer + POS_XREF_SYMBOL, "Symbol");
     sprintf (buffer + POS_XREF_MEM_TYPE, "Type");
     sprintf (buffer + POS_XREF_ADDRESS_DM, "Address");
     sprintf (buffer + POS_XREF_SCLASS, "Class");
     map_output_line ();
     map_output_line ();
	
     qsort( (char *) mem_map_ptr->symbol_table, mem_map_ptr->symbol_index, 
	   sizeof(MEM_MAP_SYMBOL *), comp_alpha );
     for( i = 0; i < mem_map_ptr->symbol_index; ++i )
     {
       symbol_ptr = mem_map_ptr->symbol_table[i];

/* . . Skip the . symbol */
       if( strcmp(symbol_ptr->symbol_name, ".") == 0)
	 continue;

/* . . Skip segments */
       if ((symbol_ptr->sclass == C_STAT) && (symbol_ptr->num_aux > 0) && (symbol_ptr->type == T_NULL)) {
	 continue;
       }

        
       sprintf (buffer + POS_XREF_SYMBOL, "%s", symbol_ptr->symbol_name);
	
       switch( (int)symbol_ptr->mem_type )
       {
       case SECTION_PM:
	 sprintf (buffer + POS_XREF_MEM_TYPE, "PM");
	 sprintf (buffer + POS_XREF_ADDRESS_PM, "%6.6lx", symbol_ptr->address );
	 break;
	       
       case SECTION_DM:
	 sprintf (buffer + POS_XREF_MEM_TYPE, "DM");
	 sprintf (buffer + POS_XREF_ADDRESS_DM, "%8.8lx", symbol_ptr->address );
	 break;
	       
       default:
	 if( (symbol_ptr->sclass == C_EXT || symbol_ptr->sclass == C_CRTL )
	    && symbol_ptr->input_sect )
	 {
	   global_sym = findsym( symbol_ptr->symbol_name );
	   switch( (int)(global_sym->input_sect->sec_hdr.s_flags
			 & (SECTION_PM | SECTION_DM)) )
	   {
	   case SECTION_PM:
	     sprintf (buffer + POS_XREF_MEM_TYPE, "PM");
	     sprintf (buffer + POS_XREF_ADDRESS_PM, "%6.6lx", symbol_ptr->address );
	     break;
		
	   case SECTION_DM:
	     sprintf (buffer + POS_XREF_MEM_TYPE, "DM");
	     sprintf (buffer + POS_XREF_ADDRESS_DM, "%8.8lx", symbol_ptr->address );
	     break;
		
	   default:
	     /* should never get here */
	     LINKER_ERROR( "Memory type unavailable");
	     break;
	   }
	 }
       }
	
	
       switch( symbol_ptr->sclass )
       {
       case C_STAT:
	 if( symbol_ptr->num_aux > 0 && symbol_ptr->type == T_NULL )
	   sprintf (buffer + POS_XREF_SCLASS, "segment" );
	 else
	   sprintf (buffer + POS_XREF_SCLASS, "static" );
	 break;
	       
       case C_EXT:
       case C_EXTDEF:
       case C_CRTL:
	 if( !symbol_ptr->input_sect )
	   sprintf (buffer + POS_XREF_SCLASS, "global");
	 else
	   sprintf (buffer + POS_XREF_SCLASS, "external (%s:%.8s)",
		    (C_filename == symbol_ptr->input_sect->file_ptr->file_name ?
		     "C Support Object" : symbol_ptr->input_sect->file_ptr->file_name),
		    symbol_ptr->input_sect->sec_hdr.s_name);
       }
       map_output_line ();
     }
     mem_map_ptr = mem_map_ptr->next;
     map_output_line ();
     map_output_line ();
   }
 }


int comp_alpha( MEM_MAP_SYMBOL **symbol1, MEM_MAP_SYMBOL **symbol2 )
{
    return( strcmp((*symbol1)->symbol_name, (*symbol2)->symbol_name) );
}
