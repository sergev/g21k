/* Loop Pipeliner Pass
   Copyright (C) 1992 Free Software Foundation, Inc.
   Contributed by Marc Hoffman (marc.hoffman@analog.com)

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdio.h>
#include "config.h"
#include "rtl.h"
#include "basic-block.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "flags.h"
#include "insn-config.h"
#include "insn-attr.h"
#include "doloop.h"

#define NPIPELINE_MAX 20

typedef struct acons {
  struct acons *rest;
  int regno;
  rtx insn;
} acons;

typedef struct {
  rtx *insns;
  int n_insns;
  regset live_info;
} stage_info ;

#define LAST_INSN_IN_STAGE(S) ((S)->insns[(S)->n_insns-1])
#define FIRST_INSN_IN_STAGE(S) ((S)->insns[0])

static stage_info **stages;
static int n_stages;
static int cur_stage;

/*
 * Stage managment
 */
static
initialize_stages (n_s)
     int n_s;
{
  n_stages = n_s;
  cur_stage = 0;
  stages = (stage_info **)xmalloc (n_stages * sizeof (stage_info));
}

static
destroy_stage (x)
     stage_info *x;
{
  if (!x)        abort();
  if (!x->insns) abort();
  free (x->insns);
  free (x);
}

static
destroy_stages ()
{
  int i;
  for (i=0;i<n_stages; i++)
    destroy_stage (stages[i]);
  free (stages);
}

static stage_info *
create_stage (start, end)
     rtx start, end;
{
  int count;
  rtx x;
  stage_info *stage = (stage_info *)xmalloc (sizeof (stage_info));

  for (count=0, x = start; x && x != NEXT_INSN (end) ; x = NEXT_INSN (x), count++);

  stage->n_insns = count;
  stage->insns   = (rtx *)xmalloc (count * sizeof (rtx));
  for (count=0, x = start; x && x != NEXT_INSN (end) ; x = NEXT_INSN (x), count++)
    stage->insns[count] = x;
  return stage;
}

static int
make_stage (start, end)
     rtx start, end;
{
  stages[cur_stage++] = create_stage (start, end);
  return cur_stage -1;
}


/*
 * Flow analysis
 */
static void
update_n_sets (x, inc)
     rtx x;
     int inc;
{
  rtx dest = SET_DEST (x);

  while (GET_CODE (dest) == STRICT_LOW_PART || GET_CODE (dest) == SUBREG
	 || GET_CODE (dest) == ZERO_EXTRACT || GET_CODE (dest) == SIGN_EXTRACT)
    dest = SUBREG_REG (dest);
	  
  if (GET_CODE (dest) == REG)
    {
      int regno = REGNO (dest);
      
      if (regno < FIRST_PSEUDO_REGISTER)
	{
	  register int i;
	  int endregno = regno + HARD_REGNO_NREGS (regno, GET_MODE (dest));
	  
	  for (i = regno; i < endregno; i++)
	    reg_n_sets[i] += inc;
	}
      else
	reg_n_sets[regno] += inc;
    }
}

static void
update_n_reg_info (x, inc)
     rtx x;
     int inc;
{
  rtx dest = SET_DEST (x);

  while (GET_CODE (dest) == STRICT_LOW_PART || GET_CODE (dest) == SUBREG
	 || GET_CODE (dest) == ZERO_EXTRACT || GET_CODE (dest) == SIGN_EXTRACT)
    dest = SUBREG_REG (dest);

  if (GET_CODE (dest) == REG)
    {
      int regno = REGNO (dest);
      
      if (regno < FIRST_PSEUDO_REGISTER)
	{
	  register int i;
	  int endregno = regno + HARD_REGNO_NREGS (regno, GET_MODE (dest));
	  
	  for (i = regno; i < endregno; i++)
	    { 
	      reg_n_deaths[i] += inc;
	      reg_n_sets[i] += inc;
	      reg_basic_block[i] = REG_BLOCK_GLOBAL;
	    }
	}
      else
	{
	  reg_n_deaths[regno]    += inc;
	  reg_n_sets[regno]      += inc;
	  reg_basic_block[regno] = REG_BLOCK_GLOBAL;
	}
    }
}

