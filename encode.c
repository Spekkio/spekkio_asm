#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>
#include <regex.h>
#include "parse.h"
#include "encode.h"

void get_const_mask_bits(const char * op_desc, unmask * unmask)
{
  unsigned int i;
  unsigned int strlen_op_desc;

  strlen_op_desc=strlen(op_desc);

  unmask->value = 0;
  unmask->mask = 0;
  unmask->arg_len = 0;
  unmask->arg_shift = 0;

  for(i=0;i<strlen_op_desc;i++)
    {
      if((op_desc[i]=='1') || (op_desc[i]=='0'))
	{
	  if(op_desc[i]=='1')
	    {
	      unmask->value |= 1<<(strlen_op_desc-i-1);
	    }
	  unmask->mask |= 1<<(strlen_op_desc-i-1);
	}
    }

  printf("value=0x%lX  ",unmask->value);
  printf("mask=0x%lX\n",unmask->mask);

}

void match_maskstring_to_args(const char * op_args, const char * op_desc)
{
  uint64_t const_,const_mask;
  unsigned int i,a,arg_c;
  unsigned int strlen_op_args;
  unsigned int strlen_op_desc;
  signed int c;

  /*Number of arguments cannot be more than max number of bits in a OP code.*/
  Parsed_ARG op_list[MAX_OP_DESC];

  c=0;
  const_=0;
  const_mask=0;
  const_=const_;
  const_mask=const_mask;

  strlen_op_args=strlen(op_args);
  strlen_op_desc=strlen(op_desc);

  for(i=0,arg_c=0;i<strlen_op_args;i++)
    {
      if(op_args[i]!=',')
	{
	  if(op_args[i]!=' ')
	    {
	      op_list[arg_c].arg_t = op_args[i];
	      arg_c++;
	    }
	}
      else
	{

	}
    }

  /*Parse and assemble the op_desc string from arguments given*/
  for(i=0;i<strlen_op_desc;i++)
    {
      c++;
      op_list[i].mask=0;
      op_list[i].mask_value=0;
      op_list[i].arg_len=0;
      op_list[i].arg_val=0;
      op_list[i].arg_shift=0;


      for(a=0;a<arg_c;a++)
	{
	  if(op_list[a].arg_t == op_desc[i])
	    {
	      op_list[a].mask |= 1<<(strlen_op_desc-i-1);
	      op_list[a].arg_len++;
	      op_list[a].arg_shift=strlen_op_desc-c;
	    } else
	    if((op_desc[i]=='1') || (op_desc[i]=='0'))
	      {
		if(op_desc[i]=='1')
		  {
		    const_ |= 1<<(strlen_op_desc-i-1);
		  }
		const_mask |= 1<<(strlen_op_desc-i-1);
	      }
	}
    }

  /*assign values, and mask out overflow bits*/
  for(i=0;i<arg_c;i++)
    {
      /*
      op_list[i].mask_value= op_list[i].arg_val & ((1<<(op_list[i].arg_len+1))-1);
      op_list[i].mask_value= op_list[i].mask_value << op_list[i].arg_shift;
      */
      /*
      op_list[i].mask = ((1<<(op_list[i].arg_len))-1)<<op_list[i].arg_shift;
      */
      /*ret|=op_list[i].mask_value;*/
    }

  for(i=0;i<arg_c;i++)
    {
      printf("%c val=%lu [0x%lX] len=%u shift=%u mask=0x%lx\n",op_list[i].arg_t, op_list[i].arg_val, op_list[i].arg_val,op_list[i].arg_len,op_list[i].arg_shift,op_list[i].mask);
    }

}

