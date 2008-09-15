/* @(#)address.h	1.4  6/11/91 */

struct address
{
    ADDRESS       *next;         /* Pointer to next address node */
    ADDRESS       *previous;     /* Pointer to previous address node */
    int           addr_type;     /* Address type */
    unsigned long addr;          /* Physical address */
    unsigned long size;          /* Size of the address region */
    MEMORY        *mem_ptr;      /* Pointer to memory region */
    OUTPUT_SECT   *sect_ptr;     /* Pointer to output section */
};

#define ADDRESS_FREE         0x1L  /* Unused memory */
#define ADDRESS_ALLOCATED    0x2L  /* Allocated to a output section */
