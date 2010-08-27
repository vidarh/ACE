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
void gadget_rectangle()
{
/* (x1,y1)-(x2,y2) */

    if (sym != lparen) _error(14);
    else
    {
     insymbol();
     make_sure_long(expr());	/* x1 */

     if (sym != comma) _error(16);
     else
     {
      insymbol();
      make_sure_long(expr()); /* y1 */

      if (sym != rparen) _error(9);
      else
      {
       insymbol();
       if (sym != minus) _error(21);
       else
       {
        insymbol();
        if (sym != lparen) _error(14);
        else
        {
         insymbol(); 
     	 make_sure_long(expr());	/* x2 */

         if (sym != comma) _error(16);
         else
         {
          insymbol();
     	  make_sure_long(expr());	/* y2 */

          if (sym != rparen) _error(9);
	  insymbol();
         }
	}
       }
      }
     }
    }
}

/* GADGET CLOSE gadget-id */
void close_gadget() {
  insymbol();
  make_sure_long(expr());	/* gadget-id */
  gen_call_void("_CloseGadget",4);
}

void gadget_output() {
/*
** GADGET OUTPUT id
**
** Make the specified gadget the one from which 
** information may be obtained via the GADGET(n)
** function. This allows the value of a string,
** longint or slider gadget to be obtained at 
** any time rather than just when a gadget event
** occurs.
*/
	insymbol();
	make_sure_long(expr());	/* gadget-id */
	gen_call_void("_SetCurrentGadget",4);
}

/* GADGET WAIT gadget-id */
void wait_gadget() {
	insymbol();
	make_sure_long(expr());	/* gadget-id */
	gen_call_void("_WaitGadget",4);
}

void modify_gadget() {
/* 
** GADGET MOD gadget-id,knob-position[,max-position] 
**
** For the modification of (proportional) gadgets.
*/

	insymbol();
	make_sure_long(expr());	/* gadget-id */
	
	if (sym != comma) _error(16);
	else {
	  insymbol();
	  make_sure_long(expr());	/* knob-position */

	  /* specify new maximum notches for slider? */
	  if (sym != comma) gen_push32_val(-1);
	  else {
		insymbol();
		make_sure_long(expr());	/* max-position */
	  }
	  gen_call_void("_modify_gad",12);
	}
}

void gadget()
{
/* GADGET gadget-id,status[,gadget-value,(x1,y1)-(x2,y2),type[,style][,font,size,txtstyle]]
   GADGET MOD gadget-id,knob-pos[,max-notches]
   GADGET WAIT gadget-id
   GADGET CLOSE gadget-id
   GADGET ON | OFF | STOP
*/
int  gtype;

	insymbol();
	
	if (sym == onsym || sym == offsym || sym == stopsym)
		change_event_trapping_status(lastsym);
	else if (sym == closesym)  close_gadget();
	else if (sym == outputsym) gadget_output();
	else if (sym == waitsym)   wait_gadget();
	else if (sym == modsym)    modify_gadget();
	else
	{
	  make_sure_long(expr());	/* gadget-id */

	  if (sym != comma) _error(16);
	  else {
		insymbol();
		if (sym == onsym) {
		  gen_push32_val(1);
		  insymbol();
		} else if (sym == offsym) {
		  gen_push32_val(0);
		  insymbol();
		} else {
		  make_sure_long(expr()); /* status */
		}

		if (sym != comma) {
		  gen_call_void("_ChangeGadgetStatus",8);
		  return;	
		}
	  }

	  if (sym != comma) _error(16);
	  else {
		insymbol();
		gtype = expr();	
		
		/* string or integer expression for 3rd parameter */
		if (gtype != stringtype) make_sure_long(gtype);
	  }

	  if (sym != comma) _error(16);
	  else {
		insymbol();
		gadget_rectangle();	/* (x1,y1)-(x2,y2) */

		if (sym != comma) _error(16);
		else {
		  /* 
		  ** Gadget Type.
		  */
		  insymbol();

		  if (sym == buttonsym) {
			gen_push32_val(1);
			insymbol();
		  } else if (sym == stringsym) {
			gen_push32_val(2);
			insymbol();
		  } else if (sym == longintsym) {
			gen_push32_val(3);
			insymbol();
		  } else if (sym == potxsym) {
			gen_push32_val(4);
			insymbol();
		  } else if (sym == potysym) {
			gen_push32_val(5);
			insymbol();
		  } else {
			make_sure_long(expr()); /* type */
		  }
		}

		/*
		** Optional gadget style parameter.
		*/
		if (sym != comma) gen_push32_val(0);	/* style = 0 */
		else {
		  insymbol();
		  
		  if (sym != comma) make_sure_long(expr());	/* style */
		  else gen_push32_val(0);  /* style = 0 */	
		}		
		
		/*
		** Optional font and font-size parameters (for button).
		*/
		if (sym != comma) {
		  gen_push32_val(0);  /* font name = NULL */
		  gen_push32_val(0);  /* font size = 0 */
		  gen_push32_val(0);  /* font style = 0 */
		} else {
		  insymbol();
		  if (expr() != stringtype)    /* font name */
			_error(4);
		  else {
			if (sym != comma) _error(16);
			else {
			  insymbol();
			  make_sure_long(expr());	/* font size */
			}

			if (sym != comma) _error(16);
			else {
			  insymbol();
			  make_sure_long(expr());	/* font style */
			}
		  }
		}					
	  }

	 gen_call_void("_CreateGadget",48);
	}
}

void	bevel_box()
{
/* 
** BEVELBOX (x1,y1)-(x2,y2),type
*/
	insymbol();

	gadget_rectangle();
	if (sym != comma) _error(16);
	else {
	  insymbol();
	  make_sure_long(expr());	/* type */
	  gen_call_void("_BevelBox",20);
	}
}
