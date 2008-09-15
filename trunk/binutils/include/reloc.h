/*EK*
 *  must define a packed structure for those compilers that need it.
 *  the size of struct reloc should be 10, not 12.
 */

#ifndef PACKED
	#error must include config.h to define PACKED
#endif
#if SIZEOF_SHORT != 2
	#error sorry, this source code depends on sizeof(short)==2
#endif
#define SIZEOF_CHAR_P 4
#if SIZEOF_LONG != 4
	#error sorry, this source code depends on sizeof(long)==4
#endif
#if SIZEOF_CHAR_P != 4
	#error sorry, this source code depends on sizeof(char *)==4
#endif

struct reloc
{
    long            r_vaddr; /* address of refrence */
    long            r_symndx; /* index into symbol table */
    unsigned short  r_type;     /* relocation type */
} PACKED;

#define     RELOC   struct reloc

/* Brought over verbatum from /usr/xsoft/21k/include/reloc.h */

/* relocation types */

#define R_ABS           0x0001L  /* reference is absolute */
#define R_ADDR24        0x0002L  /* relocate the 24-bit address in the lower 24 bits
				 * of the instruction word
				 */
#define R_ADDR32        0x0003L  /* relocate the 32-bit address in the lower 32 bits
				 * of the instruction word
				 */
#define R_ADDR_VAR      0x0004L  /* address of a variable in a data section*/
#define R_PCRSHORT      0x0005L  /* pc-relative short, 6-bit relative address*/
#define R_PCRLONG       0x0006L  /* pc-relative long, 24-bit relative address */

#define R_2100_ABS      0x0007
#define R_2100_OFFSET   0x0008

#define RELOC_SIZE sizeof(RELOC)
