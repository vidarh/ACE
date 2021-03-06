/* << ACE >>

   -- Amiga BASIC Compiler --

   ** Parser: standard output and screen printing code **
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
	   6th,7th,13th December 1992,
	   6th January 1993,
	   18th February 1993,
	   10th,11th,31st October 1993,
	   14th,15th December 1993,
	   2nd January 1994
*/

#include "acedef.h"
#include "codegen.h"

#define QUN_CODE 3

/* externals */
extern	int	sym;
extern	int	lastsym;
extern	int	obj;
extern	BOOL	end_of_source;
extern	BOOL	dosused;
extern	int  	fstrcount;

/* functions */
void gen_printcode(code)
int code;
{
 switch(code)
 {
  /* LF */
  case LF_CODE : 	gen_push32_val(0);
		 	break;
  /* TAB */
  case TAB_CODE :  	gen_push32_val(1);
			break;
  /* SPACE */
  case SPACE_CODE :	gen_push32_val(2);
			break;  
  /* QUN */
  case QUN_CODE :	gen_push32_val(3);
			break;	
 }

 /* call function */
 gen_call_void("_Ucodeprint",4);
}

void print_statement()
{
/* - Print expression(s) to a DOS or Intuition window.
   - Expressions can be separated by spaces, commas or semicolons.
   - Text is clipped at the right edge of an Intuition window, but
     wraps around in a DOS window. The former is in line with AmigaBASIC.
   - Both window-types scroll text when the bottom is reached.
*/  
int exprtype,arguments=0;

do
{
 if (sym != ident && !strfunc() && !numfunc() && !factorfunc() &&
     obj != constant) 
    insymbol();  /* ident/func/literal after space or as first parameter */

 /* PRINT# ? */
 if (sym == hash && (lastsym == printsym || lastsym == question)) return;

 /* end of line, multi-statement, ";", "," ELSE or comment 
    after PRINT command word? -> don't proceed to expr! */
 if ((sym == endofline) || (sym == colon) || (sym == apostrophe) || 
     (sym == semicolon) || (sym == comma) || (sym == elsesym) ||
     (end_of_source))
 {
  if (sym == comma) gen_printcode(TAB_CODE);
  else
  if ((arguments == 0) && (sym != semicolon)) 
     gen_printcode(LF_CODE);  /* PRINT with no args */ 

  if (sym != colon && sym != elsesym) 
     insymbol();  /* leave colon for multi-statement 
		     in statement() or leave ELSE for if_statement() */
  return;
 }

  /* get an expression */
  exprtype = expr();
   
  if (exprtype == undefined) { _error(0); return; } /* illegal syms? */

  switch(exprtype)
	{
	case shorttype :
	  make_long();
	  gen_call_void("_Ushortprint",4);
	  break;
	case longtype :   gen_call_void("_Ulongprint",4); break;
	case singletype : gen_call_void("_Usingleprint",4); break;
	case stringtype : gen_call_void("_Ustringprint",4); break;
	}

  if (exprtype != stringtype) gen_printcode(SPACE_CODE); /* trailing space
							    for any number */
  arguments++;

  if (sym == comma) gen_printcode(TAB_CODE);
   
 }
 while ((sym == comma) || (sym == semicolon) || (sym == ident) ||
	strfunc() || numfunc() || factorfunc() || obj == constant);  

 /* no comma or semicolon at end of PRINT -> LF */
 gen_printcode(LF_CODE);

 /* need mathffp.library for _Usingleprint and 
    _beep if called by _Ustringprint. 
 */ 
 enter_XREF("_MathBase");
}  

void gen_printscode(code)
int code;
{
  switch(code)
	{
	case LF_CODE :    gen_jsr("_printsLF"); break;
	case TAB_CODE :   gen_jsr("_printsTAB"); break;
	case SPACE_CODE : gen_jsr("_printsSPC"); break;
 }
}

void prints_statement()
{
/* - print expression(s) to current rastport (intended for screens or
     Intuition windows).
   - expressions can be separated by spaces, commas or semicolons which
     have the same effect as in the PRINT statement.
*/  
int exprtype,arguments=0;

do
{
 if (sym != ident && !strfunc() && !numfunc() && !factorfunc() &&
     obj != constant) 
    insymbol();  /* ident/func/literal after space or as first parameter */

 /* end of line, multi-statement, ";", "," ELSE or comment 
    after PRINTS command word? -> don't proceed to expr! */
 if ((sym == endofline) || (sym == colon) || (sym == apostrophe) || 
     (sym == semicolon) || (sym == comma) || (sym == elsesym) ||
     (end_of_source))
 {
  if (sym == comma) gen_printscode(TAB_CODE);
  else
  if ((arguments == 0) && (sym != semicolon)) 
     gen_printscode(LF_CODE);  /* PRINTS with no args */ 

  if (sym != colon && sym != elsesym) 
     insymbol();  /* leave colon for multi-statement 
		     in statement() or leave ELSE for if_statement() */
  return;
 }

  /* get an expression */
  exprtype = expr();
   
  if (exprtype == undefined) { _error(0); return; } /* illegal syms? */

  switch(exprtype)
  {
  case shorttype : gen_call_args("_shortprints","d0.w",0); break;
  case longtype :  gen_call_args("_longprints","d0",0); break;
  case singletype :
	gen_call_args("_singleprints","d0",0);
	enter_XREF("_MathBase");
	break;
  case stringtype : gen_call_args("_stringprints","a0",0); break;
  }

  if (exprtype != stringtype) gen_printscode(SPACE_CODE); /* trailing space
							     for any number */
  arguments++;

  if (sym == comma) gen_printscode(TAB_CODE);
   
 }
 while ((sym == comma) || (sym == semicolon) || (sym == ident) ||
	strfunc() || numfunc() || factorfunc() || obj == constant);  

 /* no comma or semicolon at end of PRINTS -> LF */
 gen_printscode(LF_CODE);
}  
