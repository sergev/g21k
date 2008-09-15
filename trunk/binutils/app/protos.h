/* from mp.c */
void  usage           (char *exename);
char *get_root_name   (char *file_ptr);
int   main            (int argc,char * *argv,char * *envp);
void  cond            (int dir_kind);
void  non_cond        (int dir_kind);
int   readline        (void );
void  writeline       (char *lineptr);
int   dir_find        (char *cp);
int   formal          (void );
char *actual          (char *src,int argcnt);
char *get_def         (char *src,int numargs);
int   expand_pound    (char ch, char *dst, char *dstmax, int pound);
int   expand          (char *src,char * *dst,char *dstmax,int argcnt,int poundflag);
void  exp_err         (char *msg,int ala_cnt);
char *get_buffer      (void );
char *release_buffer  (void );

/* from mpdata.c */

/* from mpexpr.c */
int expr              (char *exptr);
int eval              (void );
int query             (void );
int lor               (void );
int land              (void );
int bor               (void );
int bxor              (void );
int band              (void );
int eql               (void );
int relat             (void );
int shift             (void );
int primary           (void );
int term              (void );
int unary             (void );
int factor            (void );
int constant          (void );
int num               (int flag);
int geteql            (void );
int getrel            (void );
char getch             (void );
int ungetch           (void );
int skipws            (void );
void experr           (char *msg);

/* from mpsym.c */
struct sym *lookup    (char *symbol);
struct sym *sym_enter (char *symbol,int nargs,char *defptr);
int         sym_del   (char *symbol);
int         bkt_map   (char c);
void        sym_print (void );
void        sym_init  (void );

/* from mputil.c */
void  printerr                  (char *msg,char *oa);
void  printwarn                 (char *msg,char *oa);
void  screech                   (char *msg);
char *skipblnk                  (char *ptr);
char *get_mem                   (int nbytes);
char *pop                       (struct stack *stkaddr);
int   push                      (char *item,struct stack *stkaddr);
int   pushi                     (int item,struct stacki *stkaddr);
int   popi                      (struct stacki *stkaddr);
int   check_else_count          (struct stacki *stkaddr);
void  update_elif_logic         (int logic,struct stacki *stkaddr);
int   get_elif_logic            (struct stacki *stkaddr);
int   empty                     (struct stack *stkaddr);
int   empty_ifstack             (struct stacki *stkaddr);
char *skipq                     (char *src,char * *dst,char *dstmax);
char *skipcom                   (char *src);
int   myputch                   (char ch,char * *where,char *limit);
void  rlse_ala                  (int last);
char *get_id                    (char *src,char *dst);
int   c_alpha                   (char c);
int   c_alnum                   (int c);
int   reserved_macro            (struct sym *stp);
void  push_ala                  (int numargs);
void  pop_ala                   (int numargs);
void  include_init_command_line (char *include_path);
void  include_init              (void );
char *is_defined                (char *src,char *ch);

