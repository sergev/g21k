#include "obstack.h"
#include "rtl.h"
#include "iter.h"

tree get_free_iterators(tree);
void expand_stmt_with_iterators_1 (tree, tree);
tree collect_iterators();
tree merge_iterator_lists();
static tree build_limit_decl();
static void delete_ixpansion();

extern struct obstack permanent_obstack;


/*
  		ITERATOR DECLS

Iterators are  implemented  as integer decls  with a special  flag set
(rms's   idea).  This  makes  eliminates  the need   for  special type
checking.  The  flag  is accesed using   the  ITERATOR_P  macro.  Each
iterator's limit is saved as a  decl with a special  name. The decl is
initialized with the limit value -- this way we  get all the necessary
semantical processing for free by calling finish  decl. We might still
eliminate  that decl  later  -- it takes up  time and  space and, more
importantly, produces strange error  messages when  something is wrong
with the initializing expresison.  */

tree
build_iterator_decl(id, limit)
    tree id, limit;
{
    tree type = integer_type_node, lim_decl;
    tree t1, t2, t3;
    tree start_node, limit_node, step_node;
    tree decl;
    
    if (limit) {
	limit_node = save_expr(limit);
	SAVE_EXPR_CONTEXT(limit_node) = current_function_decl;
    }
    else
	abort();
    lim_decl = build_limit_decl(id, limit_node);
    push_obstacks_nochange ();
    decl = build_decl(VAR_DECL, id, type);
    ITERATOR_P(decl) = 1;
    ITERATOR_LIMIT(decl) = lim_decl;
    finish_decl(pushdecl(decl), 0, 0);
    return decl;
}

#define LIMIT_PREFIX "____ilimit__"

/* Build a decl for holding the limit expression of the iterator */
/* whose id is ITER_ID */

tree
build_limit_decl(iter_id, limit_expr)
    tree iter_id, limit_expr;
{
    tree idnode, lim_decl, tem;
    char *iter_name = IDENTIFIER_POINTER(iter_id);
    int iname_len = strlen(iter_name);
    char *limit_name = 
	(char *) obstack_alloc(&permanent_obstack,
			       iname_len + strlen(LIMIT_PREFIX) + 1);

    push_obstacks_nochange ();
    strcpy(limit_name, LIMIT_PREFIX);
    strcat(limit_name, iter_name);
    idnode = get_identifier(limit_name);
    lim_decl = build_decl(VAR_DECL, idnode, integer_type_node);
    TREE_READONLY(lim_decl) = 1;
    DECL_INITIAL(lim_decl) = limit_expr;
    TREE_STATIC(lim_decl) = 
	current_binding_level == global_binding_level;
    tem = pushdecl(lim_decl);
    expand_decl(tem);
    finish_decl(tem, limit_expr, 0);
    return tem;
}

/*
  		ITERATOR RTL EXPANSIONS

Expanding simple statements with iterators is  pretty straightforward:
collect (collect_iterators) the list  of  all "free" iterators  in the
statement and for each  of them add  a  special prologue before and an
epilogue after the expansion for  the statement. Iterator is "free" if
it has not been "bound" by a FOR operator. The rtx associated with the
iterator's  decl is used as  the loop counter.  Special processing  is
used  for "{(...)}" constructs:  each iterator expansion is registered
(by "add_ixpansion" function)  and inner expansions are superseded  by
outer ones. The cleanup of superseded expansions is done by  a call to
delete_ixpansion.
*/


void
expand_stmt_with_iterators (stmt)
    tree stmt;
{
    tree iter_list = collect_iterators (stmt);
    expand_stmt_with_iterators_1 (stmt, iter_list);
    istack_sublevel_to_current();
}