static
update_live_info_for_register_live (live_set, reg, insn)
     regset live_set;
     rtx insn, reg;
{
  register int regno  = REGNO (reg);
  register int offset = regno / REGSET_ELT_BITS;
  register int bit    = 1 << (regno % REGSET_ELT_BITS);
  enum machine_mode mode = GET_MODE (reg);

  if (regno < FIRST_PSEUDO_REGISTER)
    {
      int j = HARD_REGNO_NREGS (regno, mode);
      if (!(live_set[offset] & bit))
	{
	  while (--j >= 0)
	    {
	      offset = (regno + j) / REGSET_ELT_BITS;
	      bit = 1 << ((regno + j) % REGSET_ELT_BITS);

	      live_set[offset] |= bit;
	    }
	  if (! find_regno_note (insn, REG_DEAD, regno)) 
	    {
	      rtx note = rtx_alloc (EXPR_LIST);
	      PUT_REG_NOTE_KIND (note, REG_DEAD);
	      XEXP (note, 0)   = reg;
	      XEXP (note, 1)   = REG_NOTES (insn);
	      REG_NOTES (insn) = note;
	    }
	}
    }
  else
    {
      if (!(live_set[offset] & bit)) 
	{
	  if (! find_regno_note (insn, REG_DEAD, regno)) 
	    {
	      rtx note = rtx_alloc (EXPR_LIST);
	      PUT_REG_NOTE_KIND (note, REG_DEAD);
	      XEXP (note, 0)   = reg;
	      XEXP (note, 1)   = REG_NOTES (insn);
	      REG_NOTES (insn) = note;
	    }
	  live_set[offset] |= bit;
	}
    }
}

static
update_live_info_for_register_live_not_dead (live_set, reg, insn)
     regset live_set;
     rtx insn, reg;
{
  register int regno  = REGNO (reg);
  register int offset = regno / REGSET_ELT_BITS;
  register int bit    = 1 << (regno % REGSET_ELT_BITS);
  enum machine_mode mode = GET_MODE (reg);

  if (regno < FIRST_PSEUDO_REGISTER)
    {
      int j = HARD_REGNO_NREGS (regno, mode);
      if (!(live_set[offset] & bit))
	{
	  while (--j >= 0)
	    {
	      offset = (regno + j) / REGSET_ELT_BITS;
	      bit = 1 << ((regno + j) % REGSET_ELT_BITS);

	      live_set[offset] |= bit;
	    }
	}
    }
  else
    {
      if (!(live_set[offset] & bit)) 
	{
	  live_set[offset] |= bit;
	}
    }
}

static
update_live_info_for_register_kill (live_set, reg, insn)
     regset live_set;
     rtx insn, reg;
{
  register int regno  = REGNO (reg);
  register int offset = regno / REGSET_ELT_BITS;
  register int bit    = 1 << (regno % REGSET_ELT_BITS);
  enum machine_mode mode = GET_MODE (reg);

  update_n_sets (PATTERN (insn), 1);
#if 0
  if (regno < FIRST_PSEUDO_REGISTER)
    {
      int j = HARD_REGNO_NREGS (regno, mode);
      while (--j >= 0)
	{
	  offset = (regno + j) / REGSET_ELT_BITS;
	  bit = 1 << ((regno + j) % REGSET_ELT_BITS);

	  live_set[offset] &= ~bit;
	}
    }
  else
    {
      live_set[offset] &= ~bit;
    }
#endif
}

