/* This function is meant to be  called once for  each loop right */
/* after loop_scan while all information computed by loop_scan is */
/* still valid.                                                   */



#include "doloop.h"

#define CHECK_DOLOOP 1

/* Identification String */ static char *__ident___doloop_c___ = "@(#) doloop.c 2.5@(#)";

#ifndef MAX_DOLOOP_DEPTH
#define MAX_DOLOOP_DEPTH 1
#endif

typedef struct {
    int loop_count;	/* Positive if loop count is constant, otherwise -1 */
    			/* The rest of fields is meaningless if loop_count */
			/* is set */
    rtx start_value;	/* Starting value for basic induction variable (biv)*/
    rtx end_value;	/* Limit for biv */
    int incr;		/* Increment for biv -- must be constant */
    int off_by_one;	/* 1 for "<", 0 for "<=" */
    int unsigned_p;	/* True if unsigned comparison in the end of loop */
    rtx loop_start;
} doloop_info;

static int *loop_depth_array;


extern void iteration_info();
extern int  comparison_operator();


static void compute_loop_depths(void);
static int  find_loop_comp_and_jump(rtx, rtx *, rtx *);
static int  unjumped_loop_p (rtx, rtx, rtx *, int *);
static int  get_loop_depth (rtx, rtx);
static int  can_compute_loop_counter (rtx, rtx, doloop_info *);

static void fix_label(rtx);
static rtx  label_after_insn (rtx);
static void mark_doloop_jump (rtx);
static void convert_comparison_to_doloop_end(rtx, rtx, rtx);
static rtx  emit_doloop_start(rtx, rtx, rtx, int, int);
static rtx  emit_counter_init_expr (doloop_info *);

static void get_strength_reduction_benefit(void);
static int  insn_in_range(rtx, rtx, rtx);

static rtx handle_posttest_loops (rtx counter_init, rtx start_label, rtx end_label);

#define boolean int

/** doloop_info has enough information to compute **/
/** rtx for loop counter **/


/***************************************************************************/
/*  The routines of this file are called at different times:               */
/*  -- during loop optimaization "prepare_doloop" is called on each loop   */
/*     while the data computed for that loop is still alive                */
/*  -- after loop optimization "label_doloops" is called to add extra      */
/*     labels to "stabilize" doloops, i.e. to reduce the risk of them      */
/*     being disturbed by the following phases.                            */
/*  -- recognizer calls the "undisturbed_doloop_p" predicate to match      */
/*     doloop patterns. This function asserts that the doloop has not      */
/*     been screwed up by some phase after the loop optimization.          */
/*  -- finally "assemble_doloop" is called at the final phase. It asserts  */
/*     once again that the doloop is still intact. If the assertion fails, */
/*     nothing is emitted, but if it holds, final comparison and jump are  */
/*     deleted and "DO" instruction is emitted.                            */
/***************************************************************************/


void
prepare_doloop (rtx loop_start, rtx loop_end)
{
    int loop_depth, need_padding_p = 0;
    rtx lcntr_init;
    rtx start_label, end_label;
    rtx comp_insn, jump_insn;
    rtx doloop_start;
    doloop_info dl_info;
    rtx continued_p;		/* either set to NIL or to reference insn */
    int hasbranch;

/*   Collect data and check possibility of using doloops */

    loop_depth = get_loop_depth(loop_start, loop_end);
    if (loop_depth > MAX_DOLOOP_DEPTH) 
	return;
    if (! unjumped_loop_p(loop_start, loop_end, &continued_p, &hasbranch))
	return;
    start_label = next_nonnote_insn (loop_start);
    if (GET_CODE(start_label) != CODE_LABEL)
	return;
    if (! find_loop_comp_and_jump (loop_end, &comp_insn, &jump_insn))
	return;
    if (start_label != JUMP_LABEL(jump_insn)) 
	return;
    
    if (! can_compute_loop_counter (loop_start, loop_end, &dl_info))
	return;
    
    /* Actually build and modify the program */

    lcntr_init = emit_counter_init_expr(&dl_info);
    if (lcntr_init == 0) 
	abort();
    end_label = label_after_insn(jump_insn);
    lcntr_init = handle_posttest_loops(lcntr_init, start_label, end_label);
    doloop_start = emit_doloop_start (lcntr_init, start_label,
				      end_label, loop_depth, hasbranch);
    convert_comparison_to_doloop_end(comp_insn, jump_insn, continued_p);
    fix_label(start_label);
    fix_label(end_label);
}


