/* @(#)output_section.h	1.4 6/11/91 1 */

struct output_section
{
    SCNHDR   sec_hdr;           /* COFF section header */
    short        section_num;       /* Output section number */
    INPUT_SECT   *in_sect_head;     /* Head of list of input sections */
    INPUT_SECT   *in_sect_tail;     /* Tail of list of input sections */
    OUTPUT_SECT  *next;             /* Next output section on list */
    short        flags;             /* flags */
};