static
update_live_info_1 (live_set, x, insn, mechanism)
     regset live_set;
     rtx insn,x;
     int (* mechanism)();
{
  register enum rtx_code code;
  rtx ll;
  int i,j;
  char *fmt;

  if (x == 0)
    return;

  code = GET_CODE (x);

  switch (code)
    {
    case REG: 
      /* Check to see if any of the insns that are in this bb are sets of regno.
	 if so then we don't want to claim that it is live at the start of the bb. */
      mechanism (live_set, x, insn);

      break;

    case MEM:
      update_live_info_1 (live_set, XEXP (x, 0), insn, update_live_info_for_register_live);
      return;

    default:
      /* Other cases: walk the insn.  */
      fmt = GET_RTX_FORMAT (code);
      for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
	{
	  if (fmt[i] == 'e')
	    update_live_info_1 (live_set, XEXP (x, i), insn, mechanism);
	  else if (fmt[i] == 'E')
	    for (j = 0; j < XVECLEN (x, i); j++)
	      update_live_info_1 (live_set, XVECEXP (x, i, j), insn, mechanism);
	}
    }
}


static
update_live_dead_info (live_set, insn)
     regset live_set;
     rtx insn;
{
  rtx x = single_set (insn);
  if (!x) 
    return;

  update_n_reg_info (PATTERN (insn), 1);

  update_live_info_1 (live_set, SET_SRC (x), insn, update_live_info_for_register_live);
  update_live_info_1 (live_set, SET_DEST (x), insn, update_live_info_for_register_kill);
}

/*
 * Here we assume that insns are allocated in the reverse order..
 * If they are not this will not work!.
 * It is also nice to know that we don't need to wory about pred blocks.
 * 
 */
static rtx
emit_insn_for_pipeline (live_set, pat, after, orig_regnotes)
     regset live_set;
     rtx pat, after, orig_regnotes;
{
  rtx insn;
  rtx note;
  insn = emit_insn_after (pat, after);
  
  for (note = orig_regnotes ; note ; note = XEXP (note , 1))
    if (REG_NOTE_KIND (note) != REG_DEAD)
      {
	rtx new = copy_rtx (note);
	XEXP (new, 1)   = REG_NOTES (insn);
	REG_NOTES (insn) = new;
      }

  /* All of the flow analysis should be correct now. */
  update_live_dead_info (live_set, insn);

  return insn;
}

/* 
 * This is probably very inefficent but it is okay because the 
 * number of insn we are going to look at is probably 20 or less.
 * Well for now.
 */
static 
remove_some_log_links (insn, start, end)
     rtx insn, start, end;
{
  rtx prev = 0, links;

  if (GET_CODE (insn) != INSN)
    return;

  if (start == end)
    {
      LOG_LINKS (insn) = 0;
      return;
    }
  
  links = LOG_LINKS (insn);
  while (links)
    {
      rtx walk;
      rtx find = XEXP (links, 0);
      int found = 0;

      for (walk = start; NEXT_INSN (walk) != end ; walk = NEXT_INSN (walk))
	{
	  if (find == walk) 
	    found = 1;
	}

      if (!found)
	{
	  if (prev == 0)
	    LOG_LINKS (insn) = links = XEXP (links,1);
	  else
	    XEXP (prev,1) = XEXP (links,1);
	}
      else
	{
	  prev = links;
	  links = XEXP (links, 1);
	}
    }
}

static
add_log_links (x, insn, start)
     rtx x,insn,start;
{
  register enum rtx_code code;
  int i,j;
  char *fmt;
  rtx insnp;

  if (x == 0)
    return;

  code = GET_CODE (x);

  switch (code)
    {
    case REG: 
      /* Check to see if any of the insns that are in this bb are sets of regno.
	 if so then we don't want to claim that it is live at the start of the bb. */
      for (insnp = PREV_INSN (insn); insnp != start ; insnp = PREV_INSN (insnp))
	{
	  if (GET_CODE (insnp) == INSN)
	    {
	      rtx set = single_set (insnp);
	      if (set && 
		  rtx_equal_p (SET_DEST (set), x)) 
		{
		  LOG_LINKS (insn)
		    = gen_rtx (INSN_LIST, VOIDmode, insnp, LOG_LINKS (insn));
		}
	    }
	}
      
      break;

    default:
      /* Other cases: walk the insn.  */
      fmt = GET_RTX_FORMAT (code);
      for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
	{
	  if (fmt[i] == 'e')
	    add_log_links (XEXP (x, i), insn, start);
	  else if (fmt[i] == 'E')
	    for (j = 0; j < XVECLEN (x, i); j++)
	      add_log_links (XVECEXP (x, i, j), insn, start);
	}
    }
}

