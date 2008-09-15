/* @(#)stk.h	2.2 12/23/93 2 */
/*************************************************************************

   File:          stk.h

   Description:   Contains all defines and declarations for the stacks

*************************************************************************/

#include "reg.h"  /* get the reg type IDs for STK_TYPE */

#ifdef ADSPZ3
 #define PCSTK_SIZE (31)/* Size of PC stack for Z3*/
#else
 #define PCSTK_SIZE (20)    /* Size of PC stack (no emulator support) */
#endif
#define LPSTK_SIZE (6)     /* Size of Loop stacks */
#define STSTK_SIZE (5)     /* Size of Status stack */

typedef enum {
    STK_TYPE_PC           = REG_TYPE_PC_STACK,
    STK_TYPE_LOOP_ADDRESS = REG_TYPE_LA_STACK,
    STK_TYPE_LOOP_COUNTER = REG_TYPE_LC_STACK,
    STK_TYPE_STATUS       = REG_TYPE_ST_STACK,
    } STK_TYPE;

typedef struct {             /* Structure of entry on status stack */
   unsigned long mode1;
   unsigned long astat;
   } STK_STATUS;
