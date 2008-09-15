#include <stdio.h>
#include "config.h"
#include "rtl.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "tree.h"
#include "basic-block.h"
#include "doloop.h"

/* Identification String */ static char *__ident__ = "@(#) debug-prints.c 2.8@(#)";

/*** This file contains function which can be used to do various ***/
/*** dumps in a debugger.     (az, 12/20/92) ***/

/** variable x is convenient for using "set x=" instead of "p" **/
/** this allows one to call a function without printing its returned */
/** value **/

int x;

char *helparr[] = {
    "pr (rtx_first)\t\t-- print all rtl's",
    "dr (rtx)\t\t-- display single rtl",
    "fi (n,rtx)\t\t--find insn by its uid",
    "drl (rtx)\t\t-- print rtx list",
    "drtxl (f,l)\t\t -- print inclusive list of rtx",
    "llen (rtx)\t\t-- return length of rtx list",
    "pregs()\t\t\t-- Prints mapping of pseudo to hard regs.",
    "",
    "p_reg_subclasses()\t-- Shows \"Subclass\" relation matrix",
    "p_reg_superclasses()\t-- Shows \"Superclass\" relation matrix",
    "p_reg_subunion()\t-- Shows \"SuperUnion\" relation matrix",
    "p_reg_superunion()\t-- Shows \"SubUnion\" relation matrix",
    "p_reg_inclusion()\t-- Shows inclusion relation matrix(boolean)",
    "pclobbers()\t-- Shows how many times registers were set/clobbered",
    "p_reg_class_size()\t-- Sizes of Register Classes",
    "pclasses()\t\t-- Dump Contents of Each Register Class",
    "reg_uservar_p(x)\t-- True if reg. rtx correspond to user variable",
    "",
    "insn_deleted_p(x)",
    "get_rtx_length(x)",
    "get_code(x)",
    "get_rtx_class(x)",
    "pattern(i)\t\t-- PATTERN of insn i",
    "xexp(x,n)\t\t-- Expression Operand number n",
    "xint (x,n)\t\t-- Integer Operand number n",
    "xstr(x,n)\t\t-- String Operand number n",
    "xvec(x,n)\t\t-- Vector Operand number n",
    "xveclen(x,n)\t\t-- Length of Vector Operand number n",
    "xvecexp(x,n,en)\t\t-- Element number en of Vector Operand number n",
    "jump_label(x)\t\t-- The target LABEL of a branch instruction",
    "label_refs(x)\t\t-- The label refs chain",
    "doloop_start_label(x)\t\t-- the start of a do loop",
    "doloop_end_label(x)\t\t-- the end of a do loop",
    "regmemberp(r,rclass)\t--True if register R belongs to RCLASS",
    "dumpivs()\t\t--Dump Induction Variables",
    "all_union_check()\t--Check Subunions and Superunions or Reg.Classes",
    0
    };

char *reg_name(rn)
    int rn;
{
    static int n_named_regs = sizeof(reg_names)/sizeof (char *);
    static char buf[11];

    char *name = buf;
    if (rn < n_named_regs) name = reg_names[rn];
    else sprintf(buf, "R%d", rn);
    return(name);
}

#define MAX_REG_CLASS_NAME_LEN 8
static char* reg_class_names[] = REG_CLASS_NAMES;

char *reg_class_name(class)
    int class;
{
    static char* reg_class_names[] = REG_CLASS_NAMES;
    char *name;
    static char buf[MAX_REG_CLASS_NAME_LEN + 1];

    if (class == GENERAL_REGS) name = "GEN_REGS";
    else if (class >= LIM_REG_CLASSES)name = "-LIM-";
    else if (class < NO_REGS) name = "---";
    else name = reg_class_names[class];
    if (strlen(name) > MAX_REG_CLASS_NAME_LEN) {
	strncpy(buf, name, 8);
	*(buf + 8) = '\0';
	name = buf;
	}
    return(name);
}


void pr(rtx_first)
     rtx rtx_first;

{
    print_rtl (stderr, rtx_first);
}

/* A version of print_rtl which prints a specified number of insn's around
   the given one. **/