void 
expand_stmt_with_iterators_1 (stmt, iter_list)
    tree stmt, iter_list;
{
    if (iter_list == 0) expand_expr_stmt(stmt);
    else
    {
	tree current_iterator = TREE_VALUE(iter_list);
	tree iter_list_tail   = TREE_CHAIN(iter_list);
	rtx p_start, p_end, e_start, e_end;

	iterator_loop_prologue(current_iterator, &p_start, &p_end);
	expand_stmt_with_iterators_1(stmt, iter_list_tail);
	iterator_loop_epilogue(current_iterator, &e_start, &e_end);

	/** Delete all inner expansions based on current_iterator **/
	/** before adding the outer one. **/

	delete_ixpansion(current_iterator);
	add_ixpansion(current_iterator, p_start, p_end, e_start, e_end);
    }
}


/* Returned is a list of free, i.e. not bound by "for" statement or an */
/* accumulator, iterator decls collected from EXP */

tree
collect_iterators(exp)
    tree exp;
{
    enum tree_code code;
    tree the_list = (tree) 0;

    if (exp == (tree) 0) return (tree) 0;

    code = TREE_CODE(exp);
    switch (code) {
      case VAR_DECL: {
	  if (ITERATOR_P(exp) &&
	      ! ITERATOR_BOUND_P(exp))
	      the_list = build_tree_list(0,exp);
      }
	break;
      case TREE_LIST: {
	  tree tail;
	  the_list = (tree) 0;
	  for(tail = exp; tail; tail = TREE_CHAIN(tail)) {
	      tree op = TREE_VALUE(tail);
	      tree op_iterators = collect_iterators(op);
	      the_list = merge_iterator_lists(the_list, op_iterators);
	  }
      }
	break;

	/* we do not automatically iterate blocks -- one must */
	/* use the FOR construct to do that */

      case BLOCK: return (tree) 0;

      default: {
	  char class = TREE_CODE_CLASS(code);
	  if (strchr("12<er", class)) {
	      int num_args = tree_code_length[code];
	      int i;
	      the_list = (tree) 0;
	      for(i=0; i<num_args; i++) {
		  tree op = TREE_OPERAND(exp, i);
		  tree op_iterators = collect_iterators(op);
		  the_list = merge_iterator_lists(the_list, op_iterators);
	      }
	  }
      }
    }
    return the_list;
}

/*
Merge two lists without replication. This is inefficient algorithm,
but should be good enough for short lists.   This is a NON-DESTRUCTIVE
merge, which makes it even less efficient.
*/

tree
merge_iterator_lists(list1, list2)
    tree list1, list2;
{
    tree new_list = list1;
    tree l2;
    if (list1  == 0)
	return list2;
    for (l2=list2; l2; l2 = TREE_CHAIN(l2)) {
	tree l1;
	tree it = TREE_VALUE(l2);
	int duplicate = 0;

	for(l1 = new_list; l1 ; l1 = TREE_CHAIN(l1)) {
	    if (it == TREE_VALUE(l1)) {
		duplicate = 1;
		break;
	    }
	}
	if (! duplicate) {
	    tree new_head = build_tree_list(0, it);
	    TREE_CHAIN(new_head) = new_list;
	    new_list = new_head;
	    }
    }
    return new_list;
}

/*
Emit rtl stream for the prologue of the loop generated by iterator
IDECL. If necessary, create an rtl for loop counter and store it as DECL_RTL 
of IDECL. The prologue always starts and ends with notes, which are returned
by this function in parameters START_NOTE and END_NODE.
*/

iterator_loop_prologue(idecl, start_note, end_note)
    tree idecl;
    rtx *start_note, *end_note;
{

    /*    rtx counter_rtx = gen_reg_rtx(SImode);*/
    rtx counter_rtx;
    

    tree limit_expr = ITERATOR_LIMIT(idecl);
    rtx limit_rtx = RTL_EXPR_RTL(limit_expr);
    rtx start_rtx = const0_rtx;
    rtx seq_pattern;
    rtx cond_check_label = gen_label_rtx();
    rtx start_label      = gen_label_rtx();
    rtx exit_label       = gen_label_rtx();
    rtx the_insn;
    
    if (DECL_RTL(idecl) == 0)
	DECL_RTL(idecl) = gen_reg_rtx(SImode);
    counter_rtx = DECL_RTL(idecl);
    if (start_note)
	*start_note = 
	    emit_note("Start Iterator Prologue", NOTE_ITERATOR_EXPANSION);
    emit_move_insn(counter_rtx, start_rtx); /* initialize counter */
    expand_start_loop_continue_elsewhere(1);
    ITERATOR_BOUND_P(idecl) = 1;

    if (end_note)
	*end_note =
	    emit_note("End Iterator Prologue", NOTE_ITERATOR_EXPANSION);
}

