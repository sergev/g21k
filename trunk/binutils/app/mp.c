/* @(#)mp.c     1.18 2/27/91 1 */

/*
 * MP -- Macro and conditional compilation preprocessor for assembler
 */

#include <stdio.h>
#include <string.h>
#include <stddef.h>

#ifdef  MSDOS
#undef  stderr
#define stderr  stdout
#endif

#include "app.h"
#include "mpdefs.h"
#include "release.h"
#include "util.h"
#include "protos.h"

#ifdef  MSDOS
#include        <process.h>
#endif

#ifdef ATFILE
#include "atfile.h"
#endif

static void banner (void);
static short banner_printed = 0;

#define         IS_FORMAL       0x80
#define         FORMAL_NUMBER   0x3f    /* 64 is maximum number of formals */
#define         IS_POUND        0x40    /* #define  str(s)      # s        */
#define         IS_POUNDPOUND   0x20    /* #define  d( a, b)    a ## b     */
#define         IS_A_MARK       0xc0    /* only IS_FORMAL and IS_POUND get */

/* a marked in the define buffer   */


#define PEB_LEN                 300
#define EXPAND_BUFF_LIMIT       20
char            expand_buffer[EXPAND_BUFF_LIMIT][PEB_LEN];
int             expand_index = 0;

#define     MAXARG      15

static char system_string[132];
char            name_buf[NAME_BUFF_SIZE];
static char     file_name[NAME_BUFF_SIZE];
static char     tryagain[NAME_BUFF_SIZE];
#ifndef MSDOS /*EK*/
char            call_buff[NAME_BUFF_SIZE];

#endif

extern char    *getenv();
static char    *include_path = (char *) NULL;
static int      dopp = FALSE;
static char    *execargv[MAXARG] = 

{

#ifdef  MSDOS
    "a21000.exe",
#else   /*EK*/
    "a21000",
#endif
    (char *) 0,
};


/*
 * Main driver
 */

void usage(exename)
char    *exename;
{
    exename;
    fprintf( stderr, "\n" );    
    fprintf(stderr, "Usage: asm21k [-Dname -h -l -o name -pp -sp -v] filename\n");
    fprintf( stderr, "Where:\n");
    fprintf( stderr, "\t-Dname\t\tDefine 'name' to the assembler preprocessor\n" );
    fprintf( stderr, "\t-h\t\tDisplay usage\n" );
    fprintf( stderr, "\t-l\t\tGenerate a listing file\n" );
    fprintf( stderr, "\t-o name\t\tName the output object file\n" );
    fprintf(stderr, "\t-pp\t\tPreprocessor only\n");
    fprintf( stderr, "\t-sp\t\tSkip the assembler preprocessor\n" );
    fprintf( stderr, "\t-v\t\tVerbose\n" );
    fprintf( stderr, "\n" );    
    exit(-1);
}

char *get_root_name(file_ptr)
char *file_ptr;
{
  char *root_name;

  root_name = strdup (file_ptr);
  strip_path (root_name);
  strip_ext (root_name);
  return (root_name);
}

