
union auxent
{
    struct
    {
	long                    x_tagndx;
	union
	{
	    struct
	    {
		unsigned short  x_lnno;
		unsigned short  x_size;
	    } PACKED x_lnsz;
	    long                x_fsize;
	} PACKED x_misc;
	union
	{
	    struct
	    {
		long            x_lnnoptr;
		long            x_endndx;
	    } PACKED x_fcn;
	    struct
	    {
		unsigned short  x_dimen[DIMNUM];
	    } PACKED x_ary;
	} PACKED x_fcnary;
	unsigned short          x_tvndx;
    } PACKED x_sym;
    struct
    {
	char                    x_fname[FILNMLEN];
    } PACKED x_file;
    struct
    {
	long                    x_scnlen;
	unsigned short          x_nreloc;
	unsigned short          x_nlinno;
    } PACKED x_scn;
} PACKED;

typedef union auxent AUXENT;

#define     AUXESZ  sizeof(union auxent)

/******************************************************/
/* Note that auxiliary entries must be the same size  */
/* as a symbol entry, i.e., AUXESZ must equal SYMESZ. */
/******************************************************/

#define AUX_FILENAME(aux)  ((aux).x_file.x_fname)
#define AUX_TAGINDEX(aux)  ((aux).x_sym.x_tagndx)
#define AUX_FUNC_SIZE(aux) ((aux).x_sym.x_misc.x_fsize)
#define AUX_LINENO(aux)    ((aux).x_sym.x_misc.x_lnsz.x_lnno)
#define AUX_END_INDEX(aux) ((aux).x_sym.x_fcnary.x_fcn.x_endndx)
#define AUX_SIZE(aux)      ((aux).x_sym.x_misc.x_lnsz.x_size)
#define AUX_DIMEN(aux)     ((aux).x_sym.x_fcnary.x_ary.x_dimen)
#define AUX_SCNLEN(aux)    ((aux).x_scn.x_scnlen)
#define AUX_NRELOC(aux)    ((aux).x_scn.x_nreloc)
#define AUX_NLINENO(aux)   ((aux).x_scn.x_nlinno) 
#define AUX_LINEPTR(aux)   ((aux).x_sym.x_fcnary.x_fcn.x_lnnoptr)
#define AUX_TVNDX(aux)     ((aux).x_sym.x_tvndx)

/* Define the three classes of auxillary symbols. The three classifications
 * correspond to the structures defined above. This distinction is necessary
 * because we need to know what fields we are writing out in the interest of
 * object file portability.
 */

#define AUX_CLASS_SYM          0x01
#define AUX_CLASS_FILE_NAME    0x02
#define AUX_CLASS_SECTION      0x03


/* These are handy-dandy macros for taking apart aux symbol entries as
   is done in cswap or cdump.  Most of this is based upon the
   "Auxiliary Table Entries" table (figure 11-34) in the "Common
   Object File Format (COFF) chapter of the ATT Programmer's Guide. */

/* not of type "member of enumeration" */
#define NOT_MOE(x) ((x->n_type & OBJ_BASIC_TYPE_MASK) != T_MOE)

/* not a derived type */
#define NO_DT(x) ((x->n_type & OBJ_TYPE_MASK) == DT_NON)

/* validate names related to arrays, structures, unions, and
   enumarations according to their type and storage class. */
#define VALIDNAME(x)                  \
(NOT_MOE(x)                           \
 && ((x->n_sclass == C_AUTO)          \
     || (x->n_sclass == C_EXT)        \
     || (x->n_sclass == C_STAT)       \
     || (x->n_sclass == C_REG)        \
     || (x->n_sclass == C_MOS)        \
     || (x->n_sclass == C_ARG)        \
     || (x->n_sclass == C_MOU)        \
     || (x->n_sclass == C_TPDEF)      \
     || (x->n_sclass == C_REGPARM)))  \

/* filename */
#define X_FILENAME(x)                 \
((x->n_sclass == C_FILE)              \
 && (x->n_type == T_NULL))

/* section */
#define X_SECTION(x)                  \
((x->n_sclass == C_STAT)              \
 && (x->n_type == T_NULL))

/* tag name */
#define X_TAGNAME(x)                  \
(((x->n_sclass == C_STRTAG)           \
  && (x->n_type == T_STRUCT))         \
 || ((x->n_sclass == C_UNTAG)         \
     && (x->n_type == T_UNION))       \
 || ((x->n_sclass == C_ENTAG)         \
     && (x->n_type == T_ENUM)))

/* end of structure */
#define X_EOS(x)                      \
((x->n_sclass == C_EOS)               \
 && (x->n_type == T_NULL))

/* function */
#define X_FUNCNAME(x)                 \
(((x->n_sclass == C_EXT)              \
  || (x->n_sclass == C_STAT))         \
 && (IS_FCN(x->n_type))               \
 && NOT_MOE(x))

/* array */
#define X_ARRAY(x)                    \
(VALIDNAME(x)                         \
 && (IS_ARY(x->n_type)))

/* beginning and end of block */
#define X_BLOCK(x)                    \
((x->n_sclass == C_BLOCK)             \
 && (x->n_type == T_NULL))

/* befinning and end of function */
#define X_FUNCTION(x)                 \
((x->n_sclass == C_FCN)               \
 && (x->n_type == T_NULL))

/* name related to structure, union, enumeration */
#define X_SUENAME(x)                  \
(VALIDNAME(x)                         \
 && ((IS_PTR(x->n_type))              \
     || (IS_ARY(x->n_type))           \
     || NO_DT(x))                     \
 && ((x->n_type & T_STRUCT)           \
     || (x->n_type & T_UNION)         \
     || (x->n_type & T_ENUM)))

/* bitfield element */
#define X_BITFLDEL(x)                 \
((x->n_sclass == C_FIELD)             \
 && NO_DT(x))

