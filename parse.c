#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>
#include <inttypes.h>
#include "parse.h"
#include "encode.h"
#include "setup_regex.h"
#include "smallfunc.h"
#include "assemble.h"

#ifndef MAX_LONG_REGEX
#define MAX_LONG_REGEX 1000
#endif

unsigned int size_counter=0;

/*return 0 on success, 1 if no match, index stored in ret*/
int match_symbol(unsigned int * ret, const char * match, const symbol_table * symb, const unsigned int strl)
{
  unsigned int i;

  for(i=0;i<symb->n_symbols;i++)
    {
      if(!strncmp(match, symb->table[i].string, strl))
	{
	  *ret=i;
	  return 0;
	}
    }

  return 1;
}

/*returns 0 if success, -1 if memory error or other error, 1 if no match*/
int match_argument(char * result, const int max_result_len, const char * match, const argument * arg, const unsigned int arg_number)
{
  char tempstring[MAX_ARG_PARSED_LEN];
  signed success;  
  size_t nmatch;
  regmatch_t * pmatch;

  success = 0;
  pmatch = 0;

  /*printf("N Args: %u\n", arg->n_args);*/

  if(arg->n_args>0)
    {
      nmatch = arg->n_args+1;
      pmatch = malloc(sizeof(regmatch_t)*nmatch);
    }

  if(pmatch)
    {
      if(1)
	{
	  if(regexec(&arg->reg, match, nmatch, pmatch, 0))
	    {
	      /*printf("No match here...");*/
	    }
	  else
	    {
	      /*pmatch contains the index where the found subarg is*/
	      /*rm_so is the start index*/
	      /*rm_eo is the end index*/
	      /*rm_so-rm_eo is the lenght of the string*/

	      /*
	      for(i=0;i<arg->n_args;i++)
		{
		  printf("Start index: %u\n",pmatch[i].rm_so);
		  printf("End index: %u\n",pmatch[i].rm_eo);
		}
	      */

	      if(arg_number<=arg->n_args)
		{
		  /*Potential Bug*/
		  /*Could need some more checking, check if rm_so and rm_eo doesn't point outside string, etc*/
		  if((pmatch[arg_number+1].rm_eo-pmatch[arg_number+1].rm_so)>=0)
		    {
		      /*printf("Match[%s] @ %u, %u\n", match, pmatch[arg_number+1].rm_so, pmatch[arg_number+1].rm_eo);*/
		      memcpy(tempstring, &match[pmatch[arg_number+1].rm_so], pmatch[arg_number+1].rm_eo-pmatch[arg_number+1].rm_so);
		      tempstring[(pmatch[arg_number+1].rm_eo-pmatch[arg_number+1].rm_so)]='\0';
		      strncpy(result, tempstring, max_result_len);
		      success=1;
		    }
		}

	      /*Make all args*/
	      /*
	      for(i=1;i<(arg->n_args+1);i++)
		{
		  memcpy(tempstring, &match[pmatch[i].rm_so], pmatch[i].rm_eo-pmatch[i].rm_so);
		  tempstring[(pmatch[i].rm_eo-pmatch[i].rm_so)]='\0';
		  //printf("found subarg %u [%u, %u]: %s\n",i,pmatch[i].rm_so, pmatch[i].rm_eo,tempstring);
		}
	      */
	    }
	}
      else
	{
	  /*regfree(&regex);*/
	  free(pmatch);
	  return -1;
	}

      /*regfree(&regex);*/
      free(pmatch);

    } /*if(pmatch)*/
  else
    {
      return -1; /**/
    }

  if(success)
    {
      return 0;
    }
  return 1;
}

