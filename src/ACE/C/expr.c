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
extern	CODE	*curr_code;
extern	int	labelcount;
extern	char	tempstrname[80];

/* functions */
BOOL coerce(int * typ1,int * typ2,CODE * cx[])
{
 if ((*typ1 == stringtype) && (*typ2 != stringtype)) return(FALSE);
 if ((*typ2 == stringtype) && (*typ1 != stringtype)) return(FALSE);
 if (((*typ1 == shorttype) || (*typ1 == longtype)) && (*typ2 == singletype)) {
   change_Flt(*typ1,cx);
   *typ1=singletype;
   return(TRUE);
 }
 if (((*typ2 == shorttype) || (*typ2 == longtype)) && (*typ1 == singletype)) {
   gen_Flt(*typ2);
   *typ2=singletype;
   return(TRUE);
 }
 if ((*typ1 == shorttype) && (*typ2 == longtype))  {
     coerce_word_to_long(cx);
   *typ1=longtype;
   return(TRUE);
 }
 if ((*typ2 == shorttype) && (*typ1 == longtype))  {
   make_long();
   *typ2=longtype;
   return(TRUE);
 }
 return(TRUE); /* both shorttype, longtype or singletype OR notype! */
}

static int ptr_term()
{
  /* pointer operators -- higher precedence
	 than unary negation and exponentiation. */

  int  localtype,op;
  BOOL dereference=FALSE;

  if (sym == shortpointer || sym == longpointer || sym == singlepointer) {
	op=sym;
	dereference=TRUE;
	insymbol();
  }
  
  localtype=factor();

  if (dereference) {
   if (localtype != longtype) {
	 _error(4);
	 localtype=undefined;
   } else {
	 /* store address into a0 */
	 gen_pop_addr(0);

	 /* get value at address in a0 */    
	 switch(op) {
	 case shortpointer : /* *%<address> */	
	   localtype=shorttype;
	   break;
     case longpointer  :  /* *&<address> */	
	   localtype=longtype;
	   break;
     case singlepointer : /* *!<address> */	
	   localtype=singletype;
	   break;
	 }
     gen_push_indirect(localtype);
   }
  }
  
  return(localtype);
}

static void alloc_coerce_space(int type,CODE ** cx, int num)
{
  int i;
  if ((type == shorttype) || (type == longtype)) {
	/* may need to coerce */
	for (i=0;i< num;i++) {
	  gen_nop();
	  cx[i]=curr_code;
	}
  }
}

static int expterm()
{
  /* exponentiation -> ALWAYS returns single */
  int  firsttype,original_firsttype,localtype,coercedtype, factype;
  BOOL coercion;
  CODE *cx[5];

  firsttype=ptr_term();
  localtype=firsttype;
 
  while (sym == raiseto) {
	alloc_coerce_space(firsttype,cx,5);

	insymbol();
	factype=ptr_term();
	if (factype == undefined) return(factype);
	original_firsttype=firsttype; /* in case of SHORT->single coerce later */
	coercion=coerce(&firsttype,&factype,cx);
    
	if (coercion) {
	  coercedtype=factype;

	  /* already singletype? */
	  if (coercedtype != singletype) {
		/* neither operands are singletype */ 
		change_Flt(original_firsttype,cx); /* handles SHORT->single coerce */
		gen_Flt(factype);
	  }

	  gen_power();

	  localtype=singletype;  /* MUST always return a single-precision value
								because exponent might be -ve! */ 
	} else _error(4); /* type mismatch */

	firsttype=localtype;  /* moving record of last sub-expression type */
  }
  return(localtype);
}

static int negterm()
{
  int  localtype;
  BOOL negate=FALSE;

  /* unary negation? */
  if (sym == minus) negate=TRUE;
  if (!eat(minus)) eat(plus);
  
  localtype=expterm();
  if (localtype == undefined) return(localtype);
  
  if (negate) {
	if (localtype == stringtype) _error(4);
	else gen_neg(localtype);
  }
  return(localtype);
}


static int generic_expr(int mysym, int (* subexpr)(), void(* gen)(int)) {
  int  firsttype,localtype;
  CODE *cx[5];
  
  firsttype=subexpr();
  localtype=firsttype;

  if (sym == mysym) {
	firsttype=make_integer(firsttype);  
	if (firsttype != notype) {
	  while (sym == mysym) {
		alloc_coerce_space(firsttype,cx,5);
		insymbol();
		localtype=subexpr(); 
		if (localtype == undefined) return undefined;
		localtype=make_integer(localtype);
		coerce(&firsttype,&localtype,cx);
		if (localtype != notype) {
		  pop_operands(localtype);
		  gen(localtype);
		  push_result(localtype);
		} else _error(4);
	  }
	} else _error(4);
	firsttype=localtype;
  }
  return(localtype);
}


