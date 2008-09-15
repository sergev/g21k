/* COPYRIGHT 1984, 1985, 1986, 1987   PERENNIAL                */
/* @(#)File: testhead.h    Version: 1.1    Date: 3/27/85 */
			/* testhead.h */
/*===========================================================================
|	This is the include file for all tests developed for the PERENNIAL
| Unix Validation Suite.  It should be included in any test that uses the 
| support functions in "libd".
===========================================================================*/
#include "21stdio.h"

#define  PASSED 0
#define  FAILED 1
#define  ERROR -1 
#define  FATAL -2

#define  DFAIL  0
#define  DPASS  1
#define  DTERS  2
#define  DVERB  3
#define  DDEBUG 4
#define PMODE 0644

extern int block_number ;
extern int condition_number ;
extern int global_flag ;

/*
extern char testname[];
*/