void
prn (insn, count)
    rtx insn;
    int count;
{
  register rtx tmp_rtx, i;
  register rtx rtx_first;
  int n=count;
  
  /** find the one which is "count" insns before "insn" **/

  for(i=insn; i && n >= 0; n--) {
      rtx_first = i;
      i = PREV_INSN(i);
  }

  if (rtx_first == 0)
    fprintf (stderr, "(nil)\n");
  else switch (GET_CODE (rtx_first)) {
    case INSN:
    case JUMP_INSN:
    case CALL_INSN:
    case NOTE:
    case CODE_LABEL:
    case BARRIER:
      n = - count;
      for (tmp_rtx = rtx_first;
	   NULL != tmp_rtx && n++ <= count;
	   tmp_rtx = NEXT_INSN (tmp_rtx))
      {
	  debug_rtx (tmp_rtx);
	  fprintf (stderr, "\n");
      }
      break;
    default: debug_rtx (rtx_first);
  }
}


/**** A shorter name for "debug_rtx" ***/

rtx dr(the_rtx)
    rtx the_rtx;
{
    debug_rtx(the_rtx);
    return(the_rtx);
}

tree dt(the_tree)
    tree the_tree;
{
    debug_tree(the_tree);
    return(the_tree);
}

/****** Print list of rtx's **************/

void drl(first_rtx)
    rtx first_rtx;
{
   rtx cons = first_rtx;
   rtx car;
   while (cons) {
       car = XEXP(cons,0);
       debug_rtx(car);
       cons=XEXP(cons,1);
   }
}


int llen(first_rtx)
    rtx first_rtx;
{
   int len = 0;
   rtx cons = first_rtx;
   while (cons) {
       len++;
       cons=XEXP(cons,1);
   }
   return(len);
}

void drtxlf (file, first_rtx, last_rtx)
     FILE *file;
     rtx first_rtx, last_rtx;
{
  rtx x;

  fprintf (file, ";;-- [ %d ... %d ]\n", INSN_UID (first_rtx), INSN_UID (last_rtx));

  for (x = first_rtx; 
       x && x != NEXT_INSN (last_rtx); 
       x = NEXT_INSN (x))
    debug_rtxf (file, x);
}

void drtxl (first_rtx, last_rtx)
     rtx first_rtx, last_rtx;
{
  drtxlf (stderr, first_rtx, last_rtx);
}

/****** Register Class print-outs *************/

