#include <inttypes.h>

int string_contains(const char * str, const char * chars, const unsigned int size);
uint64_t binstr_to_uint64(const char * str, const unsigned int size);
unsigned int count_args(const char * op_args, const unsigned int strlen_op_args);
ARG_TYPE isHex(const char * str, const unsigned int len);
ARG_TYPE isDigit(const char * str, const unsigned int len);
ARG_TYPE isNumberType(const char * str, const unsigned int len);

uint64_t bitSize(uint64_t a);
uint64_t nWords(const uint64_t bitsize, const uint64_t wordsize);
char * remWhite(char * s, const unsigned int len);
void splitString(char * store, const char * str, const unsigned int len, const char split, const unsigned int nth);