void loadCPUFile(const char * filename, cpu_instr_set * set, argument_list * arg_list, symbol_table * sym_table, symbol_table * hsym_table)
{
  FILE *f;
  char c;
  char lineBuffer[10000];
  unsigned int line_counter=0;
  PARSE_TYPE type;
  argument arg;
  cpu_instr instr;
  symbol symb;

  hsym_table = hsym_table;
  
  set->num=0;

  f = fopen(filename, "r");
  type=OPT;

  if(f)
    {
      c=' ';
      while(c!=EOF)
	{
	  c=fgetc(f);

      /*Remove comments*/
	  if(c==';')
	    {
	      while(c!='\n')
		{
		  c=fgetc(f);
		}
	    }

      /*Shorten trailing white-spaces*/
      /*But keep 1 space*/
	  if((c==' ') || (c=='\t'))
	    {
	      lineBuffer[line_counter]=' ';
	      line_counter++;
	      while( ((c==' ') || (c=='\t')) && (c!='\n') )
		{
		  c=fgetc(f);
		}
	    }

      /*Remove comments in end of line*/
	  if(c==';')
	    {
	      while(c!='\n')
		{
		  c=fgetc(f);
		}
	    }

	  if(1)
	    {
	      lineBuffer[line_counter]=c;
	      line_counter++;
	      if((c=='\n'))
		{
		  lineBuffer[line_counter]='\0';
		  if(line_counter>1)
		    {
		      if(!strncmp(lineBuffer,"-INSTRUCTIONS",13))
			{
			  printf("Parsing instructions..\n");
			  type=INSTR;
			}
		      if(!strncmp(lineBuffer,"-ARGUMENTS",10))
			{
			  printf("Parsing arguments..\n");
			  type=ARGU;
			}
		      if(!strncmp(lineBuffer,"-SYMBOLS",8))
			{
			  printf("Parsing symbols..\n");
			  type=SYMB;
			}
		      if(!strncmp(lineBuffer,"-HARDSYMBOLS",8))
			{
			  printf("Parsing hard symbols..\n");
			  type=HSYMB;
			}

		      switch(type)
			{
			case INSTR:
			  if(parseCPULine(lineBuffer,&instr))
			    {
			      addInstruction(instr, set);
			    }
			  break;

			case ARGU:
			  if(parseARGLine(lineBuffer,&arg))
			    {
			      addArgument(arg, arg_list);
			    }
			  break;

			case SYMB:
			  if(parseSYMBLine(lineBuffer,&symb))
			    {
			      addSymbol(symb, sym_table);
			    }

			case HSYMB:
			  if(parseSYMBLine(lineBuffer,&symb))
			    {
			      addSymbol(symb, hsym_table);
			    }
			  break;

			default: break;
			}

		    }
		  line_counter=0;
		}
	    }
	}
      fclose(f);
    }

}

void addInstruction(const cpu_instr instr, cpu_instr_set * set)
{
  set->instr[set->num] = instr;
  set->num++;
}

void addArgument(const argument arg, argument_list * arg_list)
{
  arg_list->arg[arg_list->num] = arg;
  arg_list->num++;
}

