#include <inttypes.h>

typedef struct Parsed_ARG
{
  uint64_t arg_val;
  uint64_t mask_value;

  char arg_t;
  unsigned int arg_len;
  unsigned int arg_shift;
}Parsed_ARG;

#define MAX_ARGS 100

unsigned int encode_opcode(const char * op_args, const char * op_desc, const int num, ...);
