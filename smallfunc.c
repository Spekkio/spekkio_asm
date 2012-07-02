#include <inttypes.h>
#include <stdio.h>
#include <regex.h>
#include <string.h>
#include <ctype.h>
#include "parse.h"
#include "smallfunc.h"

/*
A place for short simple functions, counting items in strings,
converting strings to numbers.. etc.
*/



/*
  Example, If you want to check if a string is only
  made up of "0" and "1" like this "0101010101"
  you call string_contains("0101010101","10",strlen("0101010101"));
 */
/*return 1 if all match, else zero if fail*/
int string_contains(const char * str, const char * chars, const unsigned int size)
{
  unsigned int i,a;
  unsigned ret;
  ret=0;
  for(i=0;(i<size) && (str[i]!='\0');i++)
    {      
      for(a=0;chars[a]!='\0';a++)
	{
	  ret |= (str[i]==chars[a]);
	}
      if(!ret)
	{
	  return 0;
	}
    }
  return 1;
}

/*Convert a string of 1's and 0's to a number, max length is 64 bits*/
uint64_t binstr_to_uint64(const char * str, const unsigned int size)
{
  uint64_t temp;
  unsigned int i;

  temp=0;
  /*
    string is encodeded, MSB first, LSB last.
   */
  for(i=0;(i<size) && (i<64);i++)
    {
      if(str[i]=='1')
	{
	  temp |= 1<<(size-i-1);
	}
    }
  return temp;
}

/*count n of arguments int a string "a,b,c,d"=4*/
unsigned int count_args(const char * op_args, const unsigned int strlen_op_args)
{
  unsigned int i;
  unsigned int arg_c;
  /*Parse op_args, and count number of args*/

  for(i=0,arg_c=0;i<strlen_op_args;i++)
    {
      if(op_args[i]!=',')
	{
	  if(op_args[i]!=' ')
	    {
	      /*this was used in encode_opcode to make the string "abcd" out of "a,b,c,d"*/
	      /*op_list[arg_c].arg_t = op_args[i];*/
	      arg_c++;
	    }
	}
      else
	{

	}
    }
  return arg_c;
}

ARG_TYPE isHex(const char * str, const unsigned int len)
{
  ARG_TYPE is;
  unsigned int sc;
  if(!strncmp(str, "0x", 2))
    {
      /*This has potential to be a hex number*/
      is=ISHEX;
      for(sc=2;(sc<len) && (is==ISHEX);sc++)
	{
	  if(isxdigit(str[sc]))
	    {
	      /*It is still hex*/
	      is=ISHEX;
	    }else
	    {
	      /*It is not hex*/
	      is=ISUNDEFINED;
	    }
	}
    } else
    {
      is=ISUNDEFINED;
    }
  return is;
}

ARG_TYPE isDigit(const char * str, const unsigned int len)
{
  ARG_TYPE is;
  unsigned int sc;
  is=ISNUMBER;
  for(sc=0;(sc<len) && (is==ISNUMBER);sc++)
    {
      if(isdigit(str[sc]))
	{
	  /*It is still a number*/
	  is=ISNUMBER;
	}else
	{
	  /*It is not a number*/
	  is=ISUNDEFINED;
	}
    }
  return is;
}

ARG_TYPE isNumberType(const char * str, const unsigned int len)
{
  ARG_TYPE is;
  is=isHex(str, len);

  if(is==ISUNDEFINED)
    {
      is=isDigit(str, len);
    }

  return is;
}

/*returns number of 'word_size' the number 'a' fits in*/
uint64_t bitSize(uint64_t a)
{
  const uint64_t size = sizeof(uint64_t)*8;
  uint64_t i;
  
  for(i=0;(i<size) && ((a & (((uint64_t)1)<<((size-1)-i)))==0) ;i++);
  
  i=size-i;

  return i;
}

uint64_t nWords(const uint64_t bitsize, const uint64_t wordsize)
{
  if((bitsize % wordsize) == 0)
    {
      return (bitsize/wordsize)*16;
    }

  return (bitsize / wordsize) * wordsize + wordsize;
}

char * remWhite(char * s, const unsigned int len)
{
  unsigned int i,a;
  signed foundother=0;
  for(i=0;i<len;i++)
    {
      if( (isspace(s[i]) || s[i]==' ') && foundother==0)
	{
	  for(a=i;a<len;a++)
	    {
	      s[a]=s[a+1];
	    }
	} else foundother=1;
      if(isspace(s[i]) && foundother==1)
	{
	  s[i]=' ';
	}
    }
  foundother=0;
  for(i=len-1;i>0;i--)
    {
      if( (isspace(s[i]) || s[i]==' ') && foundother==0)
	{
	  s[i]=0;
	} else foundother=1;
    }
  
  return s;
}

/*copies the string from str to store until it finds the character split*/
/*if the character split is within a [], () or {} it ignores that one*/
/*if there is a string like this "abc,def,xyz" and we want the string "def"*/
/*specify split=',' and nth=1 for the second string split by the character ','*/
void splitString(char * store, const char * str, const unsigned int len, const char split, const unsigned int nth)
{
  unsigned int i,a,b;
  i=0;
  for(b=0;b<(nth+1);b++)
    {
      a=0;
      for(i=i;(i<len) && (str[i]!=split);i++)
	{
	  if(str[i]=='[')
	    {
	      for(i=i;(i<len) && (str[i]!=']');i++)
		{
		  store[a++]=str[i];
		}
	    }
	  if(str[i]=='(')
	    {
	      for(i=i;(i<len) && (str[i]!=')');i++)
		{
		  store[a++]=str[i];
		}
	    }
	  if(str[i]=='{')
	    {
	      for(i=i;(i<len) && (str[i]!='}');i++)
		{
		  store[a++]=str[i];
		}
	    }
	  store[a++]=str[i];
	}
      i++;
      store[a++]='\0';
    }
}
