/* @(#)asm.h    2.2 6/1/95 2 */


#ifndef putc
#include <stdio.h>
#endif

#define PRESENT 1L
#define AFFIRMATIVE 1L

#ifndef FALSE
#define FALSE 0L
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

#include <ctype.h>

#define FATAL 1
#define SUCCESS 0

#define ADSP_21020    20
#define ADSP_21040    40

#define IS_21020      (processor == ADSP_21020)
#define IS_21040      (processor == ADSP_21040)

#define IDENTIFIER_LENGTH   32
#define FREAD(ptr,size,count,stream)  fread( (char *) ptr, size, count, stream)
#define FWRITE(ptr,size,count,stream)  fwrite( (char *) ptr, size, count, stream)

#define TOKEN_TABLE_SIZE    181
#define SYMBOL_TABLE_SIZE   1001


#define NULL_STR (char *) NULL

typedef char BOOLEAN;

#define PM_WORD_SIZE            6 /* bytes */
#define DM_WORD_SIZE            5 /* bytes */

typedef unsigned char PM_WORD[6];
typedef unsigned char DM_WORD[5];

#define DATA32_TOP8(x)         (((x) & 0xFF000000L) >> 24L)
#define DATA32_TOP_MIDDLE8(x)  (((x) & 0x00FF0000L) >> 16L)
#define DATA32_MIDDLE8(x)      (((x) & 0x0000FF00L) >> 8L)
#define DATA32_BOTTOM8(x)      ((x) & 0x000000FFL)

#define MAX_FILE_NAME_SIZE  512 


#ifndef FOPEN_MAX
#define FOPEN_MAX           20
#endif

#define MAX_SECTIONS        100
#define MAX_TEMPS           MAX_SECTIONS
#define MAX_STACK           125
#define TEMP_PREFIX         "astmp"

extern short error_debug;
extern short pass;
extern short processor;
extern int yylineno;
extern int yynerrs;
extern char yytext[];
extern char lex_buff[];
extern FILE *src_stream;