PARSE_LINE_RET parseLine(const char * line, const cpu_instr_set * set, instruction * store, symbol_table * sym_table/*, macro * store*/)
{
  PARSE_LINE_RET return_value;
  unsigned int i,c,a,cnt_args,the_instr;
  char temp[MAX_ARG_PARSED_LEN+MAX_NAME_LEN]; /*stores names, args, and macros*/
  signed found_instr;
  char case_line[MAX_CNT_OF_LINE];
  signed whitespace_clear_flag, whitespace_clear_flag_delay;
  symbol new_symbol;


  for(i=0;(i<MAX_CNT_OF_LINE) && (line[i]!='\n') && (line[i]!='\0');i++)
    {
      case_line[i] = toupper(line[i]);
    }

  temp[0]=temp[0];

  found_instr=0;
  cnt_args=0;
  return_value=PARSE_LINE_RET_NOTHING;
  
  whitespace_clear_flag=0;
  whitespace_clear_flag_delay=0;
  for(i=0;(i<strlen(line)) && (line[i]!='\n');i++)
    {

      /*printf("[%u/%c]",i,line[i]);*/

      if(!isspace(line[i]))
	{
	  /*Set if any other character than a whitespace is found*/
	  whitespace_clear_flag_delay=whitespace_clear_flag;
	  whitespace_clear_flag=1;
	}

      if(line[i]==':')
	{
	  a=0;
	  for(i=i+1,a=0;((i<strlen(line)) && (line[i]!=' '));i++,a++)
	    {
	      temp[a]=line[i];
	    }
	  temp[a]='\0';
	  printf("found tag: %s\n",remWhite(temp,strlen(temp)));
	  /*addSymbol tag*/
	  strncpy(new_symbol.string, remWhite(temp,strlen(temp)), MAX_SYM_NAME_LEN);
	  new_symbol.value = size_counter / 16; /*each pointer is 16 bit, this whould not be hardcoded*/
	  new_symbol.bitlen = 0;
	  new_symbol.is = DEFINED;
	  addSymbol(new_symbol, sym_table);

	  whitespace_clear_flag=0; /*Count tags as whitespace*/
	  whitespace_clear_flag_delay=0;

	}
      else if(line[i]=='#')
	{
	  a=0;
	  for(i=i+1,a=0;((i<strlen(line)) && (line[i]!=' '));i++,a++)
	    {
	      temp[a]=line[i];
	    }
	  temp[a]='\0';

	  printf("found macro: %s ",temp);	  
	  if(strncmp(temp,"define",6)==0)
	    {
	      for(i=i+1,a=0;((i<strlen(line)) && (line[i]!=' '));i++,a++)
		{
		  temp[a]=line[i];
		}
	      temp[a]='\0';
	      printf("%s = ",temp);
	      for(i=i+1,a=0;((i<strlen(line)) && (line[i]!='\n') && (line[i]!='\r'));i++,a++)
		{
		  temp[a]=line[i];
		}
	      temp[a]='\0';
	      printf("%s (Use addSymbol())\n",temp);

	      /*found_macro=1;*/ /*we can use parse arguments further down, and define args for macros somewhere.*/
	      return_value = PARSE_LINE_RET_MACRO;

	    } else
	    {
	      fprintf(stderr,"Unknown macro \"%s\" \n",temp);
	      return PARSE_LINE_RET_ERROR; /*macro not defined*/
	    }
	}

      if(found_instr==0)
	{
	  for(c=0;c<set->num;c++)
	    {
	      if(whitespace_clear_flag_delay==0)
		if(!strncmp(&case_line[i],set->instr[c].instr_name,set->instr[c].instr_name_len))
		  {
		    /*printf("found instr: %s ",set->instr[c].instr_name);*/
		    found_instr=1;
		    the_instr=c;
		    i+=set->instr[c].instr_name_len;

		    store->instr_index = c;

		    if(i>strlen(line) || line[i]=='\n' || line[i]=='\r')
		      {
			/*printf("%s",line);*/
			/*return 1;*/
		      }
		  }
	    }
	}else /*Parse arguments*/
	{
	  /*expected number of arguments*/
	  /*set->instr[c].n_args;*/

	  /*printf("[Parse args]");*/
	  if(cnt_args<set->instr[the_instr].n_args)
	    {
	      for(a=0;i<strlen(line) && (line[i]!=',') && (line[i]!='\n') && (line[i]!='\r');i++)
		{
		  if(line[i]!=' ')
		    {
		      temp[a]=line[i];
		      a++;
		    }
		}
	      temp[a]='\0';
	      if(line[i]==',' || line[i]=='\n' || line[i]=='\r')
		{
		  /*printf("%s,",temp);*/
		  strncpy(store->arg[cnt_args].arg, temp,MAX_ARG_PARSED_LEN);
		  store->arg[cnt_args].arg_len=strlen(temp);
		  cnt_args++;
		  store->n_args=cnt_args;
		}
	    }
	}
    }

  if(whitespace_clear_flag==0)
    {
      /*Found only white spaces*/
      return PARSE_LINE_RET_NOTHING;
    }
  else
    {
      if(found_instr)
	{
	  if((cnt_args+1)==set->instr[the_instr].n_args)
	    {
	      fprintf(stderr,"Missing argument in instruction \"%s\" ",set->instr[the_instr].instr_name);
	      return_value = PARSE_LINE_RET_ERROR;
	    }
	  else
	    {
	      return_value = PARSE_LINE_RET_INSTRUCTION;
	    }
	} else if(whitespace_clear_flag==1)
	{
	  return_value = PARSE_LINE_RET_ERROR;
	}
      /*printf(" == %s",line);*/
    }

  return return_value;
}

