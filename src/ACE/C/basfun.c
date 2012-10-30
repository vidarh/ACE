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
static int find_object_size();

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

void gen_pop_short_addr(unsigned char tmp,unsigned char dest) {
  gen_pop16d(tmp);
  gen_ext16to32(tmp);
  gen_move32da(tmp,dest);
}

void load_temp_string(unsigned char reg)
{
  make_temp_string();
  gen_load_addr(tempstrname,reg);
}

void opt_arg(int type, int val) {
    if (try_comma()) {
        if (sym != comma) {
            if (type == longtype) long_expr();
            else if (expr() != type) _error(4);
        } else gen_push32_val(val);
    } else gen_push32_val(val);
}

static int chrstr(int sftype) {
  char buf[80];
  make_sure_short(sftype);
  /* Ascii value to copy to string */
  gen_pop16d(0);
  /* create a string to copy value to */
  make_string_store();
  strcpy(buf,strstorename);
  gen_load_addr(buf,0);
  gen_jsr("_chrstring");
  gen_pea(strstorename);
  enter_BSS(strstorelabel,"ds.b 2");
  return stringtype;
}

int argstr(int sftype) {
  make_sure_long(sftype); /* argument number */
  make_temp_string();  	/* destination buffer */
  gen_pea(tempstrname);
  gen_call("_arg",8);
  cli_args=TRUE;
  return stringtype;
}

int parse_gen_params(int type, const char * params) {
  while (*params && *params != ':') {
	switch(params[0]) {
	case 'i':
        insymbol();
	case 'E':
	  check_for_event();
	  break;
    case 'r':
      parse_rect();
      break;
	case 's':
	  if (type != stringtype) _error(4); return undefined;
	  break;
	case 'w':
	  if (make_sure_short(type) == undefined) return undefined;
	  break;
	case 'l':
	  if (make_sure_long(type) == undefined) return undefined;
	  break;
	case 'f':
	  if (type == stringtype) {
		_error(4); return undefined;
	  }
	  gen_Flt(type);
	  type = singletype;
	  break;
	case '#':
	  if (sym == hash) insymbol();
	  break;
	case ',':
	  if (sym != comma) { _error(16); return undefined; }
	  insymbol();
	  type = expr();
	  break;
	}
	params+=1;
	while (*params == ' ') params += 1;
  }
  return type;
}