int main(argc, argv, envp)
    int             argc;
    char           *argv[];
    char           *envp[];
{

    short           i;
    short           arg;
    short           dir_kind;
    short           filecnt = 0;
    short           skip_pp = FALSE;
    short           pponly  = FALSE;
    short           verbose = FALSE;
    char            *tp1 = NULL;
    char            *apc = NULL;
    char            *infile = NULL;
    char            *root_ptr = NULL;
    char            *outfile = NULL;
    char            *named_outfile = NULL;
    char            name[MAXIDLEN + 1];
    char            *path;

#ifdef ATFILE
    process_atfiles(&argc, &argv);
#endif

    lineno[0] = 0;              /* Initialize line number stack */
    sym_init();                 /* Initialize symbol table */
	

/* Check for verbose flag first */
   for( i = 1; i < argc; i++) {
     if( *argv[i] == '-' ) {
       switch(*(argv[i] + 1) ) {
	 case 'v':
	 case 'V':
	 case 'h':
	 case 'H':
	   verbose = TRUE;
	   break;
	 }
     }
   }
   if (verbose) {
     if (!banner_printed) {
       banner ();
       banner_printed = 1;
     }
   }


/* Process arguments */
    for( i = 1, arg = 1; i < argc; i++ ) 
    {
	 if( *argv[i] == '-' ) 
	 {
	     switch(*(argv[i] + 1) ) 
	     {
		     case 'p':
		     case 'P':
			if( (*(argv[i] + 2) == 'p') || (*(argv[i] + 2) == 'P') )
			    pponly = TRUE;
			break;

		     case 's':
		     case 'S':
			if( (*(argv[i] + 2) == 'p') || (*(argv[i] + 2) == 'P') )
			    skip_pp = TRUE;
			break;

		     case 'l':
		     case 'L':
			execargv[arg++] = argv[i];
			break;

		     case 'z':
		     case 'Z':
			execargv[arg++] = argv[i];      
			break;

		     case 'u':
		     case 'U':
			execargv[arg++] = argv[i];
			break;

		     case 'c':
		     case 'C':
			execargv[arg++] = argv[i];
			break;
		
		     case 'v':
		     case 'V':
			execargv[arg++] = argv[i];  /* Pass it thru to assembler */
			break;

		     case 'd':
		     case 'D':
			lp = argv[i] + 2;
			apc = lp;
			while( *apc && (*apc != '=') )
			       apc++;
			if( *apc == '=' )
			    *apc = ' ';
			non_cond( DEF );
			break;

		     case 'o':
		     case 'O':
			execargv[arg++] = argv[i];
			i++;
			execargv[arg++] = argv[i];
			named_outfile = make_my_slash(argv[i], FALSE);
			break;

		     case 'h':
		     case 'H':
		     default:
			usage( argv[0] );
	     }
	 }
	 else 
	 {
	     if( infile == NULL ) 
	     {
		 infile = make_my_slash(argv[i], FALSE);
		 filecnt = 1;
	     }
	     else
		 printwarn( "file already specified", "" );
	 }
    }


    if( (filecnt == 0) || (argc == 1) ) {
      if (!banner_printed) {
	banner ();
	banner_printed = 1;
      }
      usage(argv[0]);
    }


    if( strlen(infile) > NAME_BUFF_SIZE - 2 )
    {
	fprintf( stderr, "Filename %s is too long\n", argv[i] );
	exit( 1 );
    }

    root_ptr = get_root_name( infile );

/*
    infile = (char *) my_malloc((long)(strlen(root_ptr) + 5));
    strcpy( infile, root_ptr );
    strcat( infile, ".asm" );
 */

    /* Load the root file name into the fname stack */

    strncpy( fname[0], infile, NAME_BUFF_SIZE );

    outfile = (char *) my_malloc((long)(strlen(root_ptr) + 125));
    strcpy( outfile, root_ptr );
    strcat( outfile, ".cpp" );
    if( pponly && named_outfile )
	strcpy( outfile, named_outfile);

#ifdef DEBUG
    i = 0;
    while( execargv[i] )
    {
	   fprintf( stderr, "%d\t%s\n", i, execargv[i] );
	   i++;
    }
#endif

    execargv[arg++] = "-r";    
    execargv[arg++] = outfile;
    execargv[arg++] = (char *)NULL;

    if( !skip_pp )
    {
	if( (curinfil = fopen( infile, "r")) == (FILE *) NULL )
	{
	    strcpy( tryagain, infile );
	    strcat( tryagain, ".asm" );

	    if( (curinfil = fopen(tryagain, "r")) == NULL )
	    {
		fprintf(stderr, "Input file %s does not exist.", infile );
		exit(1);                
	    }
	}

	if( (outfil = fopen(outfile, "w")) == (FILE *) NULL )
	{
	    fprintf(stderr, "Error opening %s for output.", outfile);
	    exit(1);
	}

	include_init_command_line( include_path );
	include_init();                            /* Initialize include directory list */

	/*
	 * Main driving loop of the processor
	 */

	while( readline() != EOF )
	{
	       tp1 = skipblnk( lp );

	       if( tp1[0] == '#' && !in_com )
	       {  
		   lp = skipblnk(&tp1[1]);
		   lp = get_id(lp, name);
		   lp = skipblnk(lp);
		   if( (dir_kind = dir_find(name)) == ERROR )
		       printerr("\"%s\" Illegal directive", name);
		   else 
		   {
		       if( father && self )
			   non_cond(dir_kind);
		       cond(dir_kind);
		   }
	       }
	       if( *tp1 == '#' || !(father && self) )
		   line[0] = EOS;

	       writeline(line);
	}

	if( !empty_ifstack( &ifstack ) )        /* Check for dangling if's       */
	    printerr( "Unterminated #if", "" );

	fclose( outfil );
	fclose( curinfil );

	if( err_cnt ) 
	{
	    fprintf(stderr, "Preprocessor errors - can not continue\n");
	    exit(1);
	}
    }

    if( pponly )
	exit( (err_cnt != 0) ? 1 : 0);


    if( (path = get_adi_dsp()) != NULL ) /*EK*/
    {
	i = 0;
	while( *path && *path != ';' )
	{
	    file_name[i] = *path;
	    path++;
	    i++;
	}

	if( file_name[i-1] != '\\' && file_name[i-1] != '/' )
	{
	    file_name[i] = FILENAME_SEPARATOR;
	    ++i;
	}
	file_name[i] = '\0';
#ifdef MSDOS
	strcat (file_name,"21k\\etc\\");
#else
	strcat (file_name,"21k/etc/");
#endif
	strcat( file_name, execargv[0] );
    }
    else
	strcpy(file_name, execargv[0] );

/* Build the system string */
#ifdef MSDOS
   strcpy (system_string, file_name);
   strcat (system_string, " ");
   i = 1;
   while (execargv[i]) {
     strcat (system_string, execargv[i]);
     strcat (system_string, " ");
     i++;
   }
#endif

/* Call the real assembler */
#ifdef MSDOS
    i = system (system_string);
#else
    i = execvp( file_name, execargv);
#endif

    if (i) {
      fprintf(stderr, "acpp could not activate  \"%s\". Execution terminated\n", execargv[0]);
    }

    envp;

#ifdef ATFILE
    free_argv();
#endif

    return i;
}

