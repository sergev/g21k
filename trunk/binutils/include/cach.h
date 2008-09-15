/* @(#)cach.h	1.1 11/8/90 1 */

/*************************************************************************

   File:          cach.h

   Description:   Contains all defines and declarations for the 21k cache

*************************************************************************/

#define CACH_ENTRIES_PER_SET (2)
#define CACH_SETS_PER_CACHE (16)

typedef struct {
  unsigned long address [CACH_ENTRIES_PER_SET];
  UNS48 opcode[CACH_ENTRIES_PER_SET];
  unsigned int valid_bit [CACH_ENTRIES_PER_SET];
  unsigned int lru_bit;
  } CACH_SET;

typedef struct {
  CACH_SET set[CACH_SETS_PER_CACHE];
  } CACH;

