#include "config.h"
#include "machmode.h"
#include "rtl.h"
#include "stat-md.h"

#ifdef __WATCOMC__
#include "watproto.h"
#include <stdlib.h>
#endif

/** "trace_match_before" and "trace_match_after" are called before and**/
/** after call to    predicate of  "define_insn".   This  means   that**/
/** "trace_match_before" is called only if the rtl template is matched**/
/** and  "trace_match_after" is called  only if  both rtl template and**/
/** the predicate  is matched.  In  gdb,  it might be  helpful  to set**/
/** breakpoints at return's of those  two functions -- at those points**/
/** the rule_num can be displayed as a string.**/



extern char *insn_name[];

static current_rule_num;

int
trace_match_before(insn, rule_num)
    rtx insn;
    int rule_num;
{
    char *rule_name = insn_name[rule_num];
    current_rule_num = rule_num;
#if KEEP_STAT
    if (KEEP_STAT) mark_as_used(rule_num,0);
#endif
    return(1);
}

int
trace_match_after(insn, rule_num)
    rtx insn;
    int rule_num;
{
    char *rule_name = insn_name[rule_num];
#if KEEP_STAT
    if (KEEP_STAT) mark_as_used(rule_num,1);
#endif
    return(1);
}

int check_constraints()
{
    return constrain_operands(current_rule_num, 1);
}
