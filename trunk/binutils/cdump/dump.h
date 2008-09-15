/*
 * flag bit constants
 */
#define         FILE_HEADER             0x1
#define         OPTIONAL_HEADER         0x2
#define         SECTION_HEADER          0x4
#define         SECTION_DATA            0x8
#define         RELOCATION              0x10
#define         LINE_NUMBER             0x20
#define         AUX_ENTRY               0x40
#define         STRING_TABLE            0x80
#define         SYMBOL_TABLE            0x100



typedef enum
{
		DESC_FILHDR =0,
		DESC_OPTHDR,
		DESC_SCNHDR,
		DESC_SCNDAT,
		DESC_RELOC,
		DESC_LINENO,
		DESC_AUXENT,
		DESC_STRING,
		DESC_SYMBOL
} DESC_CODE;
		
/*
 * error stuff
 */
typedef enum
{
	ERR_FOPEN = 0,
	ERR_FSEEK,
	ERR_FREAD,
	ERR_BAD_SWITCH,
	ERR_FILE_CORRUPTED,
	ERR_OUT_OF_RANGE,
	ERR_IMPROPER_ALLIGNMENT,
	ERR_USAGE
} ERR_CODE;

#define         FATAL                   0x1
#define         WARNING                 0x0

#define         PM_WORDS_PER_LINE       5
#define         DM_WORDS_PER_LINE       6

#ifndef SEEK_SET
#define SEEK_SET        0       /* Set file pointer to "offset" */
#define SEEK_CUR        1       /* Set file pointer to current plus "offset" */
#define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
#endif

/*
 * prototypes
 */
int print_file_header(FILE *fp, FILHDR header);
int print_optional_header(FILE *fp, FILHDR header);
int print_section_header(FILE *fp, FILHDR header);
int print_section_data(FILE *fp, FILHDR header);
int print_relocation_information(FILE *fp, FILHDR header);
int print_line_number_information(FILE *fp, FILHDR header);
int print_auxiliary_entry(FILE *fp, FILHDR header);
int print_string_table(FILE *fp, FILHDR header);
int print_symbol_table(FILE *fp, FILHDR header);
      



void error( int error_code, int numargs, ... );
int get_section(SCNHDR *section_header, FILE *fp, short section_number, FILHDR file_header);


