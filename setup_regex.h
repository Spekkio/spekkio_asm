extern regex_t symbol_list;
extern regmatch_t sym_pmatch[];
extern const size_t sym_nmatch;

extern regex_t instr_list;
extern regmatch_t instr_pmatch[];
extern const size_t instr_nmatch;

extern regex_t arg_list;
extern regmatch_t arg_pmatch[];
extern const size_t arg_nmatch;

int make_arg_regex(argument * arg);
int make_all_arg_regex(argument_list * arg);
void free_all_regex(argument_list * arg);
int setup_global_regex(void);
void free_global_regex(void);
