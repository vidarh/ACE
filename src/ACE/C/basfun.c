/* << ACE >>
   
   -- Amiga BASIC Compiler --

   ** Intrinsic Functions **
   ** Copyright (C) 1998 David Benn
   ** 
   ** This program is free software; you can redistribute it and/or
   ** modify it under the terms of the GNU General Public License
   ** as published by the Free Software Foundation; either version 2
   ** of the License, or (at your option) any later version.
   **
   ** This program is distributed in the hope that it will be useful,
   ** but WITHOUT ANY WARRANTY; without even the implied warranty of
   ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   ** GNU General Public License for more details.
   **
   ** You should have received a copy of the GNU General Public License
   ** along with this program; if not, write to the Free Software
   ** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

   Author: David J Benn
     Date: 16th-30th November, 1st-12th December 1991,
	   27th January 1992, 
           6th,11th,14th,17th,28th,29th February 1992,
	   23rd March 1992,
	   21st April 1992,
	   2nd,11th,15th May 1992,
	   8th,14th,28th June 1992,
	   2nd,5th,14th,15th,26th-28th July 1992,
	   2nd,9th August 1992,
	   6th,7th,8th,13th,29th December 1992,
	   5th January 1993,
	   14th,18th February 1993,
	   8th,10th March 1993,
	   25th,30th May 1993,
	   6th,13th,19th,30th June 1993,
	   1st,3rd,4th July 1993,
	   5th,25th September 1993,
	   10th,11th October 1993,
	   2nd,9th November 1993,
	   24th,28th December 1993,
	   6th January 1994,
	   7th,26th,27th February 1994,
	   4th April 1994,
	   28th August 1994,
	   3rd,4th September 1994,
	   5th,11th March 1995,
	   10th March 1996
*/

#include "acedef.h"
#include "codegen.h" 
/* locals */
static	char	*addreg[] = { "a4","a5" };

/* externals */   
extern	int 	sym;
extern	int 	lev;
extern	int	struct_member_type;
extern	char 	id[MAXIDSIZE];   
extern	char 	ut_id[MAXIDSIZE];
extern	SYM	*curr_item;
extern	char 	tempstrname[80];
extern	char	strstorename[80];
extern	char	strstorelabel[80];
extern	BOOL 	cli_args;
extern	BOOL 	break_opt;
extern	BOOL	have_lparen;

/* string functions */
BOOL strfunc()
{
 switch(sym)
 {
  case argstrsym	: return(TRUE);
  case ascsym	   	: return(TRUE);
  case binstrsym	: return(TRUE);
  case chrstrsym   	: return(TRUE);
  case cstrsym		: return(TRUE);
  case fileboxstrsym	: return(TRUE);
  case hexsym	   	: return(TRUE); 
  case inputboxsym	: return(TRUE);	/* this is here for convienience */
  case inputboxstrsym	: return(TRUE);
  case inputstrsym 	: return(TRUE);
  case instrsym		: return(TRUE);
  case leftstrsym  	: return(TRUE);
  case lensym	   	: return(TRUE);
  case midstrsym   	: return(TRUE);
  case octstrsym   	: return(TRUE);
  case ptabsym		: return(TRUE);
  case rightstrsym 	: return(TRUE);
  case saddsym		: return(TRUE);
  case spacestrsym	: return(TRUE);
  case spcsym		: return(TRUE);
  case strstrsym   	: return(TRUE);
  case stringstrsym	: return(TRUE);
  case tabsym 		: return(TRUE);
  case translatestrsym 	: return(TRUE);
  case ucasestrsym 	: return(TRUE);
  case valsym      	: return(TRUE);
 }
 return(FALSE);
}

void load_temp_string(unsigned char reg)
{
  make_temp_string();
  gen_load_addr(tempstrname,reg);
}

void opt_arg(int type) {
  if (sym == comma) {
	insymbol();
	if (sym != comma) {
	  if (type == longtype) make_sure_long(expr());
	  else if (expr() != type) _error(4);
	} else gen_push32_val(0);
  } else gen_push32_val(0);
}

