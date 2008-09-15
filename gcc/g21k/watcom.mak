CC1_OBJS = c-parse.obj c-lang.obj c-lex.obj c-decl.obj c-typeck.obj &
       c-conver.obj c-aux-in.obj c-common.obj toplev.obj version.obj tree.obj &
       print-tr.obj stor-lay.obj fold-con.obj function.obj stmt.obj expr.obj &
       calls.obj expmed.obj explow.obj optabs.obj varasm.obj rtl.obj &
       print-rt.obj rtlanal.obj emit-rtl.obj dbxout.obj sdbout.obj &
       dwarfout.obj xcoffout.obj integrat.obj jump.obj cse.obj loop.obj &
       unroll.obj flow.obj stupid.obj combine.obj regclass.obj local-al.obj &
       global-a.obj reload.obj reload1.obj caller-s.obj insn-pee.obj &
       reorg.obj sched.obj final.obj recog.obj reg-stac.obj insn-rec.obj &
       insn-ext.obj insn-out.obj insn-emi.obj insn-att.obj aux-outp.obj &
       getpwd.obj atof.obj obstack.obj

G21K_OBJS = gcc.obj version.obj gcc_ach.obj achparse.obj util.obj atfile.obj obstack.obj
CPP_OBJS  = cccp.obj cexp.obj version.obj obstack.obj

WONTCOMPILE=

CC=wcl386
CFLAGS=/d2/fpc

cc1.exe: $(CC1_OBJS)
    wlink @watcc1

cpp.exe: $(CPP_OBJS)
	$(CC) $(CFLAGS) $(CPP_OBJS) /fo=cpp.exe

g21k.exe:
	$(CC) $(CFLAGS) $(G21K_OBJS) /fo=g21k.exe

.c.obj :
    $(CC) $(CFLAGS)/c $[*