static
add_log_links_to_range (b,first, last)
     rtx first,last;
     int b;
{
  rtx insn,next;

  for (insn = first; insn && PREV_INSN (insn) != last ; insn = next) 
    {
      next = PREV_INSN (insn);
      
      if (GET_CODE (insn) == INSN) 
	{
	  rtx set = single_set (insn);
	  if (set) 
	    {
	      rtx x = SET_SRC (set);
	      add_log_links (x, insn, basic_block_head[b]);
	    }
	}
    }
}

static
delete_loop_for_pipeliner (b)
     int b;
{
}

static rtx
move_insn (insn , prev)
     rtx insn, prev;
{
  rtx next;
  
  /* Unlink the insn from it current chain.. */
  NEXT_INSN (PREV_INSN (insn)) = NEXT_INSN (insn);
  PREV_INSN (NEXT_INSN (insn)) = PREV_INSN (insn);

  next  = NEXT_INSN(prev);
  /* Relink it onto its new chain. */
  NEXT_INSN (insn) = next;
  PREV_INSN (insn) = prev;
  NEXT_INSN (prev) = insn;
  PREV_INSN (next) = insn;
  return insn;
}

/*
 * Here we start the pipeline up:
 *
 *        S0 
 *     S1 S0
 * Sn-1...S0
 *
 * What needs to be done.
 *  1. we need to maintain flow info.
 *  2. we need to maintain live info.  
 *     (Build the bit vector and maintain DEAD's correctly)
 *  3. update the basic block info end.
 *
 */
static
fill_pipeline (file, b)
     FILE *file;
     int b;
{
  rtx endbm1     = basic_block_end [b-1];
  int stage_index, triangle;
  rtx emit_p = endbm1;  /* This will point to the end of the block */
  rtx first = 0, last,insn;
  regset bb_live_regs = (regset) alloca (regset_bytes);
  bcopy (basic_block_live_at_start[b], bb_live_regs, regset_bytes);
  

  if (file)  fprintf (file, ";;\tgenerating prologue for %d: ",b);
  /*
   * We need to reverse the order of how the insns are emited
   * The reason that this needs to be done is so we can maintain the
   * appropriate live info.
   * 1. we need to have acurate REG_DEADS and a live vector which represents 
   *    whats live.
   */


  for (stage_index = 0; stage_index < n_stages-1 ; stage_index++) 
    {
      int i, triangle;
      for (triangle = stage_index; triangle >= 0; triangle--) 
	{
	  stage_info *stage = stages[triangle];

	  if (file)
	    fprintf (file, " %d", triangle);

	  for (i=stage->n_insns-1; i >= 0 ; i--) 
	    {
	      rtx m = stage->insns[i];
	      if (GET_RTX_CLASS (GET_CODE (m)) == 'i') 
		{
		  rtx x;
		  rtx ip = copy_rtx (PATTERN (m));
		  x = emit_insn_for_pipeline (bb_live_regs, ip, emit_p, REG_NOTES (m));
		  if (!first ) first = x;
		  last = x;
		}
	    }
	}
    }

  basic_block_end[b-1] = first;
  add_log_links_to_range (b-1, first, last);
  
  bcopy (basic_block_live_at_start[b], bb_live_regs, regset_bytes);
  for (insn = last; insn != NEXT_INSN (first) ; insn = NEXT_INSN (insn))
    {
      rtx x = single_set (insn);
      if (!x) 
	continue;
      
      update_live_info_1 (bb_live_regs, SET_SRC (x), insn, update_live_info_for_register_kill);
      update_live_info_1 (bb_live_regs, SET_DEST (x), insn, update_live_info_for_register_live_not_dead);
    }
  bcopy (bb_live_regs, basic_block_live_at_start[b], regset_bytes);
  
  if (file) fprintf (file, "\n");
  if (file) dump_regset (file, "fill reg-set: ", bb_live_regs);
}