int stringfunction()
{
int  func;
int  sftype=undefined;
int  ntype=undefined;
char buf[80],srcbuf[80];
BOOL offset_on_stack;
 
 if (strfunc()) 
 {
  func=sym;
  insymbol();
  if (sym != lparen) _error(14);
  else
  {
   insymbol();
   sftype=expr();
    
   switch(func)
      {
        /* CHR$ */
        case chrstrsym : sftype=make_integer(sftype);
        		 if (sftype == longtype) 
        		 {
    	 		  make_short();
    	 		  sftype=shorttype;
        		 }

      			 if (sftype != notype)
      			 {
      			  /* Ascii value to copy to string */
        		  gen_pop16d(0);
      			  /* create a string to copy value to */
      			  make_string_store();
      			  strcpy(buf,strstorename);
      			  gen_load_addr(buf,0);
      			  gen_jsr("_chrstring");
      			  gen("pea",strstorename,"  ");
      			  enter_BSS(strstorelabel,"ds.b 2");
      			  sftype=stringtype;
      			 }
     			 else { _error(4); sftype=undefined; }
        		break;

    /* ARG$ */
    case argstrsym  :
	  make_sure_long(sftype); /* argument number */
	  make_temp_string();  	/* destination buffer */
	  gen("pea",tempstrname,"  ");
	  gen_call("_arg",8);
	  cli_args=TRUE;
	  sftype=stringtype;
	  break;

    /* ASC */
    case ascsym  :	
	  if (sftype == stringtype) {
		gen_call_args("_asc","a2 : d0.w",0);
		sftype=shorttype;
	  } else { _error(4); sftype=undefined; }	 
	  break;

    /* BIN$ */
    case binstrsym  :	
	  make_sure_long(sftype);
	  gen_call_args("_binstr","t0,d0 : a0",0);
	  sftype=stringtype;
	  break;

    /* CSTR */
    case cstrsym : 
	  if ((sftype == stringtype) || (sftype == longtype)) sftype=stringtype;
	  else { _error(4); sftype=undefined; }
	  break;

    /* FILEBOX$ */
    case fileboxstrsym : 
	  if (sftype == stringtype) {  /* title */
		/* default directory? */
		if (sym == comma) {
		  insymbol();
		  if (expr() != stringtype) _error(4);
		} else gen_push32_val(0);
		gen_call("_filerequest",8);
		sftype=stringtype;
	  } else { _error(4); sftype=undefined; }
	  break;

    /* HEX$ */
    case hexsym  :	
	  if (sftype != stringtype) {
		sftype = make_integer(sftype);
		if (sftype == longtype) gen_call_args("_hexstrlong","t0,d0 : a0",0);
		else gen_call_args("_hexstrshort","t0,d0.w : a0",0);
		sftype=stringtype;
	  }
	  else { _error(4); sftype=undefined; }
	  break;
			  
    /* INPUTBOX and INPUTBOX$ */
    case inputboxsym :
    case inputboxstrsym : 
	  if (sftype == stringtype) { 	/* prompt */ 	
		/* all other parameters are optional */
		opt_arg(stringtype); /* title */
		opt_arg(stringtype); /* default value */
		opt_arg(longtype);   /* xpos */
		opt_arg(longtype);   /* ypos */

		if (func == inputboxsym) { /* INPUTBOX */
		  gen_call("_longint_input_box",20);
		  sftype = longtype;
		} else { /* INPUTBOX$ */
		  gen_call("_string_input_box",20);
		  sftype = stringtype;
		}
	  } else { _error(4); sftype=undefined; }
	  break;
			 
    /* INPUT$(X,[#]filenumber) */
    case inputstrsym : 
	  if (sftype != stringtype)
		{ 
		  check_for_event();

		  make_sure_long(sftype);

			if (sym == comma)
			{
			 insymbol();
			 if (sym == hash) insymbol();
			 make_sure_long(expr());
			}
			else { _error(16); sftype=undefined; }

			/* filenumber , no. of characters */
			gen_call_args("_inputstrfromfile", "d0,d1",0);
			sftype=stringtype;
		}
	  else { _error(4); sftype=undefined; }
	  break;     
		
    /* INSTR$([I,]X$,Y$) */
    case instrsym  :	if (sftype != stringtype)
			{
			  make_sure_long(sftype);
 
			 if (sym == comma) 
			 { 
			  offset_on_stack=TRUE;		/* optional offset I */
			  insymbol(); sftype=expr(); 
			 }
			 else
			     { _error(16); sftype=undefined; }	   
			}	
		    	else 
			    offset_on_stack=FALSE;

			/* get X$ and Y$ */
		     	if (sftype == stringtype)
			{
			 if (sym == comma) 
			 {
			  insymbol();			 
			  if (expr() == stringtype)
			  {
				gen_pop_addr(1);		/* Y$ */
				gen_pop_addr(0);		/* X$ */
				if (offset_on_stack) 
			      gen_pop32d(0);	/* I */
			   else
			      gen("moveq","#1","d0");		/* I=1 */
			   
			   /* call INSTR */
			   gen_call("_instr",0); /* returns posn of Y$ in X$ */
			   sftype=longtype;
			  }
			  else { _error(4); sftype=undefined; }
			 }
			 else { _error(16); sftype=undefined; }
			}
			else { _error(4); sftype=undefined; }
			break;
			
    /* LEFT$ */
    case leftstrsym :	if (sftype == stringtype)
			{
			  if (sym == comma) {
				insymbol();
				make_sure_short(expr());
				gen_call_args("_leftstr","d0.w,a0,t1 : a0",0); /* returns addr of left$ */
				sftype=stringtype;
			  } else { _error(16); sftype=undefined; }
			} else { _error(4); sftype=undefined; }
			break;

    /* LEN */
    case lensym  : 
	  if (sftype == stringtype) {
		gen_call_args("_strlen","a2",0);
		sftype=longtype;
	  }
	  else { _error(4); sftype=undefined; }	 
	  break;

    /* OCT$ */
	  case octstrsym:
		if (sftype != stringtype) {
		  make_sure_long(sftype);
		  gen_call_args("_octstr","t0,d0 : a0",0);
		  sftype=stringtype;
		} else { _error(4); sftype=undefined; }
		break;
 
    /* RIGHT$ */
    case rightstrsym :	
	  if (sftype == stringtype) {
		if (sym == comma) {
		  insymbol();
		  make_sure_short(expr());
		  gen_call_args("_rightstr","d0.w,a0,t1 : a0",0);
		  sftype=stringtype;
		} else { _error(16); sftype=undefined; }
	  } else { _error(4); sftype=undefined; }
	  break;

    /* SADD */
    case saddsym : 	if (sftype == stringtype)
		      	   sftype=longtype; /* address is on stack */
		   	else { _error(4); sftype=undefined; }
		   	break;

    /* SPC, SPACE$ */
    case spcsym:
    case spacestrsym :  if (sftype != stringtype)
			{
			  make_sure_short(sftype);
			  if (func == spacestrsym) gen_call_args("_spacestring","d0.w,t0",0);
			  else gen_call_args("_spc","d0.w,t0",0);
			  sftype=stringtype;
			}
			else { _error(4); sftype=undefined; }
			break;

    /* STR$ */
    case strstrsym :	
	  if (sftype != stringtype) {
		load_temp_string(0);
		if (sftype == longtype) gen_call_args("_strlong","d0 : a0",0);
		else if (sftype == shorttype) gen_call_args("_strshort"," d0.w : a0",0);
		else if (sftype == singletype) {
		  gen_call("_strsingle",4);
		  enter_XREF("_MathBase");
		}
		sftype=stringtype;
	  } else { _error(4); sftype=undefined; }
	  break;

    /*   STRING$(I,J) 
      or STRING$(I,X$) */
    case stringstrsym : if (sftype != stringtype)
			{
			 make_sure_short(sftype);

			 if (sym == comma) {
			  insymbol();
			  ntype=expr();

			  if (ntype == stringtype) {
				gen_pop_addr(0);
				gen("move.b","(a0)","d1");
				gen("ext.w","d1","  ");
				gen("ext.l","d1","  ");	/* MID$(X$,1,1) */
			  } else {
				make_sure_long(ntype);
				gen_pop32d(1);	/* J */			
			  }
			  
			  gen_call_args("_stringstr","d0.w,t0",0);
			  sftype=stringtype;
			 }
			 else { _error(16); sftype=undefined; }
		    	}
			else { _error(4); sftype=undefined; }	
			break;    

    /* MID$ -> MID$(X$,n[,m]) */
    case midstrsym :	if (sftype == stringtype)
			{
			  if (sym == comma) {
				insymbol();	       /* start position */
				make_sure_short(expr());

				if (sym == comma) {
				  insymbol();        /* character count */
				  gen_pop_as_short(expr(),1); /* char count */
				} else {
				  /* take the full length of the string */
				  gen("move.w","#-1","d1");  
				}

			   gen_call_args("_midstr","d0.w,a0,t1 : a0",0);
			   sftype=stringtype;
			 }
			 else { _error(16); sftype=undefined; }
			}
			else { _error(4); sftype=undefined; }
			break;

			/* PTAB */
	  case ptabsym: sftype = gen_sfunc("_ptab", sftype, shorttype, stringtype); break;
		/* TAB */
	  case tabsym: sftype =gen_sfunc("_horiz_tab",sftype,shorttype,stringtype); break;
	
    /* TRANSLATE$ */
    case translatestrsym :if (sftype == stringtype)
			  {
			   gen_pop_addr(0); /* instr */
			   load_temp_string(1); /* outstr */
			   gen_move32aa(0,2);
			   gen_jsr("_strlen"); /* inlen in d0 */
			   sprintf(srcbuf,"#%d",MAXSTRLEN); /* #MAXSTRLEN */
			   gen("move.l",srcbuf,"d1"); /* outlen = MAXSTRLEN */
			   gen_libbase("Trans");
			   gen_libcall("Translate","Trans");
			   gen("pea",tempstrname,"  "); /* outstr on stack */
			   sftype=stringtype;
			  }
			  else { _error(4); sftype=undefined; }
			  break;
	
    /* UCASE$ */
    case ucasestrsym  :	
	  if (sftype == stringtype) gen_call_args("_ucase","a1,t0 : a0",0);
	  else { _error(4); sftype=undefined; }	 
	  break;

    /* VAL */
    case valsym :	if (sftype == stringtype)
			{
			 gen_call("_val",4); /* string is on the stack */
			 enter_XREF("_MathBase");  /* _val needs math libs */
			 enter_XREF("_MathTransBase");
			 sftype=singletype;
			}
			else { _error(4); sftype=undefined; } 
			break;
   }

   if (sym != rparen) { _error(9); sftype=undefined; }
  }  
  insymbol();
 }
 return(sftype);
}

