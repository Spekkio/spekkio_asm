#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>
#include "parse.h"
#include "encode.h"
#include "setup_regex.h"
#include "smallfunc.h"

#ifndef MAX_LONG_REGEX
#define MAX_LONG_REGEX 1000
#endif

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
		  memcpy(tempstring, &match[pmatch[arg_number+1].rm_so], pmatch[arg_number+1].rm_eo-pmatch[arg_number+1].rm_so);
		  tempstring[(pmatch[arg_number+1].rm_eo-pmatch[arg_number+1].rm_so)]='\0';
		  strncpy(result, tempstring, max_result_len);
		  success=1;
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

void loadCPUFile(const char * filename, cpu_instr_set * set, argument_list * arg_list, symbol_table * sym_table)
{
  FILE *f;
  char c;
  char lineBuffer[1000];
  unsigned int line_counter=0;
  PARSE_TYPE type;
  argument arg;
  cpu_instr instr;
  symbol symb;

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

PARSE_LINE_RET parseLine(const char * line, const cpu_instr_set * set, instruction * store/*, macro * store*/)
{
  PARSE_LINE_RET return_value;
  unsigned int i,c,a,cnt_args,the_instr;
  char temp[MAX_ARG_PARSED_LEN+MAX_NAME_LEN]; /*stores names, args, and macros*/
  signed found_instr;
  char case_line[MAX_CNT_OF_LINE];
  signed whitespace_clear_flag, whitespace_clear_flag_delay;

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
	  printf("found tag: %s\n",temp);

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
	      printf("%s (Use addSymbol())",temp);

	      /*found_macro=1;*/ /*we can use parse arguments further down, and define args for macros somewhere.*/
	      return_value = PARSE_LINE_RET_MACRO;

	    } else
	    {
	      fprintf(stderr,"Unknown macro \"%s\" ",temp);
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
	}
      /*printf(" == %s",line);*/
    }

  return return_value;
}


