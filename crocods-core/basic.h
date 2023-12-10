
struct s_basic_debug_error {
    char *filename;
    int line;
    char msg[256];
    int lenmsg, lenfilename;
};

struct s_basic_info {
    struct s_basic_debug_error error;
    int nberror, maxerror;
    // struct s_debug_symbol *symbol;
    // int nbsymbol, maxsymbol;
    // int run, start;
    // unsigned char *emuram;
    int lenram;
    char err;
};

void untokenizeBasic(u8 *input, char *output);
int tokenizeBasic(u8 *text, u8 *bin, u16 *finallength, struct s_basic_info **debug);

u8 * clean_basic(u8 *dsk, long dsk_size, u16 *length);