/* Generate a simple function with one argument. Return type
   is assumed to be the same as the arg type if not specified */
int gen_sfunc(const char * funcname, int nftype, int srctype,int rettype)
{
  if (nftype == stringtype) {
	_error(4); 
	return undefined;
  }
  if (srctype == shorttype) gen_pop_as_short(nftype,0);
  else {
	make_sure_long(nftype);
	gen_pop32d(0);
  }
  gen_jsr(funcname);
  if (rettype == shorttype) gen_push16d(0);
  else if (rettype == longtype) gen_push32d(0);
  else if (rettype == stringtype) gen_push_addr(0);
  return rettype;
}

/* numeric functions */
int gen_single_func(char * funcname,int nftype)
{
  
  if (nftype != stringtype)
	{
	  if (nftype != singletype) gen_Flt(nftype);  
	  gen_pop32d(0);
	  gen_libbase("MathTrans");
	  gen_libcall(funcname,"MathTrans");
	  gen_push32d(0);
	  enter_XREF("_MathBase");
	  nftype=singletype;
	}
  else { _error(4); nftype=undefined; }
  return(nftype);
}

BOOL numfunc()
{
 switch(sym)
 {
  case abssym    	: return(TRUE);
  case allocsym	 	: return(TRUE);
  case atnsym    	: return(TRUE);
  case cintsym   	: return(TRUE);
  case clngsym   	: return(TRUE);
  case cossym    	: return(TRUE);
  case csngsym   	: return(TRUE);
  case eofsym	 	: return(TRUE);
  case expsym    	: return(TRUE); 
  case fixsym    	: return(TRUE);
  case fresym	 	: return(TRUE);
  case gadgetsym 	: return(TRUE);
  case handlesym 	: return(TRUE);
  case iffsym		: return(TRUE);
  case intsym    	: return(TRUE);
  case locsym	 	: return(TRUE);
  case lofsym	 	: return(TRUE);
  case logsym    	: return(TRUE);
  case longintsym    	: return(TRUE);
  case menusym	 	: return(TRUE);
  case mousesym  	: return(TRUE);
  case msgboxsym 	: return(TRUE);
  case peeksym   	: return(TRUE);
  case peekwsym 	: return(TRUE);
  case peeklsym  	: return(TRUE);
  case pointsym  	: return(TRUE);
  case potxsym	 	: return(TRUE);
  case potysym	 	: return(TRUE);
  case saysym	 	: return(TRUE);
  case screensym 	: return(TRUE);
  case serialsym 	: return(TRUE);
  case sgnsym    	: return(TRUE);
  case shlsym	 	: return(TRUE);
  case shrsym	 	: return(TRUE);
  case sinsym    	: return(TRUE);
  case sizeofsym 	: return(TRUE);
  case sqrsym    	: return(TRUE);
  case sticksym  	: return(TRUE);
  case strigsym  	: return(TRUE);
  case tansym    	: return(TRUE);
  case varptrsym 	: return(TRUE);
  case windowsym 	: return(TRUE);
 }
 return(FALSE);
}