int parseARGLine(const char * line, argument * ret)
{
  unsigned int i,strl,a;
  char tempstr[MAX_ARG_LEN]; /*Possible BUG, check this*/
  char store[MAX_ARG_LEN];
  signed success;

  success=0;

  if(!regexec(&arg_list, line, arg_nmatch, arg_pmatch, 0))
    {
      /*printf("match!\n");*/
      for(i=1;i<(arg_nmatch);i++)
	{
	  /*if(arg_pmatch[i].rm_so != arg_pmatch[i].rm_eo)*/
	    if((arg_pmatch[i].rm_eo-arg_pmatch[i].rm_so) < MAX_ARG_LEN)
	      if(arg_pmatch[i].rm_so < MAX_ARG_LEN)
		if(arg_pmatch[i].rm_eo < MAX_ARG_LEN)
		  {
		    strl=arg_pmatch[i].rm_eo - arg_pmatch[i].rm_so;
		    strncpy(tempstr, &line[arg_pmatch[i].rm_so], strl);
		    tempstr[strl]='\0';

		    switch(i)
		      {
		      case 0:
			printf("Arg: %s\n",tempstr);
			break;

		      case 1: /*Value of the symbol, currently it can hade binary encoding*/
			strncpy(ret->arg_regex, tempstr, MAX_NAME_LEN);
			ret->arg_regex_len = strl;


			/*printf("Regex string: %s\n",ret->arg_regex);*/

			break;

		      case 2:/*argument list*/
			strncpy(ret->arg_subargs, tempstr, MAX_ARG_LEN);
			ret->arg_subargs_len = strl;
			ret->n_args = count_args(ret->arg_subargs, strl);
			/*printf("Args string: %s\n",ret->arg_subargs);*/
			break;

		      case 3:/*opcode*/
			strncpy(ret->arg_desc, tempstr, MAX_OP_DESC);
			ret->arg_desc_len = strl;
			/*printf("Opcode string: %s\n",ret->arg_desc);*/
			break;

		      case 4:/*Next word*/
			strncpy(ret->arg_overflow, tempstr, MAX_OP_DESC);
			ret->arg_overflow_len = strl;
			/*printf("Next Word string: %s\n",ret->arg_overflow);*/
			break;

		      case 5:/*Shifts*/
			/*printf("Shift string: %s\n",tempstr);*/
			break;

		      case 6:/*Extra regex*/
			for(a=0;a<MAX_ARG_LEN;a++) store[a]='\0';
			for(a=0;a<ret->n_args;a++)
			  {
			    ret->sub_arg[a].subarg_regex[0]='\0';
			    splitString(store, tempstr, strlen(tempstr), ',', a);
			    strncpy(ret->sub_arg[a].subarg_regex, store, MAX_SUBARG_REGEX_LEN);
			    printf("Value string #%u: %s\n",a,store);
			  }
			break;

		      case 7:/**/
			/*printf("#7 string: %s\n\n",tempstr);*/
			/*printf("Value string: %s\n\n",tempstr);*/
			success|=1;
			break;

		      default:
			fprintf(stderr,"Unkown parantesis #%u found in regex when parsing Arguments.\n",i);
			success=0;
			break;
		      }
		  }
	}
    }

  return success;
}

