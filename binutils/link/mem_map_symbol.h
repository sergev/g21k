

typedef struct mem_map_symbol MEM_MAP_SYMBOL;

 struct mem_map_symbol
{
    char          *symbol_name;
    unsigned long address;
    unsigned long type;
    long          mem_type;
    char          sclass;
    char          num_aux;
    INPUT_SECT    *input_sect;
};
