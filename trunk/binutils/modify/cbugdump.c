// FILENAME:		cbugdump.c
// AUTHOR:			BL
// Created Date:	Jan 14, 1997
//	
// Functions in file are used to generate standalone CBUG test vector. They are
// actually not part of simulator or emulator. Because they use OutputDebugString()
// to display, they only can be used under DEBUG MODE

#ifndef NDEBUG

#include <windows.h>
#include <stdio.h>
#include <assert.h>

#include "app.h"
#include "reg.h"
#include "mem_adi.h"

#define DEBUGOUTSTRLEN		100

unsigned short 
reg_debugout(unsigned short reg_type, unsigned short reg_number, void *buffer);
unsigned short
mem_debugout(unsigned short type,unsigned long count,unsigned long address,void * const buffer);
short	OpenDumpForCbugFiles(char * pszRegIn, char * pszMemIn );
short	CloseDumpForCbugFiles(void);

// The reason why they are global, because reg_debugout()
// and mem_debugout() can not take FILE * as input parameter,
// See comment in front of reg_debugout() and mem_debugout()
FILE * pFRegIn	= NULL;
FILE * pFRegOut = NULL;
FILE * pFMemIn  = NULL;
FILE * pFMemOut = NULL;

const char szCbugRegIn[] ="c:\\xsoft\\cbugstd\\testdata\\cbug_reg.in\0";
const char szCbugRegout[]="c:\\xsoft\\cbugstd\\testdata\\cbug_reg.out\0";
const char szCbugMemIn[] ="c:\\xsoft\\cbugstd\\testdata\\cbug_mem.in\0";
const char szCbugMemOut[]="c:\\xsoft\\cbugstd\\testdata\\cbug_mem.out\0";

// Function:	OpenDumpForCbug	
// Note:		Direction of stream is easy confused here.
//				Open for write, we call it "in", because it will be used
//				as input for standard alone  CBUG	
//				At this moment, we do not care szCbugRegOut and szCbugMemOut,
//				which should be used to compare output from standalone CBUG
short	OpenDumpForCbugFiles(char * pszRegIn, char * pszMemIn )
{
	assert(pszRegIn != NULL);
	assert(pszMemIn != NULL);

	pFRegIn = fopen(pszRegIn, "wt" );
	pFMemIn = fopen(pszMemIn, "wt");

	if( (NULL == pFRegIn)||(NULL == pFMemIn) )
	{
		OutputDebugString("Can not write to Cbug Reg or Mem files.\n");
		return FALSE;
	}
	return TRUE;
}
short	CloseDumpForCbugFiles(void)
{
	fclose(pFRegIn);
	fclose(pFMemIn);
}

