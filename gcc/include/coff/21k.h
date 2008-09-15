#define     ADSP21k     1

#define _TEXT	"seg_pmco"
#define _DATA	"seg_dmda"
#define _BSS	"seg_bss"

#define     E_SYMNMLEN    8
#define     E_FILNMLEN    14
#define     E_DIMNUM      4

/* magic number*/
#define M_2100         0x0834
#define M_2101         0x0835
#define M_21000        0x521C
#define M_21000lendian 0x1c52

struct external_filehdr
{
	char f_magic[2];	/* magic number			*/
	char f_nscns[2];	/* number of sections		*/
	char f_timdat[4];	/* time & date stamp		*/
	char f_symptr[4];	/* file pointer to symtab	*/
	char f_nsyms[4];	/* number of symtab entries	*/
	char f_opthdr[2];	/* sizeof(optional hdr)		*/
	char f_flags[2];	/* flags			*/
};

#define	    FILHDR	struct external_filehdr
#define	    FILHSZ	sizeof(FILHDR)


/* Brought over verbatum from /usr/xsoft/21k/include/object.h */



#define ISCOFF(x)    ((x) == M_2100 || (x) == M_2101 || (x) == M_21000 || (x) == M_21000lendian)
#define BADMAG21K(x) (((x).f_magic!=M_2100) \
		      && ((x).f_magic!=M_2101)\
		      && ((x).f_magic!=M_21000)\
		      && ((x).f_magic!=M_21000lendian))

/* file header f_flags bits... */

struct external_aouthdr
{
    char    magic[2];	    /* magic number */
    char    vstamp[2];	    /* version stamp */
    char    tsize[4];	    /* text size in bytes */
    char    dsize[4];	    /* initialized data size */
    char    bsize[4];	    /* uninitialized data size */
    char    entry[4];	    /* entry point */
    char    text_start[4];	    /* base of text used for this file */
    char    data_start[4];	    /* base of data used for this file */
};

#define AOUTHDR struct external_aouthdr
#define AOUTSZ sizeof(struct external_aouthdr)

/* @(#)scnhdr.h	2.2 4/28/93 2 */

struct extern_scnhdr
{
    char	    s_name[E_SYMNMLEN];	/* section name */
    char	    s_paddr[4];		/* physical address */
    char	    s_vaddr[4];		/* virtual address */
    char	    s_size[4];		/* section size */
    char	    s_scnptr[4];		/* file ptr to raw data for section */
    char	    s_relptr[4];		/* file ptr to relocation */
    char	    s_lnnoptr[4];		/* file ptr to line numbers */
    char            s_nreloc[2];		/* number of relocation entries */
    char            s_nlnno[2];		/* number of line number entries */
    char	    s_flags[4];		/* type and content flags */
};

#define	    SCNHDR	struct extern_scnhdr
#define	    SCNHSZ	sizeof(SCNHDR)


/* section header flags */

#define SECTION_PM          0x0001L
#define SECTION_DM          0x0002L
#define SECTION_RAM	    0x0004L
#define SECTION_ROM         0x0008L
#define SECTION_16          0x00000010L
#define SECTION_32          0x00000020L
#define SECTION_40          0x00000040L
#define SECTION_48          0x00000080L

/* Make this the last flag of the list*/
#define SECTION_NEXT_FLAG   0x0010L


/* This stuff was put here by MDA for the fixed-point linker/loader,
   refering to the Nutshell Handbook "Using and Understanding COFF";
   it should be orthogonal to out existing COFF implementation */

/* The original COFF notion of TEXT, DATA, and BSS sections was that
   TEXT sections were read-only and data sections were read/write, as
   controlled by some MMU or something.  So I will use TEXT for ROM
   and DATA for RAM. */


/* Note: two LSBytes are for section TYPE, two MSBytes are for section
   CONTENTS.  "Using and Understanding COFF" - p68 */

#define STYPE_TYPE_MASK	0x0000FFFFL
#define STYPE_CONT_MASK	0xFFFF0000L