/* 
 * Here we drain the pipeline.
 * given n-stages we need to write
 *
 *  Sn-1 ... ... S2
 *      Sn-2 ... S2
 *       ...
 *               S2
 *
 * What needs to be done.
 *  1. we need to maintain flow info.
 *  2. we need to maintain live info.  
 *     (Build the bit vector and maintain DEAD's correctly)
 */
static
drain_pipeline (file, b)
     FILE *file;
     int b;
{
  rtx topbp1     = basic_block_head [b+1];
  int stage_index;
  rtx emit_p = topbp1;  /* This will point to the end of the block */
  rtx first = 0, last = 0;

  regset bb_dead_regs = (regset) alloca (regset_bytes);
  regset bb_live_regs = (regset) alloca (regset_bytes);

  if (file)  fprintf (file, ";;\tgenerating epilogue for %d: ", b);

  if (file) dump_regsets_for_block (file, b-1);
  if (file) dump_regsets_for_block (file, b);
  if (file) dump_regsets_for_block (file, b+1);

  bcopy (basic_block_live_at_start[b+1], bb_live_regs, regset_bytes);
  bzero (bb_dead_regs, regset_bytes);


  
  for (stage_index = n_stages-1; stage_index > 0; stage_index--) 
    {
      int triangle;
      for (triangle = stage_index ; triangle < n_stages; triangle++) 
	{
	  int i;
	  stage_info *stage = stages[triangle];

	  if (file)
	    fprintf (file, " %d", triangle);

	  for (i=stage->n_insns-1; i >= 0 ; i--) 
	    {
	      rtx m = stage->insns[i];
	      if (GET_RTX_CLASS (GET_CODE (m)) == 'i') 
		{
		  rtx x;
		  rtx ip = copy_rtx (PATTERN (m));
		  x = emit_insn_for_pipeline (bb_live_regs, ip, emit_p, REG_NOTES (m));
		  if (!first ) first = x;
		  last = x;
		}
	    }
	}
    }

  add_log_links_to_range (b+1, first, last);

  bcopy (bb_live_regs, basic_block_live_at_start[b+1], regset_bytes);

  if (file) fprintf (file, "\n");
  if (file) dump_regset (file, "drain reg-set:", bb_live_regs);
}


/*
 * This should be more accurate this depends on how many stages there are.
 */
static int
maybe_decimate_loop_counter_for_pipeline (loop, b)
     rtx loop;
     int b;
{
  rtx counter = DOLOOP1_COUNTER (PATTERN (loop));
  rtx top = prev_nonnote_insn (prev_nonnote_insn (loop));
  if (GET_CODE (counter) != CONST_INT)
    {
      rtx p,x = counter;
      x = find_last_value (x, &top, loop);
      if (x)
	{
	  if (GET_CODE (x) == CONST_INT)
	    {
	      counter = x;
	    }
	  if (GET_CODE (x) == REG && RTX_UNCHANGING_P (x))
	    {
	      top = prev_nonnote_insn (top);
	      x = find_last_value (x, &top, next_nonnote_insn(top));
	      if (x != 0 && GET_CODE (x) == CONST_INT)
		{
		  counter = x;
		}
	    }
	}
    }

  if (GET_CODE (counter) == CONST_INT)
    {
      int iterations = INTVAL (counter);

      if (iterations < n_stages)
	return -1;

      else
	DOLOOP1_COUNTER (PATTERN (loop)) = gen_rtx (CONST_INT, SImode, iterations -= n_stages - 1);

      return iterations;
    }
  else if (n_stages - 1 != 1) 
    return -1;
  else
    { /* I'm assuming that its okay to decimate by 1 always.. */
      rtx new_counter = emit_insn_before (gen_rtx (SET, GET_MODE (counter), counter,
						   gen_rtx (MINUS, GET_MODE (counter),
							    counter,
							    const1_rtx)),
					  basic_block_end[b-1]);
      basic_block_end[b-1] = new_counter;
      return 0;
    }

  return 1;
}



