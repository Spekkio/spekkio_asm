#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <inttypes.h>
#include "main.h"
#include "parse.h"
#include "encode.h"
#include "smallfunc.h"
#include "assemble.h"


/*Finds out what kind of symbol we have, HEX, NUMBER, SYMBOL, HSYMBOL or matched argument*/
/*Returns the type and puts a index pointer in match_found*/
ARG_TYPE detectType(unsigned int * match_found, const small_argument arg, const argument_list * arg_list, const symbol_table * symb_list, const symbol_table * hsymb_table)
{
  char result[MAX_ARG_PARSED_LEN];
  int match_ret;
  unsigned int sc;
  ARG_TYPE is;
  
  is=ISUNDEFINED;
  /*Check if argument is an encoded number, hex or decimal*/
  /*found_instr->arg[i].arg/arg_len*/

  is=isNumberType(arg.arg, arg.arg_len);

  /*Search the symbol list*/
  if(is==ISUNDEFINED)
    {
      if(!match_symbol(match_found, arg.arg, hsymb_table, MAX_ARG_PARSED_LEN))
	{
	  is=ISHSYMBOL;
	}
      if(!match_symbol(match_found, arg.arg, symb_list, MAX_ARG_PARSED_LEN))
	{
	  is=ISSYMBOL;
	}
    }

  if(is==ISUNDEFINED)
    {
      for(sc=0;(sc<arg_list->num) && (is==ISUNDEFINED);sc++)
	{
	  /*arg_list->arg[sc].n_args;*/
	  /*printf(",ARGLIST[%u,\"%s\"]",sc,arg_list->arg[sc].arg_regex);*/
	  match_ret=match_argument(result,MAX_ARG_PARSED_LEN,arg.arg,&arg_list->arg[sc],0); /*sub argument 0*/
	  if(!match_ret)
	    {
	      is=IS_MATCHED;
	      *match_found=sc;
	    }
	}
    }
  return is;
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
assemble_ret assemble(instruction * found_instr, const cpu_instr_set * set, const argument_list * arg_list, const symbol_table * symb_list, const symbol_table * hsymb_table)
{
  uint64_t p_buf;
  unsigned int i, sc, size_calc, smallest_size, smallest_index, match_found;
  signed found_one, found_overflow;
  int match_ret;
  ARG_TYPE is;
  assemble_ret ret, rec;
  char result[MAX_ARG_PARSED_LEN];
  encode_op_ret opret;
  instruction rec_instr;
  signed cont;

  hsymb_table=hsymb_table;

  match_found=0;

  cont=0;

  ret.is=UNDEFINED;
  ret.num=0;
  for(i=0;i<MAX_OVERFLOWS;i++)
    {
      ret.opcode[i]=0;
      ret.size[i]=0;
    }

  set=set;
  arg_list=arg_list;
  symb_list=symb_list;
  /*These two must be the same, I think they are checked before*/
  /*I'm starting to loose track...*/

      switch(found_instr->is)
	{
	case ISINSTRUCTION:
	  if((found_instr->n_args == set->instr[found_instr->instr_index].n_args))
	    {
	      cont=1;
	    }
	  break;
	case ISARGUMENT:
	  if((found_instr->n_args == arg_list->arg[found_instr->instr_index].n_args))
	    {
	      cont=1;
	    }
	  break;
	default:
	  cont=0;
	  break;
	}

  if(cont)
    {

      if(verbose==1)
	{
	  switch(found_instr->is)
	    {
	    case ISINSTRUCTION:
	      printf("%s ",set->instr[found_instr->instr_index].instr_name);
	      break;
	    case ISARGUMENT:
	      printf("%s ",arg_list->arg[found_instr->instr_index].arg_regex);
	      break;
	    default: break;
	    }
	}

     

      /*loop through the arguments*/
      /*to be able to parse subarguments, this can be made into a function*/
      /*that can be used recursive.*/
      for(i=0;i<found_instr->n_args;i++)
	{

	  /*Finds out what kind of symbol we have, HEX, NUMBER, SYMBOL, HSYMBOL or matched argument*/
	  /*Returns the type and puts a index pointer in match_found*/
	  is=detectType(&match_found, found_instr->arg[i], arg_list, symb_list, hsymb_table);

	  if(verbose==1)
	    printf("%s, ",found_instr->arg[i].arg); /*used in VERBOSE DEBUG*/;
	  found_instr->arg[i].is=UNDEFINED;
	  p_buf = 0;
	  switch(is)
	    {
	    case ISHEX:
	      if(verbose==1)
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
	      if(verbose==1)
		printf("=NUMBER");

	      if(sscanf(found_instr->arg[i].arg,"%lu",&p_buf)==1)
		{
		  found_instr->arg[i].value = p_buf;
		  found_instr->arg[i].is=DEFINED;
		}

	      break;

	    case ISSYMBOL:
	      if(verbose==1)
		printf("=SYMBOL");

	      if(symb_list->table[match_found].is==DEFINED)
		{
		  found_instr->arg[i].value = symb_list->table[match_found].value;
		  found_instr->arg[i].is=DEFINED;
		}
	      else if(symb_list->table[match_found].is==UPDATED)
		{
		  found_instr->arg[i].value = symb_list->table[match_found].value;
		  found_instr->arg[i].is=DEFINED;
		}
	      break;

	    case ISHSYMBOL:
	      if(verbose==1)
		printf("=HARDSYMBOL");

	      if(hsymb_table->table[match_found].is==DEFINED)
		{
		  found_instr->arg[i].value = hsymb_table->table[match_found].value;
		  found_instr->arg[i].is=DEFINED;
		}
	      break;

	    case IS_MATCHED:
	      if(found_instr->is==ISINSTRUCTION)
		{
		  /*Denna kod skall vara i detectType(), mojligtvis dela upp detectType, detectSymbol.*/
		  if(verbose==1)
		    printf("=MATCHED_ARG->");
		  rec_instr.instr_index=match_found;
		  rec_instr.n_args=arg_list->arg[match_found].n_args; /*match_found may be uninit...*/
		  rec_instr.is=ISARGUMENT;
		  for(sc=0;sc<arg_list->arg[match_found].n_args;sc++)
		    {
		      match_ret=match_argument(result,MAX_ARG_PARSED_LEN,found_instr->arg[i].arg,&arg_list->arg[match_found],sc);
		      if(!match_ret)
			{/*
			  opret = encode_op(arg_list->arg[sc].arg_subargs, arg_list->arg[sc].arg_desc, 0, found_instr->arg[i].value);
			  if(!opret.error)
			    {
			    printf("Too big");
			    }
			 */
			  rec_instr.arg[sc].arg_len = strlen(result);
			  strncpy(rec_instr.arg[sc].arg, result, MAX_ARG_PARSED_LEN);
			  rec_instr.arg[sc].value = 0;
			  rec_instr.arg[sc].is = ISUNDEFINED;
			}
		    }
		  rec=assemble(&rec_instr, set, arg_list, symb_list, hsymb_table);
		  /*printf("RETURNED: %u, ",rec.num);*/
		      if(rec.is==DEFINED)
		      {
			if(rec.num>1)
			  for(sc=0;sc<(rec.num-1);sc++)
			    {
			      /*printf("SUBARG:%u code=0x%lX, size=%u, ", sc, rec.opcode[sc], rec.size[sc]);*/
			      ret.size[ret.num] = rec.size[sc];
			      ret.opcode[ret.num++] = rec.opcode[sc];
			    }
			/*Fixa dubbel matchning*/
			/*opret = encode_op(arg_list->arg[sc].arg_subargs, arg_list->arg[sc].arg_desc, 0, found_instr->arg[i].value);*/
			found_instr->arg[i].value = rec.opcode[rec.num-1];
			found_instr->arg[i].is=DEFINED;
		      }

		}
	      break;

	    default:
	      if(verbose==1)
		printf("=UNDEFSYMBOL");
	      break;
	    }

	  if((is==ISHEX || is==ISNUMBER || is==ISSYMBOL) && (found_instr->is==ISINSTRUCTION))
	    {
	      /*There could be an encoding rule for a literal*/
	      /*It's currently defined in arguments list*/
	      size_calc=0;
	      found_one=0;
	      found_overflow=0;
	      smallest_size=MAX_OP_DESC;
	      for(sc=0;sc<arg_list->num;sc++)
		{
		  /*arg_list->arg[sc].n_args;*/
		  /*printf(",ARGLIST[%u,\"%s\"]",sc,arg_list->arg[sc].arg_regex);*/
		  match_ret=match_argument(result,MAX_ARG_PARSED_LEN,found_instr->arg[i].arg,&arg_list->arg[sc],0); /*sub argument 0*/
		  if(!match_ret)
		    {
		      opret = encode_op(arg_list->arg[sc].arg_subargs, arg_list->arg[sc].arg_desc, 0, found_instr->arg[i].value);
		      if(!opret.error)
			{
			  size_calc=arg_list->arg[sc].arg_overflow_len;
			  if(size_calc<smallest_size)
			    {
			      smallest_size=size_calc;
			      smallest_index=sc;
			      found_one=1;
			    }
			}
		      opret = encode_op(arg_list->arg[sc].arg_subargs, arg_list->arg[sc].arg_overflow, 0, found_instr->arg[i].value);
		      if(!opret.error)
			{
			  size_calc=arg_list->arg[sc].arg_overflow_len;
			  if(size_calc<smallest_size)
			    {
			      smallest_size=size_calc;
			      smallest_index=sc;
			      found_overflow=1;
			    }
			}
		    }
		}

	      if(found_one)
		{
		  /*printf("E:[%s, %s]",arg_list->arg[smallest_index].arg_subargs, arg_list->arg[smallest_index].arg_desc);*/
		  opret = encode_op(arg_list->arg[smallest_index].arg_subargs, arg_list->arg[smallest_index].arg_desc, 0, found_instr->arg[i].value);
		  if(!opret.error)
		    {
		      found_instr->arg[i].value=opret.value;
		      found_instr->arg[i].is=DEFINED;
		    }
		} else if(found_overflow)
		{
		  /*printf("E:[%s, %s, %s]",arg_list->arg[smallest_index].arg_subargs, arg_list->arg[smallest_index].arg_desc, arg_list->arg[smallest_index].arg_overflow);*/
		  opret = encode_op(arg_list->arg[smallest_index].arg_subargs, arg_list->arg[smallest_index].arg_overflow, 0, found_instr->arg[i].value);
		  if(!opret.error)
		    {
		      ret.opcode[ret.num] = opret.value;
		      ret.size[ret.num++] = arg_list->arg[smallest_index].arg_overflow_len;

		      opret = encode_op(arg_list->arg[smallest_index].arg_subargs, arg_list->arg[smallest_index].arg_desc, 0, 0);
		      if(!opret.error)
			{
			  found_instr->arg[i].value=opret.value;
			  found_instr->arg[i].is=DEFINED;
			}
		    }
		}else
		{
		  /*printf("-NOENCODE-");*/
		}
	    }

	  if(found_instr->arg[i].is==DEFINED)
	    {
	      if(verbose==1)
		printf("(0x%lX), ", found_instr->arg[i].value);
	      if(found_instr->is==ISARGUMENT)
		{ 
		  if(arg_list->arg[found_instr->instr_index].arg_overflow_len>0)
		    {
		      /*printf("[%s, %u]",arg_list->arg[found_instr->instr_index].arg_subargs,i);*/
		      opret = encode_op(arg_list->arg[found_instr->instr_index].arg_subargs, arg_list->arg[found_instr->instr_index].arg_overflow, i, found_instr->arg[i].value);
		      if(!opret.error)
			{
			  ret.size[ret.num] = nWords(bitSize(found_instr->arg[i].value),16);
			  ret.opcode[ret.num++] = found_instr->arg[i].value;
			}
		    }
		}
	    }else
	    {
	      if(verbose==1)
		printf(", ");
	    }

	}/*Loop through arguments hex.*/

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
	  switch(found_instr->is)
	    {
	    case ISINSTRUCTION:
	      ret.size[ret.num] = set->instr[found_instr->instr_index].op_len;

	      ret.opcode[ret.num++] = encode_opcode_n(found_instr, set->instr[found_instr->instr_index].args, set->instr[found_instr->instr_index].op_desc);
	      
	      break;
	    case ISARGUMENT:
	      ret.size[ret.num] = arg_list->arg[found_instr->instr_index].arg_desc_len;
	      ret.opcode[ret.num++] = encode_opcode_n(found_instr, arg_list->arg[found_instr->instr_index].arg_subargs, arg_list->arg[found_instr->instr_index].arg_desc);
	      break;
	    default:
	      break;
	    }
	}
    }

  return ret;
}
