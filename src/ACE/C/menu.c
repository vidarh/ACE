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

/* MENU menu-id,item-id,state[,title-string[,command-key]]
   MENU WAIT menu-id
   MENU CLEAR
   MENU ON | OFF | STOP
*/
void menu() {
  insymbol();
  
  if (sym == onsym || sym == offsym || sym == stopsym)
	change_event_trapping_status(lastsym);
  else if (sym == clearsym) { /* MENU CLEAR */
      insymbol();
      gen_jsr("_ClearMenu");
  } else if (sym == waitsym) { /* MENU WAIT */
      insymbol();
      gen_jsr("_WaitMenu");
  } else {
      int t = parse_gen_params(expr(),"l,l,l"); /* menu-id,item-id,state */
      if (sym != comma) {
          gen_call_void("_ChangeMenuState",12);
          return;	
      }
      if (!parse_gen_params(0,",s")) /* title-string */
          gen_push32_val(0);
      if (!parse_gen_params(0,",s")) /* command-key */
          gen_push32_val(0);	
      gen_call_void("_ModifyMenu",20);
  }
}