/*
 * Process conditional compilation directives.  This procedure is always
 * executed, regardless of whether or not we are flushing due to a false
 * conditional. The general technique used here is as follows. The variables
 * 'self' and 'father' contain the truth value of the current if-else-endif
 * clause and the immediately enclosing clause, respectively.  Whenever an if
 * is encountered, both are stacked, father &= self (to propagate any false
 * conditionals), and self = eval(arg).
 * 
 * Upon encountering an else, self = !self, to reverse the sense of the current
 * condition.  An endif causes the condition stack to be popped into father
 * and self, restoring our context to the immediately enclosing level.
 * 
 * Note: This is a bad algorithm, as it requires a stack (and thus a maximum
 * nesting level).  A true/false counter arrangement is to be preferred.
 * 
 */

void cond(dir_kind)
register int dir_kind;
{
    char  symbol[MAXIDLEN + 1];       /* buffer for ifdef/ifndef  */

    switch( dir_kind ) 
    {
	case IF:
	case IFD:
	case IFN:
	    if( (pushi(father, &ifstack) == ERROR) || (pushi(self, &ifstack) == ERROR) )
	    {
		printerr("Maximum #if depth exceeded", "");
		break;
	    }
	    if( father &= self )
	    {
		switch( dir_kind ) 
		{
		    case IF:
			self = (expr(lp) != 0);
			break;
		    case IFD:
			lp = get_id(lp, symbol);
			self = (lookup(symbol) != (struct sym *) NIL);
			break;
		    case IFN:
			lp = get_id(lp, symbol);
			self = (lookup(symbol) == (struct sym *) NIL);
			break;
		}
		update_elif_logic( self, &ifstack );

	    }
	    else
		self = FALSE;
	    break;

	case ELIF:
	    if( empty_ifstack(&ifstack) )
		printerr("#elif without #if", "");
	    else if( !get_elif_logic(&ifstack) ) 
	    {
		self = (expr(lp) != 0);
		update_elif_logic(self, &ifstack);
	    }
	    break;

	case ELSE:
	    if (empty_ifstack(&ifstack))
		printerr("#else without #if", "");
	    else if (!check_else_count(&ifstack))
		self = !self;
	    break;

	case ENDIF:

	    /*
	     * if (*lp != EOS) printerr("Extraneous argument", "");
	     */
	    if (empty_ifstack(&ifstack))
		printerr("#endif without #if", "");
	    else 
	    {
		self = (int) popi(&ifstack);
		father = (int) popi(&ifstack);
	    }
	    break;
    }
    return;
}

/*
 * Non-conditional directive processing is performed here.  This procedure is
 * executed only if we are not currently flushing.  The message and undef
 * directives are implemented in a very straightforward manner.  The others
 * are a little more complex.  Macro definition is accomplished in four
 * steps.  First the name of the macro being defined is picked up; second
 * formal parameters are processed; third, the actual definition is processed
 * (and index marker substitution is peformed), and finally, the symbol is
 * entered into the symbol table. Include processing goes as follows.  First
 * the path name and its delimiter are picked up, then we attempt to find the
 * file by prepending the paths specified in 'srchlist[]' to it.  If the file
 * is found, the current input file is stacked (without closing it) and the
 * included file is set up as the new current input file.
 */

