/* << ACE >>

   -- Amiga BASIC Compiler --

   ** Parser: gadget functions **
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
     Date: 1st,2nd,6th November 1993,
	   23rd-25th June 1994,
	   10th,12th July 1994,
	   12th March 1995,
	   6th November 1995
*/

#include "acedef.h"
#include "codegen.h"

/* externals */
extern	int	sym;
extern	int	lastsym;

void gadget() {
    int  gtype;
    int val;
	insymbol();
	if (try_change_event_trapping_status(lastsym)) return;
    if (parse_alt_sequence(seq_gadget,4)) return;

    parse_call_func_args("l,b",0); /* gadget-id , status */
    
    if (!try_comma()) {
        gen_call_void("_ChangeGadgetStatus",8);
        return;	
    }
    
    gtype = expr();	
    /* string or integer expression for 3rd parameter */
    if (gtype != stringtype) make_sure_long(gtype);
    
    if (eat_comma()) {
        parse_rect();	/* (x1,y1)-(x2,y2) */
        if (try_comma()) {
            /* Gadget Type. */

            /* FIXME: Regression: This is not actually *used* anywhere, should probably push32_val if val != -1 */
            val = -1;
            switch (sym) {
            case buttonsym:  val = 1; break;
            case stringsym:  val = 2; break;
            case longintsym: val = 3; break;
            case potxsym:    val = 4; break;
            case potysym:    val = 5; break;
            default:
                long_expr(); /* type */
            }
            if (val != -1) insymbol();
        }
        
        /* Optional gadget style parameter. */
        opt_arg(longtype,0); /* style = 0 */
        
        /* Optional font and font-size parameters (for button). */
        if (sym != comma) {
            gen_push32_val(0);  /* font name  = NULL */
            gen_push32_val(0);  /* font size  = 0 */
            gen_push32_val(0);  /* font style = 0 */
        } else {
            static short font_tokens[] = {comma, 16, stringtype /* font name  */, 4, 
                                          comma, 16, longtype   /* font size  */, 0, 
                                          comma, 16, longtype   /* font style */, 0};
            insymbol();
            if (!expect_token_sequence(font_tokens)) return;
        }					
    }
    
    gen_call_void("_CreateGadget",48);
}