/*
Similar to the previous function, but for the epilogue.

DECL_RTL is zeroed unless we are inside "({...})". The reason for that is
described below.

When we create two (or more)  loops based on the  same IDECL, and both
inside the same "({...})"  construct, we  must be prepared  to  delete
both of the loops  and create a single one  on the  level  above, i.e.
enclosing the "({...})". The new loop has to use  the same counter rtl
because the references to the iterator decl  (IDECL) have already been
expanded as references to the counter rtl.

We don't want to use the same counter rtl accross different functions,
though, becuase rtl_obstack, in which  pseudo regs are allocated, gets
cleaned up.  Moreover, this would mean creating counter rtx before any
function is comiled,  but at that time  pseudo regs can not be created
because they make sense only inside functions.  */

iterator_loop_epilogue(idecl, start_note, end_note)
    tree idecl;
    rtx *start_note, *end_note;
{
    rtx counter_rtx = DECL_RTL(idecl);
    rtx limit_rtx = DECL_RTL(ITERATOR_LIMIT(idecl));
    rtx exit_label       = gen_label_rtx();
    rtx step_rtx  = const1_rtx;
    rtx seq_pattern, the_insn;
    rtx bge;

    if (start_note)
	*start_note = emit_note("Start Iterator Epilogue", NOTE_ITERATOR_EXPANSION);
    expand_loop_continue_here();
    emit_insn(gen_addsi3(counter_rtx, counter_rtx, step_rtx));

    bge = (rtx)gen_bge(exit_label);

    emit_insn(gen_cmpsi(counter_rtx, 
			force_reg(SImode, limit_rtx),
			XEXP (XEXP (bge,1), 0)));
    emit_jump_insn(bge);
    expand_end_loop();
    emit_label(exit_label);
    ITERATOR_BOUND_P(idecl) = 0;
    /* we can reset rtl since there is not chance that this expansion */
    /* would be superceded by a higher level one */
    if (top_level_ixpansion_p())
	DECL_RTL(idecl) = 0;
    if (end_note)
	*end_note = emit_note("End Iterator Epilogue", NOTE_ITERATOR_EXPANSION);
}

/*
		KEEPING TRACK OF EXPANSIONS

In order to  clean  out expansions corresponding to  statements inside
"{(...)}" constructs we  have to   keep track of   all expansions. The
cleanup  is needed  when   an automatic,  or implicit,  expansion   on
iterator, say X, happens to a statement  which contains a {(...)} form
with a  statement already expanded  on X. In  this  case we have to go
back and cleanup the inner expansion. This can  be further complicated
by the fact that {(...)} can be nested.

To make this cleanup possible, we keep lists of all expansions, and to
make it work for nested constructs, we  keep a stack. The list  at the
top  of  the  stack (ITER_STACK.CURRENT_LEVEL)   corresponds  to   the
currently parsed level. All expansions of the levels below the current
one    are  kept in    one    list whose  head   is   pointed   to  by
ITER_STACK.SUBLEVEL_FIRST (SUBLEVEL_LAST  is there  for  making merges
easy). The process works as follows:

-- On "({"  a new node is added to the stack by PUSH_ITERATOR_STACK.
	    The sublevel list is not changed at this point.

-- On "})" the list for the current level is appended to the sublevel
	   list. 

-- On ";"  sublevel lists are appended to the current level lists.
	   The reason is this: if they have not been superseded by the
	   expansion at the current level, they still might be
	   superseded later by the expansion on the higher level.
	   The levels do not have to distinguish levels below, so we
	   can merge the lists togethere.
*/

