#ifndef _AOUTHDR_H
#define _AOUTHDR_H

typedef struct aouthdr
{
    short   magic;	    /* magic number */
    short   vstamp;	    /* version stamp */
    long    tsize;	    /* text size in bytes */
    long    dsize;	    /* initialized data size */
    long    bsize;	    /* uninitialized data size */
    long    entry;	    /* entry point */
    long    text_start;	    /* base of text used for this file */
    long    data_start;	    /* base of data used for this file */
} AOUTHDR;


#define AOUTSZ sizeof(AOUTHDR)

#endif