/* Find and record in PCOMP and PJUMP final comparison and jump */
/* insns of the loop specified by LOOP_END. Return 1 if both */
/* has been found, otherwise return 0. */

static int
find_loop_comp_and_jump ( rtx loop_end, rtx *pcomp, rtx *pjump)
{
    rtx final_comp, comp_pat;
    rtx final_jump = prev_nonnote_insn(loop_end);

    if (!final_jump) 
	return 0;

    final_comp = PREV_INSN(final_jump);
    if (! final_comp) 
	return 0;

    if ((GET_CODE(final_comp) != INSN))
      return 0;

    comp_pat = PATTERN(final_comp);

    if ((GET_CODE(comp_pat) != SET) ||
	(SET_DEST(comp_pat) != cc0_rtx))
	return 0;
    
    *pcomp = final_comp;
    *pjump = final_jump;
    return 1;
}

int
continued_doloop_p (insn)
    rtx insn;
{
    rtx cond, cond_subexpr, x1;
    rtx pattern = PATTERN(insn);

    if (GET_CODE(insn) != JUMP_INSN) 
	return 0;
    
    if (GET_CODE(pattern) != SET) return 0;
    if (GET_CODE(XEXP(pattern,0)) != PC) return 0;
    x1 = XEXP(pattern,1);
    if (GET_CODE(x1) != IF_THEN_ELSE) return 0;
    cond = XEXP(x1, 0);
    cond_subexpr = XEXP(cond,0);
    if (! ((GET_CODE(cond_subexpr) == UNSPEC_VOLATILE) &&
	   (XINT(cond_subexpr, 1) == 4)))
	abort();

    if (XVECEXP(cond_subexpr, 0, 0) == const0_rtx)
	return 0;
    if (XVECEXP(cond_subexpr, 0, 0) == const1_rtx)
	return 1;
    abort();
}

/* Convert the last comparison of the loop into a special insn */
/* and mark the following jump as that of the doloop */

static void
convert_comparison_to_doloop_end(rtx comp_insn, rtx jump_insn, rtx contp)
{
    rtx set_pat, uv_pat, jump_pat;

    jump_pat = PATTERN(jump_insn);
    
    if ((GET_CODE(jump_pat) == SET) &&
	(SET_DEST(jump_pat) == pc_rtx) &&
	(GET_CODE (SET_SRC (jump_pat)) == IF_THEN_ELSE))

	mark_doloop_jump (XEXP (SET_SRC (jump_pat),0));

    else
	abort();	/** Not a "good" jump */
	
    uv_pat = gen_rtx(UNSPEC_VOLATILE, VOIDmode,
		     gen_rtvec(1, contp ? const1_rtx : const0_rtx),
		     4);

    set_pat = gen_rtx(SET, VOIDmode, cc0_rtx, uv_pat);
    PATTERN(comp_insn) = set_pat;
    /* Unrecognizable after the surgery: */
    INSN_CODE(comp_insn) = -1;
}

/* Return the next insn if it is a label, otherwise create a new */
/* label, insert after jump INSN and return it. If NEED_PADDING_P is on, */
/* insert a noop before INSN (insertin before the jump insn is incorrect */
/* and screws up the logic in machine description which prevents doloops */
/* ending on the same address. */