struct  ixpansion {
    tree ixdecl;		/* Iterator decl */
    rtx  ixprologue_start;	/* First insn of epilogue. NULL means */
				/* explicit (FOR) expansion*/
    rtx  ixprologue_end;
    rtx  ixepilogue_start;
    rtx  ixepilogue_end;
    struct ixpansion *next;	/* Next in the list */
};

static struct obstack ixp_obstack;

struct iter_stack_node {
    struct ixpansion *first;	  /* Head of list of ixpansions */
    struct ixpansion *last;	  /* Last node in list  of ixpansions */
    struct iter_stack_node *next; /* Next level iterator stack node  */
};

struct  {
    struct iter_stack_node *current_level;
    struct iter_stack_node sublevel;
} iter_stack;

/** Return true if we are not currently inside a "({...})" construct */

top_level_ixpansion_p()
{
    return iter_stack.current_level == 0;
}

/** Append list of X to that of Y. Result in Y, X "unchanged" **/
/** This shoud work even when any of the lists is empty **/

void isn_append(x,y)
    struct iter_stack_node *x,*y;
{
    int
	empty_x = x->first == 0,
	empty_y = y->first == 0;
    if (empty_x) 
	return;
    if (empty_y) {
	y->first = x->first;
	y->last  = x->last;
    }
    else {
	y->last->next = x->first;
	y->last = x->last;
    }
}

/** Make X empty **/

#define ISN_ZERO(X) (X).first=(X).last=0

/*
Called on ";", this  function  appends  sublevel list to   the current
level and empty the sublevel This is done  at  the end  of a statement
when there is no possible expansion at the curent level and no need to
distinguish sublevels from the current level
*/

istack_sublevel_to_current()
{
    if (iter_stack.sublevel.last &&
	iter_stack.sublevel.last->next != 0)
	abort();
    /* At the top level we can throw away sublevel's expansions  **/
    /* because there is nobody above us to ask for a cleanup **/
    if (iter_stack.current_level != 0)
	/** Merging with empty sublevel list is a no-op **/
	if (iter_stack.sublevel.last)
	    isn_append (&iter_stack.sublevel, iter_stack.current_level);
    ISN_ZERO(iter_stack.sublevel);
}

/*
Called on "})", this routine appends  top-level list to  sublevels and
pops the top-level
*/

pop_iterator_stack()
{
    if (iter_stack.sublevel.last && iter_stack.sublevel.last->next != 0)
	abort();

    /** We might not have expansion list, but stack nodes must be there*/
    if (iter_stack.current_level == 0)
	abort();
    isn_append(iter_stack.current_level, &iter_stack.sublevel);
    /** Pop current level node: */
      iter_stack.current_level = iter_stack.current_level->next;
}

/********** ON "({" **********/

/*
Called on "({", this function pushs an empty list on top of the 
iterator stack, i.e. the current level. This list will be used for
recording all iterator expansions inside this "({...})".
*/

push_iterator_stack()
{
    struct iter_stack_node *new_top = 
	(struct iter_stack_node*) 
	    obstack_alloc(&ixp_obstack, sizeof(struct iter_stack_node));

    new_top->first = 0;
    new_top->last = 0;
    new_top->next = iter_stack.current_level;
    iter_stack.current_level = new_top;
}


/*
Record  an iterator expansion  ("ixpansion") for IDECL.  The remaining
paramters describe prologue and epilogue. The node for he expansion is
registed at the current level of the iterator stack.
*/

add_ixpansion(idecl, pro_start, pro_end, epi_start, epi_end)
    tree idecl;
    rtx pro_start, pro_end, epi_start, epi_end;
{
    struct ixpansion* newix;
    
    /* If current_level is 0 we are not inside "({...})", */
    /*   so we do not do anything */

    if (iter_stack.current_level == 0) return;

    newix = (struct ixpansion*) obstack_alloc(&ixp_obstack,
					      sizeof(struct ixpansion));
    newix->ixdecl = idecl;
    newix->ixprologue_start = pro_start;
    newix->ixprologue_end   = pro_end;
    newix->ixepilogue_start = epi_start;
    newix->ixepilogue_end   = epi_end;

    newix->next = iter_stack.current_level->first;
    iter_stack.current_level->first = newix;
    if (iter_stack.current_level->last == 0)
	iter_stack.current_level->last = newix;
}

