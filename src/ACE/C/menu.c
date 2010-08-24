/* << ACE >>

   -- Amiga BASIC Compiler --

   ** Parser: menu functions **
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
     Date: 9th,10th November 1993,
	   13th January 1994
*/

#include "acedef.h"
#include "codegen.h"

/* externals */
extern	int	sym;
extern	int	lastsym;

/* functions */
void clear_menu()
{
/* MENU CLEAR */

	insymbol();
	gen_jsr("_ClearMenu");
}

void wait_menu()
{
/* MENU WAIT */
	
	insymbol();
	gen_jsr("_WaitMenu");
}

void menu()
{
/* MENU menu-id,item-id,state[,title-string[,command-key]]
   MENU WAIT menu-id
   MENU CLEAR
   MENU ON | OFF | STOP
*/
  insymbol();
  
  if (sym == onsym || sym == offsym || sym == stopsym)
	change_event_trapping_status(lastsym);
  else if (sym == clearsym) clear_menu();
  else if (sym == waitsym)  wait_menu();
  else {
	make_sure_long(expr()); /* menu-id */
	if (sym != comma) _error(16);
	else {
	  insymbol();
	  make_sure_long(expr()); /* item-id */
	  if (sym != comma) _error(16);
	  else {
		insymbol();
		make_sure_long(expr()); /* state */
		if (sym != comma) {
		  gen_call_void("_ChangeMenuState",12);
		  return;	
		}
	  }
	}
	
	if (sym != comma) _error(16);
	else {
	  insymbol();
	  if (expr() != stringtype) _error(4); /* title-string */
	}
	
	if (sym == comma) {
	  insymbol();
	  if (expr() != stringtype) _error(4);
	} else gen_push32_val(0);	/* command-key */
	gen_call_void("_ModifyMenu",20);
  }
}