// Function:	reg_debugout
//				dump register value to debug windows, and such log data will help 
//				standalone CBUG developing. It is modified from context_dump()
//	
// Note:		It must be used adjacent with reg_get() and reg_set()
//				all of its parameters are same as reg_get() and reg_set()
unsigned short 
reg_debugout(unsigned short reg_type, unsigned short reg_number, void *buffer)
{
	char char_buffer[40];
	char szRegNum[DEBUGOUTSTRLEN];
	char szRegOut[DEBUGOUTSTRLEN];
	char szNoName[]="NoName";

	// Original raw input
	// sprintf(szRegNum, "type %d num %2x, buffer %p  ", reg_type, reg_number, buffer);
	sprintf(szRegNum, "%d %2x %p  ", reg_type, reg_number, buffer);

	// Decode some data for universal and computational register
	// and output to 32bit, 48bit or 40 bit.
	switch(reg_type) 
	{
		case	REG_TYPE_UNIV:
			// get register name, note, semicolon is necessary
			if (dis_reg (REG_TYPE_UNIV, reg_number, char_buffer)) ;
			else strcpy(char_buffer, szNoName);

			if (IS_REG_FILE (reg_number))
			{
				sprintf (szRegOut, "%-12s -> %02x%02x%02x%02x%02x", char_buffer,
									((UNS40 *)buffer)->byte[0],
									((UNS40 *)buffer)->byte[1],
									((UNS40 *)buffer)->byte[2],
									((UNS40 *)buffer)->byte[3],
									((UNS40 *)buffer)->byte[4]);
			}
			else
			{
				if (reg_number == REG_PX)
				{
					sprintf (szRegOut, "%-12s -> %02x%02x%02x%02x%02x%02x", char_buffer,
									((UNS48 *)buffer)->byte[0],
									((UNS48 *)buffer)->byte[1],
									((UNS48 *)buffer)->byte[2],
									((UNS48 *)buffer)->byte[3],
									((UNS48 *)buffer)->byte[4],
									((UNS48 *)buffer)->byte[5]);										
				}
				else
				{
					sprintf (szRegOut, "%-12s -> %08lx", char_buffer, *(unsigned long *)buffer);
				}
			}
		break;

		// computational register 32 bit
		case	REG_TYPE_CU:
			// get register name, note, semicolon is necessary
			if (dis_reg (REG_TYPE_CU, reg_number, char_buffer)) ;
			else strcpy(char_buffer, szNoName);

			sprintf (szRegOut, "%-12s -> %08lx", char_buffer, *(unsigned long *)buffer);
		break;
		
		// cache register 32 bit
		case REG_TYPE_CACHE:
			// get register name, note, semicolon is necessary
			if (dis_reg (REG_TYPE_CACHE, reg_number, char_buffer)) ;
			else strcpy(char_buffer, szNoName);

			sprintf (szRegOut, "%-12s -> %08lx", char_buffer, *(unsigned long *)buffer);
		break;

		// all other register are 32 bit, We will not decoce their name
		// now, if we have time in future, We may do it. Include
		// REG_TYPE_PC_STACK (0x4)     /* pc stack */
		// REG_TYPE_LA_STACK (0x5)     /* loop address stack */
		// REG_TYPE_LC_STACK (0x6)     /* loop count stack */
		// REG_TYPE_ST_STACK (0x7)     /* status stack */
		// REG_TYPE_IOP      (0x8)     /* IOP registers */
		default:
			sprintf (szRegOut, "%-12s -> %08lx", "NoName", *(unsigned long *)buffer);				
		break;
	}

	OutputDebugString(szRegNum);
	OutputDebugString(szRegOut);
    OutputDebugString("\n");

	assert(pFRegIn != NULL);
	fprintf(pFRegIn, szRegNum);
	fprintf(pFRegIn, szRegOut);
	fprintf(pFRegIn, "\n");
}

// Function:	mem_debugout
//				Very lucky for us. For the purpose of generating CBUG test vector, 
//				all function call to mem_get(type, count, address, buffer) with
//				count = 1, this make it is easy to print out.
// Note:		It must be used adjacent with mem_get() and mem_set()
//				All of its parameters are same as mem_get() and mem_set()
unsigned short
mem_debugout(	unsigned short type,
				unsigned long count,
				unsigned long address,
				void * const buffer)
{
	char szMemOut[DEBUGOUTSTRLEN];
	
	// We only handle count = 1 case now
	if(count != 1)
	{
		sprintf(szMemOut,"count > 1");
	}
	else
	{
		switch(type)
		{
			// We assume all MEM_DM 32 bit
			case MEM_DM:
				sprintf (szMemOut, "%1d, %1d, %08lx, -> %08lx", type, count, address, *(unsigned long *)buffer);	
			break;
			
			// We assume all MEM_PM 48 bit
			case MEM_PM:
				sprintf (szMemOut, "%1d, %1d, %08lx, -> %02x%02x%02x%02x%02x%02x", type, count, address,
									((UNS48 *)buffer)->byte[0],
									((UNS48 *)buffer)->byte[1],
									((UNS48 *)buffer)->byte[2],
									((UNS48 *)buffer)->byte[3],
									((UNS48 *)buffer)->byte[4],
									((UNS48 *)buffer)->byte[5]);		

			break;

			default:
				sprintf(szMemOut, "mem type not know");

			break;
		}
	}
	OutputDebugString(szMemOut);
	OutputDebugString("\n");

	fprintf(pFMemIn, szMemOut);
	fprintf(pFMemIn, "\n");
}
#endif
