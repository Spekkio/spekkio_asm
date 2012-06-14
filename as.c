#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
#include <regex.h>
#include "encode.h"
#include "parse.h"

#define REG_A 0
#define REG_B 1
#define REG_C 2
#define REG_X 3
#define REG_Y 4
#define REG_Z 5
#define REG_I 6
#define REG_J 7
#define LITERAL(a) (0x20+a)

int main(int argc, char **argv)
{
  FILE * f;
  cpu_instr_set set;
  argument_list arg_list;

  symbol * symbols;
  symbol_table sym_table;
  /*unmask u;*/
  /*char result[30];*/

  f=0;

  symbols = malloc(sizeof(symbol)*MAX_SYMBOLS);
  sym_table.table = symbols;
  sym_table.table = sym_table.table;
  
  printf("instruction set: %lu kB\n",sizeof(cpu_instr_set)/1024);
  printf("argument list: %lu kB\n",sizeof(arg_list)/1024);

  loadCPUFile("instr_sets/DCPU-16-1_7.set",&set,&arg_list);

  /*get_const_mask_bits("0101asdkjas010", &u);*/
  /*match_maskstring_to_args("a,b", "aaaaaabbbbbb1010");*/

  /*Testing
  match_argument(result,30,"[0x80+A]",&arg_list.arg[9],0);
  printf("Argument 1 of %s is %s\n","[0x80+A]",result);
  match_argument(result,30,"[0x80+A]",&arg_list.arg[9],1);
  printf("Argument 2 of %s is %s\n","[0x80+A]",result);
  */

  if(argc==2)
    {
      printf("Assembling %s..\n",argv[1]);
      f=fopen(argv[1],"r");
    }

  if((f!=0) && (symbols!=0))
    {
      if(parseFile(f,&set, &arg_list)==1)
	{
	  printf("Success!\n");
	}

      fclose(f);
      free(symbols);
    } else printf("Specify a file.\n");

  
  return 0;
}