int parseARGLine(const char * line, argument * ret)
{
  unsigned int i,c,a,d,quit;
  long int result;
  char tempstring[1000];
  /*
  char regex1_result[20];
  char regex2_result[20];

  const char regex1[] = "^\\[\\{1,1\\}\\([su]\\{1,1\\}\\)|\\{1,1\\}\\([-]\\{0,1\\}[0-9A-Fa-f]\\{1,20}\\)..\\([0-9xA-Fa-f]\\)\\]\\{1,1\\}$";
  const char regex2[] = "^\\[\\{1,1\\}\\([su]\\{1,1\\}\\)|\\{1,1\\}\\([-]\\{0,1\\}[0-9A-Fa-f]\\{1,20}\\)\\]\\{1,1\\}$";
  regex_t range1, single;

  regcomp(&range1, regex1, 0);
  regcomp(&single, regex2, 0);
  */
  
  const char regex_test[] = "^[\\ ]*\\([a-zA-Z\\[\\],+-]*\\)[\\ ]*[:]\\{1\\}$";
  regex_t range1;
  if(!regcomp(&range1, regex_test, 0))
    {
      regfree(&range1);
    }
  else
    {
      printf("regex compilation error\n");
    }


  if(line[0]=='-')
    return 0;

   /*Read the argument regex code*/
  for(i=0,a=0;(i<strlen(line)) && (line[i]!=':');i++)
    {
      ret->arg_regex[a]='\0';
      if(!isblank(line[i]))
	{
	  ret->arg_regex[a]=line[i];
	  a++;
	}
    }
  ret->arg_regex[a]='\0';
  ret->arg_regex_len=a;

  /*printf("%s, %u\n",ret->arg_regex, a);*/ /*DEBUG*/

  /*Read and count the argument list*/
  for(i=i+1,c=0,a=0,d=0;(i<strlen(line)) && (line[i]!=':');i++,a++)
    {
      if((line[i]>='a' && line[i]<='z') || (line[i]>='A' && line[i]<='Z'))
	{
	  ret->arg_subargs[a]=line[i];
	  c++;
	  d++;
	}else
      if(line[i]==',')
	{
	  ret->arg_subargs[a]=line[i];
	  d++;
	}
    }
  ret->arg_subargs[d]='\0';
  ret->arg_subargs_len=d;
  ret->n_args=c;

  for(i=i+1,a=0;(i<strlen(line)) && (line[i]!=':');i++,a++)
    {
      ret->arg_desc[i]='\0';
      if(isalpha(line[i]) || (line[i]=='0' || line[i]=='1'))
	{
	  ret->arg_desc[a]=line[i];
	}
    }
  ret->arg_desc[i]='\0';
  ret->arg_desc_len=a;

  for(i=i+1,a=0;(i<strlen(line)) && (line[i]!=':');i++,a++)
    {
      if(isalpha(line[i]) || (line[i]=='0' || line[i]=='1'))
	{
	  ret->arg_overflow[a]=line[i];
	}
    }
  ret->arg_overflow[a]='\0';
  ret->arg_overflow_len=a;


  quit=0;
  for(i=i+1,a=0;(i<strlen(line)) && (quit==0);i++)
    {
      if(isdigit(line[i]) || line[i]=='+' || line[i]=='-')
	{
	  tempstring[a]=line[i];
	  a++;
	} else if(line[i]==',' || line[i]==':')
	{
	  if(line[i]==':')
	    {
	      quit=1;
	    }
	  tempstring[a]='\0';
	  a=0;
	  if(sscanf(tempstring,"%li",&result)==1)
	    {
	      ret->shift = result;
	      /*Potential problem here*/
	    } else return 0; /*Make error here*/
	}
    }


  /*printf("%s, %u\n",ret->arg_subargs, ret->n_args);*/ /*DEBUG*/

  return 1;
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

int parseFile(FILE * f, const cpu_instr_set * set, const argument_list * arg_list, const symbol_table * symb_list)
{
  char c;
  char lineBuffer[MAX_CNT_OF_LINE];
  unsigned int line_counter;
  unsigned int real_line_counter;
  instruction found_instr;
  PARSE_LINE_RET ret;
  assemble_ret as_ret;

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
		  ret=parseLine(lineBuffer,set,&found_instr);
		  switch(ret)
		    {
		    case PARSE_LINE_RET_ERROR:
		      fprintf(stderr,"[parseLine() returned -1] @ line: %u\n",real_line_counter);
		      return -1;
		    case PARSE_LINE_RET_INSTRUCTION:

		      /*
			parse_arguments();
		       */
		      as_ret = assemble(&found_instr, set, arg_list, symb_list);
		      if(as_ret.is==DEFINED)
		      {
			printf("..OK, code=0x%lX, size=%u\n", as_ret.opcode, as_ret.size);
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

/*This function gets all the strings of one instruction*/
/*parsed into different variables and pointers*/
/*String of the instruction = set->instr[found_instr->instr_index].instr_name*/
/*String of each argument[x] = found_instr->arg[x].arg*/
/*Number of arguments that was found = found_instr->n_args*/
/*Number of arguments instruction should have = set->instr[found_instr->instr_index].n_args*/

/*This functon assemble all the values/opcodes for each instruction */
/*and save the unknown symbols that doesn't have a value for later.*/
/*This function can be called, assemble 1st pass*/
assemble_ret assemble(instruction * found_instr, const cpu_instr_set * set, const argument_list * arg_list, const symbol_table * symb_list)
{
  uint64_t p_buf;
  unsigned int i, sc, ret_sym;
  ARG_TYPE is;
  assemble_ret ret;

  ret.is=UNDEFINED;
  ret.opcode=0;
  ret.size=0;

  set=set;
  arg_list=arg_list;
  symb_list=symb_list;
  /*These two must be the same, I think they are checked before*/
  /*I'm starting to loose track...*/
  if(found_instr->n_args == set->instr[found_instr->instr_index].n_args)
    {
      printf("%s ",set->instr[found_instr->instr_index].instr_name);

      /*loop through the arguments*/
      /*to be able to parse subarguments, this can be made into a function*/
      /*that can be used recursive.*/
      for(i=0;i<found_instr->n_args;i++)
	{
	  is=ISUNDEFINED;
	  /*Check if argument is an encoded number, hex or decimal*/

	  
	  /*This can be a function in smallfunc.c*/
	  if(!strncmp(found_instr->arg[i].arg, "0x", 2))
	    {
	      /*This has potential to be a hex number*/
	      is=ISHEX;
	      for(sc=2;(sc<found_instr->arg[i].arg_len) && (is==ISHEX);sc++)
		{
		  if(isxdigit(found_instr->arg[i].arg[sc]))
		    {
		      /*It is still hex*/
		      is=ISHEX;
		    }else
		    {
		      /*It is not hex*/
		      is=ISUNDEFINED;
		    }
		}
	    }

	  /*Test if normal number*/
	  if(is==ISUNDEFINED)
	    {
	      is=ISNUMBER;
	      for(sc=0;(sc<found_instr->arg[i].arg_len) && (is==ISNUMBER);sc++)
		{
		  if(isdigit(found_instr->arg[i].arg[sc]))
		    {
		      /*It is still a number*/
		      is=ISNUMBER;
		    }else
		    {
		      /*It is not a number*/
		      is=ISUNDEFINED;
		    }
		}
	    }

	  /*Search the symbol list*/
	  if(is==ISUNDEFINED)
	    {
	      if(!match_symbol(&ret_sym, found_instr->arg[i].arg, symb_list, MAX_ARG_PARSED_LEN))
		{
		  is=ISSYMBOL;
		}
	    }

	  printf("%s",found_instr->arg[i].arg);
	  found_instr->arg[i].is=UNDEFINED;
	  p_buf = 0;
	  switch(is)
	    {
	    case ISHEX:
	      printf("=HEX");

	      if(sscanf(found_instr->arg[i].arg,"0x%lX",&p_buf)==1)
		{
		  found_instr->arg[i].value = p_buf;
		  found_instr->arg[i].is=DEFINED;
		}
	      else if(sscanf(found_instr->arg[i].arg,"0x%lx",&p_buf)==1)
		{
		  found_instr->arg[i].value = p_buf;
		  found_instr->arg[i].is=DEFINED;
		}
	      break;

	    case ISNUMBER:
	      printf("=NUMBER");

	      if(sscanf(found_instr->arg[i].arg,"%lu",&p_buf)==1)
		{
		  found_instr->arg[i].value = p_buf;
		  found_instr->arg[i].is=DEFINED;
		}

	      break;

	    case ISSYMBOL:
	      printf("=SYMBOL");
	      if(symb_list->table[ret_sym].is==DEFINED)
		{
		  found_instr->arg[i].value = symb_list->table[ret_sym].value;
		  found_instr->arg[i].is=DEFINED;
		}
	      break;

	    default:
	      printf("=UNDEFSYMBOL");
	      break;
	    }

	  if(found_instr->arg[i].is==DEFINED)
	    {
	      printf("(0x%lX), ", found_instr->arg[i].value);
	    }else
	    {
	      printf(", ");
	    }

	}/*Loop through argumets hex.*/

      /*loop again to see if we got an undef*/
      ret.is=DEFINED;
      for(i=0;i<found_instr->n_args;i++)
	{
	  if(found_instr->arg[i].is==UNDEFINED)
	    {
	      ret.is=UNDEFINED;
	    }
	}

      /*If it's defined*/
      if(ret.is==DEFINED)
	{
	  /*put in the argument values and get the opcode.*/
	  ret.opcode = encode_opcode_n(found_instr, set->instr[found_instr->instr_index].args, set->instr[found_instr->instr_index].op_desc);
	}
    }

  return ret;
}

/*This code is becoming really hard to follow*/
void parse_line_ret_instr_(const instruction * found_instr, const cpu_instr_set * set, const argument_list * arg_list, const symbol_table * symb_list)
{
  int match_ret,isdigit_,ishexa_, ismatch_=0, issymbol=0;
  unsigned int inner_ctr, inner_ctr2, i, ret_index;
  char result[MAX_ARG_PARSED_LEN];

  printf("%s ",set->instr[found_instr->instr_index].instr_name);
  for(inner_ctr=0;inner_ctr<found_instr->n_args;inner_ctr++)
    {
      printf("%s",found_instr->arg[inner_ctr].arg);
      inner_ctr2=0;
      match_ret=1;
	/*printf("Argument %u: ",inner_ctr+1);*/
	while((match_ret!=0) && (inner_ctr2<arg_list->num))
	  {
	    isdigit_=1;
	    ismatch_=0;
	    if(!strncmp(found_instr->arg[inner_ctr].arg, "0x", 2))
	      {
		ishexa_=1;
	      }else ishexa_=0;
	    for(i=0;i<found_instr->arg[inner_ctr].arg_len;i++)
	      {
		if(!isdigit(found_instr->arg[inner_ctr].arg[0]))
		  {
		    isdigit_=0;
		  }
		if((isxdigit(found_instr->arg[inner_ctr].arg[0]))==0)
		{
		  ishexa_=0;
		}
	      }

	    if(isdigit_ || ishexa_)
	      {
		if(isdigit_)
		  {
		    /*printf("  It's a digit: %s\n",found_instr->arg[inner_ctr].arg);*/
		    match_ret=0;
		  }
		else if(ishexa_)
		  {
		    /*printf("  It's hexadec: %s\n",found_instr->arg[inner_ctr].arg);*/
		    match_ret=0;
		  }
	      }
	    else
	      {
		if(match_symbol(&ret_index, found_instr->arg[inner_ctr].arg, symb_list, found_instr->arg[inner_ctr].arg_len))
		  {
		    printf(", %s",symb_list->table[ret_index].string);
		    issymbol|=1;
		    ismatch_=0;
		  }else
		  {
		    result[0]='\0';
		    match_ret=0;
		    match_ret=match_argument(result,MAX_ARG_PARSED_LEN,found_instr->arg[inner_ctr].arg,&arg_list->arg[inner_ctr2],inner_ctr);
		    if(!match_ret)
		      {
			ismatch_|=1;
		      }
		  }
	      }
	    inner_ctr2++;
	  }
      if(ismatch_==1)
	{
	  inner_ctr2--;
			      /*
			      printf(" [matched \"%s\" with \"%s\" ret=%u, value=%s]\n",found_instr->arg[inner_ctr].arg,arg_list->arg[inner_ctr2].arg_regex,match_ret, arg_list->arg[inner_ctr2].arg_desc);
			      */
	  printf("=%s[+%s], ",arg_list->arg[inner_ctr2].arg_desc,arg_list->arg[inner_ctr2].arg_overflow);
	}
      if(match_ret!=0)
	{
	  printf(" match: %s = %s, ",result,arg_list->arg[inner_ctr2].arg_desc);
	}
    }
  printf("\n");
}
