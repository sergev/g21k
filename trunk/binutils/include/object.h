
typedef struct 
{
    long magic;             /* magic number */
    long num_sections;	    /* number of sections in file */
    long time;		    /* creation time of file */
    long symbol_ptr;	    /* offset in file of symbol table */
    long num_symbols;	    /* number of symbol table entries */
    unsigned short opt_hdr; /* size of optional header */
    unsigned short flags;
} OBJ_HEADER;

#define OBJECT_HDR_SIZE sizeof(OBJ_HEADER)

/* magic number*/

#define M_2100   0x0834L
#define M_2101   0x0835L
#define M_21000  0x5208L

#define ISCOFF(x)  ((x) == M_2100 || (x) == M_2101 || (x) == M_21000)

/* obj header flags */

#define OBJ_EXEC  0x0001L    /* File is executable (no unresolved references) */
#define OBJ_RELOC 0x0002L    /* Relocation information stripped from file */
#define OBJ_LINE  0x0004L    /* Line number information stripped from file */
#define OBJ_LSYMS 0x0008L    /* Local symbol information stripped from file */
