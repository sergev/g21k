/* @(#)app.h    2.5 8/10/95 2 */

/*************************************************************************

   File:          app.h

   Description:   This file defines all common definitions and
		  will include the application specific include file

*************************************************************************/

#ifndef _APP_H_
#define _APP_H_

/*EK* get all info from the configuration run */
#include "../config.h"

/****** Generic Definitions *****/

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE 
#define FALSE 0
#endif

/****** File Definitions ******/

#ifdef MSDOS

#define MAX_FILENAME_LENGTH 144         /* this is true for dos anyway */
		/* I have not make modification for WIN32 MODIFICATION_BL */

#define FILENAME_SEPARATOR '\\'
#define FILENAME_SEPARATOR_STR "\\"

#define FOPEN_RT "rt"
#define FOPEN_RB "rb"
#define FOPEN_WT "wt"
#define FOPEN_WB "wb"

#define READ_TEXT     "rt"
#define READ_BINARY   "rb"
#define WRITE_TEXT    "wt"
#define WRITE_BINARY  "wb"
#define UPDATE_TEXT   "wt+"
#define UPDATE_BINARY "wb+"
#define APPEND_TEXT   "at"
#define APPEND_BINARY "ab"

#else

#define MAX_FILENAME_LENGTH 512         /* BUG: wrong... */

#define FILENAME_SEPARATOR '/'
#define FILENAME_SEPARATOR_STR "/"

#define FOPEN_RT "r"
#define FOPEN_RB "r"
#define FOPEN_WT "w"
#define FOPEN_WB "w"

#define READ_TEXT     "r"
#define READ_BINARY   "r"
#define WRITE_TEXT    "w"
#define WRITE_BINARY  "w"
#define UPDATE_TEXT   "w+"
#define UPDATE_BINARY "w+"
#define APPEND_TEXT   "a"
#define APPEND_BINARY "a"

#endif

/****** Definitions for numeric formats *****/

#define FORMAT_HEX   (0)
#define FORMAT_FIX   (1)
#define FORMAT_FLOAT (2)


/****** Definitions for odd sized variables *****/

typedef struct {
	unsigned char byte[5];        /* byte 0 is the MSB */
	} UNS40;

typedef struct {
	unsigned char byte[6];        /* byte 0 is the MSB */
	} UNS48;


/****** Include application specific files ******/

#ifdef SIM
#include "../sim/sim.h"
#endif

#ifdef ASM
#include "../asm/asm.h" /*EK*/
#endif

#ifdef LINK
#include "../link/linker.h" /*EK*/
#endif

#ifdef AR
#include "archiver.h"
#endif

#ifdef SPLIT
#include "split.h"
#endif

#ifdef ICE
#include "../ice/ice21k.h" /*EK*/
#endif

#ifndef WIN31
#ifndef BOOL
#define BOOL boolean
#endif /* !BOOL */
#endif /* !WIN31 */

#endif  /* _APP_H_ */
