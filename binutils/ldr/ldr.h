#ifndef MSDOS
#define huge  
#endif
struct init_info
{
    unsigned long int start_address;
    unsigned long int end_address;
    unsigned long int px2_data;
    unsigned short int px1_data;     		/* 16 LSBs of 48 bit word*/
    struct init_info *next;
}; 

struct my_section_list
{
	SCNHDR	new_hdr;
	SCNHDR  old_hdr;
	long int delta_section_ptr;
	struct my_section_list *next;
};

extern FILE *input_fd, *output_fd;

extern struct init_info init_base_dm, init_base_pm;
extern struct init_info init_bss_dm_base, init_bss_pm_base;
extern struct init_info *init_ptr;

extern struct my_section_list my_section_base;

extern long int delta_symptr;
extern long int delta_section_ptr;

extern unsigned long int zero_count_dm, zero_count_pm;
extern unsigned long int init_length_dm, init_length_pm;
extern unsigned char huge *init_string_dm;
extern unsigned char huge *init_string_pm;

#define FATAL 1

void fatal_mem_error( char *file, int line, char *error_string, ... );
void message(long error);
void linker_exit(int error);


#define DM_WORD_SIZE 5
#define PM_WORD_SIZE 6


void	copy_block(long int bytes);
void	makestandalone(void);
void	fatal_linker_error(char *file, int line, char *error, ...);
void 	create_init_list(struct my_section_list *section, int word_size);
void 	output_init_data(void);
void 	output_init_section(struct my_section_list *s, unsigned long int a);
void	clean_list(void);
void	init_pass1(void);
void	process_arguments(int argc, char **argv);
void	make_standalone(void);
void 	user_warn(char *file, int line, char *error, ...);