void non_cond(dir_kind)
    int             dir_kind;
{
    register int    i,argcnt;
    char           *defptr = 0,
		   *fid,
		    delim,
		    name[MAXIDLEN + 1];
    extern char    *get_def();
    FILE           *tbp = (FILE *) NULL;
    struct sym     *stp;
    char           *pgb;
    char            pragma_id[MAXIDLEN];

#define OPEN_BUFF_LEN   NAME_BUFF_SIZE * 2
    static char     open_buff[OPEN_BUFF_LEN];

    switch (dir_kind) {
	case PRAGMA:            /* check thru acceptable list of PRAGMA
				 * directives */

	    lp = skipblnk(lp);
	    lp = get_id(lp, pragma_id);
	    i = 0;
	    while (pragma_list[i].pragma_string) {
		if (strcmp(pragma_id, pragma_list[i].pragma_string) == 0)
		    break;
		i++;
	    }

	    if (!pragma_list[i].pragma_string)
		printerr("Unknown PRAGMA operand \"%s\"", pragma_id);

	    else {
		fprintf(outfil, "%s\n", line);
		if (pragma_list[i].pragma_match) {
		    do {
			if (readline() == EOF) {
			    printerr("unmatched \"pragma\"", "");
			    break;
			}
			fprintf(outfil, "%s\n", line);
		    } while (strncmp(line, "#pragma", 7) != 0);
		}
	    }
	    break;

	case LINE:
	    lp = skipblnk(lp);
	    if (!isdigit(*lp)) {
		/* Macro expansion takes place for line parameter */
		pgb = get_buffer();
		expand(lp, &pgb, pgb + PEB_LEN, 0, 0);
		lp = release_buffer();
	    }
	    if (isdigit(*lp)) {
		pgb = lp;
		while (isdigit(*lp))
		    lp++;
		lp = skipblnk(lp);
		myputch('#', (char **) NIL, (char *) NIL);
		while (pgb < lp) {
		    myputch(*pgb, (char **) NIL, (char *) NIL);
		    pgb++;
		}
		if (*lp == '\"') {
		    lp = skipq(lp, (char **) NIL, (char *) NIL);

		}
		else if (*lp)
		    printerr("String literal expected", "");

	    }
	    else
		printerr("Digit expected", "");

	    break;
	case ERR:
	    printerr(lp, "");
	    break;
	case UNDEF:
	    lp = get_id(lp, name);
	    if (sym_del(name) == ERROR)
		printerr("Symbol not defined", "");
	    break;
	case DEF:
	    lp = get_id(lp, name);
	    if (*name == EOS) {
		printerr("No symbol given", "");
		break;
	    }
	    if ((argcnt = formal()) == ERROR) {
		printerr("Illegal argument list", "");
		break;
	    }

	    stp = lookup(name);
	    if (stp) {
		if (reserved_macro(stp)) {
		    printwarn("Cannot redefine %s macro", name);
		    break;
		}
		else {
		    if (defptr && strcmp(stp->defptr, defptr) != 0) {
			printwarn("%s redefined", name);
			sym_del((char *)stp);
			/* break; */
		    }
		}
	    }

	    defptr = get_def(lp, argcnt);
	    sym_enter(name, argcnt, defptr);
	    break;

	case INCL:
	    if (inclvl == (INCDEPTH - 1)) {
		printerr("Maximum #include depth exceeded", "");
		break;
	    }
	    delim = *lp;
	    if (delim != '<' && delim != '"') {
		/* Macro expansion takes place for include parameter */
		pgb = get_buffer();
		expand(lp, &pgb, pgb + PEB_LEN, 0, 0);
		lp = release_buffer();
		delim = *lp;
		if (delim != '<' && delim != '"') {
		    printerr("Illegal file specification delimiter", "");
		    break;
		}
	    }

	    delim = (delim == '<') ? (char)'>' : (char)'"';
	    fid = ++lp;
	    while (*lp != delim && *lp != EOS)
		lp++;
	    if (*lp == EOS) {
		printerr("Illegal file specification", "");
		break;
	    }
	    *lp = EOS;

	    /*
	     * At this point 'fid' points to the null-terminated file
	     * specification.  If the file specification is enclosed in '"'
	     * then the current directory is first searched for the specified
	     * file; if this fails or if the file spec is delimited by '<'
	     * and '>' then a standard list of paths is searched (see
	     * 'srchlist' array).
	     */

	    for (i = (delim == '>') ? include_system_list : 0; srchlist[i]; i++) {
		strcpy(open_buff, srchlist[i]);
		if (debug_include_list)
		    printf("PP-Include [%s] ==> %s\n", fid, srchlist[i]);
		strncat(open_buff, fid,
			OPEN_BUFF_LEN - (strlen(open_buff) + strlen(fid)));
		if ((tbp = fopen(open_buff, "r")) != (FILE *) NULL)
		    break;
	    }
	    if (tbp == (FILE *) NULL) {
		printerr(" ", "");
		fprintf(stderr, "Failed to open '#include' file %s\n",
			fid);
		break;
	    }
	    if (push((char *)curinfil, &inclstk) == ERROR)
		screech("#include stack overflow (impossible)");
	    inclvl++;
	    strncpy(fname[inclvl], fid, NAME_BUFF_SIZE);
	    lineno[inclvl] = 0;
	    fprintf(outfil, "#%d \"%s\"\n",
		    lineno[inclvl] + 1,
		    fname[inclvl]);
	    curinfil = tbp;

	    /*
	     * Inhibit <SOH> in front of #include line at top level
	     */
	    if (inclvl == 1)
		inclflag = TRUE;
	    break;
    }                           /* end switch */
    return;
}                               /* end non_cond() */