int numericfunction()
{
int  func;
int  nftype=undefined;
char labname[80],lablabel[80];
char varptr_obj_name[MAXIDSIZE];

 if (numfunc()) 
 {
  func=sym;
  insymbol();
  if (sym != lparen) _error(14);
  else
  {
   insymbol();
   if ((func != varptrsym) && (func != sizeofsym)) nftype=expr();
    
   switch(func)
      {
       /* ABS */
       case abssym : 
		 if (nftype == shorttype) gen_call_args("_absw","d0.w : d0.w",0);
		 else if (nftype == longtype) gen_call_args("_absl","d0 : d0",0);
		 else if (nftype == singletype) {
		   gen_call_args("_absf", "d0 : d0",0);
		   enter_XREF("_MathBase");
		 } else { _error(4); nftype=undefined; }
		 break;

	 /* ALLOC */ 
	 case allocsym :
	   if (nftype != stringtype)
		 {
		   /* minimum number of bytes to reserve */
		   make_sure_long(nftype);
		
		   if (sym != comma) {
			 gen_push32_val(9);	/* 9 = default type */
			 nftype=longtype;
		   } else {
			  /* memory type specification */
			  insymbol();
			  nftype=expr();
			  if (nftype != stringtype) {
				make_sure_long(nftype);
				nftype=longtype;
			  } else { _error(4); nftype=undefined; }
			 }

			 /* call ACEalloc() function */
			 gen_call("_ACEalloc",8);
			}
			else { _error(4); nftype=undefined; }
			break;
  	 
	 /* ATN */
	  case atnsym  : nftype = gen_single_func("SPAtan",nftype); break;

	 /* CINT */
	 case cintsym :
	   nftype = make_sure_short(nftype);
	   break;

	 /* CLNG */
	 case clngsym : if (nftype == singletype)
			{
			 gen_round(nftype);
			 nftype=longtype;
			}
			else
			  if (nftype == shorttype)
			  {
 			   gen_pop16d(0);
			   gen("ext.l","d0","  ");
			   gen_push32d(0);
			   nftype=longtype;
			  }
			  else
			      if (nftype == stringtype)
				  { _error(4); nftype=undefined; }
			break;

	 /* COS */
         case cossym  : nftype = gen_single_func("SPCos",nftype);
		        break;

	 /* CSNG */
	 case csngsym : if ((nftype == shorttype) || (nftype == longtype))
			{
			   gen_Flt(nftype);
			   nftype=singletype;
			}
 			else 
			    if (nftype == stringtype) 
			       { _error(4); nftype=undefined; }
			break; 
			    
    	/* EOF */
    	case eofsym   : if (nftype != stringtype)
		  	{ 
			 check_for_event();
			 nftype = gen_sfunc("_eoftest",nftype, longtype, longtype);
		  	}
		  	else { _error(4); nftype=undefined; }
		  	break;

	 /* EXP */
	  case expsym  : nftype = gen_single_func("SPExp",nftype); break;

	 /* FIX */
	 case fixsym  : if (nftype == singletype)
			{
			 gen_pop32d(0);
			 gen_libbase("Math");
			 gen_libcall("SPFix","Math");
			 gen_push32d(0);
			 nftype=longtype;
			}
			else
			  if (nftype == stringtype)
			     { _error(4); nftype=undefined; }

			/* else if short or long, leave on stack 
			   and let nftype remain the same! */
			break;

			/* FRE */
	  case fresym : if (nftype != stringtype)
		{
		  nftype = gen_sfunc("_fre",nftype, shorttype,longtype);
		}
	  else { _error(4); nftype=undefined; }
		break;

	 /* GADGET */
	 case gadgetsym :
			  make_sure_long(nftype);
			  gen_call("_GadFunc",4);
			  nftype=longtype;
			  break;

	 /* HANDLE */
	 case handlesym : if (nftype != stringtype)
			  {
			   check_for_event();
			   nftype = gen_sfunc("_handle", nftype, longtype,longtype);
			  }
			  else { _error(4); nftype=undefined; }
			  break;

	 /* IFF */
	 case iffsym : if (nftype != stringtype)
			  {
			   check_for_event();

			   /* channel */
			   make_sure_long(nftype);

			   /* function number */
			   if (sym == comma) 
			   {
			    insymbol();
			    make_sure_long(expr());
			    gen_call("_iff_func",8);
			    nftype = longtype;
			   }
			   else { _error(16); nftype=undefined; }
			  }
			  else { _error(4); nftype=undefined; }
			  break;

	 /* INT */
	 case intsym  : if (nftype == singletype)
			{
			 gen_pop32d(0);
			 gen_libbase("Math");
			 gen_libcall("SPFloor","Math");
			 gen_libcall("SPFix","Math");
			 gen_push32d(0);
			 nftype=longtype;
			}
			else
			  if (nftype == stringtype)
			     { _error(4); nftype=undefined; }

			/* else if short or long, leave on stack 
			   and let nftype remain the same! */
			break;

	 /* LOC */
	 case locsym  : if (nftype != stringtype)
			{
			 check_for_event();

			 make_sure_long(nftype);
			 gen_call("_FilePosition",4);
			 nftype=longtype;
			}
			else { _error(4); nftype=undefined; } 
 			break;
			   
	 /* LOF */
	 case lofsym  : if (nftype != stringtype)
			{
			 check_for_event();
			 nftype = gen_sfunc("_lof",nftype,longtype,longtype);
			}
			else { _error(4); nftype=undefined; } 
 			break;
			   
	 /* LOG */
         case logsym  : nftype = gen_single_func("SPLog",nftype);
		        break;

	 /* LONGINT */
	 case longintsym: if (nftype == stringtype)
			  {	
				gen_call("_long_from_string",4);
				nftype=longtype;
			  }
			  else { _error(4); nftype=undefined; }
			  break;
		
	 /* MENU */		
	 case menusym : if (nftype != stringtype)
			{
			  make_sure_long(nftype);
			  gen_call("_MenuFunc",4);
			  nftype=longtype;
			}
			else { _error(4); nftype=undefined; }
			break;
			
	 /* MOUSE */
	 case mousesym : 
	   if (nftype != stringtype) {
		 nftype = gen_sfunc("_mouse",nftype,shorttype,shorttype);
	   } else nftype=undefined; /* FIXME: Is it intentional that there's no _error(4) here? */
	   break;

	 /* MSGBOX */
	 case msgboxsym : if (nftype == stringtype)     /* message */
			  {
			   if (sym != comma) { _error(16); nftype=undefined; }
			   else {
			    insymbol();
			    if (expr() == stringtype) {  /* response #1 */
				  opt_arg(stringtype); /* response #2 */
				  gen_call_args("_sysrequest",":d0.w",12);
				  nftype=shorttype;
			    }
			    else { _error(4); nftype=undefined; }
			   }
			  }
			  else { _error(4); nftype=undefined; }
			  break;

	 /* PEEK */
	 case peeksym : nftype=make_integer(nftype);
			if ((nftype == longtype) || (nftype == shorttype))
			{
			 /* get address */
      			 if (nftype == shorttype)
			 {
			    gen_pop16d(0);
			    gen("ext.l","d0","  ");
			    gen("move.l","d0","a0");    
			 }
			 else
			    gen_pop_addr(0); 
			 /* get value */
			 gen("move.b","(a0)","d0");
			 gen("ext.w","d0","  ");
			 /* if n<0 n=255-not(n) */
			 gen("cmp.w","#0","d0");
			 make_label(labname,lablabel);
			 gen("bge.s",labname,"  ");
			 gen("not.w","d0","  ");
			 gen("move.w","#255","d1");
			 gen("sub.w","d0","d1");
			 gen("move.w","d1","d0");
			 gen(lablabel,"  ","  ");
			 gen_push16d(0);
			 nftype=shorttype;
			}
			else { _error(4); nftype=undefined; }
			break;

	 /* PEEKW */
	 case peekwsym : nftype=make_integer(nftype); 
			 if ((nftype == longtype) || (nftype == shorttype))
			 {
			  /* get address */
      			  if (nftype == shorttype)
			  {
			     gen_pop16d(0);
			     gen("ext.l","d0","  ");
			     gen("move.l","d0","a0");    
			  }
			  else
			     gen_pop_addr(0); 
			  /* get value */
			  gen("move.w","(a0)","-(sp)");
			  nftype=shorttype;
			 }
            		 break;

	 /* PEEKL */
	 case peeklsym : nftype=make_integer(nftype); 
			 if ((nftype == longtype) || (nftype == shorttype))
			 {
			  /* get address */
      			  if (nftype == shorttype)
			  {
			     gen_pop16d(0);
			     gen("ext.l","d0","  ");
			     gen("move.l","d0","a0");    
			  }
			  else
			     gen_pop_addr(0); 
			  /* get value */
			  gen("move.l","(a0)","-(sp)");
			  nftype=longtype;
			 }			
			 break;

	/* POINT */
	case pointsym :	if (nftype != stringtype)
			{
			 make_sure_short(nftype);
			 if (sym != comma) { _error(16); nftype=undefined; }
			 else {
			  insymbol();
			  gen_pop_as_short(expr(), 1); /* y */
			  gen_pop16d(0);  /* x */
			  gen_gfxcall("ReadPixel");
			  gen_push32d(0);
			  nftype=longtype;
			 }
			}
			else { _error(4); nftype=undefined; }
			break;
			
	  case potxsym : nftype = gen_sfunc("_potx",nftype,shorttype, shorttype); break;
	  case potysym : nftype = gen_sfunc("_poty",nftype,shorttype, shorttype); break;

	 /* SERIAL */
	 case serialsym : if (nftype != stringtype)
			  {
			   check_for_event();
			   make_sure_long(nftype); /* channel */

			   /* function number */
			   if (sym == comma) {
			    insymbol();
			    make_sure_long(expr());
			    gen_call("_serial_func",8);
			    nftype = longtype;
			   }
			   else { _error(16); nftype=undefined; }
			  }
			  else { _error(4); nftype=undefined; }
			  break;

	 /* SGN */
	 case sgnsym  : 
	   if (nftype == shorttype) nftype = gen_sfunc("_sgnw", nftype, shorttype, longtype);
	   else if (nftype == longtype) gen_sfunc("_sgnl",nftype, longtype,longtype);
	   else if (nftype == singletype) {
		 gen_call_args("_sgnf","d1 : d0", 0);
		 enter_XREF("_MathBase");
		 nftype=longtype;
	   } else { 
		 _error(4); nftype=undefined;
	   }
	   break; 
 
	 /* SHL */
	 case shlsym  : if (nftype != stringtype)
			{
			 /* value to be shifted */
			  make_sure_long(nftype);
			 
			 if (sym == comma)
			 {
			  insymbol();
			  /* shifted by how many bits? */
			  if ((nftype=expr()) != stringtype)
			  {
				make_sure_long(nftype);
			   
			   gen_pop32d(0); /* pop shift factor */
			   gen_pop32d(1); /* pop value */
			   gen("asl.l","d0","d1");     /* shift d1 by d0 */
			   gen_push32d(1); /* push result */
			   nftype=longtype;
			  }
			  else { _error(4); nftype=undefined; }
			 }
			 else { _error(16); nftype=undefined; }
			}
			else { _error(4); nftype=undefined; }
			break;
			 
	 /* SHR */
	 case shrsym  : if (nftype != stringtype)
			{
			 /* value to be shifted */
			  make_sure_long(nftype);
			 
			 if (sym == comma)
			 {
			  insymbol();
			  /* shifted by how many bits? */
			  if ((nftype=expr()) != stringtype)
			  {
				make_sure_long(nftype);
			   
			   gen_pop32d(0); /* pop shift factor */
			   gen_pop32d(1); /* pop value */
			   gen("asr.l","d0","d1");     /* shift d1 by d0 */
			   gen_push32d(1); /* push result */
			   nftype=longtype;
			  }
			  else { _error(4); nftype=undefined; }
			 }
			 else { _error(16); nftype=undefined; }
			}
			else { _error(4); nftype=undefined; }
			break;
			
	  case sqrsym:    nftype = gen_single_func("SPSqrt",nftype); break;
	  case sinsym:    nftype = gen_single_func("SPSin",nftype); break;
	  case sizeofsym: nftype = find_object_size(); break;
	  case sticksym:  nftype = gen_sfunc("_stick",nftype, shorttype,shorttype); break;
	  case strigsym:  nftype = gen_sfunc("_strig",nftype, shorttype,shorttype); break;
	  case tansym:    nftype = gen_single_func("SPTan",nftype); break;

	 /* VARPTR */
	 case varptrsym : if (sym == ident) 
			  {
			   strcpy(varptr_obj_name,id);
			   nftype=address_of_object();
			   /* structure and array code returns next symbol */
			   if (!exist(varptr_obj_name,structure) &&
			       !exist(varptr_obj_name,array)) 
			      insymbol();
			  }
			  else 
			     { _error(7); nftype=undefined; insymbol(); }
			  break;

	 /* WINDOW */
	  case windowsym : nftype = gen_sfunc("_windowfunc",nftype, shorttype,longtype); break;

	 /* SAY */
	 case saysym	: if (nftype != stringtype)
			  {
				make_sure_long(nftype);
				gen_call("_sayfunc",4);
				nftype=longtype;
			  }
			  else { _error(4); nftype=undefined; }
			  break;

	 /* SCREEN */
	 case screensym : if (nftype != stringtype)
			  {
				make_sure_long(nftype);
				gen_call("_screenfunc",4);
				nftype=longtype;
			  }
			  else { _error(4); nftype=undefined; }
			  break;
	  }
   if (sym != rparen) { _error(9); nftype=undefined; }
  }
  insymbol();
 }
 return(nftype);
}

