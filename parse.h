#include <inttypes.h>

typedef enum is_def {DEFINED, UNDEFINED} is_def;
typedef enum PARSE_TYPE {INSTR, ARGU, OPT} PARSE_TYPE;

#ifndef MAX_INSTRUCTIONS
#define MAX_INSTRUCTIONS 100 /*max number of instructions in instruction set*/
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

typedef struct
{
  int instr_name_len;
  char instr_name[MAX_NAME_LEN];
  int n_args;
  char args[MAX_ARG_LEN];
  int op_len;
  char op_desc[MAX_OP_DESC];
}cpu_instr;

typedef struct
{
  cpu_instr instr[MAX_INSTRUCTIONS];
  unsigned int num;
}cpu_instr_set;

typedef struct
{
  char parsed_arg[MAX_ARG_PARSED_LEN];
  int arg_index;
}arg_parse;

typedef struct
{
  int instr_index;
  arg_parse arg[MAX_ARGS];
}asm_parse;

typedef struct
{
  char string[MAX_SYM_NAME_LEN];
  uint64_t value;
  is_def is;
}symbol;

void parseLine(const char * line, cpu_instr_set * set);
void parseFile(FILE * f, cpu_instr_set * set);
void loadCPUFile(const char * filename, cpu_instr_set * set);
int parseCPULine(const char * line, cpu_instr * ret);
void addInstruction(const cpu_instr * instr, cpu_instr_set * set);
