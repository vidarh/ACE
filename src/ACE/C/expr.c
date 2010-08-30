/* << ACE >>

   -- Amiga BASIC Compiler --

   ** Parser: Expression code **
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
     Date: 26th October-30th November, 1st-13th December 1991,
	   14th,20th-27th January 1992, 
           2nd-17th, 21st-29th February 1992, 
	   1st,13th,14th,22nd,23rd March 1992,
	   21st,22nd April 1992,
	   2nd,3rd,11th,15th,16th May 1992,
	   7th,8th,9th,11th,13th,14th,28th,29th,30th June 1992,
	   2nd-8th,14th-19th,26th-29th July 1992,
	   1st-3rd,7th,8th,9th August 1992,
	   6th,7th,21st December 1992,
	   28th February 1993,
	   12th June 1993,
	   12th,29th July 1994,
	   5th,6th November 1995
*/

#include "acedef.h"
#include "codegen.h"

/* externals */
extern	int	sym;
extern	int	factype;
extern	int	negtype;
extern	int	prodtype;
extern	int	idivtype;
extern	int	modtype;
extern	int	simptype;
extern	int	nottype;
extern	int	andtype;
extern	int	ortype;
extern	int	eqvtype;
extern	CODE	*curr_code;
extern	int	labelcount;
extern	char	tempstrname[80];

/* functions */
BOOL coerce(typ1,typ2,cx)
int  *typ1;
int  *typ2;
CODE *cx[];
{
 if ((*typ1 == stringtype) && (*typ2 != stringtype)) return(FALSE);
 else
 if ((*typ2 == stringtype) && (*typ1 != stringtype)) return(FALSE);
 else
 if (((*typ1 == shorttype) || (*typ1 == longtype)) && (*typ2 == singletype))
 {
  change_Flt(*typ1,cx);
  *typ1=singletype;
  return(TRUE);
 }
 else
 if (((*typ2 == shorttype) || (*typ2 == longtype)) && (*typ1 == singletype))
 {
  gen_Flt(*typ2);
  *typ2=singletype;
  return(TRUE);
 }
 else
 if ((*typ1 == shorttype) && (*typ2 == longtype)) 
 {
  change(cx[0],"move.w","(sp)+","d0");
  change(cx[1],"ext.l","d0","  ");
  change(cx[2],"move.l","d0","-(sp)");
  *typ1=longtype;
  return(TRUE);
 }
 else
 if ((*typ2 == shorttype) && (*typ1 == longtype)) 
 {
   make_long();
   *typ2=longtype;
   return(TRUE);
 }
 else
     return(TRUE); /* both shorttype, longtype or singletype OR notype! */
}

void make_short() {
 gen_pop32d(0);
 gen_push16d(0);
}

void make_long() {
 gen_pop16d(0);
 gen_ext16to32(0);
 gen_push32d(0);
}

int ptr_term()
{
/* pointer operators -- higher precedence
   than unary negation and exponentiation. */

int  localtype,op;
BOOL dereference=FALSE;

  if (sym == shortpointer || sym == longpointer || sym == singlepointer)
  {
   op=sym;
   dereference=TRUE;
   insymbol();
  }
  
  localtype=factor();

  if (dereference)
  {
   if (localtype != longtype)
   {
    _error(4);
    localtype=undefined;
   }
   else
   {
    /* store address into a0 */
    gen_pop_addr(0);

    /* get value at address in a0 */    
    switch(op)
    {
     /* *%<address> */	
	case shortpointer : gen_push_indirect16(0);
 		         localtype=shorttype;
		         break;

     /* *&<address> */	
     case longpointer  : gen_push_indirect32(0);
		         localtype=longtype;
		         break;

     /* *!<address> */	
     case singlepointer :gen_push_indirect32(0);
		         localtype=singletype;
		         break;
    }
   }
  }
  
 return(localtype);
}