static int prodterm()
{
  /*
	multiplication     	    -> returns long or single
	floating-point division  -> returns single
  */
  int  op,firsttype,original_firsttype,localtype,coercedtype,negtype;
  BOOL coercion;
  CODE *cx[5];
  
  firsttype=negterm();
  localtype=firsttype;
  
  while ((sym == multiply) || (sym == fdiv)) {
	alloc_coerce_space(firsttype,cx,5);
	
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
	if (coercion) {
	  coercedtype=negtype;

	  /* make sure operands are singletype if FFP division */
	  if ((op == fdiv) && (coercedtype != singletype)) {
		/* neither operands are singletype 
		   -> both short or long */ 
		change_Flt(original_firsttype,cx); /* handles SHORT->single correctly */
		gen_Flt(negtype);
		coercedtype=singletype;
	  }

	  switch(op) {
	  case multiply: localtype = gen_muls(coercedtype); break;
	  case fdiv:
		gen_fdiv();
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

static int idivterm()
{
  /* integer division -- LONG = LONG \ LONG */
  int  firsttype,localtype,prodtype;
  int  targettype=longtype;
  CODE *cx[5];
  
  firsttype=prodterm();
  localtype=firsttype;
  
  while (sym == idiv) {
	firsttype=make_integer(firsttype);  /* short or long -> 1st approximation */ 
	alloc_coerce_space(longtype,cx,3);
	
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
		gen_ldiv();
	  } else _error(4); /* notype -> type mismatch */
	firsttype=localtype;  /* moving record of last sub-expression type */
  }
  return(localtype);
}

static int modterm()
{
  /* modulo arithmetic -> returns remainder of long integer or FFP division */
  int  firsttype,localtype,idivtype;
  int  targettype=longtype;
  CODE *cx[5];
  
  firsttype=idivterm();
  localtype=firsttype;
  
  while (sym == modsym)  {
	alloc_coerce_space(firsttype,cx,5);
	if (firsttype == undefined) return(firsttype);
	
	insymbol();
	idivtype=idivterm();
	
	if (idivtype == undefined) return(idivtype);
	
	/* perform integer or FFP modulo operation */
	if ((firsttype != notype) && (idivtype != notype) &&
		(firsttype != stringtype) && (idivtype != stringtype))
	  {
		/* dividend (firsttype) is either short, long or single */
		
		if ((firsttype == singletype) || (idivtype == singletype)) {
		  /* single MOD */
		  coerce(&firsttype,&idivtype,cx);
		  /***************/
		} else {
		  /* integer MOD */
		  if (idivtype == shorttype) {
			make_long();  /* ensure that divisor is LONG! */
			idivtype=longtype;
		  }
    
		  if (firsttype == shorttype)
			coerce(&firsttype,&targettype,cx);
		  /***************/
		}
		
		localtype=idivtype;  /* short or single */
		
		if (localtype == longtype) gen_lmod();
		else {
		  gen_fmod();
		  localtype=singletype;
		}
	  } else _error(4); /* notype -> type mismatch */
	firsttype=localtype;  /* moving record of last sub-expression type */
  }
  return(localtype);
}

static int simple_expr()
{
  int  op,firsttype,localtype;
  BOOL coercion;
  CODE *cx[5];
  
  firsttype=modterm();
  localtype=firsttype;
  
  while ((sym == plus) || (sym == minus)) {
	alloc_coerce_space(firsttype,cx,5);
	op=sym;
	insymbol();
	localtype=modterm();
	if (localtype == undefined) return(localtype);
	coercion=coerce(&firsttype,&localtype,cx);
	if (coercion) {
	  if (localtype == stringtype) {
		if (op == plus) gen_str_concat();
		else _error(4);
	  } else if (localtype == notype) _error(4);
	  else {
		pop_operands(localtype);
		if (op == plus) gen_add(localtype);
		else gen_sub(localtype);
		pop_operands(localtype);
	  }
	}
	firsttype=localtype;  /* moving record of last sub-expression type */
  }
 return(localtype);
}

static BOOL relop(int op)
{
  return ((op == equal) || (op == notequal) || (op == gtrthan) || 
		  (op == lessthan) || (op == gtorequal) || (op == ltorequal));
}

void make_label(char * name,char * lab)
{
  char num[40];
  strcpy(name,"_lab");
  itoa(labelcount++,num,10);
  strcat(name,num);
  strcpy(lab,name);
  strcat(lab,":\0");
} 

static int relexpr() {
  /* relational expression -> pass through this only ONCE */
  int  op=undefined;
  int  firsttype,localtype;
  BOOL coercion;
  CODE *cx[5];
  int simptype;

 firsttype=simple_expr();
 localtype=firsttype;

 if (relop(sym)) {
   alloc_coerce_space(firsttype,cx,5);
   op=sym;
   insymbol();
   simptype=simple_expr();
   if (simptype == undefined) return(simptype);
   coercion=coerce(&firsttype,&simptype,cx);
   localtype=simptype;
   if (coercion) gen_cmp(simptype,op);
   else _error(4);
 }
 
 return (op == undefined) ? localtype : longtype; /* BOOLEAN! */
}

static int notexpr() {
  int localtype,op;

  op=sym;
  if (sym == notsym) insymbol();

  localtype=relexpr();

  if (op == notsym) {
	localtype=make_integer(localtype);
	if (localtype == notype) return(localtype);
	gen_not(localtype);
  }
  return(localtype);
}

static int andexpr() { return generic_expr(andsym, &notexpr,&gen_and); }
static int eorexpr() { return generic_expr(orsym, &andexpr,&gen_or); }
static int orexpr() { return generic_expr(orsym, &eorexpr,&gen_eor); }

static int eqvexpr() {
  return generic_expr(eqvsym,&orexpr,&gen_eqv);
}

int expr() {
  return generic_expr(impsym, &eqvexpr, &gen_imp);
}