void addSymbol(const symbol symb, symbol_table * sym_table)
{

  strncpy(sym_table->table[sym_table->n_symbols].string, symb.string, MAX_SYM_NAME_LEN);
  strncpy(sym_table->table[sym_table->n_symbols].value_str, symb.value_str, MAX_SYM_NAME_LEN);
  sym_table->table[sym_table->n_symbols].value = symb.value;
  sym_table->table[sym_table->n_symbols].bitlen = symb.bitlen;
  sym_table->table[sym_table->n_symbols].is = symb.is;

  sym_table->n_symbols++;
}

/*return 0 if value was found and stored, else 1*/
int parseAssignSymbolValue(const char * tempstr, const unsigned int strl,  symbol * symb)
{
  if(string_contains(tempstr,"01",strl))
    {
      symb->value=binstr_to_uint64(tempstr, strl);
      symb->bitlen=strl;
      symb->is=DEFINED;
      /*printf("This is binary: %lu, len:%u\n",symb->value,symb->bitlen);*/
      return 0;
    }else
    if(string_contains(tempstr,"0123456789ABCDEFabcdefx",strl))
      {
	if(sscanf(tempstr,"0x%lX",&symb->value)==1)
	  {
	    symb->bitlen=(strl-2)*4;
	    return 0;
	  }else
	  if(sscanf(tempstr,"0x%lx",&symb->value)==1)
	    {
	      symb->bitlen=(strl-2)*4;
	      return 0;
	    }
      }
  return 1;
}

int parseSYMBLine(const char * line, symbol * symb)
{
  unsigned int i,strl;
  char tempstr[MAX_SYM_NAME_LEN];
  signed success;

  success=0;

  symb->is=UNDEFINED;

  if(!regexec(&symbol_list, line, sym_nmatch, sym_pmatch, 0))
    {
      /*printf("match!\n");*/
      for(i=1;i<(sym_nmatch);i++)
	{
	  if(sym_pmatch[i].rm_so != sym_pmatch[i].rm_eo)
	    if((sym_pmatch[i].rm_eo-sym_pmatch[i].rm_so) < MAX_SYM_NAME_LEN)
	      if(sym_pmatch[i].rm_so < MAX_SYM_NAME_LEN)
		if(sym_pmatch[i].rm_eo < MAX_SYM_NAME_LEN)
		  {
		    strl=sym_pmatch[i].rm_eo - sym_pmatch[i].rm_so;
		    strncpy(tempstr, &line[sym_pmatch[i].rm_so], strl);
		    tempstr[strl]='\0';

		    switch(i)
		      {
		      case 1: /*This is where the string, name of the symbol is*/
			/*printf("Symbol name: %s\n",tempstr);*/
			strncpy(symb->string, tempstr, MAX_SYM_NAME_LEN);
			break;
		      case 2: /*Value of the symbol, currently it can hade binary encoding*/
			strncpy(symb->value_str, tempstr, MAX_SYM_NAME_LEN);
			/*printf("Value string: %s\n",symb->value_str);*/

			parseAssignSymbolValue(tempstr, strl, symb);

			success|=1;

			break;
		      case 3:/*Extended*/
			break;
		      default:
			fprintf(stderr,"Syntax error in set file: Unkown[%u] when parsing Symbols, regular expression doesn't match set file syntax.\n",i);
			break;
		      }
		  }
	}
    }

  return success;
}

