/* @(#)linker.h 2.1  2/13/95 */

#ifndef putc
#include <stdio.h>
#endif

#ifdef sun
#include <strings.h>
#endif

#ifdef MSDOS
#undef stderr
#define stderr stdout
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#endif

#ifndef FALSE
#define FALSE 0L
#endif

#ifndef TRUE
#define TRUE 1L
#endif

#define FATAL 1
#define PASSED 0

#ifndef max
#define max(x,y) ((x) < (y) ? (y) : (x))
#define min(x,y) ((x) < (y) ? (x) : (y))
#endif

#define PM_WORD_SIZE    6       /* bytes */
#define DM_WORD_SIZE    5       /* bytes */

#define LEGAL_SECTION_NAME(x) ( 1 )
#define MAX_STRING_TABLE_SIZE 1024
#define MAX_NUMBER_INPUT_FILES  100

#define MAP_FILE_EXTENSION   ".map"

/* Let's save ourself some headaches during compiles */

typedef struct address ADDRESS;
typedef struct input_file INPUT_FILE;
typedef struct input_section INPUT_SECT;
typedef struct memory MEMORY;
typedef struct output_section OUTPUT_SECT;
typedef struct list LIST;

extern int yynerrs;
extern int yylineno;
extern int processor;

