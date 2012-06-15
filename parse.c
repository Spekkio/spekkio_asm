#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>
#include "parse.h"
#include "setup_regex.h"

#ifndef MAX_LONG_REGEX
#define MAX_LONG_REGEX 1000
#endif

/*returns 0 if success, -1 if memory error or other error, 1 if no match*/
int match_argument(char * result, const int max_result_len, const char * match, const argument * arg, const unsigned int arg_number)
{
  char tempstring[MAX_ARG_PARSED_LEN];
  signed success;  
  size_t nmatch;
  regmatch_t * pmatch;

  success = 0;

  nmatch = arg->n_args+1;
  pmatch = malloc(sizeof(regmatch_t)*nmatch);

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

void loadCPUFile(const char * filename, cpu_instr_set * set, argument_list * arg_list)
{
  FILE *f;
  char c;
  char lineBuffer[1000];
  unsigned int line_counter=0;
  PARSE_TYPE type;
  argument arg;
  cpu_instr instr;

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
  unsigned int i,c,a,d;
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

  for(i=i+1,a=0;(i<strlen(line)) && (line[i]!=':') && (line[i]!='*');i++)
    {
      if(isxdigit(line[i]) || (line[i]=='[' || line[i]=='u' || line[i]=='s' ||
              line[i]=='.' || line[i]=='x' || line[i]=='|' || line[i]=='-'))
	{
	  tempstring[a]=line[i];
	  a++;
	}else
      if(line[i]==']')
	{
	  /*parse it*/
	  /* "^\\[\\([0-9x]\\{\\}\\)\\]" */
	  tempstring[a]=line[i];
	  tempstring[a+1]='\0';
	  a=0;
	  printf("\nRange: %s\n",tempstring);
	}
    }


  /*printf("%s, %u\n",ret->arg_subargs, ret->n_args);*/ /*DEBUG*/

  return 1;
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
  ret->op_desc[i]='\0';
  ret->op_len=a;
  /*
  printf("-- %s %u:%s %u:%s %u\n",ret->instr_name,ret->instr_name_len,ret->args, ret->n_args,ret->op_desc,ret->op_len);
  */
  return 1;
}

int parseFile(FILE * f, const cpu_instr_set * set, const argument_list * arg_list)
{
  char c;
  char lineBuffer[MAX_CNT_OF_LINE];
  unsigned int line_counter;
  unsigned int real_line_counter;
  instruction found_instr;
  PARSE_LINE_RET ret;

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
		      parse_line_ret_instr(&found_instr, set, arg_list);

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


void parse_line_ret_instr(const instruction * found_instr, const cpu_instr_set * set, const argument_list * arg_list)
{
  int match_ret,isdigit_,ishexa_, ismatch_=0;
  unsigned int inner_ctr, inner_ctr2, i;
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
		match_ret=match_argument(result,MAX_ARG_PARSED_LEN,found_instr->arg[inner_ctr].arg,&arg_list->arg[inner_ctr2],inner_ctr);
		ismatch_=1;
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
