
#define MAX_SECTION_NAME_SIZE 34

typedef struct 
{
    char sec_name[MAX_SECTION_NAME_SIZE];	    /* name of the section */
    long paddr;                     /* physical address of section */
    long vaddr;                     /* virtual address of section */
    long data_ptr;	            /* offset in file of data area */
    long data_size;	            /* size of data */
    long reloc_ptr;	            /* offset in file of first relocation item */
    unsigned short num_reloc;	    /* number of relocation items */
    long line_ptr;                  /* offset in file of first line number entry */
    unsigned short num_line;        /* number of line number entries */
    long flags;
} SEC_HEADER;

typedef struct
{
    FILE *file_desc;
    short temp_file_index;
    long sec_type;
    char buf[BUFSIZ];
    long buf_cnt;
    short code_gen;     /* Did we generate any code */
    short is_open;      /* Is the file already open */
} SEC_DATA;

#define SECTION_HDR_SIZE sizeof(SEC_HEADER)

/* section header flags */

#define SECTION_CODE        0x0001L
#define SECTION_DMDATA      0x0002L
#define SECTION_PMDATA      0x0004L

