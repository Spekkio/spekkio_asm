#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
#include <regex.h>
#include "encode.h"
#include "parse.h"
#include "setup_regex.h"

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
  /*00000000 0000 0001*/

  f=0;
  symbols = 0;
  sym_table.n_symbols=0;

  if(!setup_global_regex())
    {
      fprintf(stderr,"Failed to setup Global Regex strings\n");
      free_global_regex();
      return 1;
    }

  symbols = malloc(sizeof(symbol)*MAX_SYMBOLS);
  sym_table.table = symbols;

  if((symbols!=0))
    {
      memset(symbols, '\0', sizeof(symbol)*MAX_SYMBOLS);
      sym_table.table_limit = MAX_SYMBOLS;
    }

  printf("instruction set: %lu kB\n",sizeof(cpu_instr_set)/1024);
  printf("argument list: %lu kB\n",sizeof(argument_list)/1024);

  loadCPUFile("instr_sets/DCPU-16-1_7.set",&set,&arg_list, &sym_table);

  printf("Creating POSIX regular expressions...");
  if(make_all_arg_regex(&arg_list))
    {
      fprintf(stderr,"Could not create POSIX regular expressions.\n");
      free_all_regex(&arg_list);
      free_global_regex();
      free(symbols);
      fclose(f);
      return 1;
    } else
    {
      printf("OK\n");
    }

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
      if(parseFile(f,&set, &arg_list, &sym_table)==1)
	{
	  printf("Success!\n");
	}

      fclose(f);
    } else printf("Specify a file.\n");

  free_all_regex(&arg_list);
  free_global_regex();
  free(symbols);
  
  return 0;
}