static rtx
label_after_insn(rtx insn)
{

    /*** Question: should we take next_nonnote_insn ? How about */
    /* skip loop notes ? On the other hand, if we don't, could the */
    /* label get eliminated because it is separated from other */
    /* labels by notes only? */

    rtx label = NEXT_INSN(insn);
    if (GET_CODE(label) != CODE_LABEL) {
	label = gen_label_rtx();
	emit_label_after(label, insn);
    }
    return(label);
}

/* Make LABEL as "undistructable" as possible */

static void
fix_label(rtx label)
{
    LABEL_NUSES(label)+=10;
    LABEL_PRESERVE_P(label) = 1;
}

/*   The following  function, not  fully implemented  yet, will */
/*   emit insns computing  the value of a  loop limit  when the */
/*   latter is not known at   compile time The  value  returned */
/*   should   be an rtx containing  either  the pseudo register */
/*   with   the  result or    a constant  The  current  version */
/*   (3/27/92) always returns a constant                        */


int
can_compute_loop_counter (rtx loop_start, rtx loop_end, doloop_info *p_dl_info)
{
    rtx iteration_var, initial_value, increment, comparison;
    enum rtx_code cc;	/** comparison code */
    rtx comparison_value, result;
    int loop_count = loop_iterations(loop_start, loop_end);
    int incr_log2, swap_p;

    p_dl_info->loop_start = loop_start;
    p_dl_info->loop_count = loop_count;
    if (loop_count > 0) 
	return 1;

    comparison = get_condition_for_loop	(prev_nonnote_insn(loop_end));
    if (comparison == 0) 
	return 0;
    cc  = GET_CODE (comparison);
    iteration_var    = XEXP (comparison, 0);
    comparison_value = XEXP (comparison, 1);
    if (! invariant_p (comparison_value))
	return 0;
    if (GET_CODE (iteration_var) != REG) 
	return 0;
    iteration_info (iteration_var, &initial_value, &increment,
		    loop_start, loop_end);
    if (initial_value == 0 || increment == 0) 
	return 0;
    if (GET_CODE (increment) != CONST_INT)
	return 0;
    p_dl_info->incr = INTVAL(increment);
    if ((incr_log2 = exact_log2(abs(p_dl_info->incr))) < 0) 
	return 0;
    
    /** The front end changes GT to NE for unsigned numbers, so **/
    /** we "undo" this here for clarity **/

    if (GET_CODE(increment) == CONST_INT &&
	INTVAL(increment) == -1 && cc == NE)
	cc = GT;

    if ( ! (cc==LT || cc==LE || cc==LTU || cc==LEU ||
	    cc==GT || cc==GE || cc==GTU || cc==GEU))
	return 0;
    swap_p = (cc==GT || cc==GE || cc==GTU || cc==GEU);
    if (swap_p) {
	p_dl_info->start_value = comparison_value;
	p_dl_info->end_value   = initial_value;
	p_dl_info->incr        = - p_dl_info->incr;
    }
    else {
	p_dl_info->start_value = initial_value;
	p_dl_info->end_value   = comparison_value;
    }
    if (p_dl_info->incr <= 0) 
	return 0;
    p_dl_info->unsigned_p = (cc==LTU || cc==LEU || cc==GTU || cc==GEU);
    p_dl_info->off_by_one = (cc==LT || cc==LTU || cc==GT || cc==GTU);
    return 1;
}

