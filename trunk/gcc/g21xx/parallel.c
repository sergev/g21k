#include "config.h"
#include "rtl.h"
#include "insn-config.h"
#include "output.h"
#include "recog.h"
#include "hard-reg-set.h"

#ifdef __WATCOMC__
#include "watproto.h"
#include <stdlib.h>
#endif 

/** this should be in tm.h */

#define MAX_PARALLEL 3

extern rtx peep_insn[];

#include "recog.h"

/*** Delete ';' from the copy of TEMPLATE and return the copy ***/

char *
fix_template(template)
    char *template;
{
    int len = strlen(template);
    char *p;
    static char new_template[111];

    if (*template == '*') abort();

    strcpy(new_template, template);
    for(p = new_template + len;
	p > new_template && *p != ';';
	p--);
    if (p != new_template) *p = 0;
    return(new_template);
}

char *peep_templates[6];
char *peep_note;
int peep_n_operands[6];
static struct 
{
  int found;
  int where;
  char *what;
  char *note;
} special;

rtx peep_old_operands[6][10];

void
collect_templates_and_operands(n, perm)
    int n;
    int perm[];
{
    int ii, i, insn_code_number;
    char* template; 
    peep_note=0;
    for(ii=0; ii<n; ii++) {
	rtx insn = peep_insn[perm[ii]];

	insn_code_number = recog_memoized (insn);
	insn_extract (insn);
	constrain_operands(insn_code_number, 1);
	if(special.found && special.where==ii)
	  {
	    template=special.what;
	    peep_note=special.note;
	  }
	else
	  {
	    template = insn_template[insn_code_number];
	    if (template == 0)
	      template = (*insn_outfun[insn_code_number]) (recog_operand, insn);
	  }
	peep_templates[ii] = template;
	peep_n_operands[ii] = insn_n_operands[insn_code_number];
	for(i=0; i<peep_n_operands[ii]; i++)
	    peep_old_operands[ii][i] = recog_operand[i];
    }
    
}

int reorder(n,perm,s,f)
int n, *perm, f();
rtx *s;
{
  int i;
  rtx *tmp=alloca(sizeof (rtx)*n);
  special.found=0;
  for(i=0;i<n;i++)
    tmp[i]=NULL;
  return re_x(n,n,perm,s,tmp,f);
}

int re_x(n,x,perm,in,try,f)
rtx in[],try[];
int *perm;
int n,x,f();
{
  int i;
  if(n==0)
    return (*f)(try);
  for(i=0;i<x;i++)
    if(try[i]==NULL)
      {
	try[i]=in[n-1];
	if(re_x(n-1,x,perm,in,try,f))
	  {
	    perm[i]=n-1;
	    return 1;
	  }
	try[i]=NULL;
      }
  return 0;
}

#if MAX_PARALLEL >= 2

int
peep_parallelizeable_2(active)
    int active;
{
    int perm[2];
    extern int parallel_2_p();
    
    if (INSN_CODE(peep_insn[0]) < 0) 
      return 0;
    if (INSN_CODE(peep_insn[1]) < 0) 
      return 0;

    if (! set_use_independent_p(peep_insn, 2))
	return 0;
    
    if(reorder(2,perm,peep_insn,parallel_2_p))
      {
	  if (active)
	      collect_templates_and_operands(2, perm);
	return 1;
      }

    return 0;
}

#if MAX_PARALLEL >= 3

int
peep_parallelizeable_3(active)
    int active;
{
    int perm[3];
    extern int parallel_3_p();

    if (INSN_CODE(peep_insn[0]) < 0) return 0;
    if (INSN_CODE(peep_insn[1]) < 0) return 0;
    if (INSN_CODE(peep_insn[2]) < 0) return 0;

    if (! set_use_independent_p(peep_insn, 3))
      return 0;

    if(reorder(3,perm,peep_insn,parallel_3_p))
      {
	  if (active)
	      collect_templates_and_operands(3, perm);
	  return 1;
      }
    return 0;
}

#if MAX_PARALLEL >= 4

int
peep_parallelizeable_4(active)
    int active;
{
    int perm[4];
    extern void parallel_4_p();

    if (INSN_CODE(peep_insn[0]) < 0) 
      return 0;
    if (INSN_CODE(peep_insn[1]) < 0) 
      return 0;
    if (INSN_CODE(peep_insn[2]) < 0) 
      return 0;
    if (INSN_CODE(peep_insn[3]) < 0) 
      return 0;

    if (! set_use_independent_p(peep_insn, 4)) 
      return 0;

    if(reorder(4,perm,peep_insn,parallel_4_p))
      {
	  if (active)
	      collect_templates_and_operands(4, perm);
	  return 1;
      }
    return 0;
}

#if MAX_PARALLEL >= 5

