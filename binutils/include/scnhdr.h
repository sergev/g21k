/* @(#)scnhdr.h	2.3 2/13/95 2 */
#ifndef _SCNHDR_H
#define _SCNHDR_H

struct scnhdr
{
    char	    s_name[SYMNMLEN];	/* section name */
    long	    s_paddr;		/* physical address */
    long	    s_vaddr;		/* virtual address */
    long	    s_size;		/* section size */
    long	    s_scnptr;		/* file ptr to raw data for section */
    long	    s_relptr;		/* file ptr to relocation */
    long	    s_lnnoptr;		/* file ptr to line numbers */
    unsigned short  s_nreloc;		/* number of relocation entries */
    unsigned short  s_nlnno;		/* number of line number entries */
    long	    s_flags;		/* type and content flags */
};

#define	    SCNHDR	struct scnhdr
#define	    SCNHSZ	sizeof(SCNHDR)

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

/* section header flags */
#define SECTION_PM          0x00000001L
#define SECTION_DM          0x00000002L
#define SECTION_RAM	    0x00000004L
#define SECTION_ROM         0x00000008L
#define SECTION_16          0x00000010L
#define SECTION_32          0x00000020L
#define SECTION_40          0x00000040L
#define SECTION_48          0x00000080L
#define SECTION_TEXT        0x00000100L /* kludge for cbug ".text" fake */

#define SECTION_MASK        0x000001ffL /* update as new s_flag bits are added */

#endif