/*
 * Read a line of input routine -- loads 'line[]' with the next line from the
 * current input file.  Line continuation is handled here -- any occurrence
 * of backslash-newline is replaced by a blank and the next line is tacked on
 * to the current one.
 * 
 * Upon hitting end of file, the include stack is popped to restore input to the
 * previous level.  If the stack is empty, EOF is returned (indicating end of
 * file at the top level.
 */

#define     TRI_STATE0  0
#define     TRI_STATE1  1
#define     TRI_STATE2  2

int 
readline()
{
    register int    buf_indx,
		    curch,
		    i,
		    trigraph_state;
    int             ctn_cnt;    /* Continuation line counter */

    buf_indx = 0, ctn_cnt = 0;
    trigraph_state = TRI_STATE0;

    do {
	while ((curch = getc(curinfil)) != '\n') {
	    if (curch == EOF) { /* Current file exhausted */
		fclose(curinfil);
		if (empty(&inclstk)) {

		    /*
		     * Don't lose xyz<EOF>
		     */
		    if (buf_indx > 0)
			break;
		    else
			return (EOF);
		}
		else {
		    curinfil =
			(FILE *)
			pop(&inclstk);
		    inclvl--;
		    fprintf(outfil, "#%d \"%s\"\n",
			    lineno[inclvl] + 1,
			    fname[inclvl]);
		    linefeeds = 1;
		}
		continue;
	    }
	    else if (curch == '?') {
		switch (trigraph_state) {
		    case TRI_STATE0:
			trigraph_state = TRI_STATE1;
			break;
		    case TRI_STATE1:
			trigraph_state = TRI_STATE2;
			break;
		    case TRI_STATE2:
			break;
		}
	    }
	    else if (trigraph_state == TRI_STATE2) {
		switch (curch) {
		    case '=':
			buf_indx = buf_indx - 2;
			curch = '#';
			break;
		    case '(':
			buf_indx = buf_indx - 2;
			curch = '[';
			break;
		    case '/':
			buf_indx = buf_indx - 2;
			curch = '\\';
			break;
		    case ')':
			buf_indx = buf_indx - 2;
			curch = ']';
			break;
		    case '\'':
			buf_indx = buf_indx - 2;
			curch = '^';
			break;
		    case '<':
			buf_indx = buf_indx - 2;
			curch = '{';
			break;
		    case '!':
			buf_indx = buf_indx - 2;
			curch = '|';
			break;
		    case '>':
			buf_indx = buf_indx - 2;
			curch = '}';
			break;
		    case '-':
			buf_indx = buf_indx - 2;
			curch = '~';
			break;
		}
		trigraph_state = TRI_STATE0;
	    }

	    if ( (int)( (char)curch ) != curch)
	      printerr("Data conversion error on char.", "");
		
	    line[buf_indx++] = (char)curch;
	    if (buf_indx >= LINESZ) {
		printerr("Next line too long, truncated.", "");
		while (curch != '\n' && curch != EOF)
		    curch = getc(curinfil);
		break;
	    }
	}
	if (line[0] == '\f') {
	    if (buf_indx == 1) {
		curch = ' ';
		buf_indx = 0;
	    }

#ifdef  UNIMATION
	    lineno[inclvl] = 0;
#else
	    lineno[inclvl]++;   /* <FF> == <Newline>     */
#endif
	}
	else {
	    lineno[inclvl]++;
	    if (line[buf_indx - 1] == '\\') {
		curch = ' ';
		buf_indx--;
		ctn_cnt++;      /* Count continuations   */
	    }
	}
    } while (curch != '\n' && curch != EOF);

    line[buf_indx] = EOS;
    lp = line;
    for (i = 0; i < ctn_cnt; i++)
	writeline("");          /* Keep compiler in synch        */
    return (OK);
}

