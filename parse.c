#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "parse.h"

int match_argument(const char * match, const argument * arg)
{
  unsigned int i,a;
  unsigned int strlen_match;
  unsigned int reg_ctr;
  char long_regex[1000];
  /*regex_t regex;*/

  strcpy(long_regex,"^[\\ ]*"); /*start*/
  reg_ctr=strlen(long_regex);
  reg_ctr=reg_ctr;

  strlen_match = strlen(match);

  /*match a character to characters in arg_string*/
  for(i=0;i<strlen_match;i++)
    {
      for(a=0;a<arg->arg_subargs_len;a++)
	{
	  if((arg->arg_subargs[a]!=' ') && (arg->arg_subargs[a]!=','))
	    {
	      if(arg->arg_subargs[a] == arg->arg_regex[i])
		{
		  printf("%c", match[i]);
		}
	    }
	}
    }

  return 0;
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
  unsigned int i,c,a;
  char temp[100];
  signed found_instr;

  temp[0]=temp[0];

  found_instr=0;

  for(i=0;i<strlen(line);i++)
    {

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

      if(!found_instr)
	{
	  for(c=0;c<set->num;c++)
	    {
	      if(!strncmp(&line[i],set->instr[c].instr_name,set->instr[c].instr_name_len))
		{
		  printf("found instr: %s - ",set->instr[c].instr_name);
		  found_instr=1;
		}
	    }
	}else /*Parse arguments*/
	{
	}
    }
  /*
  if(!strncmp(line," \n",2))
    {
  */
      printf("%s",line);
      /*
      return 1;
    } printf("\n");
      */
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