int expterm()
{
/* exponentiation -> ALWAYS returns single */
int  i;
int  firsttype,original_firsttype,localtype,coercedtype;
BOOL coercion;
CODE *cx[5];

 firsttype=ptr_term();
 localtype=firsttype;
 
 while (sym == raiseto)
 {
  if ((firsttype == shorttype) || (firsttype == longtype))
  {
   /* may need to coerce to singletype */
   for (i=0;i<=4;i++)
   {
    gen_nop();
    cx[i]=curr_code;
   }
  }

  insymbol();
  factype=ptr_term();
  if (factype == undefined) return(factype);
  original_firsttype=firsttype; /* in case of SHORT->single coerce later */
  coercion=coerce(&firsttype,&factype,cx);
    
  if (coercion)
  {
   coercedtype=factype;

   /* already singletype? */
   if (coercedtype != singletype)
   {
    /* neither operands are singletype */ 
    change_Flt(original_firsttype,cx); /* handles SHORT->single coerce */
    gen_Flt(factype);
   }

   gen_call("_power",8);	/* - Call exponentiation function. */
   enter_XREF("_MathTransBase"); /* opens FFP+IEEE SP transcendental libraries */

   localtype=singletype;  /* MUST always return a single-precision value
			     because exponent might be -ve! */ 
  } 
  else _error(4); /* type mismatch */

  firsttype=localtype;  /* moving record of last sub-expression type */
 }
 return(localtype);
}

int negterm()
{
int  localtype;
BOOL negate=FALSE;

 /* unary negation? */
 if (sym == minus) negate=TRUE;
 if ((sym == minus) || (sym == plus)) insymbol();
 
 localtype=expterm();
 if (localtype == undefined) return(localtype);

 if (negate)
 {
  switch(localtype)
  {
   case shorttype  : gen_neg16sp(); break;
   case longtype   : gen_neg16sp(); break;
   case singletype : gen_pop32d(0); 
       gen_libbase("Math");
       gen_libcall("SPNeg","Math");
       gen_push32d(0);
       break;
   case stringtype : _error(4); break;
  }
 }
 return(localtype);
}

int prodterm()
{
/*
   multiplication     	    -> returns long or single
   floating-point division  -> returns single
*/
int  op,i;
int  firsttype,original_firsttype,localtype,coercedtype;
BOOL coercion;
CODE *cx[5];

 firsttype=negterm();
 localtype=firsttype;
 
 while ((sym == multiply) || (sym == fdiv))
 {
  if ((firsttype == shorttype) || (firsttype == longtype))
  {
   /* may need to coerce to singletype or longtype */
   for (i=0;i<=4;i++)
   {
    gen_nop();
    cx[i]=curr_code;
   }
  }

  op=sym;  /* multiply or fdiv ? */

  insymbol();
  negtype=negterm();

  if (negtype == undefined) return(negtype);

  /* save firsttype in case of a SHORT->single coercion 
     later because coercion will need to start over again 
     to avoid short->single code overwriting short->long code */

  original_firsttype=firsttype; 
  coercion=coerce(&firsttype,&negtype,cx);
    
  /* if not stringtype, perform operation */
  if (coercion) 
  {
   coercedtype=negtype;

   /* make sure operands are singletype if FFP division */
   if ((op == fdiv) && (coercedtype != singletype))
   {
    /* neither operands are singletype 
       -> both short or long */ 
    change_Flt(original_firsttype,cx); /* handles SHORT->single correctly */
    gen_Flt(negtype);
    coercedtype=singletype;
   }

   if ((op == multiply) && (coercedtype != longtype)) 
      pop_operands(coercedtype);

   switch(op)
   {
    case multiply : switch(coercedtype)
		    {
		     case shorttype  :  gen_muls();
					localtype=longtype;
					break;
			
		     case longtype   :	/* args on stack */
					gen_call_void("lmul",8); 
					localtype=longtype;
					break;

			case singletype :  
			  gen_libbase("Math");
			  gen_libcall("SPMul","Math");
			  localtype=singletype;
			  break;
		    }
		    break;

    case fdiv:
	  gen_pop32d(1);  /* 2nd operand */
	  gen_pop32d(0);  /* 1st operand */
	  gen_libbase("Math");
	  gen_libcall("SPDiv","Math");  
	  localtype=singletype;
	  break;
   }

   push_result(localtype);  /* result */

  } 
  else _error(4); /* notype -> type mismatch */

  firsttype=localtype;  /* moving record of last sub-expression type */
 }
 return(localtype);
}

