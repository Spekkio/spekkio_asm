typedef struct
{
  unsigned int num;
  uint64_t opcode[MAX_OVERFLOWS];
  unsigned int size[MAX_OVERFLOWS]; /*Resulting size of the opcode in bits*/
  is_def is; /*Was there an undefined value? Or was the assembling complete.*/
}assemble_ret;

assemble_ret assemble(instruction * found_instr, const cpu_instr_set * set, const argument_list * arg_list, const symbol_table * symb_list, const symbol_table * hsymb_table);

ARG_TYPE detectType(unsigned int * match_found, const small_argument arg, const argument_list * arg_list, const symbol_table * symb_list, const symbol_table * hsymb_table);

