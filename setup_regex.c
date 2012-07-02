#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>
#include <inttypes.h>
#include "parse.h"
#include "setup_regex.h"

/*used to parse the .set file, symbols part*/
const char symbol_regstr[] = "^[\\ ]*\\([a-zA-Z]\\{1,100\\}\\)[\\ ]*:\\{1\\}[\\ ]*\\([01]\\{1,64\\}\\)[\\ ]*:\\{1\\}[\\ ]*\\([a-zA-Z]*\\)[\\ ]*:\\{1\\}.*$";
regex_t symbol_list;
regmatch_t sym_pmatch[4]; /*increase with ':'*/
const size_t sym_nmatch = 4;

/*used to parse the .set file, instruction part*/
const char instr_regstr[] = "^[\\ ]*\\([a-zA-Z]\\{1,100\\}\\)[\\ ]*:\\{1\\}[\\ ]*\\([01]\\{1,64\\}\\)[\\ ]*:\\{1\\}[\\ ]*\\([a-zA-Z]*\\)[\\ ]*:\\{1\\}.*$";
regex_t instr_list;
regmatch_t instr_pmatch[4]; /*increase with ':'*/
const size_t instr_nmatch;

/*used to parse the .set file, argument part*/
const char arg_regstr[] = 
"^\
[\\ ]*\\([a-zA-Z[:punct:]]\\{1,100\\}\\)[\\ ]*:\\{1\\}\
[\\ ]*\\([a-zA-Z,-]*\\)[\\ ]*:\\{1\\}\
[\\ ]*\\([a-zA-Z01-]\\{1,64\\}\\)[\\ ]*:\\{1\\}\
[\\ ]*\\([a-zA-Z01-]*\\)[\\ ]*:\\{1\\}\
[\\ ]*\\([0-9+,-]*\\)[\\ ]*:\\{1\\}\
[\\ ]*\\([0-9a-zA-Z,[:punct:]]*\\)[\\ ]*:\\{1\\}\
[\\ ]*\\(All\\)[\\ ]*:\\{1\\}\
.*$";
regex_t arg_list;
regmatch_t arg_pmatch[7+1]; /*increase with ':'*/
const size_t arg_nmatch=7+1;

/*Return 0 if success, return 1 if error.*/
int setup_global_regex(void)
{
  int ret;
  char errbuf[100];

  ret = regcomp(&symbol_list, symbol_regstr, 0);
  if(ret!=0)
    {
      regerror(ret, &symbol_list, errbuf, 100);
      fprintf(stderr,"Error in Symbol Regex: %s\n",errbuf);
    }

  ret = regcomp(&arg_list, arg_regstr, 0);
  if(ret!=0)
    {
      regerror(ret, &arg_list, errbuf, 100);
      fprintf(stderr,"Error in Argument Regex: %s\n",errbuf);
    }

  ret = regcomp(&instr_list, instr_regstr, 0);
  if(ret!=0)
    {
      regerror(ret, &instr_list, errbuf, 100);
      fprintf(stderr,"Error in instruction Regex: %s\n",errbuf);
    }

  return 1;
}

void free_global_regex(void)
{
  regfree(&symbol_list);
  regfree(&instr_list);
  regfree(&arg_list);
}

/*create a regex POSIX line used by regexec(), from a simple regex line in the */
/*.set file, that looks like this "[a+b]" combined with the argument list "a,b"*/
/*it takes an argument struct and stores the compiled regex_t in the argument struct.*/

/*returns 0 if success, 1 if memory error or other error*/
int make_arg_regex(argument * arg)
{
  char tempstr[MAX_SUBARG_REGEX_LEN] = " ";
  char * long_regex;
  size_t nmatch;
  regmatch_t * pmatch;
  unsigned int i,a,reg_ctr,subarg_index;
  signed found,success;

  const char basic[]="\\(%s\\)[\\ ]*";
  const char variable[]="\\([0-9A-Za-z]\\{1,100\\}\\)[\\ ]*";
  const unsigned int strlen_var = strlen(variable);
  const char character[]="%c\\{1\\}[\\ ]*";
  /*const unsigned int strlen_char = strlen(character);*/
  const char e_character[]="\\%c\\{1\\}[\\ ]*";
  const unsigned int strlen_e_char = strlen(character);
  const unsigned int long_regex_len = arg->arg_regex_len*(strlen_var+strlen_e_char);

  reg_ctr=0;
  success=0;
  subarg_index=0;

  nmatch = arg->n_args+1;
  pmatch = malloc(sizeof(regmatch_t)*nmatch);
  long_regex = malloc(long_regex_len);

  if((pmatch!=0) && (long_regex!=0))
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
		      if(arg->sub_arg[subarg_index].subarg_regex[0]!='\0')
			{
			  snprintf(tempstr, MAX_SUBARG_REGEX_LEN, basic, arg->sub_arg[subarg_index].subarg_regex);
			  reg_ctr+=snprintf(&long_regex[reg_ctr],long_regex_len,tempstr,(const char *)"");
			}
		      else
			{
			  reg_ctr+=snprintf(&long_regex[reg_ctr],long_regex_len,variable);
			}
		      subarg_index++;
		      found=1;
		    }
		}
	    }
	  if(!found)
	    {
	      if((arg->arg_regex[i]=='[') || arg->arg_regex[i]==']') /*these need to be escaped*/
		{
		  reg_ctr+=snprintf(&long_regex[reg_ctr],long_regex_len,e_character,arg->arg_regex[i]);
		} else
		{
		  reg_ctr+=snprintf(&long_regex[reg_ctr],long_regex_len,character,arg->arg_regex[i]);
		}
	    }
	}
      reg_ctr+=snprintf(&long_regex[reg_ctr],long_regex_len,"$");
      long_regex[reg_ctr]='\0';

      /*Compile the regex and store in arg struct.*/
      if(!regcomp(&arg->reg, long_regex, 0))
	{
	  success=1;
	}
    } else success=0;

  free(pmatch);
  free(long_regex);

  if(success)
    {
      return 0;
    }
  else
    {
      return 1;
    }
}

/*returns 0 if success, 1 if memory error or other error*/
int make_all_arg_regex(argument_list * arg)
{
  unsigned int i;

  for(i=0;i<arg->num;i++)
    {
      if(make_arg_regex(&arg->arg[i]))
	{
	  return 1;
	}
    }
  return 0;
}

void free_all_regex(argument_list * arg)
{
  unsigned int i;
  for(i=0;i<arg->num;i++)
    {
      regfree(&arg->arg[i].reg);
    }
}