int address_of_object()
{
/* return the address of a variable, array or structure */
SYM    *varptr_item;
char   buf[50],numbuf[40];
char   addrbuf[40];
char   extobjid[MAXIDSIZE];
char   subname[MAXIDSIZE+5];
SYM    *structype;
STRUCM *member;
BOOL   found;

			/* 
			** Make external variable/function
   			** name by removing qualifier and 
   			** adding an underscore prefix 
   			** if one is not present. 
			*/
 			strcpy(buf,ut_id);
 			remove_qualifier(buf);
 			if (buf[0] != '_')
 			{
  			 strcpy(extobjid,"_\0");
  			 strcat(extobjid,buf);
 			}
	   		else 
                   	    strcpy(extobjid,buf);

			/*
			** Make SUB name.
			*/
			sprintf(subname,"_SUB_%s",id);

			/*
			** Push address of valid object
			** [see ref.doc].
			*/
			   /* external variable or function? */
			   if (exist(extobjid,extvar) || 
			       exist(extobjid,extfunc))
			   {
			   	gen("pea",extobjid,"  ");
			   	return(longtype);			      
			   }
			   else
			   if (exist(subname,subprogram))
			   {
				gen("pea",subname,"  ");
				return(longtype);
			   }
			   else
			   /* ordinary variable? */
			   if (exist(id,variable))
			   {
			    varptr_item=curr_item;

			    /* get the frame start address */
			    strcpy(addrbuf,addreg[lev]);

			    /* get the frame offset */
			    sprintf(numbuf,"#%d",varptr_item->address);

			    /* calculate the absolute address */
			    gen_load32d(addrbuf,0);
			    gen("sub.l",numbuf,"d0");
			    if ((varptr_item->type == stringtype)
			       || ((varptr_item->shared) && (lev == ONE)))
			    {
			     /* location in frame contains address */  
			     gen("move.l","d0","a0");
			     gen("move.l","(a0)","-(sp)");
			    }
			    else
				/* absolute address in frame of variable */
			        gen_push32d(0);
			    return(longtype);    
			   }
			   else
			   if ((exist(id,array)) || (exist(id,structure)))
			   {
			    varptr_item=curr_item;

			    /* get the frame start address */
			    strcpy(addrbuf,addreg[lev]);

			    /* get the frame offset */
			    sprintf(numbuf,"#%d",varptr_item->address);

			    /* calculate the absolute address */
			    gen_load32d(addrbuf,0);
			    gen("sub.l",numbuf,"d0");

			    /* location in frame contains array/struct address 
			       (except for shared structure (see below) */  
			    gen("movea.l","d0","a0");
			    
			    /* address of a structure member? */
			    if (exist(id,structure))
			    {
			     /* shared struct? -> get struct variable address */
			     if (varptr_item->shared && lev == ONE) 
				gen("movea.l","(a0)","a0");

			     insymbol();  
			     if (sym == memberpointer)
			     {
			      insymbol();  
			      if (sym != ident) 
			         _error(7);
			      {
			       structype = varptr_item->other;
			       member = structype->structmem->next;
			       found=FALSE;
			       while ((member != NULL) && (!found))
			       {
			        if (strcmp(member->name,id) == 0)
				   found=TRUE;
			        else
			 	   member = member->next;
			       }
			       if (!found)
			 	  _error(67);  /* not a valid member */
			       else
				 {
				  /* push address of struct member */
				  sprintf(numbuf,"#%ld",(long)member->offset);
				  gen("movea.l","(a0)","a0");
				  gen("adda.l",numbuf,"a0");
				  gen_push_addr(0);
				  /* store type for SWAP command */
				  struct_member_type = member->type;	  	
				 }
			       }
			      insymbol();
			     }
			     else
			     {
			      /* address of struct variable in stack frame */
			      gen_push_addr(0); 
			      /* store type for SWAP command */
			      struct_member_type = longtype;
			     }	  	
			    }
			    else
			    /* array or array element address? */
			    {
				/* push array address */
			        gen("move.l","(a0)","-(sp)"); 

				insymbol();

				if (sym == lparen)
				{
				 /* calculate array element address */
				 have_lparen=TRUE;
				 push_indices(varptr_item);
				 get_abs_ndx(varptr_item); /* offset -> d7 */
				 gen_pop32d(0); /* array start */
				 gen("add.l","d7","d0"); /* start+offset=addr */
			 	 gen_push32d(0); /* push address */
				 insymbol(); /* symbol after rparen */
				}
			    }
			    return(longtype);	    
			   }
			   else { _error(43); return(undefined); }
}

