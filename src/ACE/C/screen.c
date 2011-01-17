/*
** <<ACE>>
**
** Amiga BASIC Compiler **
**
** Parser: screens **
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
**
** Author: David J Benn
**   Date: 16th February 1994 (code removed from statement.c)
*/

#include "acedef.h"
#include "codegen.h"

/* external */
extern	int	sym;

/* functions */
void	screen() {
/*
** SCREEN [CLOSE|FORWARD|BACK]
*/
  int	rword,stype;

  insymbol();

  /* SCREEN CLOSE screen-id */
  if (sym == closesym) {
	insymbol();
	gen_pop_as_short(expr(),0); /* screen-id */
	gen_jsr("_closescreen");
  } else if (sym == forwardsym || sym == backsym) {
	/* SCREEN FORWARD|BACK screen-id */
	rword = sym;

	insymbol();
	stype = expr();
	if (stype == stringtype) _error(4);
	else {
	  gen_pop_as_short(stype,0); /* screen-id */

		/* forward or back? */
	  switch(rword) {
	  case forwardsym : gen_load16d_val(1,1); break;
	  case backsym 	:   gen_load16d_val(2,1); break;
	  }

	  gen_jsr("_change_screen_depth");
	}
  } else {
	/* SCREEN screen-id,width,height,colors,mode */
	/* open a screen */

	static short tokens[] = {shorttype /* screen-d */,0, comma, 16, shorttype /* width */   ,0, comma, 16,
							 shorttype /* height */  ,0, comma, 16, shorttype /* depth */   ,0, comma, 16,
							 shorttype /* mode */    ,0, -1   , -1};

	if (expect_token_sequence(&tokens)) {
	  gen_call_args("_openscreen","d4.w,d3.w,d2.w,d1.w,d0.w",0);
	}
  }
}