unsigned int encode_opcode_n(const instruction * found_instr, const char * op_args, const char * op_desc)
{
  uint64_t ret,const_,const_mask, value;
  unsigned int i,a,arg_c;
  unsigned int strlen_op_args;
  unsigned int strlen_op_desc;
  signed int c;

  Parsed_ARG op_list[MAX_ARGS];

  ret=0;

  const_=0;
  const_mask=0;
  const_=const_;
  const_mask=const_mask;

  strlen_op_args=strlen(op_args);
  strlen_op_desc=strlen(op_desc);

  /*printf("\nBuilding: %s, %s-%u\n", op_args, op_desc, strlen_op_desc);*/

  for(i=0;i<MAX_ARGS;i++)
    {
      op_list[i].arg_val=0;
      op_list[i].arg_t='\0';
      op_list[i].arg_len=0;
      op_list[i].arg_shift=0;
      op_list[i].mask_value=0;
    }

  /*Parse op_args, and count number of args*/
  for(i=0,arg_c=0;i<strlen_op_args;i++)
    {
      if(op_args[i]!=',')
	{
	  if(op_args[i]!=' ')
	    {
	      op_list[arg_c].arg_t = op_args[i];
	      arg_c++;
	    }
	}
      else
	{

	}
    }

  c=0;

  /*Parse and assemble the op_desc string from arguments given*/
  for(i=0;i<strlen_op_desc;i++)
    {
      c++;
      for(a=0;a<arg_c;a++)
	{

	  if(op_list[a].arg_t == op_desc[i])
	    {
	      /*printf("%c %u\n",op_desc[i],op_list[a].arg_shift);*/
	      op_list[a].arg_len++;
	      op_list[a].arg_shift=strlen_op_desc-c;
	      /*
	      for(c=a;c>0;c--) op_list[a].arg_shift-=op_list[c].arg_len;
	      */
	    } else
	    if((op_desc[i]=='1') || (op_desc[i]=='0'))
	      {
		if(op_desc[i]=='1')
		  {
		    const_ |= 1<<(strlen_op_desc-i-1);
		  }
		const_mask |= 1<<(strlen_op_desc-i-1);
	      }
	}
    }

  /*printf("0x%X - 0x%X\n",const_,const_mask);*/

  /*assign values, and mask out overflow bits*/
  for(i=0;i<arg_c;i++)
    {  
      value = found_instr->arg[i].value;
      op_list[i].mask_value= value & ((1<<(op_list[i].arg_len+1))-1);
      op_list[i].mask_value= op_list[i].mask_value << op_list[i].arg_shift;
      ret|=op_list[i].mask_value;
    }

  /*
  for(i=0;i<arg_c;i++)
    {
      printf("%c val=%lu [0x%lX] len=%u shift=%u masked_val=0x%lx\n",op_list[i].arg_t, value, value ,op_list[i].arg_len,op_list[i].arg_shift,op_list[i].mask_value);
    }
  */
  

  return ret | const_;
}


unsigned int encode_opcode(const char * op_args, const char * op_desc, const int num, ...)
{
  uint64_t ret,const_,const_mask;
  unsigned int i,x,a,arg_c;
  unsigned int strlen_op_args;
  unsigned int strlen_op_desc;
  signed int c;

  Parsed_ARG op_list[MAX_ARGS];
  va_list arg;

  ret=0;

  const_=0;
  const_mask=0;
  const_=const_;
  const_mask=const_mask;

  strlen_op_args=strlen(op_args);
  strlen_op_desc=strlen(op_desc);

  for(i=0;i<100;i++)
    {
      op_list[i].arg_val=0;
      op_list[i].arg_t='\0';
      op_list[i].arg_len=0;
      op_list[i].arg_shift=0;
      op_list[i].mask_value=0;
    }

  /*Parse op_args, and count number of args*/
  for(i=0,arg_c=0;i<strlen_op_args;i++)
    {
      if(op_args[i]!=',')
	{
	  if(op_args[i]!=' ')
	    {
	      op_list[arg_c].arg_t = op_args[i];
	      arg_c++;
	    }
	}
      else
	{

	}
    }

  /*Parse out the va_list*/
  /*if(arg_c==num)*/
  if(1)
    {
      va_start(arg,num);
      for(x=0;x < arg_c; x++)
	{
	  op_list[x].arg_val = va_arg(arg, unsigned int);	  
	}
      va_end(arg);
    }
  c=0;

  /*Parse and assemble the op_desc string from arguments given*/
  for(i=0;i<strlen_op_desc;i++)
    {
      c++;
      for(a=0;a<arg_c;a++)
	{
	  if(op_list[a].arg_t == op_desc[i])
	    {
	      /*printf("%c %u\n",op_desc[i],op_list[a].arg_shift);*/
	      op_list[a].arg_len++;
	      op_list[a].arg_shift=strlen_op_desc-c;
	      /*
	      for(c=a;c>0;c--) op_list[a].arg_shift-=op_list[c].arg_len;
	      */
	    } else
	    if((op_desc[i]=='1') || (op_desc[i]=='0'))
	      {
		if(op_desc[i]=='1')
		  {
		    const_ |= 1<<(strlen_op_desc-i-1);
		  }
		const_mask |= 1<<(strlen_op_desc-i-1);
	      }
	}
    }

  /*printf("0x%X - 0x%X\n",const_,const_mask);*/

  /*assign values, and mask out overflow bits*/
  for(i=0;i<arg_c;i++)
    {
      op_list[i].mask_value= op_list[i].arg_val & ((1<<(op_list[i].arg_len+1))-1);
      op_list[i].mask_value= op_list[i].mask_value << op_list[i].arg_shift;
      ret|=op_list[i].mask_value;
    }

  /*
  for(i=0;i<arg_c;i++)
    {
      printf("%c val=%u [0x%X] len=%u shift=%u masked_val=0x%x\n",op_list[i].arg_t, op_list[i].arg_val, op_list[i].arg_val,op_list[i].arg_len,op_list[i].arg_shift,op_list[i].mask_value);
    }
  */

  return ret | const_;
}
