#ifndef _TYPES_H_X1
#define _TYPES_H_X1

#ifndef FAR
#define FAR
#endif

typedef unsigned long  dword_t;
typedef dword_t  FAR  *dword_p;
typedef unsigned short word_t;
typedef word_t   FAR   *word_p;
typedef unsigned char  byte_t;
typedef byte_t   FAR  *byte_p;

#define xOFFSETOF(a) ((a)&0xFFFFL)
#define xPAGEOF(a)   (((a)>>16L)&0xFFL)

#endif