peep_parallelizeable_5(active)
    int active;
{
    int perm[5];
    extern void parallel_5_p();
    
    if (INSN_CODE(peep_insn[0]) < 0) 
      return 0;
    if (INSN_CODE(peep_insn[1]) < 0) 
      return 0;
    if (INSN_CODE(peep_insn[2]) < 0) 
      return 0;
    if (INSN_CODE(peep_insn[3]) < 0) 
      return 0;
    if (INSN_CODE(peep_insn[4]) < 0) 
      return 0;
    if (! set_use_independent_p(peep_insn, 5)) 
      return 0;
    if(reorder(5,perm,peep_insn,parallel_5_p))
      {
	  if (active)
	      collect_templates_and_operands(5, perm);
	  return 1;
      }
    return 0;
}

#endif
#endif
#endif
#endif

int short_hand_p;

char *
  dump_parallel (n)
{
  int ii;
  short_hand_p = 1;
  putc('\t', asm_out_file);
  for (ii=0; ii<n; ii++) 
    {
      if(ii>0) 
	putc(',', asm_out_file);
      output_asm_insn (fix_template(peep_templates[ii]),
		       peep_old_operands[ii]);
    }
  putc(';', asm_out_file);
  if(peep_note)
    {
      fputs("   !",asm_out_file);
      fputs(peep_note,asm_out_file);
    }
  short_hand_p = 0;
  return "";
}

static HARD_REG_SET set_regs;

static void
mark_reg_as_set(r, something)
    rtx r, something;
{
#ifdef __WATCOMC__
  something;
#endif
    SWITCH_SUBREG(r);
    if (REG_P(r))
      SET_HARD_REG_BIT(set_regs, REGNO(r));
    else 
      if(GET_CODE(r)==SUBREG)
	{
	  int i;
	  r = SUBREG_REG(r);
	  for(i=0;i<HARD_REGNO_NREGS(REGNO(r),GET_MODE(XEXP(r,0))); i++)
	    SET_HARD_REG_BIT(set_regs,REGNO(r)+i);
	}
}

/*** Returns true if insns in INSN_ARR (of length N_INSNS) ***/
/*** are set/use independent, i.e. no register set by any of ***/
/*** insns is used by any  of the following ones. ***/

/*** This is NOT a complete independency checking -- for example ***/
/*** it does not check the memory access. For 21k and 21xx this is enought ***/
/*** because only accesses to different memories can be parallelized ***/
/*** This will not work for chips with 2 or more buses to the same memory ***/


int
set_use_independent_p(insn_arr, n_insns)
    rtx insn_arr[];
    int  n_insns;
{
    int inum;
    void mark_reg_as_set();

    CLEAR_HARD_REG_SET(set_regs);
    for(inum=0; inum<n_insns-1; inum++) {
	rtx insn = insn_arr[inum], next_insn;
	if (GET_CODE(insn) != INSN) return 0;
	note_stores(PATTERN(insn_arr[inum]), mark_reg_as_set);
	next_insn = insn_arr[inum+1];
	if (rtx_mentions_hard_regs_p(PATTERN(next_insn), set_regs))
	    return 0;
    }
    /*** If we got out of the loop, we are fine ***/
    return 1;
}


/*** True if X refers to any register from REGSET ***/

int
rtx_mentions_hard_regs_p(x, regset)
    rtx x;
    HARD_REG_SET regset;
{
    char *format;
    enum rtx_code xcode;
    int i;

    SWITCH_SUBREG(x);
    xcode = GET_CODE(x);
    if ((xcode == REG) && (TEST_HARD_REG_BIT(regset, REGNO(x))))
	return 1;
    format = GET_RTX_FORMAT(xcode);

    for (i = GET_RTX_LENGTH (xcode) - 1; i >= 0; i--)
    {
	if (format[i] == 'E')
	{
	    register int j;
	    for (j = XVECLEN (x, i) - 1; j >= 0; j--)
		if (rtx_mentions_hard_regs_p (XVECEXP (x, i, j), regset))
		    return 1;
	}
	else if (format[i] == 'e'
		 && rtx_mentions_hard_regs_p(XEXP (x, i), regset))
	    return 1;
    }
    return 0;
}

int
incremented_memory_operand(op,mode)
     register rtx    op;
     enum machine_mode mode;
{
#ifdef __WATCOMC__
  mode;
#endif
  if (GET_CODE(op) == MEM)  
    {
      enum rtx_code   code = GET_CODE(XEXP (op,0));
      return (code == POST_INC || code == POST_MODIFY);
    }
  return 0;
}

int
opposite_banks (op1,op2)
     rtx op1,op2;
{
  enum machine_mode m1 = GET_MODE (XEXP (op1,0));
  enum machine_mode m2 = GET_MODE (XEXP (op2,0));
  return ((m1 == DMmode && m2 == PMmode) || (m1 == PMmode && m2 == DMmode));
}