int idivterm()
{
/* integer division -- LONG = LONG \ LONG */
int  i;
int  firsttype,localtype;
int  targettype=longtype;
CODE *cx[5];

 firsttype=prodterm();
 localtype=firsttype;

 while (sym == idiv) 
 {
  firsttype=make_integer(firsttype);  /* short or long -> 1st approximation */ 

  /* may need to coerce to long */
  for (i=0;i<=2;i++)
  {
   gen_nop();
   cx[i]=curr_code;
  }

  if (firsttype == undefined) return(firsttype);

  coerce(&firsttype,&targettype,cx);  /* make sure it's a long dividend */
  localtype=firsttype;

  insymbol();
  prodtype=prodterm();
  if (prodtype == undefined) return(prodtype); 
  prodtype=make_integer(prodtype);  /* short or long at first */

  if ((firsttype != notype) && (prodtype != notype) &&
      (firsttype != stringtype) && (prodtype != stringtype))
  {
   if (prodtype == shorttype) make_long();  /* ensure that divisor is LONG! */
   prodtype=longtype;
   localtype=prodtype;

   /* integer division - args on stack */
   gen_call("ace_ldiv",8);
  }
  else _error(4); /* notype -> type mismatch */
  firsttype=localtype;  /* moving record of last sub-expression type */
 }
 return(localtype);
}

int modterm()
{
/* modulo arithmetic -> returns remainder of long integer or FFP division */
int  i;
int  firsttype,localtype;
int  targettype=longtype;
CODE *cx[5];

 firsttype=idivterm();
 localtype=firsttype;

 while (sym == modsym) 
 {
  /* may need to coerce to single */
  for (i=0;i<=4;i++)
  {
   gen_nop();
   cx[i]=curr_code;
  }

  if (firsttype == undefined) return(firsttype);

  insymbol();
  idivtype=idivterm();

  if (idivtype == undefined) return(idivtype);

  /* perform integer or FFP modulo operation */
  if ((firsttype != notype) && (idivtype != notype) &&
      (firsttype != stringtype) && (idivtype != stringtype))
  {
   /* dividend (firsttype) is either short, long or single */

   if ((firsttype == singletype) || (idivtype == singletype))
   {
    /* single MOD */
    coerce(&firsttype,&idivtype,cx);
    /***************/
   }
   else 
   {
    /* integer MOD */
    if (idivtype == shorttype)
    {
     make_long();  /* ensure that divisor is LONG! */
     idivtype=longtype;
    }
    
    if (firsttype == shorttype)
       coerce(&firsttype,&targettype,cx);
    /***************/
   }

   localtype=idivtype;  /* short or single */

   if (localtype == longtype) {
    /* integer MOD - args on stack */
    gen_call("ace_lrem",8);
   } else {
    /* single MOD */
    gen_pop32d(1);   /* divisor */
    gen_pop32d(0);   /* dividend */
    gen_call("_modffp",0);
    enter_XREF("_MathBase");
    localtype=singletype;
   }
  }
  else _error(4); /* notype -> type mismatch */
  firsttype=localtype;  /* moving record of last sub-expression type */
 }
 return(localtype);
}

