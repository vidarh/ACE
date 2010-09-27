/* << ACE >>

   -- Amiga BASIC Compiler --

   ** Parser: window functions **
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
	   6th,7th December 1992,
	   24th,26th October 1993,
	   15th November 1993,
	   30th April 1994
*/

#include "acedef.h"
#include "codegen.h"

/* externals */
extern	int	sym;
extern	int	lastsym;

/* functions */

static void wdwclose() {
 insymbol();
 make_sure_long(expr()); /* Wdw-id */
 gen_call_void("_CloseWdw",4);
}

static void wdwoutput() {
 insymbol();
 make_sure_long(expr()); /* Wdw-id */
 gen_call_void("_ChangeOutputWdw",4);
}


BOOL parse_rect()
{
  short rect_tokens[] = {comma,16,lparen,14,longtype /*x1*/,0, comma, 16, longtype/*y1*/,0,rparen,9,
						 minus,21,lparen,14,longtype /*x2*/,0, comma, 16, longtype/*y2*/,0,rparen,9,-1,-1};
  return expect_token_sequence(rect_tokens);
}

void window()
{
  int wtype;

  /* WINDOW wdw-id[,title],rectangle[,type][,screen-id] 
	 WINDOW CLOSE wdw-id
	 WINDOW OUTPUT wdw-id 
	 WINDOW ON | OFF | STOP
  */


 insymbol();

 if (sym == onsym || sym == offsym || sym == stopsym)
   change_event_trapping_status(lastsym);
 else if (sym == closesym) wdwclose();
 else if (sym == outputsym) wdwoutput();
 else {
   /* open a window */
   make_sure_long(expr()); /* Wdw-id */
   if (sym != comma) _error(16);
   else {
	 insymbol();
	 if (sym != comma) {
	   wtype=expr();
	   if (wtype != stringtype) _error(4); /* type mismatch */
	 } else gen_push32_val(0);	/* NULL */
	 
	 if (!parse_rect()) return;
				 
	 /* optional window type */
	 if (sym == comma) {
	   insymbol();
	   if (sym != comma) make_sure_long(expr());
	   else gen_push32_val(-1);
	 } else gen_push32_val(-1);
				 
	 /* optional screen-id */
	 if (sym == comma) {
	   insymbol();
	   if (sym != comma) make_sure_long(expr());
	   else gen_push32_val(0);
	 } else gen_push32_val(0);		
				 
	 /* call open-window routine */
	 gen_call_void("_OpenWdw",32);
   }
 }
}

