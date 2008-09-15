/* @(#)atfile.h 1.4 7/20/92 1 */

/*
 * To use atfile's just include this header file in your main file.
 * i.e. the file which has main definied.
 */

typedef struct  cons {
    struct cons *next;
    char str[1];
} CONS;


#define CDR(C) ((C)->next)
#define CAR(C) ((C)->str)

extern CONS *allocated_pool;

extern void process_atfiles (int *pargc, char ***pargv);
extern void free_argv(void);


/* CONS *read_arg_list(char*); *EK* is static */
int    get_length(CONS*);
char **expand_argv(int, char**, int, int);
void   copy_list(char**, CONS*);
/* CONS *nconc ( CONS *f, CONS *n); *EK* is static */

