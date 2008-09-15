#include "config.h"
#include <stdio.h>
#include "tm.h"

#ifdef DSP21XX

#include "rtl.h"

static nregs = 0;
static first_reg = 0;
static last_reg = 0;
int kill_all       = 1; /* Kill all regs, as opposed to killing just one */
int remember_kills = 0; /* remember the kills for the duration of the basic */
			/* block the registers which has been killed */
int kill_threshold = 100;	/* Min. num. of pseudos to trigger kill */
int big_kill_threshold = 350;	/* Min. num. of pseudos to trigger 
				   "kill rememberance" */

int kill_period = 1;

static rnum=0;

/*==================================================================*/

static int map_size;
static rtx *reg_map;

#define REGMAP(R) reg_map[REGNO(R)-FIRST_PSEUDO_REGISTER]

static erase_reg_map()
{
    int rn;
    for(rn=0; rn<map_size; rn++)
	reg_map[rn]=0;
}

/*==================================================================*/

static rtx* find_used_pseudo_reg(rtx x)
{
    int i;
    rtx *x2;
    enum rtx_code code = GET_CODE(x);
    char *fmt = GET_RTX_FORMAT(code);
    for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
    {
	if (fmt[i] == 'e') {
	    rtx subx = XEXP(x,i);
	    if (GET_CODE(subx) == REG) {
		if (REGNO(subx) >= FIRST_PSEUDO_REGISTER
#if 0
		    && GET_MODE(subx) == SImode
#endif
		    )
		    return &XEXP(x,i);
	    }
	    else  {
		if (GET_CODE (subx) != CLOBBER) {
		    x2 = find_used_pseudo_reg(subx);
		    if (x2)
			return x2;
		}
	    }
	}
    }
    return 0;
}

static void stalk_pseudo_regs (rtx insn, rtx x, void (*killer) (rtx, rtx*))
{
    int i;
    rtx *x2;
    enum rtx_code code = GET_CODE(x);
    char *fmt = GET_RTX_FORMAT(code);
    
    /* for MOFIFY's stalk only modifier registers */

    switch (code) {
      case POST_MODIFY:
      case PRE_MODIFY:
	stalk_pseudo_regs (insn, XEXP(x,1), killer);
	break;
      case PRE_INC:
      case POST_INC:
      case PRE_DEC:
      case POST_DEC:
	return;
	break;
      default:
	for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
	{
	    if (fmt[i] == 'e') {
		rtx subx = XEXP(x,i);
		if (GET_CODE(subx) == REG) {
		    if (REGNO(subx) >= FIRST_PSEUDO_REGISTER)
			killer(insn, &XEXP(x,i)); 
		}
		else  {
		    if (GET_CODE (subx) != CLOBBER)
			stalk_pseudo_regs(insn, subx, killer);
		}
	    }
	}
    }
}

/*
  There is a danger that we might miss a register whose value is changed.
  kill_one_src_reg is supposed to kill only read-only registers !!
*/

static void substitute_reg (rtx insn, rtx *preg)
{
    *preg = REGMAP(*preg) ? REGMAP(*preg) : *preg;
}

static void kill_one_src_reg (rtx insn, rtx *preg)
{
    rtx p, ppat, new_reg, src_reg;
    int rn;

    /* We don't win much if the previous insn just defined the register 
      and the source is not a "real" operation */

    if (rnum++ % kill_period)
	return;

    p = (rtx) PREV_INSN(insn);
    ppat = PATTERN(insn);
    if (ppat && 
	GET_CODE(ppat) == SET &&
	SET_DEST(ppat) == *preg &&
	((GET_CODE (SET_SRC(ppat)) == CONST_INT) ||
	 (GET_CODE (SET_SRC(ppat)) == REG) ||
	 (GET_CODE (SET_SRC(ppat)) == SUBREG) ||
	 (GET_CODE (SET_SRC(ppat)) == MEM) ||
	 (GET_CODE (SET_SRC(ppat)) == CONST) 
	 ))
	return;

    src_reg = *preg;
    new_reg = gen_reg_rtx(GET_MODE(*preg));
    nregs++;
    rn = REGNO(new_reg);
    if (first_reg <= 0)
	first_reg = rn;
    last_reg = rn;
    emit_insn_before(gen_move_insn(new_reg, src_reg), insn);
    if (remember_kills)
	REGMAP(*preg) = new_reg;
    *preg = new_reg;
}

void kill_one_dest_reg(rtx insn, rtx *preg)
{
    rtx p, ppat, new_reg, dest_reg;
    int rn;

    /* We don't win much if the previous insn just defined the register 
      and the source is not a "real" operation */

    if (rnum++ % kill_period)
	return;

    dest_reg = *preg;
    if (dest_reg == cc0_rtx)
	return;
    new_reg = gen_reg_rtx(GET_MODE(*preg));
    nregs++;
    rn = REGNO(new_reg);
    if (first_reg <= 0)
	first_reg = rn;
    last_reg = rn;
    emit_insn_after(gen_move_insn(dest_reg, new_reg), insn);
    *preg = new_reg;
}