/*
 * Write line routine -- calls expand() to process macro calls, emitting
 * expanded text to the output file.  If we are processing a line from an
 * include file, a <SOH> character is emitted preceding the line as a flag to
 * the compiler.
 */

void writeline(lineptr)
    char           *lineptr;
{

    /*
     * Save our environment for expand() error recovery. Then, expand the
     * line to the output file.
     */
    if (setjmp(jump_env) == 0)
	expand(lineptr, (char **) NIL, (char *) NIL, 0, 0);
    myputch('\n', (char **) NIL, (char *) NIL);
}

/*
 * Find a directive
 */

dir_find(cp)
    register char  *cp;
{
    register int    i;

    for (i = 0; dir_tbl[i]; i++)
	if (strcmp(dir_tbl[i], cp) == 0)
	    return (dir_type[i]);
    return (ERROR);
}

/*
 * Set up definition argument list array (def_ala) with the formal parameters
 * of the macro currently being defined.  Returns the number of formals
 * actually processed, or ERROR if any syntax errors are encountered.  Nested
 * parantheses are handled here to permit macro calls as formal parameters.
 */

int 
formal()
{
    register int    cnt;

    if (*lp != '(')
	return (0);
    for (cnt = 0; cnt < ALASIZE; cnt++) {
	lp = skipblnk(++lp);
	lp = get_id(lp, def_ala[cnt]);
	if (def_ala[cnt][0] == EOS)
	    break;
	lp = skipblnk(lp);
	if (*lp != ',')
	    break;
    }
    if (*lp != ')')
	return (ERROR);
    lp++;                       /* Skip past the ')'             */
    return (cnt + 1);           /* Make it origin 1 and return   */
}

/*
 * Set up call argument list array (call_ala) with the actual parameters of
 * the macro currently being expanded.  Each entry of the call_ala is
 * dynamically allocated; the maximum length of any one actual is determined
 * by the compile-time constant "MAXARGSZ".  All occurrences of formal
 * parameters are replaced by their corresponding actuals at the next outer
 * level of call, thereby propagating actual parameters through nested calls.
 * The updated source pointer is returned to the call unless an error is
 * encountered, in which case "ERROR" is returned.
 */

char           *
actual(src, argcnt)
    register char  *src;
    int             argcnt;
{
    register int    argno,
		    parenlvl;
    char           *dst,
		   *actp;
    char           *dstmax;
    static char     arg_buf[MAXARGSZ];
    static char    *proto_ala[ALASIZE]; /* Call's prototype ala  */

    if (argcnt == 0)
	return (src);

    src = skipblnk(src);
    if (*src != '(') {
	printerr("Required argument(s) missing", "");
	return ((char *) NIL);
    }

    dstmax = arg_buf + MAXARGSZ - 1;

    for (argno = 0; argno < argcnt && *src != ')'; argno++) {

	dst = arg_buf, src++, parenlvl = 0;
	for (;;) {
	    src = skipq(src, &dst, dstmax);
	    if ((*src == ')' || *src == ',') && parenlvl == 0)
		break;
	    switch (*src) {
		case '(':
		    parenlvl++;
		    break;
		case ')':
		    parenlvl--;
		    break;
		case EOS:
		    printerr("Unterminated argument list", "");
		    rlse_ala(argno - 1);
		    return ((char *) NIL);
	    }
	    if (!(*src & IS_FORMAL))
		myputch(*src++, &dst, dstmax);
	    else {
		actp = call_ala[*src++ & FORMAL_NUMBER];
		while (*actp != EOS)
		    myputch(*actp++, &dst, dstmax);

	    }
	}
	if (myputch(EOS, &dst, dstmax) == ERROR) {
	    printerr("Actual parameter length exceeds maximum", "");
	    arg_buf[0] = EOS;
	}

	actp = arg_buf;
	actp = skipblnk(actp);
	dst--;
	dst--;
	while (*dst == ' ' || *dst == '\t') {
	    *dst = EOS;
	    dst--;
	}
	proto_ala[argno] = get_mem((dst - actp) + 2);
	strcpy(proto_ala[argno], actp);
    }

    if (*src++ != ')' || argno < argcnt) {
	printerr("Argument count error", "");
	rlse_ala(argno);
	return ((char *) NIL);
    }
    else {
	for (argno = 0; argno < argcnt; argno++)
	    call_ala[argno] = proto_ala[argno];
	return (src);
    }
}

