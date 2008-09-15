#define STACK_OFFSET 0
#define STACK_LENGTH_OFFSET 1
#define DMBANK1_OFFSET 2
#define DMBANK2_OFFSET 3
#define DMBANK3_OFFSET 4
#define DMWAIT_OFFSET 5
#define PMBANK1_OFFSET 6
#define PMWAIT_OFFSET 7
#define HEAP_OFFSET 8

extern unsigned long C_init_list[];

extern struct heap_list
{
	char name[SYMNMLEN+1];
        int location;
	unsigned long int address;
	unsigned long int length;
        struct heap_list *next;
} heap_base;

extern char 	Arch_descrip_name[];
extern char	Stack_segment_name[];
extern char     Init_segment_name[];
