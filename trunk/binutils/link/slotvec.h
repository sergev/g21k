/* @(#)slotvec.h	2.1  5/25/93 */

typedef struct slotvec
{
    long sv_symindex;       /* original (input) symbol table index  */
    long sv_vaddr;          /* original (input) symbol virtual addr */
    long sv_new_vaddr;      /* new (output) symbol virtual addr	    */
    long sv_new_symindex;   /* new (output) symbol table index      */
    char sv_new_secnum;     /* new (output) section number          */
    char sv_flags;
} SLOTVEC;

#define MALLOC_CUTOFF_SIZE (64000)

#define SV_TV		0
#define SV_ERR		0

#define SLOTSIZ        sizeof(SLOTVEC)

extern unsigned short sv_high_slot;
extern SLOTVEC *getslot(unsigned short slot);
