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
extern	int	lastsym;
extern  int sym;

/* WINDOW wdw-id[,title],rectangle[,type][,screen-id] 
   WINDOW CLOSE wdw-id
   WINDOW OUTPUT wdw-id 
   WINDOW ON | OFF | STOP
*/
void window() {
    /* l,[s,]r[,l][,l] */
    /* FIXME: The problem here is if there is a comma after ",",
       I must:
         - parse_rect if '('
         - expr() if !'('

       Instead of a list of symbols, this ought to be a list of parameter types,
       with an "optional" flag.
    */
    struct Function f_openwdw = {"?l?l",2, {-1,0}, "_OpenWdw", 32};
    
    insymbol();
    if (try_change_event_trapping_status(lastsym)) return;
    else if (eat(closesym))  gen_call_sargs("_CloseWdw","il",4);
    else if (eat(outputsym)) gen_call_sargs("_ChangeOutputWdw","il",4);
    else {
        //gen_arglist(&f_openwdw); 
        
        /* open a window */
        make_sure_long(expr()); /* Wdw-id */
        if (!peek(comma)) {
            _error(16);
            return;
        }

        opt_arg(stringtype,0);
        if (!eat_comma()) return;
        if (!parse_rect()) return;
        opt_arg(longtype,-1); /* optional window type */
        opt_arg(longtype,0);  /* optional screen-id */
        gen_call_void("_OpenWdw",32);
    }
}


