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

unsigned int encode_opcode(const char * op_args, const char * op_desc, const int num, ...);
void match_maskstring_to_args(const char * op_args, const char * op_desc);
void get_const_mask_bits(const char * op_desc, unmask * unmask);

