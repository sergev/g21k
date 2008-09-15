
/* This file provides support for the routines related
   to the architecture of the target system. */

#include <stdio.h>
#ifdef MSDOS
#include <stddef.h>
#endif
#include <string.h>

#include "app.h"
#include "util.h"

#include "achparse.h"
#include "a_out.h"

#define PROCESSOR_UNDEFINED 0
#define PROCESSOR_ADSP_21020 1

extern char *init_section_name;
extern char *code_section_name;

int 	         processor = PROCESSOR_UNDEFINED;
int	         Ram_segment_count = 0;
char	         Ram_segment_list[125][9];
int	         Width_segment_count = 0;
char	         Width_segment_list[125][9];
long	         Width_segment_width[125];
unsigned long    Init_segment_ach_size;

static unsigned long int last_segment_size;


/* stub for z3 overlay support */
void ach_overlay (char *stor_seg, char *swap_seg, char *label) {}


void ach_system(char *label)
{
	label = label;
	Init_segment_ach_size = 0;
	last_segment_size = 0;
}

void ach_processor(int processor_token)
{
	if( processor_token == ACH_ADSP21020 )
		processor = PROCESSOR_ADSP_21020;	
}

void ach_segment(unsigned long begin, unsigned long end, int type, int access, char *symbol, unsigned short have_uninit, unsigned long width)
{
	int attributes, link_type;
	char error_msg[250];

have_uninit; /* used by linker, not mem21k */
width; /* used by linker, not mem21k */

	type = access;	/*for compiler warning*/

	last_segment_size = end - begin;
	if( strcmp(symbol, "seg_init") == 0 && !Init_segment_ach_size )
	    Init_segment_ach_size = last_segment_size;

	if( access == ACH_RAM ) 
	{
		strncpy(Ram_segment_list[Ram_segment_count], symbol, SYMNMLEN);
		Ram_segment_list[Ram_segment_count++][SYMNMLEN] = '\0';
	}
	strncpy(Width_segment_list[Width_segment_count], symbol, SYMNMLEN);
	Width_segment_list[Width_segment_count][SYMNMLEN] = '\0';
	Width_segment_width[Width_segment_count++] = width;
}
	
void ach_endsys(void) {}

void ach_error(char *msg) {}

void ach_bank      (unsigned long begin, unsigned long pgsize,
                    unsigned long wtstates, unsigned short type,
                    unsigned short wtmode, unsigned short which_bank,
                    unsigned short pgwten)
{
}

void ach_scratch_register(char *reg) {}

void ach_reserved_register(char *reg) {}

void ach_cheap(unsigned long length, int type, int access, char *label) {}

void ach_cstack(unsigned long length, int type, int access, char *label) {}

void ach_cdefaults(ACH_TOKEN which, int type, char *label)
{
	if( which == ACH_CCODE )
	{
	    code_section_name = (char *)my_malloc( (long)10 );
	    strncpy(code_section_name, label, SYMNMLEN );
	    code_section_name[SYMNMLEN] = '\0';
	}
	if( which == ACH_CINIT )
	{
		init_section_name = (char *)my_malloc( (long)10 );
		strncpy(init_section_name, label, SYMNMLEN);
		init_section_name[SYMNMLEN] = '\0';
		Init_segment_ach_size = last_segment_size;
	}
}

void ach_compiler(short map_double_as_floats, short jjb, short param_passing, short chip_rev) {}

void ach_circular_register(char *reg, char *label) {}
