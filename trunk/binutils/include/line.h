typedef struct
{
    unsigned short line_number;
    union
    {
        long sym_index;
        long address;
    } which;
} LINE;

#define LINE_SIZE sizeof(LINE)

