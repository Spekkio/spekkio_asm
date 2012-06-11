#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "parse.h"

void loadCPUFile(const char * filename, cpu_instr_set * set)
{
  FILE *f;
  char c;
  char lineBuffer[1000];
  unsigned int line_counter=0;
  PARSE_TYPE type;
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
			      addInstruction(&instr, set);
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

void addInstruction(const cpu_instr * instr, cpu_instr_set * set)
{
  set->instr[set->num] = *instr;
  set->num++;
}

void parseLine(const char * line, cpu_instr_set * set)
{
  unsigned int i,c,a;
  char temp[100];

  temp[0]=temp[0];

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

      for(c=0;c<set->num;c++)
	{
	  if(!strncmp(&line[i],set->instr[c].instr_name,set->instr[c].instr_name_len))
	    {
	      printf("found instr: %s - ",set->instr[c].instr_name);
	    }
	}
    }
  printf("%s",line);
}

int parseCPULine(const char * line, cpu_instr * ret)
{
  unsigned int i,c,a;

  ret->instr_name_len=0;
  ret->n_args=0;
  ret->op_len=0;

  if(line[0]=='-')
    return 0;

   /*Read the instruction name*/
  for(i=0;(i<strlen(line)) && (line[i]!=':');i++)
    {
      if(line[i]!=' ')
	{
	  ret->instr_name[i]=line[i];
	}
    }
  ret->instr_name[i]='\0';
  ret->instr_name_len=i;
  /*Read and count the argument list*/
  for(i=i+1,c=0,a=0;(i<strlen(line)) && (line[i]!=':');i++,a++)
    {
      if((line[i]>='a' && line[i]<='z') || (line[i]>='A' && line[i]<='Z'))
	{
	  ret->args[a]=line[i];
	  c++;
	}else
      if(line[i]==',')
	{
	  ret->args[a]=line[i];
	}
    }
  ret->args[i]='\0';
  ret->n_args=c;

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
		  parseLine(lineBuffer,set);
		}
	      line_counter=0;
	    }
	}
    }
}