void p_reg_matrix (header, matrix)
    char *header;
    int matrix[N_REG_CLASSES][N_REG_CLASSES];
{
    int i,j;
    fprintf(stderr, "--------------- %s ------------------\n", header);
    for(i=0; i<N_REG_CLASSES; i++) {
	char *name = reg_class_name(i);
	fprintf(stderr, "%-8s ", name);
    }
    fprintf(stderr, "\n\n");
    for(i=0; i<N_REG_CLASSES; i++) {
	for(j=0; j<N_REG_CLASSES; j++) {
	    int n =  matrix[i][j];
	    char *name = reg_class_name(n);
	    if (name == 0) name = "--------";
	    fprintf(stderr, "%-8s ", name);
	}
	fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

/*** print out inclusion relation between classes ***/

void p_reg_inclusion ()
{
    enum reg_class c1, c2;
    int i,j;
    for(i=0;i<N_REG_CLASSES;i++) {
	c1 = i;
	for(j=0;j<N_REG_CLASSES;j++) {
	    c2 = j;
	    fprintf(stderr, "%c",
		    reg_class_subset_p(c1, c2) ? '*' : '-');
	}
	fprintf(stderr, "\n");
    }
}

/*** Print out the matrix of subclass relation ***/

void p_reg_subclasses ()
{
    p_reg_matrix("Subclasses", reg_class_subclasses);
}

void p_reg_superclasses ()
{
    p_reg_matrix("SuperClasses", reg_class_superclasses);
}

void p_reg_subunion ()
{
    p_reg_matrix("SubUnion", reg_class_subunion);
}

void p_reg_superunion ()
{
    p_reg_matrix("SuperUnion", reg_class_superunion);
}




void pregs()
{
    fprintf(stderr, "-- Pseudo to Hard Register Mapping --\n");
    if (reg_renumber == 0)
	fprintf(stderr, " ---- Has not been mapped yet ---\n ");
    else {
	int i;
	for(i=FIRST_PSEUDO_REGISTER; i < max_regno; i++) {
	    int new = reg_renumber[i];
	    if (new != -1)	    
		fprintf(stderr, "%3d -> %s (%d), ", i,
			reg_name(new), new);
	}
	fprintf(stderr, "\n");
    }
}

void pclass(hrs)
    HARD_REG_SET hrs;
{
    int rno, first = 1;
    for (rno=0; rno<FIRST_PSEUDO_REGISTER; rno++) {
	if (TEST_HARD_REG_BIT(hrs, rno)) {
	    if (! first) fprintf(stderr,",");
	    fprintf(stderr,reg_name(rno));
	    first = 0;
	}
    }
    fprintf(stderr,"\n");
}


void pclasses()
{
    int rclass;
    for(rclass=0;rclass<LIM_REG_CLASSES; rclass++) {
	int rno, first = 1;
	fprintf(stderr,"%s:\t", reg_class_names[rclass]);
	pclass(reg_class_contents[rclass]);
    }
}

void pclobbers()
{
    int rn;
    if (reg_n_sets == 0) return;
    for(rn=0; rn<max_regno; rn++) {
	int nclob = *(reg_n_sets + rn);
	if (nclob) fprintf(stderr, "%s - %d,  ", reg_name(rn), nclob);
    }
    fprintf(stderr, "\n");
}

void p_reg_class_size()
{
    int i;
    extern reg_class_size[];
    for(i=0; i<N_REG_CLASSES; i++) {
	char *name = reg_class_name(i);
	fprintf(stderr, "%8s: %3d\n", name, reg_class_size[i]);
    }
}

void phelp()
{
    char **he;
    fprintf(stderr, "+++++++++ Available Printouts: ++++++++\n");
    for(he = helparr; he < helparr + sizeof helparr && *he != 0; he++)
	fprintf(stderr, "\t%s\n", *he);
    fprintf(stderr, "\n");
}

/*****************************************************************************/
/********** Access functions (for macros, which are unaccessibles in gdb) ****/
/*****************************************************************************/

int   insn_uid(i)   rtx  i; {return (INSN_UID(i));}

rtx   prev_insn(i)   rtx  i; {return (PREV_INSN(i));}


/*************************** next_insn is defined in emit-rtl.c ********/
/**** rtx   next_insn(i)   rtx  i; {return (NEXT_INSN(i));}     *******/
/*********************************************************************/
    
int   reg_uservar_p(x) rtx x; {return REG_USERVAR_P(x);}

int   insn_deleted_p(x) rtx x; {return INSN_DELETED_P(x);}

int   get_rtx_length(x) rtx x; {return GET_RTX_LENGTH (x);}


int   get_code(x) rtx x; {return GET_CODE(x);}


int   get_rtx_class(x) rtx x; {return GET_RTX_CLASS (x);}

rtx   xexp (x,n) rtx x; int n;{return(XEXP(x,n));}

int   xint (x,n) rtx x; int n;{return(XINT(x,n));}

char* xstr (x,n) rtx x; int n;{return(XSTR(x,n));}

rtx jump_label (x) rtx x; { return JUMP_LABEL (x); }
rtx doloop_start_label (x) rtx x; { return DOLOOP1_START_LABEL (x); }
rtx doloop_end_label (x) rtx x;   { return DOLOOP1_END_LABEL (x); }
rtx label_refs (x) rtx x; {return LABEL_REFS (x); }

struct rtvec_def* xvec(x,n) rtx x; int n;{return(XVEC(x,n));}

int xveclen(x,n) rtx x; int n;{return(XVECLEN(x,n));}

rtx xvecexp(x,n,en) rtx x; int n, en;{return(XVECEXP(x,n,en));}

rtx fi(n,f)
    rtx f;
{
    rtx i;
    for(i=f; i; i=NEXT_INSN(i))
	if (INSN_UID(i) == n) {
	    debug_rtx(i);
	    return i;
	}
    return 0;
}



int obs(size)
    int size;
{
    extern char* oballoc();
    extern void obfree();

    char *p1, *p2, *p3;
    p1 = oballoc(size);
    p2 = oballoc(size);
    p3 = oballoc(size);
    obfree(p3);
    obfree(p2);
    obfree(p1);
    return (int) p1;
}

int regmemberp (regno, regclass)
    int regno;
    HARD_REG_SET regclass;
{
    return (TEST_HARD_REG_BIT(regclass,regno));
}

rtx pattern(insn)
    rtx insn;
{
    return (PATTERN(insn));
}

int reg_class_from_letter(c)
    int c;
{
    return(REG_CLASS_FROM_LETTER(c));
}


/************** Induction Variables (strength reduction): ****************/

#include "loop.h"

#define BOOLSTR(X) ((X)?"True":"False")

#define MODESTR(M) (((M)==SImode)?"SI":((M)==PMmode)?"PM":((M)==DMmode)?"DM":\
((M)==SFmode)?"SF":((M)==VOIDmode)?"VOID":"???")


rtx shortdr(r)
    rtx r;
{
    int success = 1;
    if (r) {
	switch(GET_CODE(r)) {
	  case REG:
	    fprintf(stderr, "R%d(%s),",XINT(r,0),MODESTR(GET_MODE(r)));
	    break;
	  case CONST_INT:	fprintf(stderr, "%d,",  XINT(r,0));	break;
	  default:		dr(r);
	}
    } else fprintf(stderr, "(nil),");
    return(r);
}

char *
reg_iv_type_name (ivtype)
    enum iv_mode ivtype;
{
    switch (ivtype) {
      case UNKNOWN_INDUCT:   return "UNKNOWN_INDUCT";
      case BASIC_INDUCT:     return "BASIC_INDUCT";
      case NOT_BASIC_INDUCT: return "NOT_BASIC_INDUCT";
      case GENERAL_INDUCT:   return "GENERAL_INDUCT";
      default:
      {
	  static char buf[22];
	  sprintf(buf,"<BAD: %d> ",(int)ivtype);
	  return(buf);
      }
    }
}

/***** Dump one iv_class structure **********/

#define PRNL fprintf(stderr,"\n");

struct induction *
print_induction(indp)
    struct induction *indp;
{
    fprintf(stderr,
	    " ------- INDUCTION (mode %s)--------------------------------\n",
	    MODESTR(indp->mode));
    fprintf(stderr,"  Insn:");		shortdr(indp->insn);	PRNL;
    fprintf(stderr,"  New_reg:\t");	shortdr(indp->new_reg);	PRNL;
    fprintf(stderr,"  Src_Reg:\t");	shortdr(indp->src_reg);	PRNL;
    fprintf(stderr,"  Dest_Reg:\t");	shortdr(indp->dest_reg);	PRNL;
    fprintf(stderr,"  TYPE:\t%s\t",indp->giv_type == DEST_REG?"DEST_REG":"DEST_ADDR");	PRNL;
    fprintf(stderr,"  MULT:");		shortdr(indp->mult_val);
    fprintf(stderr,"  ADD:");		shortdr(indp->add_val);
#if 0
    fprintf(stderr,"  Final Value:");	shortdr(indp->final_value);
    fprintf(stderr,"  Benefit=%d:\n", indp->benefit);
    fprintf(stderr,"  ignore=%d:\n",   indp->ignore);
    if (indp->same) {
      fprintf(stderr,"***********SAME**************\n");
      print_induction (indp->same);
      fprintf(stderr,"***********SAME**************\n");
    }
#endif
    PRNL;
    return(indp);
}


struct iv_class* dump1iv(civp)
    struct iv_class *civp;
{
    int rnum = civp->regno;
    fprintf(stderr, "--- Reg.%d ---\n", rnum);
#if 0
    fprintf(stderr,
	    "\tCnt=%d,GIVcnt=%d ", civp->biv_count, civp->giv_count);
    if (civp->initial_value) {
	fprintf(stderr, " Init Val: ");
	shortdr(civp->initial_value);
    }
    if (civp->initial_test) {
	fprintf(stderr, "Init Test:");
	shortdr(civp->initial_test);
    }
    if (civp->init_insn) {
	fprintf(stderr, "Init Insn: %d,", INSN_UID(civp->init_insn));
    }
    if (civp->init_set) {
	fprintf(stderr, "Init Set:");
	shortdr(civp->init_set);
    }
    fprintf(stderr,
	    "Incr: %s, Elim: %s, Nonneg: %s, Reversed: %s, Mode: %s\n\n",
	    BOOLSTR(civp->incremented),
	    BOOLSTR(civp->eliminable),
	    BOOLSTR(civp->nonneg),
	    BOOLSTR(civp->reversed),
	    reg_iv_type_name(reg_iv_type[rnum])
	    );
#endif
  {
      struct induction *cgiv;
      fprintf(stderr, "----------- GIVs: ----------\n");
      for(cgiv=civp->giv; cgiv; cgiv = cgiv->next_iv) {
	  print_induction(cgiv);
      }
  }
    return civp;
}

int
print_reg_iv_info(regno)
    int regno;
{
    int f = 0;
    enum iv_mode    ivtype = reg_iv_type[regno];
    struct induction *rind = reg_iv_info[regno];
    struct iv_class   *ivc = reg_biv_class[regno];
    
    if (ivtype == GENERAL_INDUCT ||
	ivtype == BASIC_INDUCT ||
	ivtype == NOT_BASIC_INDUCT) {
	if (rind) {
	    if (! f++)
		fprintf(stderr, "  --- REG. %d --, iv type: %s\n",
			regno, reg_iv_type_name(reg_iv_type[regno]));
	    print_induction(rind);
	}
	if (ivc) {
	    if (! f++)
		fprintf(stderr, "  --- REG. %d --, iv type: %s\n",
			regno, reg_iv_type_name(reg_iv_type[regno]));
	    dump1iv(ivc);
	}
    }
    return(regno);
}

dumpivs()
{
    struct iv_class *civp;
    int rno;
    fprintf(stderr,"=============== loop_iv_list: ===========\n");
    for (civp=loop_iv_list; civp; civp=civp->next) dump1iv(civp);
    fprintf(stderr,"=============== IV Registers: ===========\n");
    for(rno=0; rno < max_reg_before_loop; rno++)
	print_reg_iv_info(rno);
}


check1induction(v)
    struct induction *v;
{
    rtx i = v->insn;
    if (i->fld[0].rtint > max_uid_for_loop)
    {
	dr(i);
	abort();
    }
}

int check_all_inductions()
{
    struct iv_class *bl;
    int n=0;
    for (bl = loop_iv_list; bl; bl = bl->next)
    {
	struct induction *v;
	for (v = bl->giv; v; v = v->next_iv) {
	    check1induction (v);
	    n++;
	}
    }
    return n;
}

/********* Unions of register classes: **********************/


#define UWARN(W) fprintf(stderr,"==> %s\tis %s as %sunion of %s\t and %s\n",\
reg_class_names[uset],W,prefix,reg_class_names[set1],reg_class_names[set2])