static rtx
emit_counter_init_expr(doloop_info *p_dl_info)
{
    rtx result;
    if (p_dl_info->loop_count > 0)
	result = gen_rtx (CONST_INT, VOIDmode, p_dl_info->loop_count);
    else
    {
	int incr_log = exact_log2(p_dl_info->incr);
	int incr1    = p_dl_info->incr - p_dl_info->off_by_one;
	rtx seq_start, r1, r2, r3;

	if (incr_log < 0) 
	    abort();
	start_sequence();
	result = p_dl_info->end_value;

	if (p_dl_info->start_value != const0_rtx) 
	  {
	    result = expand_binop(SImode, sub_optab, 
				  result, p_dl_info->start_value, 
				  0, p_dl_info->unsigned_p, OPTAB_DIRECT);
	  }
	if (incr1 > 0) 
	  {
	    result = expand_binop(SImode, add_optab, 
				  result, GEN_INT (incr1),
				  0, p_dl_info->unsigned_p, OPTAB_DIRECT);
	  }
	if (incr_log > 0) 
	  {
	    result = expand_binop(SImode, p_dl_info->unsigned_p ? lshr_optab : ashr_optab,
				  result, gen_rtx (CONST_INT, VOIDmode, incr_log),
				  0, p_dl_info->unsigned_p, OPTAB_DIRECT);
	  }
	seq_start = get_insns();
	end_sequence();
	emit_insns_before(seq_start, p_dl_info->loop_start);
    }
    return result;
}


/* test for a posttest loop (ie a do { } while () ). These
   loops will have a branch around them to end label.
   A Return of 1 indicates we are at a posttest loop. */
static int
posttest_loop (start_label, end_label)
     rtx start_label, end_label;
{
  rtx insn, jump_target;

  insn = start_label;
  while (insn 
	 && (GET_CODE(insn)==NOTE || GET_CODE(insn)==CODE_LABEL)) 
    insn = PREV_INSN(insn);   /* find previous non-junk insn */

  if( insn && GET_CODE(insn) == JUMP_INSN ) 
    {
      jump_target = JUMP_LABEL(insn);
      if (jump_target == end_label)
	return 1;
    
      for (insn = PREV_INSN (jump_target); insn; insn = PREV_INSN (insn))
	{
	  if (GET_CODE (insn) == NOTE || GET_CODE (insn) == CODE_LABEL)
	    {
	      if (insn == end_label)
		return 0;   /* branch around, indicates pretest */
	    }
	  else 
	    break;
	}
      for (insn = PREV_INSN (jump_target); insn; insn = PREV_INSN (insn))
	{
	  if (GET_CODE (insn) == NOTE || GET_CODE (insn) == CODE_LABEL)
	    {
	      if (insn == end_label)
		return 0;   /* branch around, indicates pretest */
	    }
	  else 
	    break;
	}
      return 1;
    }
  return 1;
}

/* handle the do {stmts...} until (condition) case.
   When this gets folded to a  do until ce; stmt,
   we need to issue code to protect against the case that
   the counter is negative.  In this case, the loop needs
   to be done once. */
static rtx
handle_posttest_loops (counter_init, start_label, end_label)
     rtx counter_init, start_label, end_label;
{
  rtx reg, zero, label, tem, after_correct_cntr_label, branch;

  /* If optimize > 2 then we can assume that the user will not
     be making a negative loop that monotonicaly increases
     with a negative start */
  if (optimize < 3
      && GET_CODE (counter_init) == REG
      && posttest_loop (start_label, end_label)) 
    {
      /*  We have to worry about the case where the value put into the 
	  counter comes from a register.  In this case, a negative value
	  put in the counter will cause the do_until loop to go on forever. 
	  
	  What we'll do is insert insn's to test that this is negative.  If
	  we're lucky, these insn's will go away if this is not a posttest loop 
	  
	  But first, see if we are in a posttest loop.  */

      label = gen_label_rtx();
      reg = gen_reg_rtx (GET_MODE (counter_init));
      after_correct_cntr_label = emit_label_after (label, PREV_INSN(start_label));
    
      /* Add new compare/branch insn before the loop.  */
      start_sequence ();
      emit_insn (gen_move_insn (reg, counter_init));    /* temp in case need to reset to 1 */
      emit_cmp_insn (reg, const0_rtx, GE, NULL_RTX,
		     GET_MODE (reg), 0, 0);
      branch = emit_jump_insn (gen_bgt (after_correct_cntr_label));
      JUMP_LABEL(branch) = after_correct_cntr_label;  /* cse needs this.  It is
							 normally filled in by jump optimization */
      emit_insn (gen_move_insn (reg, const1_rtx));
      
      tem = gen_sequence ();
      end_sequence ();
      emit_insn_before (tem, PREV_INSN(start_label));
      fix_label (label);      /* keep this from being moved */
      return (reg);
    } 
  else 
    {
      return (counter_init);
    }
}


 /* Emit a doloop instruction before LOOP_START.                            */
 /* COUNTER_LOOP -- rtx for expression for number of cycles. Must be either */
 /*        constant or a REG.                                               */
 /* LOOP_START -- the first label of the loop.                              */
 /* LOOP_END   -- the INSN_NOTE_LOOP_END for the lopo                       */
 /* DEPTH      -- maximum depth of nested loops inside our loop.            */
 /*               Minimal depth is 1.                                       */