/*
 * We need to rewrite the body of the loop in the reverse order of
 * stages. i.e.
 *   SN ... S2 S1 S0
 *
 * Don't for get to update the LOG_LINKS for each insn.
 *
 * Note that the information in basic_block_live_info should 
 * be okay for the steady state.
 * Also note that place where we need to worry about this info is
 * in drain/fill.  Because these change the info drasticaly.
 */
steady_pipeline (b)
     int b;
{
  int s;

  rtx top, head = NEXT_INSN (basic_block_head [b]);
  top = head;

  for (s = n_stages-1; s >= 0; s--)
    {
      int i;
      stage_info *stage = stages[s];

      for (i=0; i < stage->n_insns; i++) 
	{
	  rtx insn = stage->insns[i];
	  if (GET_RTX_CLASS (GET_CODE (insn)) == 'i') 
	    {
	      if (LOG_LINKS (insn))
		remove_some_log_links (insn, top, head);
	    }
	  head = move_insn (insn, head);
	}
    }
}


/* Here we need to traverse the loop body from the tail to the head
 * and construct stages that elliminate the dependencies of the loop.
 *
 * (insn 96 94 98 (set (reg:SF 168)
 *       (mem/s:SF (post_inc:DM (reg:DM 234)))) 16 {notdi2+1} (nil)
 *   (expr_list:REG_INC (reg:DM 234)
 *       (nil)))
 *
 * (insn 98 96 99 (set (reg:SF 169)
 *       (mem/s:SF (post_inc:PM (reg:PM 233)))) 16 {notdi2+1} (nil)
 *   (expr_list:REG_INC (reg:PM 233)
 *       (nil)))
 *
 * (insn 99 98 100 (set (reg:SF 170)
 *      (mult:SF (reg:SF 168)
 *           (reg:SF 169))) 84 {mulsf3} (insn_list 96 (insn_list 98 (nil)))
 *   (expr_list:REG_DEAD (reg:SF 168)
 *       (expr_list:REG_DEAD (reg:SF 169)
 *           (nil))))
 *
 * (insn 100 99 101 (set (reg/v:SF 156)
 *       (plus:SF (reg/v:SF 156)
 *           (reg:SF 170))) 72 {addsf3} (insn_list 99 (nil))
 *   (expr_list:REG_DEAD (reg:SF 170)
 *       (nil)))
 *
 * looking at this loop we would deduce that
 * one possible schedule could be:
 *
 *  S2   100
 *  S1   99
 *  S0   98, 96
 *
 * This algorithim doesn't take into account any resource analysis
 * I'm hoping that the schedular is good enough to do what we need.
 *
 *
 * The outcome of this would be to produce a loop that had the form:
 * 
 * S0
 * S0 S1
 * LOOP
 * S2 S1 S0
 * ENDLOOP
 * S2 S1
 * S2
 *
 *
 */
