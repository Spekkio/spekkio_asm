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
  regex_t start;
  unmask u;
  size_t nmatch = 3; /*26*/
  regmatch_t rmt[3];

  /*
  const char patt[] = "^\\[{1,1}[0123456789x]{1,6}[+]{1,1}[ABCXYZIJ]{1,1}\\]{1,1}$";
  */
  const char patt[] = "^[\\ ]*\\[\\{1\\}[\\ ]*\\([0-9x]\\{1,1000\\}\\)[\\ ]*+\\{1\\}[\\ ]*\\([0-9A-Za-z]\\{1,1000\\}\\)[\\ ]*\\]\\{1\\}[\\ ]*$";
  char match[] = "[0x90+0x90]";

  get_const_mask_bits("0101asdkjas010", &u);
  match_maskstring_to_args("a,b", "aaaaaabbbbbb1010");
  /*

   */

  symbols = malloc(sizeof(symbol)*MAX_SYMBOLS);
  sym_table.table = symbols;
  sym_table.table = sym_table.table;

  printf("%lu kB\n",sizeof(cpu_instr_set)/1024);

  if(!regcomp(&start, patt, 0))
    {
      if(regexec(&start, match, nmatch, rmt, 0))
	{
	  printf("No match\n");
	}
      else
	{
	  match[rmt[1].rm_eo]='\0';
	  match[rmt[2].rm_eo]='\0';
	  printf("Match - ARG1:%s ARG2:%s \n", &match[rmt[1].rm_so],&match[rmt[2].rm_so]);
	}

    } else printf("Some regexp error\n");  

  regfree(&start);

  f=0;

  /*
  printf("DCPU-16\nIFN I, 0    -> 0x%X\n",encode_opcode("a,b","bbbbbbaaaaaa1101",MAX_ARGS,REG_I,LITERAL(0x00)));
  printf("SUB I, 1    -> 0x%X\n",encode_opcode("a,b","bbbbbbaaaaaa0011",MAX_ARGS,REG_I,LITERAL(0x01)));
  printf("SET X, 0x04 -> 0x%X\n",encode_opcode("a,b","bbbbbbaaaaaa0001",MAX_ARGS,REG_X,LITERAL(0x04)));
  printf("SET PC, POP -> 0x%X\n",encode_opcode("a,b","bbbbbbaaaaaa0001",MAX_ARGS,0x1C,0x18));
  printf("\nMicrochip test:\nADDWF 0x10,0,1 -> 0x%X\n",encode_opcode("f,d,a","001001daffffffff",MAX_ARGS,0x10,0,1));
  */

  loadCPUFile("instr_sets/DCPU-16-1_7.set",&set,&arg_list);

  match_argument("[0x80+A]",&arg_list.arg[9]);

  /*  
  for(i=0;i<set.num;i++)
    {
      printf("%s %s -> %s = ",set.instr[i].instr_name,set.instr[i].args,set.instr[i].op_desc);
      printf("0x%04X\n",encode_opcode(set.instr[i].args, set.instr[i].op_desc, set.instr[i].n_args, 0x5, 0x21));
    }
  */

  if(argc==2)
    {
      printf("Assembling %s..\n",argv[1]);
      f=fopen(argv[1],"r");
    }

  if((f!=0) && (symbols!=0))
    {
      parseFile(f,&set);
      fclose(f);
      free(symbols);
    } else printf("Specify a file.\n");

  
  return 0;
}