int parseCPULine(const char * line, cpu_instr * ret)
{
  unsigned int i,c,a,d;

  ret->instr_name_len=0;
  ret->n_args=0;
  ret->op_len=0;

  if(line[0]=='-')
    return 0;

   /*Read the instruction name*/
  for(i=0,a=0;(i<strlen(line)) && (line[i]!=':');i++)
    {
      if(!isblank(line[i]))
	{
	  ret->instr_name[i]=line[i];
	  a++;
	}
    }
  ret->instr_name[a]='\0';
  ret->instr_name_len=a;
  
  /*printf("%s, %u\n",ret->instr_name, a);*/ /*DEBUG*/

  /*Read and count the argument list*/
  for(i=i+1,c=0,a=0,d=0;(i<strlen(line)) && (line[i]!=':');i++,a++)
    {
      if(isalpha(line[i]))
	{
	  ret->args[a]=line[i];
	  c++;
	  d++;
	}else
      if(line[i]==',')
	{
	  ret->args[a]=line[i];
	  d++;
	}
    }
  ret->args[d]='\0';
  ret->n_args=c;

  /*printf("%s, %u\n",ret->args, ret->n_args);*//*DEBUG*/

  for(i=i+1,a=0;(i<strlen(line)) && (line[i]!=':');i++,a++)
    {
      if(isalpha(line[i]) || (line[i]=='0' || line[i]=='1'))
	{
	  ret->op_desc[a]=line[i];
	}
    }
  ret->op_desc[a]='\0';
  ret->op_len=a;
  /*
  printf("-- %s %u:%s %u:%s %u\n",ret->instr_name,ret->instr_name_len,ret->args, ret->n_args,ret->op_desc,ret->op_len);
  */
  return 1;
}

int parseFile(FILE * f, const cpu_instr_set * set, const argument_list * arg_list, symbol_table * symb_list, const symbol_table * hsymb_table)
{
  char c;
  char lineBuffer[MAX_CNT_OF_LINE];
  unsigned int line_counter, i;
  unsigned int real_line_counter;
  instruction found_instr;
  PARSE_LINE_RET ret;
  assemble_ret as_ret;

  hsymb_table=hsymb_table;

  c=' ';
  line_counter=0;
  real_line_counter=0;
  while(c!=EOF)
    {
      c=fgetc(f);

      /*Remove comments*/
      if(c==';')
	{
	  while((c!='\n') && (c!='\r'))
	    {
	      c=fgetc(f);
	    }
	}

      /*Shorten trailing white-spaces*/
      /*But keep 1 space*/
      if(isblank(c))
	{
	  lineBuffer[line_counter]=' ';
	  line_counter++;
	  /*((c==' ') || (c=='\t')) && (c!='\n') && (c!='\r'))*/
	  while(isblank(c))
	    {
	      c=fgetc(f);
	    }
	}

      /*Remove comments in end of line*/
      if(c==';')
	{
	  while((c!='\n') && (c!='\r'))
	    {
	      c=fgetc(f);
	    }
	}

      if(1)
	{
	  lineBuffer[line_counter]=c;
	  line_counter++;
	  if((c=='\n'))
	    {
	      real_line_counter++;
	      lineBuffer[line_counter]='\0';
	      if(line_counter>1)
		{
		  ret=parseLine(lineBuffer,set,&found_instr, symb_list);
		  switch(ret)
		    {
		    case PARSE_LINE_RET_ERROR:
		      fprintf(stderr,"[parseLine() returned -1] @ line: %u \"%s\"\n",real_line_counter,remWhite(lineBuffer, strlen(lineBuffer)));
		      return -1;
		    case PARSE_LINE_RET_INSTRUCTION:
		      /*
			parse_arguments();
		       */
	      	      found_instr.is=ISINSTRUCTION;
		      as_ret = assemble(&found_instr, set, arg_list, symb_list, hsymb_table);
		      if(as_ret.is==DEFINED)
		      {
			printf("..OK, ");
			for(i=0;i<as_ret.num;i++)
			  {
			    printf("0x%lX(%u) ", as_ret.opcode[i], as_ret.size[i]);
			    size_counter+=as_ret.size[i]; /*16 should not be hardcoded here*/
			  }
			printf("\n");
			printf("current size: %u\n",size_counter);
		      } else printf("\n");

		      break;
		    case PARSE_LINE_RET_MACRO:
		      break;
		    case PARSE_LINE_RET_TAG:
		      break;
		    case PARSE_LINE_RET_NOTHING:
		      break;
		    default:
		      fprintf(stderr,"[parseLine() returned %u] @ line: %u\n",ret,real_line_counter);
		      return -1;
		    }
		}
	      line_counter=0;
	    }
	}
    }

  return 1;
}
