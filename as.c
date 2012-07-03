#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
#include <regex.h>
#include "parse.h"
#include "encode.h"
#include "setup_regex.h"

int main(int argc, char **argv)
{
  FILE * f;
  cpu_instr_set set;
  argument_list arg_list;
  int parseFile_ret;
  int try_count;
  unsigned int i;
  int a,retval;
  signed try_again;
  symbol * symbols;
  symbol * hardsymbols;
  symbol_table sym_table;
  symbol_table hsym_table;
  /*unmask u;*/
  /*char result[30];*/
  /*00000000 0000 0001*/

  f=0;
  symbols = 0;
  sym_table.n_symbols=0;
  hsym_table.n_symbols=0;
  retval=0;

  if(!setup_global_regex())
    {
      fprintf(stderr,"Failed to setup Global Regex strings\n");
      free_global_regex();
      return 1;
    }

  symbols = malloc(sizeof(symbol)*MAX_SYMBOLS);
  sym_table.table = symbols;
  hardsymbols = malloc(sizeof(symbol)*MAX_SYMBOLS);
  hsym_table.table = hardsymbols;

  if((symbols!=0))
    {
      memset(symbols, '\0', sizeof(symbol)*MAX_SYMBOLS);
      sym_table.table_limit = MAX_SYMBOLS;
      memset(hardsymbols, '\0', sizeof(symbol)*MAX_SYMBOLS);
      hsym_table.table_limit = MAX_SYMBOLS;
    }
  /*
  printf("instruction set: %lu kB\n",sizeof(cpu_instr_set)/1024);
  printf("argument list: %lu kB\n",sizeof(argument_list)/1024);
  */
  for(a=0;a<argc;a++)
    {
      if(strncmp(argv[a],"-set", 4)==0)
	{
	  a++;
	  loadCPUFile(argv[a],&set,&arg_list, &sym_table, &hsym_table);
	  printf("Using set: %s\n",argv[a]);
	}
    }

  /*printf("Creating POSIX regular expressions...");*/
  if(make_all_arg_regex(&arg_list))
    {
      fprintf(stderr,"Could not create POSIX regular expressions.\n");
      free_all_regex(&arg_list);
      free_global_regex();
      free(symbols);
      free(hardsymbols);
      fclose(f);
      return 1;
    } else
    {
      /*printf("OK\n");*/
    }

  /*get_const_mask_bits("0101asdkjas010", &u);*/
  /*match_maskstring_to_args("a,b", "aaaaaabbbbbb1010");*/

  /*Testing
  match_argument(result,30,"[0x80+A]",&arg_list.arg[9],0);
  printf("Argument 1 of %s is %s\n","[0x80+A]",result);
  match_argument(result,30,"[0x80+A]",&arg_list.arg[9],1);
  printf("Argument 2 of %s is %s\n","[0x80+A]",result);
  */

  if(1)
    {
      printf("Assembling %s..\n",argv[a-1]);
      f=fopen(argv[a-1],"r");
    }
  try_again=1;
  try_count=0;
  if((f!=0) && (symbols!=0))
    {
      while(try_again && (try_count<5))
	{
	  rewind(f);
	  size_counter=0;
	  parseFile_ret = parseFile(f,&set, &arg_list, &sym_table, &hsym_table);
	  switch(parseFile_ret)
	    {

	    case 1:
	      printf("\n------- Has undefined lines, try again...\n");
	      try_again=1;
	      try_count++;
	      break;

	    case 0:
	      try_again=0;
	      for(i=0;i<sym_table.n_symbols;i++)
		{
		  if(sym_table.table[i].is!=DEFINED)
		    {
		      try_again=1;
		    }
		}
	      if(!try_again)
		{
		} else
		{
		  printf("\n------- Has updated lines, try again...\n");
		}
	      break;

	    case -1:
	      try_again=0;
	      break;

	    default: break;
	    }
	}

      if(try_again)
	{
	  printf("\nFailed assembly.\n");
	  retval=1;
	}
      else if(try_again==0)
	{
	  printf("\nAssembled OK.\n");
	  retval=0;
	}

      fclose(f);
    } else 
    {
      fprintf(stderr,"File not found.\n");
      retval=1;
    }

  free_all_regex(&arg_list);
  free_global_regex();
  free(symbols);
  free(hardsymbols);
  
  return retval;
}