int
check_reg_union(prefix,set1, set2, uset)
    char *prefix;
    int set1, set2, uset;
{
    HARD_REG_SET   u_perfect, u_real;
    COPY_HARD_REG_SET(u_real,    reg_class_contents[uset]);
    COPY_HARD_REG_SET(u_perfect, reg_class_contents[set1]);
    IOR_HARD_REG_SET(u_perfect,  reg_class_contents[set2]);

    GO_IF_HARD_REG_SUBSET(u_real, u_perfect, maybe_small);
    GO_IF_HARD_REG_SUBSET(u_perfect, u_real, big_union);
    goto weird_union;
    
  maybe_small:
    GO_IF_HARD_REG_SUBSET(u_perfect, u_real, normal_union);
    goto small_union;

  weird_union:  UWARN("weird");     return(-99);
  small_union:  UWARN("too small"); return(-1);
  big_union:    UWARN("too big");   return(1);
  normal_union: return(0);
}

int
check_subunion(s1, s2)
    int s1, s2;
{
    return check_reg_union("sub",   s1, s2, reg_class_subunion[s1][s2]);
}

int
check_superunion(s1, s2)
    int s1, s2;
{
    return check_reg_union("super", s1, s2, reg_class_superunion[s1][s2]);
}

void
all_union_check()
{
    int t,i,j;
    for(t=0; t<2; t++)
	for(i=0; i<N_REG_CLASSES; i++)
	    for(j=0; j<N_REG_CLASSES; j++)
		(t ? check_subunion : check_superunion) (i, j);
}



