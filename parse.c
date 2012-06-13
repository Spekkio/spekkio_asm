#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "parse.h"

#ifndef MAX_LONG_REGEX
#define MAX_LONG_REGEX 1000
#endif

int match_argument(char * result, const int max_result_len, const char * match, const argument * arg, const unsigned int arg_number)
{
  unsigned int i,a;
  /*unsigned int strlen_match;*/
  unsigned int reg_ctr;
  char long_regex[MAX_LONG_REGEX];
  char tempstring[MAX_ARG_PARSED_LEN];
  signed found, success;  
  size_t nmatch;
  regmatch_t * pmatch;
  regex_t regex;

  success = 0;

  /*This is how to parse out the patterns*/
  /*size_t nmatch = 3; //number of arguments+1*/
  /*regmatch_t pmatch[3]; //same here*/
  /*char patt1[] = "[n+a]" // with args n,a*/
  /*Shall transform into*/
  /*char patt2[] = "^[\\ ]*\\[\\{1\\}[\\ ]*\\([0-9x]\\{1,1000\\}\\)[\\ ]*+\\{1\\}[\\ ]*\\([0-9A-Za-z]\\{1,1000\\}\\)[\\ ]*\\]\\{1\\}[\\ ]*$";*/
  /**/

  nmatch = arg->n_args+1;
  pmatch = malloc(sizeof(regmatch_t)*nmatch);

  if(pmatch)
    {
      strcpy(long_regex,"^[\\ ]*"); /*start*/
      reg_ctr=strlen(long_regex);

      /*printf("matching: \"%s\" to \"%s\"\n",arg->arg_subargs, arg->arg_regex);*/

      /*strlen_match = strlen(match);*/
      /*Transform simple regex to POSIX regex*/
      /*match a character to characters in arg_string*/
      for(i=0;i<arg->arg_regex_len;i++)
	{
	  for(a=0,found=0; (a<arg->arg_subargs_len) && (found==0);a++)
	    {
	      if((arg->arg_subargs[a]!=' ') && (arg->arg_subargs[a]!=','))
		{
		  if(arg->arg_subargs[a] == arg->arg_regex[i])
		    {
		      reg_ctr+=snprintf(&long_regex[reg_ctr],MAX_LONG_REGEX,"\\([0-9A-Za-z]\\{1,1000\\}\\)[\\ ]*");
		      found=1;
		    }
		}
	    }
	  if(!found)
	    {
	      if((arg->arg_regex[i]=='[') || arg->arg_regex[i]==']') /*these need to be escaped*/
		{
		  reg_ctr+=snprintf(&long_regex[reg_ctr],MAX_LONG_REGEX,"\\%c\\{1\\}[\\ ]*",arg->arg_regex[i]);
		} else
		{
		  reg_ctr+=snprintf(&long_regex[reg_ctr],MAX_LONG_REGEX,"%c\\{1\\}[\\ ]*",arg->arg_regex[i]);
		}
	    }
	}
      reg_ctr+=sprintf(&long_regex[reg_ctr],"$");


      if(!regcomp(&regex, long_regex, 0))
	{
	  if(regexec(&regex, match, nmatch, pmatch, 0))
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
	  regfree(&regex);
	  free(pmatch);
	  return -1;
	}

      regfree(&regex);
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

int parseLine(const char * line, cpu_instr_set * set)
{
  unsigned int i,c,a,cnt_args,the_instr;
  char temp[100];
  signed found_instr;

  temp[0]=temp[0];

  found_instr=0;
  cnt_args=0;

  for(i=0;(i<strlen(line)) && (line[i]!='\n');i++)
    {

      /*printf("[%u/%c]",i,line[i]);*/
      if(line[i]==':')
	{
	  a=0;
	  for(i=i+1,a=0;((i<strlen(line)) && (line[i]!=' '));i++,a++)
	    {
	      temp[a]=line[i];
	    }
	  temp[a]='\0';
	  printf("found tag: %s\n",temp);
	}

      if(found_instr==0)
	{
	  for(c=0;c<set->num;c++)
	    {
	      if(!strncmp(&line[i],set->instr[c].instr_name,set->instr[c].instr_name_len))
		{
		  printf("found instr: %s ",set->instr[c].instr_name);
		  found_instr=1;
		  the_instr=c;
		  i+=set->instr[c].instr_name_len;

		  if(i>strlen(line) || line[i]=='\n')
		    {
		      printf("%s",line);
		      return 1;
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
	      for(a=0;i<strlen(line) && (line[i]!=',') && (line[i]!='\n');i++)
		{
		  if(line[i]!=' ')
		    {
		      temp[a]=line[i];
		      a++;
		    }
		}
	      temp[a]='\0';
	      if(line[i]==',' || line[i]=='\n')
		{
		  printf("%s,",temp);
		  cnt_args++;
		}
	    }
	}
    }

  if( (strlen(line)<=1) && (strncmp(line," \n",2)==0))
    {
    }
  else
    {
      printf(" == %s",line);
    }

  return found_instr;
}


int parseARGLine(const char * line, argument * ret)
{
  unsigned int i,c,a,d;

  if(line[0]=='-')
    return 0;

   /*Read the argument regex code*/
  for(i=0,a=0;(i<strlen(line)) && (line[i]!=':');i++)
    {
      if(line[i]!=' ')
	{
	  ret->arg_regex[i]=line[i];
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
      if(line[i]!=' ')
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
      if((line[i]>='a' && line[i]<='z') || (line[i]>='A' && line[i]<='Z'))
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
      if((line[i]>='a' && line[i]<='z') || (line[i]>='A' && line[i]<='Z') || (line[i]>='0' && line[i]<='1'))
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

void parseFile(FILE * f, cpu_instr_set * set)
{
  char c;
  char lineBuffer[1000];
  unsigned int line_counter;
  c=' ';
  line_counter=0;
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
		  if(!parseLine(lineBuffer,set))
		    {
		      printf("---Found nothing...\n");
		    }
		}
	      line_counter=0;
	    }
	}
    }
}