/*
 * Get macro definition routine -- loads a dynamically-allocated buffer with
 * the text of the macro definition.  A pointer to the resulting definition
 * block is passed back to the caller.  A pointer to the start of the
 * definition to be processed is passed in 'srcp'; the number of arguments to
 * the macro is passed in 'numargs'. Index markers representing the index of
 * the formal parameter in the ala are substituted for formals as the
 * definition is copied.  Index markers are encoded as (IS_FORMAL | ala
 * index).  Note that no buffer limit checking is performed, because in no
 * case will the processed text be larger than the original source.
 * 
 * Leading whitespace is discarded.  05-Dec-80 RBD
 */

char           *
get_def(src, numargs)
    register char  *src;
    int             numargs;
{
    register char  *p;
    register int    ala_index;
    char           *dst;
    static char     defbuf[LINESZ];
    char            idbuf[MAXIDLEN + 1];
    int             mask;

    src = skipblnk(src);        /* Junk leading whitespace */
    dst = defbuf;

    for (;;) {
	src = skipq(src, &dst, defbuf + LINESZ - 1);

	if (*src == '#') {
	    src++;
	    if (*src == '#') {
		src++;
		if (dst == defbuf) {
		    printerr("the ## operator requires left operand", "");
		    break;
		}
		dst--;
		while (*dst == ' ' || *dst == '\t')
		    dst--;
		dst++;
		mask = IS_POUNDPOUND;
	    }
	    else
		mask = IS_POUND;

	    src = skipblnk(src);
	}
	else
	    mask = 0;

	if (c_alpha(*src)) {
	    p = get_id(src, idbuf);
	    for (ala_index = 0; ala_index < numargs; ala_index++) {
		if (strcmp(idbuf, def_ala[ala_index]) == 0) {
		  *dst++ = (char)((mask & IS_A_MARK) |
				  IS_FORMAL | ala_index);
		    src = p;
		    break;
		}
	    }

	    if (ala_index >= numargs && mask)
		printerr(
		      "The #/## operators must be followed by a parameter", "");

	    else {
		while (src < p)
		    *dst++ = *src++;
	    }

	}
	else {
	    if (mask)
		printerr(
			 "The # operator must be followed by a parameter", "");

	    else {
		if ((*dst++ = *src++) == EOS)
		    break;
	    }

	}

    }
    p = get_mem(dst - defbuf);
    strcpy(p, defbuf);
    return (p);
}

/*
 * Expand a #define     str(s)  # s     type macro
 */
int
expand_pound( char ch, char *dst, char *dstmax, int pound)
{
    int             ret = OK;

    if (pound && (int)(ch == '\\' || ch == '\"'))
	ret = myputch('\"', (char **)dst, dstmax);

    if (ret != ERROR)
	ret = myputch(ch, (char **)dst, dstmax);

    return ret;
}


/*
 * Macro expansion routine -- scans text pointed to by 'srcp' for macro calls
 * and formal parameters (which are by now represented as index markers (see
 * 'get_def()') ).  If a character is neither a part of a macro call nor an
 * index marker, it is transmitted to the output file or buffer (see
 * 'myputch()').  Upon encountering a macro call, the current argument list
 * array is pushed onto the stack, the new actual parameters are loaded, and
 * 'expand()' is called recursively.  On return from this recursive call, the
 * arguments are released and the ala stack is popped.  Upon encountering a
 * formal parameter, 'expand()' is simply called recursively to expand the
 * actual parameter corresponding to the formal.  Note that the buffer length
 * check is performed only upon encountering the end of the current
 * definition; meanwhile excess characters are flushed.  Thus only one check
 * is necessary and only one error is reported.  To prevent cyclical
 * definitions, each symbol table entry has a flag which is set whenever a
 * call on this macro is encountered.  This flag is checked for a zero value
 * before expansion of a macro begins; if it is set, the error return is
 * taken.  This prevents circular definitions from causing problems.
 */