/*
Delete all ixpansions for iterator IDECL in the sublevel(!).
This means
- finding each ixpansion for IDECL in the list for the sublevel.
- deleting each insn in  corresponding prologue and epilogue.
- removing ixpansion from the sublevel list.
*/


static void
delete_ixpansion(idecl)
    tree idecl;
{
    struct ixpansion* previx = 0, *ix;

    for(ix=iter_stack.sublevel.first; 
	ix; 
	ix = ix->next)
    {
 	if (ix->ixdecl == idecl) {
	    rtx i;
	    
	    /** zero means that this is a mark for FOR -- **/
	    /** we do not delete anything, just issue an error. **/

	    if (ix->ixprologue_start == 0)
		error
		    ("Implicitly iterated block contains `for' for the same iterator");
	    else
	    {
		/* We delete all insns, including notes because leaving loop */
		/* notes and barriers produced by iterator expansion would */
		/* be misleading to other phases */

		for(i=NEXT_INSN(ix->ixprologue_start);
		    i != ix->ixprologue_end;
		    i = NEXT_INSN(i)) 
		    delete_insn(i);
		for(i=NEXT_INSN(ix->ixepilogue_start);
		    i != ix->ixepilogue_end;
		    i = NEXT_INSN(i)) 
		    delete_insn(i);
	    }
	    if (previx)
		previx->next = ix->next;
	    else 
		iter_stack.sublevel.first = ix->next;
	    if (iter_stack.sublevel.last == ix)
		iter_stack.sublevel.last = previx;
	}
	else previx = ix;
    }
}

/*
We initialize iterators obstack once per file
*/

init_iterators()
{
    gcc_obstack_init(&ixp_obstack);
    (void) obstack_alloc(&ixp_obstack, 0);
}

#ifdef DEBUG_ITERATORS

/*
The functions below are for use from source level debugger.
They print short forms of iterator lists and the iterator stack.
*/

/* Print the name of the iterator D */
void
PRDECL(D)
    tree D;
{
    if (D)
    {
	if (TREE_CODE(D) == VAR_DECL)  {
	    tree tname = DECL_NAME(D);
	    char *dname = IDENTIFIER_POINTER(tname);
	    fprintf(stderr, dname);
	}
	else fprintf(stderr, "<<Not a Decl!!!>>");
     }
    else
	fprintf(stderr, "<<NULL!!>>");
}

/* Print Iterator List -- names only */

tree
pil(head)
    tree head;
{
    tree current, next;
    for (current=head; current; current = next)
    {
	tree node = TREE_VALUE(current);
	PRDECL(node);
	next = TREE_CHAIN(current);
	if (next) fprintf(stderr, ",");
    }
    fprintf(stderr, "\n");
}

/* Print IXpansion List */

struct ixpansion *
pixl(head)
    struct ixpansion *head;
{
    struct ixpansion *current, *next;
    fprintf(stderr, "> ");
    if (head == 0)
	fprintf(stderr, "(empty)");
	
    for (current=head; current; current = next)
    {
	tree node = current->ixdecl;
	PRDECL(node);
	next = current->next;
	if (next)
	    fprintf(stderr, ",");
    }
    fprintf(stderr, "\n");
    return head;
}

/* Print Iterator Stack*/

void
pis()
{
    struct iter_stack_node *stack_node;

    fprintf(stderr, "--SubLevel: ");
    pixl(iter_stack.sublevel.first);
    fprintf(stderr, "--Stack:--\n");
    for(stack_node = iter_stack.current_level;
	stack_node;
	stack_node = stack_node->next)
	pixl(stack_node->first);
}
#endif