analyze_loop (file, b, ceiling)
     FILE *file;
     int b, ceiling;
{
  rtx topb       = basic_block_head [b];
  rtx endb       = basic_block_end  [b];
  rtx loop       = next_nonnote_insn (topb);

  rtx tail = prev_nonnote_insn (endb);
  rtx head = next_nonnote_insn (loop);

  rtx insn, next;

  rtx *stage_markers = alloca (ceiling);
  int s,num_stages = 0;
  int iterations;

  if (tail == loop)
    return ;

  for (insn = tail ; insn != head ; insn = next) 
    {
      rtx link;
      int ndep = 0;
      
      next = PREV_INSN (insn);
      
      if (GET_CODE (insn) != INSN) 
	{
	  if (insn == head)
	    break;
	  continue;
	}

      /* Analyze insns dependencies.. */
      for (link = LOG_LINKS (insn) ; link ; link = XEXP (link,1))
	ndep++;

      if (ndep) 
	{
	  stage_markers[num_stages++] = insn;
	}
    }
  
  if (num_stages > 1) 
    {
      /* Make the stages for pipelining */
      initialize_stages (num_stages+1);
      make_stage (head, PREV_INSN (stage_markers[num_stages-1]));
      
      for (s = num_stages-1; s > 0; s--)
	make_stage (stage_markers [s], PREV_INSN (stage_markers [s-1]));
      
      make_stage ( stage_markers[0], tail);

      if (file) 
	debug_stage_info (file, b);

      if ((iterations = maybe_decimate_loop_counter_for_pipeline (loop, b)) >= 0)
	{
	  drain_pipeline (file, b);

	  if( iterations ) 
	    steady_pipeline (b);
	  else 
	    {
	      ; /* delete loop and make flow info reflect removal */
	      delete_loop_for_pipeliner (b);
	    }
	  fill_pipeline (file, b);
	}
      else {
#if TRY_SIMPLE_2STAGE
	if (file)
	  {
	    fprintf (file, ";; simple pipelining block %d\n", b);
	  }
	destroy_stages ();
	initialize_stages (2);

	make_stage (head, stage_markers[num_stages-1]);

	make_stage (NEXT_INSN (stage_markers[num_stages-1]), tail);

	if (maybe_decimate_loop_counter_for_pipeline (loop, b) == 0)
	  {
	    drain_pipeline (file, b);
	    
	    steady_pipeline (b);
	    
	    fill_pipeline (file, b);
	  }

#endif
      }
      destroy_stages ();    
    }
  if (file) fprintf (file, "\n\n");
}


dump_regset (file, format, reg_set)
     FILE *file;
     char *format;
     regset reg_set;
{
  int regno;
  if (!file) file = stdout;
  fprintf (file, "\n;;  %s", format);
  for (regno = 0; regno < max_regno; regno++)
    {
      register int offset = regno / REGSET_ELT_BITS;
      register int bit = 1 << (regno % REGSET_ELT_BITS);
      if (reg_set[offset] & bit)
	fprintf (file, " %d", regno);
    }
  fprintf (file, "\n");
  fflush (file);
}

dump_regsets_for_block (file, b)
     FILE *file;
     int b;
{
  char format[100];
  sprintf (format, "\n;;  Registers live at start of %d:", b);
  dump_regset (file, format, basic_block_live_at_start[b]);
}



debug_stage_info (file, b)
     FILE *file;
     int b;
{
  int s,i;
  if (file == 0) file = stdout;  /* This is so we can call this in the debugger... */

  fprintf (file, ";; Pipelined %d has %d stages and scheduled as follows.\n", b, n_stages);
  for (s = 0 ; s < n_stages ; s++) 
    {
      stage_info *stage = stages[s];

      fprintf (file , ";; Stage %d has %d nonnote-insns : ", s, stage->n_insns);
      for (i=0; i < stage->n_insns; i++) 
	{
	  rtx m = stage->insns[i];
	  if (GET_RTX_CLASS (GET_CODE (m)) == 'i')
	    fprintf (file , "%d ", INSN_UID (m));
	}
      fprintf (file, "\n");
    }

  fflush(file);
}

pipeline_insns (file)
     FILE *file;
{
  int b;

  for (b = 0; b < n_basic_blocks; b++) 
    {
      rtx loopp  = NEXT_INSN (basic_block_head[b]);

      if (inner_most_doloop_p (loopp)) 
	{
	  int block_has_n_insns=0;
	  rtx mem_tail = 0;
	  rtx insn, next;
	  rtx insns;
	  
	  for (insn = basic_block_head[b]; ; insn = next) 
	    {
	      rtx prev;
	      rtx set;
	      next = NEXT_INSN (insn);
	      
	      if (GET_CODE (insn) != INSN) 
		{
		  if (insn == basic_block_end[b])
		    break;
		  continue;
		}
	
	      set = single_set (insn);
	      
	      if (set
		  && GET_CODE (XEXP (set,1)) == MEM)
		mem_tail = insn;
	
	
	      block_has_n_insns++;
	      
	      if (insn == basic_block_end[b])
		break;
	    }
	  analyze_loop (file, b, block_has_n_insns);
	}
    }
  
  if(file)
    dump_flow_info(file);
}

/*
 * Local variables:
 * version-control: t
 * End:
 */
