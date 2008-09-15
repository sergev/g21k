/* @(#)action.h	2.3 2/13/95 2 */

typedef struct action ACTION;

struct action
{
        ACTION         *next;                /* Pointer to next action */
        short          type;                 /* Action type */
        int            attributes;           /* RAM,ROM,PORT */
        unsigned long  address;              /* starting address of section */
        unsigned long  width;                /* section width */
        char           sect_name[SYMNMLEN + 1];   /* section name */
        unsigned short sect_type;            /* Section type from COFF section.h file */
} ;
  
#define ACTION_DEFINE_SECTION  0x1L

extern LIST section_list;