int simple_expr()
{
int  op,i;
int  firsttype,localtype;
BOOL coercion;
CODE *cx[5];

 firsttype=modterm();
 localtype=firsttype;

 while ((sym == plus) || (sym == minus))
 {
  if ((firsttype == shorttype) || (firsttype == longtype))
  {
   /* may need to coerce */
   for (i=0;i<=4;i++)
   {
    gen_nop();
    cx[i]=curr_code;
   }
  }
  op=sym;
  insymbol();
  modtype=modterm();
  if (modtype == undefined) return(modtype);
  coercion=coerce(&firsttype,&modtype,cx);
  localtype=modtype;
  if (coercion)
  {
   switch(op)
   {
    case plus :  if ((modtype != stringtype) && (modtype != shorttype))
   {
      gen_pop32d(1);
    gen_pop32d(0);
   }

   switch(modtype)
   {
    case shorttype  : 	gen_pop16d(1);
          		gen_pop16d(0);
            		gen_add16dd(1,0);
                	break;
 
    case longtype   :	gen_add32dd(1,0);
    		    	break;

    case singletype : 	
	  gen_libbase("Math");
	  gen_libcall("SPAdd","Math");
	  break;

    case stringtype : 	/* copy source to temp string */
        		gen_pop_addr(2); /* 2nd */
        		gen_call_args("_strcpy","a1,t0",0);
        		/* prepare for strcat */
        		gen_load_addr(tempstrname,0);
        		gen_move32aa(2,1);
        		gen_jsr("_strcat");
        		gen_pea(tempstrname);
        		break;
   }
  
   if (modtype == shorttype)
            gen_push16d(0);
   else
   if (modtype != stringtype)
      gen_push32d(0);
                 break;

    case minus : if ((modtype != stringtype) && (modtype != shorttype))
   {
    gen_pop32d(1);
    gen_pop32d(0);
   }

   switch(modtype)
   {
    case shorttype  : 	gen_pop16d(1);
   		     	gen_pop16d(0);
        		gen_sub16dd(1,0);
                	break;
 
    case longtype   : 	gen_sub32dd(1,0);
         		break;

    case singletype :	
	  gen_libbase("Math");
	  gen_libcall("SPSub","Math");
	  break;

    case stringtype : 	_error(4); break;
  }
  
   	if (modtype == shorttype)
     		gen_push16d(0);
   	else
   	    if (modtype != stringtype)
      		gen_push32d(0);
     }
    } 
    else _error(4); /* notype -> type mismatch */
   firsttype=localtype;  /* moving record of last sub-expression type */
  }
 return(localtype);
}

BOOL relop(op)
int op;
{
 if ((op == equal) || (op == notequal) || (op == gtrthan) || 
     (op == lessthan) || (op == gtorequal) || (op == ltorequal))
     return(TRUE);
 else
     return(FALSE);
}

void make_label(name,lab)
char *name;
char *lab;
{
char num[40];
 
 strcpy(name,"_lab");
 itoa(labelcount++,num,10);
 strcat(name,num);
 strcpy(lab,name);
 strcat(lab,":\0");
} 

int relexpr() {
  /* relational expression -> pass through this only ONCE */
  int  i,op=undefined;
  int  firsttype,localtype;
  char labname[80],lablabel[80],branch[6];
  BOOL coercion;
  CODE *cx[5];

 firsttype=simple_expr();
 localtype=firsttype;

 if (relop(sym)) {
   if ((firsttype == shorttype) || (firsttype == longtype)) {
   /* may need to coerce */
	for (i=0;i<=4;i++) {
	  gen_nop();
	  cx[i]=curr_code;
	}
  }
  op=sym;
  insymbol();
  simptype=simple_expr();
  if (simptype == undefined) return(simptype);
  coercion=coerce(&firsttype,&simptype,cx);
  localtype=simptype;
  if (coercion) {
   /* compare on basis of type -> d5 = d0 op d1 */
   switch(simptype) {
    case shorttype  : 	
	  gen_pop16d(1);  /* 2nd */
	  gen_pop16d(0);  /* 1st */
	  gen_load32d_val(-1,5); /* assume true */
	  gen_cmp16dd(1,0);
	  break;

    case longtype:
	  gen_pop32d(1);  /* 2nd */
	  gen_pop32d(0);  /* 1st */
	  gen_load32d_val(-1,5); /* assume true */
	  gen_cmp32dd(1,0);
	  break;

    case singletype : 	
	  gen_pop32d(1);  /* 2nd */
	  gen_pop32d(0);  /* 1st */
	  gen_load32d_val(-1,5); /* assume true */
	  gen_libbase("Math");
	  gen_libcall("SPCmp","Math");
	  break;

    case stringtype :
	  switch(op) {
	  case equal     : gen_call_args("_streq","a1,a0:d0",0); break;
	  case notequal  : gen_call_args("_strne","a1,a0:d0",0); break;
	  case lessthan  : gen_call_args("_strlt","a1,a0:d0",0); break;
	  case gtrthan   : gen_call_args("_strgt","a1,a0:d0",0); break;
	  case ltorequal : gen_call_args("_strle","a1,a0:d0",0); break;
	  case gtorequal : gen_call_args("_strge","a1,a0:d0",0); break;
	  }
	  break;
    }

   /* leave result on stack according to operator (-1 = true, 0 = false) */
   /* (this code for short,long & single comparisons only) */
   if (simptype != stringtype) {	
     make_label(labname,lablabel);
	 gen_bxx(op,labname);
     gen_load32d_val(0,5);  /* not true */
     gen_label(lablabel);
     gen_push32d(5); /* boolean result on stack */
   }
  } else _error(4);
 }
 
 return (op == undefined) ? localtype : longtype; /* BOOLEAN! */
}