static rtx
emit_doloop_start(rtx counter_init, rtx start_label, rtx end_label, int depth, int snglblock)
{
    rtx  depth_expr, pattern, doloop_insn;
    
    depth_expr = gen_rtx(CONST_INT, SImode, depth);

    pattern    = gen_rtx(UNSPEC_VOLATILE, VOIDmode,
			 gen_rtvec(4, 0,0,0,0), 3);

    DOLOOP1_COUNTER(pattern)	 = counter_init;
    DOLOOP1_START_LABEL(pattern) = 
	gen_rtx(LABEL_REF, VOIDmode, start_label),
    DOLOOP1_END_LABEL(pattern)	 = 
	gen_rtx(LABEL_REF, VOIDmode, end_label);
    DOLOOP1_DEPTH(pattern)	 = depth_expr;

#ifdef DOLOOP_BEFORE_LABEL
    doloop_insn = emit_insn_before(pattern, start_label);
#else
    doloop_insn = emit_insn_after(pattern, start_label);
#endif

    if (snglblock)
      SNGL_BB_LOOP_P (doloop_insn) = 1;
    else
      SNGL_BB_LOOP_P (doloop_insn) = 0;

    return doloop_insn;
}

int
doloop_start_p(insn)
    rtx insn;
{
    rtx pat = PATTERN(insn);
    if (GET_CODE(insn) != INSN)
	return 0;
    if (GET_CODE(pat=PATTERN(insn)) != UNSPEC_VOLATILE) 
	return 0;
    if (XINT(pat,1) != 3) 				
	return 0;
    return 1;
}

/*  Initialize array LOOP_DEPTH_ARRAY with  depths of each  of */
/*  the loops.  The array is indexed by loop number.           */

static void
compute_loop_depths(void)
{
    int changed_p = 1;
    int i;
     
    loop_depth_array = (int *) oballoc (max_loop_num * sizeof (int));
    
    for(i=0; i<max_loop_num; i++) loop_depth_array[i] =1;
    while (changed_p) {
	changed_p = 0;
	for(i=0; i<max_loop_num; i++) {
	    int parent = loop_outer_loop[i];
	    int my_depth = loop_depth_array[i];
	    if (parent >= 0 && 
		loop_depth_array[parent] < my_depth + 1) {
		changed_p = 1;
		loop_depth_array[parent]= my_depth + 1;
	    }
	}
    }
    get_strength_reduction_benefit();
}


/* Translate option for "srlevel" to integer ****/

char *strength_reduction_benefit_str = 0;
int   strength_reduction_benefit = 0;

void get_strength_reduction_benefit(void)
{
    int srb;
    if (strength_reduction_benefit_str &&
	sscanf(strength_reduction_benefit_str,"%d", &srb) == 1)
	strength_reduction_benefit = srb;
}



