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

/* functions */
/* (x1,y1)-(x2,y2) */
static void gadget_rectangle() { 
    insymbol();
    parse_rect(); 
}

static void modify_gadget() {
    /* 
    ** GADGET MOD gadget-id,knob-position[,max-position] 
    **
    ** For the modification of (proportional) gadgets.
    */
	if (eat_comma()) {
	  long_expr();	/* knob-position */
	  /* specify new maximum notches for slider? */
	  if (sym != comma) gen_push32_val(-1);
	  else in_long_expr();	/* max-position */
	  gen_call_void("_modify_gad",12);
	}
}

void gadget()
{
    /* 
       GADGET ON | OFF | STOP
       GADGET (WAIT|CLOSE|OUTPUT) gadget-id
       GADGET MOD gadget-id,knob-pos[,max-notches]
       GADGET gadget-id,status[,gadget-value,(x1,y1)-(x2,y2),type[,style][,font,size,txtstyle]]
    */
    int  gtype;
    int val;
 
	insymbol();
	
	if (sym == onsym || sym == offsym || sym == stopsym)
		change_event_trapping_status(lastsym);
	else {
        in_long_expr();	/* gadget-id */
        if      (sym == closesym)  gen_call_void("_CloseGadget",4);
        else if (sym == outputsym) gen_call_void("_SetCurrentGadget",4);
        else if (sym == waitsym)   gen_call_void("_WaitGadget",4);
        else if (sym == modsym)    modify_gadget();
        else {
            if (eat_comma()) {
                if (sym == onsym) {
                    gen_push32_val(1);
                    insymbol();
                } else if (sym == offsym) {
                    gen_push32_val(0);
                    insymbol();
                } else {
                    long_expr(); /* status */
                }
                
                if (sym != comma) {
                    gen_call_void("_ChangeGadgetStatus",8);
                    return;	
                }
            }
            
            if (eat_comma()) {
                gtype = expr();	
                /* string or integer expression for 3rd parameter */
                if (gtype != stringtype) make_sure_long(gtype);
            }
            
            if (eat_comma()) {
                insymbol();
                gadget_rectangle();	/* (x1,y1)-(x2,y2) */
                
                if (eat_comma()) {
                    /* Gadget Type. */
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
                
                /*
                ** Optional gadget style parameter.
                */
                if (sym != comma) gen_push32_val(0);	/* style = 0 */
                else {
                    insymbol(); /* FIXME: Is this right? */
                    if (sym != comma) long_expr();	/* style */
                    else gen_push32_val(0);         /* style = 0 */	
                }		
                
                /*
                ** Optional font and font-size parameters (for button).
                */
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
    }
}

/* BEVELBOX (x1,y1)-(x2,y2),type */
void bevel_box() {
	gadget_rectangle();
    if (comma_long_expr()) gen_call_void("_BevelBox",20);
}
