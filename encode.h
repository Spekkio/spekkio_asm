#include <inttypes.h>

typedef struct Parsed_ARG
{
  uint64_t arg_val;
  uint64_t mask_value;
  uint64_t mask;

  char arg_t;
  unsigned int arg_len;
  unsigned int arg_shift;
}Parsed_ARG;

typedef struct
{
  uint64_t value;
  uint64_t mask;
  unsigned int arg_len;
  unsigned int arg_shift;
}unmask;

typedef struct
{
  signed error;
  uint64_t value;
}encode_op_ret;

unsigned int encode_opcode(const char * op_args, const char * op_desc, const int num, ...);
void match_maskstring_to_args(const char * op_args, const char * op_desc);
void get_const_mask_bits(const char * op_desc, unmask * unmask);
unsigned int encode_opcode_n(const instruction * found_instr, const char * op_args, const char * op_desc);
encode_op_ret encode_op(const char * op_args, const char * op_desc, const int n_arg, const uint64_t value);