/* Find the depth of the loop specified by LOOP_START and LOOP_END */
/* This function can be called only after GET_LOOP_DEPTH */

int
get_loop_depth(rtx loop_start, rtx loop_end)
{
    int loop_num;
    for(loop_num=0; loop_num<max_loop_num; loop_num++) {
	if (loop_number_loop_starts[loop_num] == loop_start &&
	    loop_number_loop_ends[loop_num] == loop_end)
	    return loop_depth_array[loop_num];
	}
    abort();
}

int
check_doloop_end(insn)
    rtx insn;
{
    return 1;
}

/* mmh
   It should be obvious but let me give some insight....
   This is used to find out if a particular insn references a label.
   
   I recursively walk an insn to the leaves until I get to a Label-ref then
   we just ensure that it doesn't make this jump; if it does we need to add
   a nop.
*/

static int
may_insn_potentially_jump_to (x, labl)
     rtx x, labl;
{
  register enum rtx_code code;
  rtx ll;
  int i,j;
  char *fmt;

  if (x == 0)
    return 0;

  code = GET_CODE (x);

  switch (code)
    {
    case LABEL_REF:
      if (labl == XEXP (x, 1))
	return 1;
      break;

    default:
      /* Other cases: walk the insn.  */
      fmt = GET_RTX_FORMAT (code);
      for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
	{
	  if (fmt[i] == 'e')
	    if (may_insn_potentially_jump_to (XEXP (x, i), labl))
	      return 1;
	  else if (fmt[i] == 'E')
	    for (j = 0; j < XVECLEN (x, i); j++)
	      if (may_insn_potentially_jump_to (XVECEXP (x, i, j), labl))
		return 1;
	}
    }
  return 0;
}

int tight_doloop_end_p(insn)
    rtx insn;
{
    rtx i;
    int result;
    
    for(i=PREV_INSN(insn);
	i && (GET_CODE(i) == CODE_LABEL || GET_CODE(i) == NOTE);
	i = PREV_INSN(i));

    /** It is OK if we hit the end without finding another loop end **/

    if (!i) 
      return 0;
    
    if (GET_CODE (i) == JUMP_INSN)
      return 1;

    /* Handle the jumps that have become delay branches. */
    if (GET_CODE (i) == INSN
	&& GET_CODE (PATTERN (i)) == SEQUENCE
	&& GET_CODE (XVECEXP (PATTERN (i), 0, 0)) == JUMP_INSN)
      return 1;

    result = doloop_end_p(i);

    if (!result)
      {
	rtx x, labl;
	labl = prev_nonnote_insn (insn);
	/* Search L from 'insn' to 'first' searching for a use of labl */
	if (GET_CODE (labl) == CODE_LABEL)
	  return 1;
#if 0
	/* At one time I thought that this might be nesscary..
	   but now I'm convinced that the compiler would never have multiple labels
	   s.t.
	   
	   l1:
	       (loop_end ...)
	   l2:

	   and not use l1:  use see on the 21k there is a problem that you must
	   insert a nooop if there is a jump to the end of the loop.
	   */
	  
	  for (x = prev_nonnote_insn (insn); x ; x = prev_nonnote_insn (x))
	    {
	      if (DOLOOP_P (x)
		  && DOLOOP1_END_LABEL (PATTERN (x)) == labl)
		return 1;
	      
	      else if ((GET_CODE (x) == INSN 
			|| GET_CODE (x) == JUMP_INSN)
		       &&  may_insn_potentially_jump_to (PATTERN (x), labl))
		return 0;
	    }
#endif
      }
    return result;
}


/*** Returns true if there are no jumps crossing the loop boundary **
/** and no calls anywhere.  NEED_PADDING_P is an additional "out"**/
/* argument which is set to True if there is a jump to a label */
/* near "loop_cont" label. In this case we need a "noop" to which */
/* we can jump. */

