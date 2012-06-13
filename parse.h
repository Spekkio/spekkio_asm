#include <inttypes.h>

typedef enum is_def {DEFINED, UNDEFINED} is_def;
typedef enum PARSE_TYPE {INSTR, ARGU, OPT} PARSE_TYPE;

#ifndef MAX_INSTRUCTIONS
#define MAX_INSTRUCTIONS 100 /*max number of instructions in instruction set*/
#endif
#ifndef MAX_ARGUMENTS
#define MAX_ARGUMENTS 100 /*max number of arguments in argument description list*/
#endif
#ifndef MAX_NAME_LEN /*maxstrlen of this string, instruction name "MOV", "SUB", "MUL"*/
#define MAX_NAME_LEN 10
#endif
#ifndef MAX_ARG_LEN /*maxstrlen of this string, list of arguments "a,b,c,d"*/
#define MAX_ARG_LEN 100
#endif
#ifndef MAX_OP_DESC /*this string, arg->binary string "aaaaaabbbbbb0101"*/
#define MAX_OP_DESC 64
#endif
#ifndef MAX_ARGS /*max number of arguments one instruction can have, count from this string "a,b,c,d"=4*/
#define MAX_ARGS 10
#endif
#ifndef MAX_OP_DESC /*max bit length of one op code, this string "aaaabbbb0101"*/
#define MAX_OP_DESC 64
#endif
#ifndef MAX_ARG_PARSED_LEN /*max length of one argument in source, "[0x1000 + X]"*/
#define MAX_ARG_PARSED_LEN 50
#endif
#ifndef MAX_SYM_NAME_LEN /*max length of one symbol name, variable, pointer*/
#define MAX_SYM_NAME_LEN 100
#endif
#ifndef MAX_SYMBOLS
#define MAX_SYMBOLS 0xFFFF
#endif


typedef struct
{
  unsigned int arg_regex_len;
  char arg_regex[MAX_NAME_LEN];
  unsigned int arg_subargs_len;
  unsigned int n_args;
  char arg_subargs[MAX_ARG_LEN];
  unsigned int arg_desc_len;
  char arg_desc[MAX_OP_DESC];
}argument;

/*The list of arguments parsed from the set file*/
typedef struct
{
  argument arg[MAX_ARGUMENTS];
  unsigned int num;
}argument_list;

typedef struct
{
  uint64_t const_;     /*The contant values of the opcode, surrounding the arguments*/
  uint64_t const_mask; /*The bitmask of the contant value*/
}asm_parse;

typedef struct
{
  int instr_name_len;
  char instr_name[MAX_NAME_LEN];
  int n_args;
  char args[MAX_ARG_LEN];
  int op_len;
  char op_desc[MAX_OP_DESC];
}cpu_instr;

/*The list of instructions parsed from the set file*/
typedef struct
{
  cpu_instr instr[MAX_INSTRUCTIONS];
  unsigned int num;
}cpu_instr_set;

typedef struct
{
  char string[MAX_SYM_NAME_LEN];
  uint64_t value;
  is_def is;
}symbol;

/*The list of symbols parsed from the set file*/
/*arguments and instructions without constant value*/
typedef struct
{
  int n_symbols;
  symbol * table;
}symbol_table;

int parseLine(const char * line, cpu_instr_set * set);
void parseFile(FILE * f, cpu_instr_set * set);
void loadCPUFile(const char * filename, cpu_instr_set * set, argument_list * arg_list);
int parseCPULine(const char * line, cpu_instr * ret);
void addInstruction(const cpu_instr instr, cpu_instr_set * set);
int parseARGLine(const char * line, argument * ret);
void addArgument(const argument arg, argument_list * arg_list);
int match_argument(char * result, const int max_result_len, const char * match, const argument * arg, const unsigned int arg_number);