void
block_live_regs(bnum)
    int bnum;
{
    int bitnum=0, first_p=1, i,j;
    regset bl = basic_block_live_at_start[bnum];

    for(i=0; i<regset_size; i++) {
	long rsword = bl[i];
	for(j=0; j<REGSET_ELT_BITS; (j++,bitnum++)) {
	    if (rsword & 1) {
		if (! first_p) putc(',', stderr);
		else first_p = 0;
		fprintf(stderr,"%d", bitnum);
	    }
	    rsword >>= 1;
	}
    }
    putc('\n', stderr);
}

void
print_live_regs()
{
    int bn;
    for(bn=0; bn<n_basic_blocks; bn++) {
	fprintf(stderr, "%2d(%d-%d):\t", bn,
		basic_block_head[bn]->fld[0].rtint,
		basic_block_end [bn]->fld[0].rtint);
	block_live_regs(bn);
    }
}

void
print_next_use(a,n)
    rtx a[];
    int n;
{
    int i, first_p=1;
    putc('[', stderr);
    for(i=0; i<n; i++) {
	if (a[i]) {
	    if (first_p) first_p = 0;
	    else putc(',', stderr);
	    fprintf(stderr, "r%d:i%d", i, a[i]->fld[0].rtint);
	}
    }
    fprintf(stderr, "]\n");
}
