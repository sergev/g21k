#ifndef _FILEHDR_H
#define _FILEHDR_H

struct filehdr
{
    unsigned short  f_magic;	/* magic number */
    unsigned short  f_nscns;	/* number of sections */
    long	    f_timdat;	/* time & date stamp */
    long	    f_symptr;	/* file pointer to symtab */
    long	    f_nsyms;	/* number of symtab entries */
    unsigned short  f_opthdr;	/* sizeof(optional hdr) */
    unsigned short  f_flags;	/* flags */
};

#define	    FILHDR	struct filehdr
#define	    FILHSZ	sizeof(FILHDR)


/* Brought over verbatum from /usr/xsoft/21k/include/object.h */

/* magic number*/

#define M_2100   0x0834
#define M_2101   0x0835
#define M_21000  0x521C

#define ISCOFF(x)  ((x) == M_2100 || (x) == M_2101 || (x) == M_21000)


/* file header f_flags bits... */

#define OBJ_EXEC   0x0001 /* File is executable (no unresolved references)             */
#define OBJ_RELOC  0x0002 /* Relocation information stripped from file                 */
#define OBJ_LINE   0x0004 /* Line number information stripped from file                */
#define OBJ_LSYMS  0x0008 /* Local symbol information stripped from file               */

/* Following four defined added by MDA 930413.

   I put these in here to have the bits meaningful with regard to
   "Using and Understanding COFF" */

#define F_RELFLG   0x0001 /* does not contain relocation information */
#define F_EXEC     0x0002 /* file is executable */
#define F_LNNO     0x0004 /* line numbers have been stripped */
#define L_SYMS     0x0008 /* local symbols have been stripped */

/* End of MDA's additions */

#define F_RTS      0x0010 /* C run-time stack location: set for PM, clear for DM       */
#define F_PCS      0x0020 /* PC-stack location: set for external, clear for internal   */
#define F_ASM      0x0040 /* source code is assembly                                   */
#define F_ANSIC    0x0080 /* set if source is ANSI-C, clear if not                     */

#define F_DSPC     0x0100 /* source code is DSP-C                                      */
#define F_DAD	   0x0200 /* set if doubles-are-doubles, clear if doubles-are-floats   */
#define F_REGPARMS 0x0400 /* set if using register parameters, clear if not            */
#define F_GCC      0x0800 /* set if compiler is g21k, clear if not                     */

#define F_RESV3    0x1000 /* reserved                                                  */
#define F_RESV2    0x2000 /* reserved                                                  */
#define F_RESV1    0x4000 /* reserved                                                  */
#define F_BOZO     0x8000 /* usage to be determined                                    */


#endif