int notexpr() {
int localtype,op;

 op=sym;
 if (sym == notsym) insymbol();

 localtype=relexpr();

 if (op == notsym) {
   localtype=make_integer(localtype);
   if (localtype == notype) return(localtype);
   if (localtype == shorttype) gen_not16sp();
   else gen_not32sp();
 }
 return(localtype);
}

int andexpr() {
  int  op,i;
  int  firsttype,localtype;
  CODE *cx[5];
  
  firsttype=notexpr();
  localtype=firsttype;
  
  if (sym == andsym) {
  firsttype=make_integer(firsttype);  
  if (firsttype != notype) {
   while (sym == andsym) {
    if ((firsttype == shorttype) || (firsttype == longtype)) {
	  /* may need to coerce */
	  for (i=0;i<=4;i++) {
		gen_nop();
		cx[i]=curr_code;
	  }
    }
    op=sym;
    insymbol();
    nottype=notexpr(); 
    if (nottype == undefined) return(nottype);
    nottype=make_integer(nottype);
    coerce(&firsttype,&nottype,cx);
    localtype=nottype;
    if (nottype != notype) {
	  pop_operands(nottype);
	  if (nottype == shorttype)  gen_and16dd(1,0);
	  else gen_and32dd(1,0);
	  push_result(nottype);
    } else _error(4);
   }
  } else _error(4);
  firsttype=localtype;
  }
 return(localtype);
}

int orexpr() {
  int  op,i;
  int  firsttype,localtype;
  CODE *cx[5];
  
  firsttype=andexpr();
  localtype=firsttype;
  
  if ((sym == orsym) || (sym == xorsym)) {
	firsttype=make_integer(firsttype);  
	if (firsttype != notype) {
	  while ((sym == orsym) || (sym == xorsym)) {
		if ((firsttype == shorttype) || (firsttype == longtype)) {
		  /* may need to coerce */
		  for (i=0;i<=4;i++) {
			gen_nop();
			cx[i]=curr_code;
		  }
		}
		op=sym;
		insymbol();
		andtype=andexpr();
		if (andtype == undefined) return(andtype); 
		andtype=make_integer(andtype);
		coerce(&firsttype,&andtype,cx);
		localtype=andtype;
		if (andtype != notype) {
		  pop_operands(andtype);  
		  switch(op) {
		  case orsym  : 
			if (andtype == shorttype) gen_or16dd(1,0);
			else gen_or32dd(1,0);
			break;
		  case xorsym : 
			if (andtype == shorttype) gen_eor16dd(1,0);
			else gen_eor32dd(1,0);
			break;
		  }
		  push_result(andtype);
		} else _error(4);
	  }
	} else _error(4);
	firsttype=localtype;
  }
  return(localtype);
}

int eqvexpr() {
  int  op,i;
  int  firsttype,localtype;
  CODE *cx[5];
  
  firsttype=orexpr();
  localtype=firsttype;

  if (sym == eqvsym) {
	firsttype=make_integer(firsttype);  
	if (firsttype != notype) {
	  while (sym == eqvsym) {
		if ((firsttype == shorttype) || (firsttype == longtype)) {
		  /* may need to coerce */
		  for (i=0;i<=4;i++) {
			gen_nop();
			cx[i]=curr_code;
		  }
		}
		op=sym;
		insymbol();
		ortype=orexpr(); 
		if (ortype == undefined) return(ortype);
		ortype=make_integer(ortype);
		coerce(&firsttype,&ortype,cx);
		localtype=ortype;
		if (ortype != notype) {
		  pop_operands(ortype);
		  if (ortype == shorttype) gen_jsr("_eqvw");
		  else gen_jsr("_eqvl");
		  push_result(ortype);
		} else _error(4);
	  }
	} else _error(4);
	firsttype=localtype;
  }
  return(localtype);
}