int
unjumped_loop_p (rtx loop_start, rtx loop_end, rtx *padded_labelp, int *hasbranch)
{
    rtx curi,
    loop_cont = 0; /* loop continue note if there is one */

    *hasbranch = 0;
    *padded_labelp = 0;
    for(curi = PREV_INSN(loop_end);
	curi && curi != loop_start;
	curi=PREV_INSN(curi))

	switch (GET_CODE(curi)) {
	  case JUMP_INSN:
	  {
	      rtx jump_label = JUMP_LABEL(curi);
	      *hasbranch = 1;
	      if (loop_cont && jump_label == next_nonnote_insn(loop_cont))
	      {
		  if (*padded_labelp != 0  &&
		      *padded_labelp != jump_label)
		      abort();
		  *padded_labelp = jump_label;
	      }
	      else if (! insn_in_range(jump_label, loop_start,
				       loop_cont ? loop_cont : loop_end))
		  return 0;
	  }
	      break;

	case CALL_INSN:
	  return 0;

	case NOTE:
	  switch(NOTE_LINE_NUMBER(curi)) {
	    case NOTE_INSN_LOOP_CONT:
	      if (loop_cont == 0)
		  loop_cont = curi;
	      break;
	    case NOTE_INSN_LOOP_END:
	      /** The assumption is that if we reached the bottom **/
	      /** of the inner loop we must have seen the loop_cont **/
	      /** note of the outer loop. If this is not true, we can't **/
	      /** tell whose loop_cont we found. This situatin can arise 
	       /* in "while" loop. */
	      if (! loop_cont)
		  return 0;
	      break;
	    default:
	      continue;
	  }
	default:
	  continue;
      }
    if (curi == 0)
	abort();
    return 1;
}

/** true if INSN is between START and END **/
/** it is a system error if END precedes START **/

static int
insn_in_range(rtx insn, rtx start, rtx end)
{
    rtx curi;
    for(curi = start; ;curi=NEXT_INSN(curi)) {
	if (curi == insn)
	    return 1;
	if (curi == end)
	    return 0;
	if (curi == 0)   
	    abort();
    }
}

/*    For  now we use  the  "used" bit  on comparison  operator  of the jump */
/*    insn to   mark  doloop back   jumps.   This   marking  allows  one  to */
/*    recognize   those jumps in machine  description   and a) inhibit delay */
/*    slot filling and b) prevent their compilation as real jumps.           */
/*                                                                           */
/*    This  bit was  chosen  because it  is the  list used throughout gcc.   */
/*    If this change, we might get in trouble                                */

void mark_doloop_jump (rtx op)
{
    op->used = 1;
}

/* Difference between doloo_end_p and doloop_jump_p is subtle:
   doloop_jump_p can recognize jumps before they have been converted to 
   unspec_volatile, while doloop_end_p. On the other hand, doloop_end_p 
   does not die, like doloo_jump_p does, when called from final */

int
doloop_end_p (insn)
    register rtx insn;
{
    rtx pattern, cond, cond_subexpr, x1;
    if (GET_CODE(insn) != JUMP_INSN) return 0;
    pattern = PATTERN(insn);
    
    if (GET_CODE(pattern) != SET) return 0;
    if (GET_CODE(XEXP(pattern,0)) != PC) return 0;
    x1 = XEXP(pattern,1);
    if (GET_CODE(x1) != IF_THEN_ELSE) return 0;
    cond = XEXP(x1, 0);
    cond_subexpr = XEXP(cond,0);
    if ((GET_CODE(cond_subexpr) == UNSPEC_VOLATILE) &&
	(XINT(cond_subexpr, 1) == 4)) 
	return 1;
    cond_subexpr = XEXP(cond,1);
    if ((GET_CODE(cond_subexpr) == UNSPEC_VOLATILE) &&
	(XINT(cond_subexpr, 1) == 4)) 
	return 1;
    return 0;
}

