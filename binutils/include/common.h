#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>

/* 
 *  To make easy porting from 16 bit world  to 32 bit world, there should be no integer type in
 *  the program, only short or long																
 */

typedef short INT16;
typedef long int  LONGINT;

#define TMPNOTUSE		0xcd			// This is a magic number

#define LONGSTR			200
#define LONGDEBUGSTR    200
#define SHORTDEBUGSTR	50

/* In original code, processor name is an initialied string, with no length */
/* checking. We put a limit here, in case something wrong, we get some kind */
/* of protection                                                            */
#define PROCESSORNAMELIMIT    20

/* macro FILENAME_MAX in stdio.h */

typedef struct  
{
    char szProcessorName[PROCESSORNAMELIMIT +1];
    char szAchFile[FILENAME_MAX +1];
    char szExeFile[FILENAME_MAX +1];
} CONFIGINFO;

extern CONFIGINFO ConfigInfo;       /* this will be a global variable */

#ifdef CONSOLE_MODE
extern short fInIdleMode;			/* flag used to get out of IDLE loop and terminate */
									/* program in CONSOLE_MODE                         */
									/* It is defined in console mode main.c            */
#endif

#endif
