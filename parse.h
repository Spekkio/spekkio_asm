typedef enum PARSE_TYPE {INSTR, ARGU, OPT} PARSE_TYPE;

#ifndef MAX_INSTRUCTIONS
#define MAX_INSTRUCTIONS 100
#endif
#ifndef MAX_NAME_LEN
#define MAX_NAME_LEN 10
#endif
#ifndef MAX_ARG_LEN
#define MAX_ARG_LEN 100
#endif
#ifndef MAX_OP_DESC
#define MAX_OP_DESC 64
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


void parseLine(const char * line, cpu_instr_set * set);
void parseFile(FILE * f, cpu_instr_set * set);
void loadCPUFile(const char * filename, cpu_instr_set * set);
int parseCPULine(const char * line, cpu_instr * ret);
void addInstruction(const cpu_instr * instr, cpu_instr_set * set);
