#include <inttypes.h>

/*
A place for short simple functions, counting items in strings,
converting strings to numbers.. etc.
*/



/*
  Example, If you want to check if a string is only
  make up of 0 and 1 like this "0101010101"
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