int stringfunction()
{
  int  func;
  int  sftype=undefined;
  int  ntype=undefined;
  BOOL offset_on_stack;
 
 if (!strfunc()) return undefined;

 func=sym;
 insymbol();
 if (sym != lparen) _error(14);
 else {
   insymbol();
   sftype=expr();
   
   switch(func) {
   case chrstrsym: sftype = chrstr(sftype); break;
   case argstrsym: sftype = argstr(sftype); break;
   case cstrsym : /* p == pointer type */
	 if ((sftype == stringtype) || (sftype == longtype)) sftype=stringtype;
	 else { _error(4); sftype=undefined; }
	 break;
   case fileboxstrsym : /* s,s? */
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

   case hexsym  : /* i */
	 if (sftype != stringtype) {
	   sftype = make_integer(sftype);
	   if (sftype == longtype) gen_call_args("_hexstrlong","t0,d0 : a0",0);
	   else gen_call_args("_hexstrshort","t0,d0.w : a0",0);
	   sftype=stringtype;
	 } else { _error(4); sftype=undefined; }
	 break;
			  
   case inputboxsym :
   case inputboxstrsym : /* s,s?,s?,l?,l? */
	 if (sftype == stringtype) { 	/* prompt */ 	
	   /* all other parameters are optional */
	   opt_arg(stringtype,0); /* title */
	   opt_arg(stringtype,0); /* default value */
	   opt_arg(longtype,0);   /* xpos */
	   opt_arg(longtype,0);   /* ypos */
	   
	   if (func == inputboxsym) { /* INPUTBOX */
		 gen_call("_longint_input_box",20);
		 sftype = longtype;
	   } else { /* INPUTBOX$ */
		 gen_call("_string_input_box",20);
		 sftype = stringtype;
	   }
	 } else { _error(4); sftype=undefined; }
	 break;
			 
    /* INSTR$([I,]X$,Y$) */
   case instrsym  : /* l?, s, s */
	 if (sftype != stringtype) {
	   make_sure_long(sftype);
	   if (eat_comma()) {
		 offset_on_stack=TRUE;		/* optional offset I */
		 sftype=expr(); 
	   }
	 } else offset_on_stack=FALSE;

	 /* get X$ and Y$ */
	 if (sftype == stringtype) {
         if (eat_comma()) {
             if (expr() == stringtype) {
                 gen_pop_addr(1);		/* Y$ */
                 gen_pop_addr(0);		/* X$ */
                 if (offset_on_stack) gen_pop32d(0);	/* I */
                 else gen_load32d_val(1,0); /* I = 1 */
                 
                 gen_call("_instr",0); /* returns posn of Y$ in X$ */
                 sftype=longtype;
             } else { _error(4); sftype=undefined; }
         }
	 } else { _error(4); sftype=undefined; }
	 break;

   case saddsym :  
	 if ((sftype=parse_gen_params(sftype,"s")) != undefined) sftype = longtype; 
	 break;
				
   case ascsym:      sftype = gen_fcall("_asc",sftype,"s",shorttype,"a2 : d0.w",0); break;
   case binstrsym:   sftype = gen_fcall("_binstr",sftype,"l",stringtype,"t0,d0:a0",0); break;
   case inputstrsym: sftype = gen_fcall("_inputstrfromfile",sftype,"E l,#l", stringtype, "d0,d1",0); break;     
   case leftstrsym:  sftype = gen_fcall("_leftstr",sftype,"s,w",stringtype,"d0.w,a0,t1:a0",0); break;
   case lensym  :    sftype = gen_fcall("_strlen",sftype,"s",longtype,"a2",0); break;
   case octstrsym:   sftype = gen_fcall("_octstr",sftype,"l",stringtype,"t0,d0:a0",0); break;
   case rightstrsym: sftype = gen_fcall("_rightstr",sftype,"s,w",stringtype,"d0.w,a0,t1:a0",0); break;
   case spcsym:      sftype = gen_fcall("_spc",sftype,"w",stringtype,"d0.w,t0",0); break;
   case spacestrsym: sftype = gen_fcall("_spacestring",sftype,"w",stringtype,"d0.w,t0",0); break;
   case ptabsym:     sftype = gen_fcall("_ptab", sftype,"w",stringtype,":a0",0); break;
   case tabsym:      sftype = gen_fcall("_horiz_tab",sftype,"w",stringtype,":a0",0); break;
   case ucasestrsym: sftype = gen_fcall("_ucase",sftype,"s",stringtype,"a1,t0:a0",0); break;

   case strstrsym : /* n == numeric */
	 if (sftype != stringtype) {
	   load_temp_string(0);
	   if (sftype == longtype)       gen_call_args("_strlong","d0 : a0",0);
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
   case stringstrsym : /* w, a */
	 make_sure_short(sftype);
	 
	 if (eat_comma()) {
	   ntype=expr();
	   if (ntype == stringtype) {
		 gen_pop_addr(0);
		 gen_load_indirect(0,1);
		 gen_ext8to16(1);
		 gen_ext16to32(1); /* MID$(X$,1,1) */
	   } else {
		 make_sure_long(ntype);
		 gen_pop32d(1);	/* J */			
	   }
	   gen_call_args("_stringstr","d0.w,t0",0);
	   sftype=stringtype;
	 }
	 break;    

    /* MID$ -> MID$(X$,n[,m]) */
   case midstrsym :  /* s,w,w=-1 */
       if (sftype == stringtype) {
           if (eat_comma()) { /* start position */
               make_sure_short(expr());
               if (try_comma()) { /* character count */
                   gen_pop_as_short(expr(),1); /* char count */
               } else {
                   /* take the full length of the string */
                   gen_load16d_val(-1,1);
              }
               gen_call_args("_midstr","d0.w,a0,t1 : a0",0);
               sftype=stringtype;
           }
       } else { _error(4); sftype=undefined; }
       break;

   case translatestrsym : /* s */
       if (sftype == stringtype) {
           gen_translate();
           sftype=stringtype;
       } else { _error(4); sftype=undefined; }
       break;
       
	 
   case valsym : /* s */
	  if (sftype == stringtype) {
		gen_val();
		sftype=singletype;
	  } else { _error(4); sftype=undefined; } 
	  break;
   }

   if (sym != rparen) { _error(9); sftype=undefined; }
 }
 insymbol();
 return(sftype);
}

/* numeric functions */
int gen_single_func(void (* func)(),int nftype)
{
  
  if (nftype != stringtype)
	{
	  if (nftype != singletype) gen_Flt(nftype);  
	  gen_pop32d(0);
	  (*func)();
	  push_result(longtype);
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

int numericfunction() {
  int  func;
  int  nftype=undefined;
  char labname[80],lablabel[80];
  char varptr_obj_name[MAXIDSIZE];

  if (!numfunc()) return undefined;
  
  func=sym;
  insymbol();
  if (sym != lparen) {
	_error(14);
	insymbol();
	return undefined;
  }

  insymbol();
  if ((func != varptrsym) && (func != sizeofsym)) nftype=expr();
    
  switch(func) {
	
  case allocsym : /* l,l=9 */
	make_sure_long(nftype); /* minimum number of bytes to reserve */
	opt_arg(longtype, 9); /* memory type specification */
	nftype=longtype;
	gen_call("_ACEalloc",8);
	break;
	
  case cintsym : nftype = make_sure_short(nftype); break;
  case clngsym : nftype = make_sure_long(nftype); break;
  case csngsym : nftype = gen_Flt(singletype); break;

  case fixsym  : /* n */
	if (nftype == singletype) {
	  gen_fix();
	  nftype=longtype;
	} else if (nftype == stringtype) { _error(4); nftype=undefined; }
	/* else if short or long, leave on stack 
	   and let nftype remain the same! */
	break;
	
	
  case intsym  : /* n */
	if (nftype == singletype) {
	  gen_roundl();
	  nftype=longtype;
	} else if (nftype == stringtype) { _error(4); nftype=undefined; }
	/* else if short or long, leave on stack 
	   and let nftype remain the same! */
	break;
	
  case eofsym:     nftype = gen_fcall("_eoftest",nftype, "E l", longtype,":d0",0); break;
  case fresym:     nftype = gen_fcall("_fre",nftype, "w",longtype,":d0",0); break;
  case gadgetsym:  nftype = gen_fcall("_GadFunc",nftype,"l",longtype,":d0",4); break;
  case handlesym:  nftype = gen_fcall("_handle",nftype,"E l",longtype,":d0",4); break;
  case iffsym :    nftype = gen_fcall("_iff_func",nftype,"E l,l",longtype,":d0",8); break;
  case locsym:     nftype = gen_fcall("_FilePosition",nftype,"E l",longtype,":d0",4); break;
  case lofsym:     nftype = gen_fcall("_lof",nftype,"E l",longtype,":d0",4); break;
  case longintsym: nftype = gen_fcall("_long_from_string",nftype,"s",longtype,":d0",4); break;
  case menusym :   nftype = gen_fcall("_MenuFunc",nftype,"E l",longtype,":d0",4); break;
  case mousesym :  nftype = gen_fcall("_mouse",nftype,"w",shorttype,":d0.w",0); break;
  case potxsym :   nftype = gen_fcall("_potx",nftype,"w",shorttype, ":d0.w",0); break;
  case potysym :   nftype = gen_fcall("_poty",nftype,"w", shorttype,":d0.w",0); break;
  case serialsym:  nftype = gen_fcall("_serial_func",nftype,"E l,l",'l',"",8); break;
  case sticksym:   nftype = gen_fcall("_stick",nftype, "w",shorttype,":d0.w",0); break;
  case strigsym:   nftype = gen_fcall("_strig",nftype, "w",shorttype,":d0.w",0); break;
  case windowsym:  nftype = gen_fcall("_windowfunc",nftype,"w",longtype,":d0",0); break;
  case saysym:     nftype = gen_fcall("_sayfunc",nftype,"l",longtype, "",4); break;
  case screensym:  nftype = gen_fcall("_screenfunc",nftype,"l",longtype,"",4); break;

  case msgboxsym : /* s,s,s=0 */ 
	if (nftype == stringtype) {    /* message */
        if (eat_comma()) {
            if (expr() == stringtype) {  /* response #1 */
                opt_arg(stringtype,0); /* response #2 */
                gen_call_args("_sysrequest",":d0.w",12);
                nftype=shorttype;
            } else { _error(4); nftype=undefined; }
        }
	} else { _error(4); nftype=undefined; }
	break;
	
  case peeksym : /* i */
	nftype=make_integer(nftype);
	if ((nftype == longtype) || (nftype == shorttype)) {
	  gen_peek(nftype);
	  nftype=shorttype;
	} else { _error(4); nftype=undefined; }
	break;

	
  case peekwsym : /* i */
	nftype=make_integer(nftype); 
	if ((nftype == longtype) || (nftype == shorttype)) {
	  /* get address */
	  if (nftype == shorttype) gen_pop_short_addr(0,0);
	  else gen_pop_addr(0); 
	  /* get value */
	  gen_push_indirect(shorttype);
	  nftype=shorttype;
	}
	break;
	
  case peeklsym : /* i */
	nftype=make_integer(nftype); 
	if ((nftype == longtype) || (nftype == shorttype)) {
	  /* get address */
	  if (nftype == shorttype) gen_pop_short_addr(0,0);
	  else gen_pop_addr(0); 
	  /* get value */
	  gen_push_indirect(longtype);
	  nftype=longtype;
	}			
	break;
	
  case sgnsym  : /* n */
	if      (nftype == shorttype) nftype = gen_fcall("_sgnw", nftype, "w",longtype,":d0",0);
	else if (nftype == longtype)  nftype = gen_fcall("_sgnl", nftype, "l",longtype,":d0",0);
	else if (nftype == singletype) {
	  gen_call_args("_sgnf","d1 : d0", 0);
	  enter_XREF("_MathBase");
	  nftype=longtype;
	} else { 
	  _error(4); nftype=undefined;
	}
	break; 

  case abssym : /* n */
	if (nftype == shorttype) gen_call_args("_absw","d0.w : d0.w",0);
	else if (nftype == longtype) gen_call_args("_absl","d0 : d0",0);
	else if (nftype == singletype) {
	  gen_call_args("_absf", "d0 : d0",0);
	  enter_XREF("_MathBase");
	} else { _error(4); nftype=undefined; }
	break;
	
	
  case pointsym :	/* w,w */
	if ((nftype = parse_gen_params(nftype,"w,w"))  != undefined) gen_point();
	break;
	
  case shlsym  : /* l,l */
	if ((nftype = parse_gen_params(nftype,"l,l")) != undefined) gen_shl();
	break;
	
  case shrsym  : /* l, l */
	if ((nftype = parse_gen_params(nftype,"l,l")) != undefined) gen_shl(); 
	break;

  case atnsym: nftype = gen_single_func(&gen_atan,nftype); break;
  case cossym: nftype = gen_single_func(&gen_cos,nftype);
  case expsym: nftype = gen_single_func(&gen_exp,nftype); break;
  case sqrsym: nftype = gen_single_func(&gen_sqrt,nftype); break;
  case sinsym: nftype = gen_single_func(&gen_sin,nftype); break;
  case tansym: nftype = gen_single_func(&gen_tan,nftype); break;
  case logsym: nftype = gen_single_func(&gen_log,nftype); break;

  case sizeofsym: nftype = find_object_size(); break;
	
  case varptrsym : /* ? */
	if (sym == ident) {
	  strcpy(varptr_obj_name,id);
	  nftype=address_of_object();
	  /* structure and array code returns next symbol */
	  if (!exist(varptr_obj_name,structure) &&
		  !exist(varptr_obj_name,array)) 
		insymbol();
	} else { _error(7); nftype=undefined; insymbol(); }
	break;
	
  }

  if (sym != rparen) { _error(9); nftype=undefined; }
  insymbol();
  return(nftype);
}

int address_of_object() {
  /* return the address of a variable, array or structure */
  SYM    *varptr_item;
  char   buf[50],addrbuf[40];
  char   extobjid[MAXIDSIZE];
  char   subname[MAXIDSIZE+5];
  SYM    *structype;
  STRUCM *member;
  BOOL   found;

  /* 
  ** Make external variable/function name by removing qualifier and 
  ** adding an underscore prefix if one is not present. 
  */
  strcpy(buf,ut_id);
  remove_qualifier(buf);
  if (buf[0] != '_') {
	strcpy(extobjid,"_\0");
	strcat(extobjid,buf);
  } else strcpy(extobjid,buf);

  /* Make SUB name. */
  sprintf(subname,"_SUB_%s",id);

  /*
  ** Push address of valid object [see ref.doc].
  */

  /* external variable or function? */
  if (exist(extobjid,extvar) || 
	  exist(extobjid,extfunc)) {
	gen_pea(extobjid);
	return(longtype);			      
  } else if (exist(subname,subprogram)) {
	gen_pea(subname);
	return(longtype);
  } else if (exist(id,variable)) { /* ordinary variable? */
	varptr_item=curr_item;
	
	/* get the frame start address */
	strcpy(addrbuf,addreg[lev]);
	
	/* calculate the absolute address */
	gen_load32d(addrbuf,0);
	gen_sub32d_val(varptr_item->address,0); /* frame offset */
	if ((varptr_item->type == stringtype)
		|| ((varptr_item->shared) && (lev == ONE)))
	  {
		/* location in frame contains address */  
		gen_move32da(0,0);
		gen_push_indirect(longtype);
	  } else
		/* absolute address in frame of variable */
		push_result(longtype);
	return(longtype);    
  } else if ((exist(id,array)) || (exist(id,structure))) {
	varptr_item=curr_item;
	
	/* get the frame start address */
	strcpy(addrbuf,addreg[lev]);
	
	/* calculate the absolute address */
	gen_load32d(addrbuf,0);
	gen_sub32d_val(varptr_item->address,0); /* frame offset */
	
	/* location in frame contains array/struct address 
	   (except for shared structure (see below) */  
	gen_move32da(0,0);
			    
	/* address of a structure member? */
	if (exist(id,structure)) {
	  /* shared struct? -> get struct variable address */
	  if (varptr_item->shared && lev == ONE) gen_load_indirect_addr(0,0);
		
	  insymbol();  
	  if (sym == memberpointer) {
		insymbol();  
		if (sym != ident) _error(7);
		else {
		  structype = varptr_item->other;
		  member = structype->structmem->next;
		  found=FALSE;
		  while ((member != NULL) && (!found)) {
			if (strcmp(member->name,id) == 0) found=TRUE;
			else member = member->next;
		  }
		  if (!found) _error(67);  /* not a valid member */
		  else {
			/* push address of struct member */
			gen_load_indirect_addr(0,0);
			gen_add_addr_offset((long)member->offset);
			gen_push_addr(0);
			/* store type for SWAP command */
			struct_member_type = member->type;	  	
		  }
		}
		insymbol();
	  } else {
		/* address of struct variable in stack frame */
		gen_push_addr(0); 
		/* store type for SWAP command */
		struct_member_type = longtype;
	  }	  	
	} else {
	  /* array or array element address? */
	  /* push array address */
	  gen_push_indirect(longtype);
	  
	  insymbol();
	  
	  if (sym == lparen) {
		/* calculate array element address */
		have_lparen=TRUE;
		push_indices(varptr_item);
		get_abs_ndx(varptr_item); /* offset -> d7 */
		gen_pop32d(SCRATCH_REG); /* array start */
		gen_add32dd(OFFSET_REG,SCRATCH_REG); /* start+offset=addr */
		push_result(longtype);
		insymbol(); /* symbol after rparen */
	  }
	}
	return(longtype);	    
  }
  else { _error(43); return(undefined); }
}

/* push the size (in bytes)  of a data object or type onto the stack.  */
static int find_object_size() {
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