#define STYPE_REG	0x00000001L	/* section type flag */

#define STYPE_DM_BSS	0x80000000L	/* STYPE_REG content type flag */
#define STYPE_DM_DATA	0x40000000L	/* STYPE_REG content type flag */
#define STYPE_DM_TEXT	0x20000000L	/* STYPE_REG content type flag */
#define STYPE_INFO	0x10000000L	/* STYPE_REG content type flag */
#define STYPE_LIB	0x08000000L	/* STYPE_REG content type flag */
#define STYPE_PM_BSS	0x04000000L	/* STYPE_REG content type flag */
#define STYPE_PM_DATA	0x02000000L	/* STYPE_REG content type flag */
#define STYPE_PM_TEXT	0x01000000L	/* STYPE_REG content type flag */

#define STYPE_DM_MASK	0xE0000000L
#define STYPE_PM_MASK	0x0E000000L

#define STYPE_BSS_MASK	0x84000000L

struct external_reloc
{
    char	    r_vaddr[4];	/* address of refrence */
    char	    r_symndx[4];	/* index into symbol table */
    char            r_type[2];	/* relocation type */
};

#define	    RELOC   struct external_reloc
#define     RELSZ   sizeof (RELOC)

/* Brought over verbatum from /usr/xsoft/21k/include/reloc.h */

#define R_2100_ABS	0x0007
#define R_2100_OFFSET   0x0008


struct external_lineno
{
    union			/* if l_lnno == 0... */
    {
	char	l_symndx[4];	/* then use l_symndx, */
	char	l_paddr[4];	/* else use l_paddr. */
    } l_addr;
    char  l_lnno[4];	/* line number */
};

#define	    LINENO  struct external_lineno
#define	    LINESZ  sizeof(LINENO)

struct external_syment
{
    union
    {
	char	e_name[E_SYMNMLEN];  /* Symbol name (if .LE. 8) */
	struct			    /* if _n_name[0-3] == 0 */
	{
	    char    e_zeroes[4];	    /* then _n_name[4-7] is an */
	    char    e_offset[4];	    /* offset into the string table */
	} e;
/*	char	*e_nptr[2];	                allows for overlaying */
      } e;
    char	    e_value[4];	    /* value of symbol */
    char	    e_scnum[2];	    /* section number */
    char            e_type[2];	    /* type and derived type */
    char	    e_sclass[1];	    /* storage class */
    char	    e_numaux[1];	    /* number of aux. entries */
    char            e_pad[2];
};

#define	    SYMENT	struct external_syment
#define	    SYMESZ	sizeof(SYMENT)

#define C_CRTL     107  /* like C_EXT, but from C RTL */
#define C_PROLOG   109  /* End of C function prolog marker */

#define	N_BTMASK	0xFL
#define N_TMASK	        (060)
#define N_BTSHFT	(4)
#define N_TSHIFT	(2)

union external_auxent
{
    struct
    {
	char		x_tagndx[4];
	union
	{
	    struct
	    {
		char	x_lnno[2];
		char    x_size[2];
	    } x_lnsz;
	    char        x_fsize[4];
	} x_misc;
	union
	{
	    struct
	    {
		char		x_lnnoptr[4];
		char		x_endndx[4];
	    } x_fcn;
	    struct
	    {
		char     	x_dimen[E_DIMNUM][2];
	    } x_ary;
	} x_fcnary;
	 char		x_tvndx[2];
    } x_sym;

    union {
      char x_fname[E_FILNMLEN];
      struct {
	char x_zeroes[4];
	char x_offset[4];
      } x_n;
    } x_file;

    struct
    {
	char			x_scnlen[4];
	char			x_nreloc[2];
	char                    x_nlinno[2];
    } x_scn;
    char pad[20];
};

#define     AUXENT  union external_auxent
#define	    AUXESZ  sizeof(AUXENT)

/******************************************************/
/* Note that auxiliary entries must be the same size  */
/* as a symbol entry, i.e., AUXESZ must equal SYMESZ. */
/******************************************************/
