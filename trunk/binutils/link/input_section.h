/* @(#)input_section.h	1.4 6/11/91 1 */

struct input_section
{
    SCNHDR       sec_hdr;              /* COFF section header */
    INPUT_SECT   *next;                /* Next input section on list of sections in this file */
    INPUT_FILE   *file_ptr;            /* Pointer to INPUT_FILE */
    OUTPUT_SECT  *out_sec;             /* Pointer to corresponding output section */
    INPUT_SECT   *inc_next;            /* Pointer to next input sect in this output section */
    long         data_disp;            /* Displacement of section data */
    long         reloc_disp;           /* Displacement of relocation entries */
    long         line_disp;            /* Displacement of line number entries */
    long         vaddr;                /* Virtual address of section */
    long         paddr;                /* Physical address of section */
    short        section_num;          /* Input section number */
};
