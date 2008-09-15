/* @(#)list.h	1.4  6/11/91 */

struct list
{
    char *head;
    char *tail;
} ;

#define LIST_SECTION          0x1L
#define LIST_ADDRESS          0x2L
#define LIST_MEMORY           0x3L
#define LIST_OUTPUT_SECTION   0x4L
#define LIST_INC_INPUT_SECTS  0x5L
#define LIST_INPUT_SECTION    0x6L
#define LIST_INPUT_FILE       0x7L
