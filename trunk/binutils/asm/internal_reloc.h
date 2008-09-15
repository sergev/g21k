/* @(#)internal_reloc.h	1.2 1/4/91 1 */


typedef struct 
{
    long r_address;         /* virtual address of reference */

    union
    {
        long r_symindex;    /* index into symbol table */
        char *r_name;       /* pointer to name in memory */
    } relname;

    short r_type;            /* relocation type */	
    
} INTERNAL_RELOC;