void forget_reg_map(rtx x, rtx ignore)
{
    if (GET_CODE(x) == SUBREG)
	SWITCH_SUBREG(x);
    if (GET_CODE(x) == REG &&
	REGNO(x) >= FIRST_PSEUDO_REGISTER)
	REGMAP(x) = 0;
}

     
void
note_changes (x, fun)
     register rtx x;
     void (*fun) ();
{
    enum rtx_code code = GET_CODE (x);
    if ((code == SET || code == CLOBBER))
    {
	register rtx dest = SET_DEST (x);
	while ((GET_CODE (dest) == SUBREG
		&& (GET_CODE (SUBREG_REG (dest)) != REG
		    || REGNO (SUBREG_REG (dest)) >= FIRST_PSEUDO_REGISTER))
	       || GET_CODE (dest) == ZERO_EXTRACT
	       || GET_CODE (dest) == SIGN_EXTRACT
	       || GET_CODE (dest) == STRICT_LOW_PART)
	    dest = XEXP (dest, 0);
	(*fun) (dest, x);
    }
    else if (code == PRE_MODIFY || code == POST_MODIFY ||
	     code == PRE_INC    || code == POST_INC ||
	     code == PRE_DEC    || code == POST_DEC) {
	rtx dest = XEXP(x,0);
	while ((GET_CODE (dest) == SUBREG
		&& (GET_CODE (SUBREG_REG (dest)) != REG
		    || REGNO (SUBREG_REG (dest)) >= FIRST_PSEUDO_REGISTER)))
	    dest = XEXP (dest, 0);
	(*fun)(dest);
    }
    else if (code == PARALLEL)
    {
	register int i;
	for (i = XVECLEN (x, 0) - 1; i >= 0; i--)
	{
	    register rtx y = XVECEXP (x, 0, i);
	    if (GET_CODE (y) == SET || GET_CODE (y) == CLOBBER)
	    {
		register rtx dest = SET_DEST (y);
		while ((GET_CODE (dest) == SUBREG
			&& (GET_CODE (SUBREG_REG (dest)) != REG
			    || (REGNO (SUBREG_REG (dest))
				>= FIRST_PSEUDO_REGISTER)))
		       || GET_CODE (dest) == ZERO_EXTRACT
		       || GET_CODE (dest) == SIGN_EXTRACT
		       || GET_CODE (dest) == STRICT_LOW_PART)
		    dest = XEXP (dest, 0);
		(*fun) (dest, y);
	    }
	}
    }
    else {
	register char *fmt = GET_RTX_FORMAT (code);
	register int i;
	
	for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
	{
	    if (fmt[i] == 'e')
		note_changes(XEXP(x,i), fun);
	    if (fmt[i] == 'E')
	    {
		register int j;
		for (j = 0; j < XVECLEN (x, i); j++)
		    note_changes(XVECEXP (x, i, j), fun);
	    }
	}
    }	
}

void kill_all_regs_insn (rtx insn)
{
    rtx pat, src;
    if (GET_CODE(insn) != INSN)
	return;
    pat = PATTERN(insn);
    if (! pat)
	return;
    note_changes(pat, forget_reg_map);
    if (GET_CODE(pat) != SET)
	return;
    src  = SET_SRC(pat);

    if (GET_CODE(src) != REG && GET_CODE(src) != SUBREG)
	stalk_pseudo_regs(insn, src, kill_one_src_reg);
}

void kill_regs_insn(rtx insn) 
{
    rtx new_reg,  *preg, src, pat, p, ppat;

    if (GET_CODE(insn) != INSN)
	return;
    pat = PATTERN(insn);
    if (! pat)
	return;
    
    if (GET_CODE(pat) == PARALLEL)
	pat = XVECEXP(pat, 0, 0);
    if (GET_CODE(pat) != SET)
	return;
    src = SET_SRC(pat);
#if 1
    preg = find_used_pseudo_reg(src);
    if (preg)
	kill_one_src_reg(insn, preg);
#else
    if (src && GET_CODE(src) != REG)
	kill_one_dest_reg(insn, &(SET_DEST(pat)));
#endif
    return;
}

extern int reg_rtx_no;

void kill_regs(rtx f, FILE *dump)
{
    rtx insn;
    int regs_so_far = max_reg_num() - FIRST_PSEUDO_REGISTER;

    nregs = 0;
    map_size = regs_so_far * 3;
    reg_map = (rtx*) alloca(map_size*sizeof(rtx*));
    erase_reg_map();
    first_reg = -1;
    
    /* Empirical evidence shows that remembering kills is useful for large
       number of pseudo-regs.
      */
    if (regs_so_far < kill_threshold)
	return;
    remember_kills = regs_so_far > big_kill_threshold;
    for(insn=f; insn; insn=next_insn(insn)) {
	if (GET_CODE(insn) == CODE_LABEL ||
	    GET_CODE(insn) == JUMP_INSN)
	    erase_reg_map();
	if (GET_CODE(insn) == INSN ||
	    GET_CODE(insn) == JUMP_INSN ||
	    GET_CODE(insn) == CALL_INSN)
	{
	    if (kill_all)
		kill_all_regs_insn(insn);
	    else 
		kill_regs_insn(insn);
	}
    }
    if (dump) {
	fprintf(dump, "=== Added %d new registers (%d -- %d) @@@@@@@\n", 
		nregs,
		first_reg - FIRST_PSEUDO_REGISTER,
		last_reg  - FIRST_PSEUDO_REGISTER);
    }
}

#endif	/* ifdef DSP21XX */