int find_object_size() {
  /* push the size (in bytes) 
	 of a data object or type 
	 onto the stack. 
  */
  char numbuf[40];
  int   nftype=longtype;

  if (sym == ident) {
	/* variable */
	if (exist(id,variable)) {
	  if (curr_item->type == shorttype)       gen_push32_val(2); 
	  else if (curr_item->type == longtype)   gen_push32_val(4); 
	  else if (curr_item->type == singletype) gen_push32_val(4); 
	  else if (curr_item->type == stringtype) gen_push32_val((long)curr_item->size);
	} else if (exist(id,array) || exist(id,structdef)) {
	  /* array variable or structure definition */
	  gen_push32_val((long)curr_item->size);
	} else  if (exist(id,structure)) { /* structure variable */
	  gen_push32_val((long)curr_item->other->size);
	} else {
	  _error(43);	 /* undeclared array or variable */
	  nftype=undefined;
	}
  }  else {
	/* type identifier? */
   if (sym == bytesym) gen_push32_val(1); 
   else if (sym == shortintsym) gen_push32_val(2); 
   else if (sym == longintsym || sym == addresssym) gen_push32_val(4); 
   else if (sym == singlesym) gen_push32_val(4); 
   else if (sym == stringsym) gen_push32_val(MAXSTRLEN);
   else {
	 /* expected an identifier or type */
	 _error(60);
	 nftype=undefined;
   }
 }

 insymbol();
 return(nftype);
}