int
expand(src, dst, dstmax, argcnt, poundflag)
    register char  *src;
    char          **dst,
		   *dstmax;
    int             argcnt;
    int             poundflag;  /* TRUE - then expand invoked with # */
{
    char           *p,
		    ch;
    register struct sym *sym;
    char            idbuf[MAXIDLEN + 1];
    char           *srcptr;
    char           *glue_buff,
		   *peb;


    for (;;) {
	src = skipq(src, dst, dstmax);
	if (c_alpha(*src)) {
	    p = get_id(src, idbuf);
	    sym = lookup(idbuf);
	    if ((sym == (struct sym *) NIL) ||
		    (sym->ref && !sym->nargs)) {
		while (src < p)
		    expand_pound(*src++, (char *)dst, dstmax,
				 poundflag);
	    }
	    else if (sym == definedloc) {
		src = is_defined(p, &ch);
		expand_pound(ch, (char *)dst, dstmax, poundflag);

	    }
	    else {

		if (sym == lineloc)
		    sprintf(lineloc->defptr, "%-*d",
			    LINE_LOC_LEN,
			    lineno[inclvl]);
		else if (sym == fileloc)
		    strcpy(fileloc->defptr, fname[inclvl]);

		src = p;
		if (sym->ref != 0) {
		    ;



		}
		else {
		    if (push((char*)sym, &refstack) == ERROR)
			exp_err("Exceeded def nest",
				argcnt);
		    else
			sym->ref = 1;
		}
		push_ala(argcnt);
		if ((src = actual(src, sym->nargs)) == (char *) NIL)
		    exp_err((char *) NIL, 0);
		expand(sym->defptr, dst, dstmax, sym->nargs,
		       poundflag);
		rlse_ala(sym->nargs - 1);
		pop_ala(argcnt);
	    }


	}
	else {

	    if (*src & IS_FORMAL) {

		/*
		 * Check for ## type macros before proceeding.  The only way
		 * two
		 */
		/* formals could be contiguous is via the ## operator.           */

		if (!(*(src + 1) & IS_FORMAL)) {

		    srcptr = call_ala[*src & FORMAL_NUMBER];
		    if (*src & IS_POUND) {
			myputch('\"', dst, dstmax);
			expand(srcptr, dst, dstmax, argcnt, TRUE);
			myputch('\"', dst, dstmax);
		    }
		    else
			expand(srcptr, dst, dstmax, argcnt, poundflag);
		    src++;

		}
		else {



		    glue_buff = get_buffer();
		    *glue_buff = EOS;

		    while (*src && IS_FORMAL) {

			srcptr = call_ala[*src & FORMAL_NUMBER];
			peb = get_buffer();
			if (*src & IS_POUND) {
			    myputch('\"', dst, dstmax);
			    expand(srcptr, &peb,
				   peb + PEB_LEN,
				   argcnt, TRUE);
			    myputch('\"', dst, dstmax);
			}
			else
			    expand(srcptr, &peb,
				   peb + PEB_LEN,
				   argcnt, poundflag);

			*peb = EOS;
			peb = release_buffer();
			strncat(glue_buff, peb,
				PEB_LEN -
				(strlen(glue_buff) +
				 strlen(peb)));
			src++;
		    }

		    release_buffer();
		    expand(glue_buff, dst, dstmax, argcnt, poundflag);
		}


	    }
	    else {
		if (*src == EOS) {
		    if (!empty(&refstack))
			((struct sym *)
			 pop(&refstack))->ref = 0;
		    return (OK);
		}
		else if (expand_pound(*src++,
				      (char *)dst,
				      dstmax,
				      poundflag) == ERROR)
		    exp_err("expansion length exceeded",
			    argcnt);
	    }
	}
    }
}


/*
 * Error routine for expand().  If 'msg' is non-NIL, the message pointed to
 * by 'msg' is printed on the standard error output.  The second argument
 * specifies the number of currently-active entries in the call ala -- that
 * is, the number of ala entries that must be freed up before aborting.
 * After the ala has been released, the ala stack is purged to release the
 * memory used by suspended calls to expand().
 */

void exp_err(msg, ala_cnt)
    register char  *msg;
    int             ala_cnt;
{

    if (msg != (char *) NIL)
	printerr(msg, "");
    while (!empty(&refstack))
	((struct sym *) pop(&refstack))->ref = 0;
    rlse_ala(ala_cnt - 1);      /* Release current ala entries   */
    while (!empty(&callstk))
	my_free(pop(&callstk)); /* Purge stack                   */
    longjmp(jump_env, -1);
}





/********************************
 *
 *
 *******************************/

char           *
get_buffer()
{

    if (expand_index == EXPAND_BUFF_LIMIT) {
	printerr("Expansion limit execeeded due to \"#\" or \"##\"", "");
	exit(1);
    }
    return expand_buffer[expand_index++];
}

char           *
release_buffer()
{
    expand_index--;
    return expand_buffer[expand_index];
}






/*******************************/
static void banner (void) {
     printf( "\nAnalog Devices ADSP-210x0 Assembler Pre-Processor\n");
     printf( "Release %s, Version %s\n", get_release_number(), get_version_number());
     printf( "%s\n", get_copyright_notice() );
     printf( "%s\n", get_warranty_notice() ); /*EK*/
     printf( "%s\n\n", get_license_notice() ); /*EK*/
     }