int
doloop_jump_p (insn)
    register rtx insn;
{
    rtx cc0_setter, cc0_set_pat, cc0_source;

    if (insn==NULL)   /* I'm not 100% sure if this is just a propogation fix or what */
      return 0;

    if (GET_CODE(insn) != JUMP_INSN) return 0;
    cc0_setter = prev_cc0_setter(insn);
    cc0_set_pat = PATTERN(cc0_setter);
    if (GET_CODE(cc0_set_pat) != SET) 
	return 0;
    cc0_source = SET_SRC(cc0_set_pat);
    
    if ((GET_CODE(cc0_source) == UNSPEC_VOLATILE) &&
	(XINT(cc0_source, 1) == 4))
	return 1;
    else return 0;
}

int
doloop_cc0_p (op, mode)
    rtx op;
    enum machine_mode mode;
{
    return ((op == cc0_rtx) ||
	    ((GET_CODE(op) == UNSPEC_VOLATILE) &&
	     (XINT(op,1) == 4)));
}


#if CHECK_DOLOOP
static void
check_doloop(insn,label)
    rtx insn, label;
{
    rtx i;
#ifdef DOLOOP_BEFORE_LABEL
    for(i=insn;             i && i != insn ; i=PREV_INSN(i))
#else
    for(i=NEXT_INSN(label); i && i != label; i=PREV_INSN(i))
#endif
    {
		if (GET_CODE(i) != INSN && GET_CODE(i) != NOTE) 
	    abort();
    }
}
#endif

#if 0
       /* Shift   doloop_start's  back  to their   start  labels. */
       /* We   do  not   care  about  doloop_end's   because they */
       /* do  not  emit  any  real    code;  we   could   do some */
       /* extra checking, though.                                 */

void
shift_doloops(first)
    rtx first;
{
    rtx insn, label, label_ref;
    for (insn = first; insn ; insn = NEXT_INSN(insn)) {
	if (doloop_start_p(insn)) {
	    label_ref = DOLOOP1_START_LABEL(PATTERN(insn));
	    label = XEXP(label_ref, 0);
#if CHECK_DOLOOP
	    check_doloop(insn, label);
#endif	    
	    if (prev_nonnote_insn(insn) != label) {
		rtx pat_copy = copy_rtx(PATTERN(insn));

		debug_rtx(insn);		abort();

		emit_insn_before(pat_copy, label);
		PUT_CODE (insn, NOTE);
		NOTE_LINE_NUMBER (insn) = NOTE_INSN_DELETED;
		NOTE_SOURCE_FILE (insn) = 0;
	    }
	} 
    }	
}
#endif

/** Returns 1 if the doloop INSN has an empty body, the body
/** has no "real" (i.e. non-notes, non-label, etc) insns **/


int
empty_doloop_p(insn)
    rtx insn;
{
    rtx i,
    pattern = PATTERN(insn),


    end_label = XEXP(DOLOOP1_END_LABEL(pattern), 0);

    for(i=NEXT_INSN(insn); i && i != end_label; i = NEXT_INSN(i))
	if ((GET_CODE(i) != NOTE && GET_CODE(i) != CODE_LABEL) &&
	    ! doloop_end_p(i)) return 0;
    if (i != end_label) 
	abort();
    return 1;
}


/*                        To Do                                */
/*                                                             */
/*  Do  some checking  on whether LCNTR    can be used  as the */
/*  index register.  It can when the  loop is counted  down by */
/*  1 to 0.  Even  when not to  0, we might  save by computing */
/*  "i" from LCNTR -- this way "i" does  not have eto  be live */
/*  through the whole loop                                     */


/*** IMPORTANT: loop_iteration will not return the same value once ***/
/*** LOOP_END_LABEL has been inserted -- it will most likely return 0 ***/
/*** for that reason it is called from this function and its value saved ***/
/*** and later passed to emit_counter_init_expr ***************************/ 


