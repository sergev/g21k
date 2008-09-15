/* @(#)memory.h 2.3 2/13/95 2 */

#define NAME_SIZE 32

struct memory
{
    char             sec_name[NAME_SIZE + 1];     /* section name     */
    unsigned long    width;                       /* section width */
    int              mem_uninit;                  /* Is segment uninitialized*/
    unsigned short   mem_attr;                    /* memory attribute */
    unsigned short   mem_type;                    /* memory type      */
    unsigned long    mem_length;                  /* memory length    */
    unsigned long    start_address;               /* starting address */
    MEMORY           *mem_next;                   /* Pointer to next item on list */
    ADDRESS          *addr_head;                  /* Start of address space */
    ADDRESS          *addr_tail;                  /* End of address space */
};

/* Memory attributes */

#define ATTR_ROM     0x1
#define ATTR_RAM     0x2
#define ATTR_PORT    0x3

/* Memory types */

#define TYPE_PM      0x1
#define TYPE_DM      0x2

extern LIST memory_list;