int expr() {
  int  op,i;
  int  firsttype,localtype;
  CODE *cx[5];
  
  firsttype=eqvexpr();
  localtype=firsttype;
  
  if (sym == impsym) {
	firsttype=make_integer(firsttype);  
	if (firsttype != notype) {
	  while (sym == impsym) {
		if ((firsttype == shorttype) || (firsttype == longtype)) {
		  /* may need to coerce */
		  for (i=0;i<=4;i++) {
			gen_nop();
			cx[i]=curr_code;
		  }
		}
		op=sym;
		insymbol();
		eqvtype=eqvexpr(); 
		if (eqvtype == undefined) return(eqvtype);
		eqvtype=make_integer(eqvtype);
		coerce(&firsttype,&eqvtype,cx);
		localtype=eqvtype;
		if (eqvtype != notype) {
		  pop_operands(eqvtype);
		  if (eqvtype == shorttype) gen_jsr("_impw");
		  else gen_jsr("_impl");
		  push_result(eqvtype);
		} else _error(4);
	  }
	} else _error(4);
	firsttype=localtype;
  }
  return(localtype);
}

void pop_operands(int typ) {
  if (typ == shorttype) {
	gen_pop16d(0);  /* 2nd operand */
	gen_pop16d(1);  /* 1st operand -> d0 = d1 op d0 */
  } else {
	gen_pop32d(0);  /* 2nd operand */
	gen_pop32d(1);  /* 1st operand -> d0 = d1 op d0 */
  } 
}

void push_result(int typ) {
 if (typ == shorttype) gen_push16d(0);
 else gen_push32d(0);
}

void gen_round(int type) {  
  /*
  ** Convert float to integer
  ** with rounding.
  */
  gen_call_args("_round","d0:d0",0);
  enter_XREF("_MathBase");

  /*
  ** Only relevant when called from
  ** assign_coerce() and STOREType=shorttype.
  */
  if (type == shorttype) {
   gen_pop32d(0);
   gen_push16d(0);
  }
}  
 
/* convert an integer to a single-precision float */
int gen_Flt(int typ) {
  if (typ == singletype) return singletype;  /* already a float! */
  if (typ == stringtype) {_error(4); return undefined; } /* can't do it */
  if (typ == shorttype) gen_pop16d(0);
  else gen_pop32d(0);
  if (typ == shorttype) gen_ext16to32(0); /* extend sign */
  gen_libbase("Math");
  gen_libcall("SPFlt","Math");
  gen_push32d(0);
  return singletype;
}

void change_Flt(int exptyp,CODE * cx[]) {
  /* convert an integer to a float */
  if (exptyp == shorttype) change(cx[0],"move.w","(sp)+","d0");
  else change(cx[0],"move.l","(sp)+","d0");
  if (exptyp == shorttype) change(cx[1],"ext.l","d0","  ");
  change(cx[2],"move.l","_MathBase","a6");
  change(cx[3],"jsr","_LVOSPFlt(a6)","  ");
  change(cx[4],"move.l","d0","-(sp)");
  enter_XREF("_LVOSPFlt");
  enter_XREF("_MathBase");
}

int make_integer(int oldtyp) {
  if (oldtyp == stringtype) return(notype); /* can't do it! */
  else if (oldtyp == singletype) { 
	gen_round(oldtyp);
	return(longtype); 
  }
  return(oldtyp);  /* already an integer */
}

int make_sure_short(int type) {
 if (type == longtype) make_short();
 else if (type == singletype) { make_integer(type); make_short(); }
 else if (type == stringtype) { _error(4); return undefined; }
 return shorttype;
}

int gen_pop_as_short(int type, unsigned char reg) {
  if (type == singletype) type = make_integer(type);
  if (type == longtype) gen_pop32d(reg);
  else if (type == shorttype) { gen_pop16d(reg); }
  else if (type == stringtype) { _error(4); return undefined; }
  return shorttype;
}

int make_sure_long(int type) {
 if      (type == shorttype) make_long();
 else if (type == singletype) make_integer(type);
 else if (type == stringtype) { _error(4); return undefined; }
 return longtype;
}
