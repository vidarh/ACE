/* << ACE >>

   -- Amiga BASIC Compiler --

   ** Parser: event trapping code **
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
	   13th,28th February 1993,
	   1st March 1993,
	   25th September 1993,
	   24th,25th,26th October 1993,
	   2nd,8th-10th November 1993,
	   24th July 1994,
	   7th,8th August 1995
*/

#include "acedef.h"
#include "codegen.h"

/* externals */
extern	int	sym;
extern	int	lastsym;
extern	SYM	*curr_item;

extern	char   	id[MAXIDSIZE]; 
extern	SHORT  	shortval;
extern	LONG   	longval; 
extern	float  	singleval;

extern	BOOL 	break_event;
extern	BOOL 	menu_event;
extern	BOOL 	mouse_event;
extern	BOOL 	timer_event;
extern	BOOL 	error_event;
extern	BOOL 	wdw_event;
extern	BOOL 	gad_event;

extern	BOOL 	break_event_label_exists;
extern	BOOL 	menu_event_label_exists;
extern	BOOL 	mouse_event_label_exists;
extern	BOOL 	timer_event_label_exists;
extern	BOOL 	error_event_label_exists;
extern	BOOL 	wdw_event_label_exists;
extern	BOOL 	gad_event_label_exists;

extern	BOOL  	ontimerused;

extern	BOOL 	break_opt;
extern	BOOL 	wdw_close_opt;

extern	int	break_event_branch;
extern	int	menu_event_branch;
extern	int	mouse_event_branch;
extern	int	timer_event_branch;
extern	int	error_event_branch;
extern	int	wdw_event_branch;
extern	int	gad_event_branch;

extern	char 	break_event_label[80];
extern	char 	menu_event_label[80];
extern	char 	mouse_event_label[80];
extern	char 	timer_event_label[80];
extern	char 	error_event_label[80];
extern	char 	wdw_event_label[80];
extern	char 	gad_event_label[80];

extern	float 	timer_event_seconds;
extern	char  	ontimer_seconds[40];

/* functions */
void get_event_trap_label()
{
  /* ON <event>|[TIMER(n)] GOSUB <label> | GOTO <label> | CALL <SUBname> */
  int   event;
  int   branchsym;
  char  theLabel[80];

 if ((sym != breaksym) && (sym != menusym) &&
     (sym != mousesym) && (sym != timersym) && 
     (sym != errorsym) && (sym != windowsym) &&
     (sym != gadgetsym))
    _error(55);
 else {
  event=sym; /* remember event specifier */

  /* if TIMER, get single-precision seconds */
  if (event == timersym) {
	insymbol();
	if (sym != lparen) _error(14);
	else {
	  insymbol();
	  if ((sym != shortconst) && (sym != longconst) &&
		  (sym != singleconst)) _error(27); /* numeric constant expected */
	  else {
		switch(sym) {
		case shortconst  : timer_event_seconds=(float)shortval; break;
		case longconst   : timer_event_seconds=(float)longval;  break;
		case singleconst : timer_event_seconds=singleval; break;
		}
		sprintf(ontimer_seconds,"#$%lx",(unsigned long)timer_event_seconds);
		insymbol();
		if (sym != rparen) _error(9);       
	  }
	}
  }
  insymbol();
  if (sym != gosubsym && sym != gotosym && sym != callsym) 
	_error(56);
  else {
	branchsym=sym; /* GOSUB, GOTO or CALL? */

	insymbol();
	if (sym != ident && sym != shortconst && sym != longconst)
	  _error(57);  /* label or SUB name expected */
	else {
	  /* Make a label from a line number? */
	  if (sym != ident) make_label_from_linenum(sym,id);
	  
	  /* 
	  ** Convert to a SUB label, check for existence 
	  ** (defined or declared) of SUB and number of 
	  ** parameters (must be zero to be valid).
	  */	
	  if (branchsym == callsym) {
		sprintf(theLabel,"_SUB_%s",id);
		if (!exist(theLabel,subprogram)) {
		  _error(59);
		  return;
		} else if (curr_item->no_of_params != 0) {
		  _error(78);
		  return;
		}
	  } else 
		strcpy(theLabel,id);

	  switch(event) {
      case breaksym : 
		strcpy(break_event_label,theLabel);
		break_event_label_exists=TRUE; 
		break_event_branch=branchsym;
		break;
			  
      case menusym: 
		strcpy(menu_event_label,theLabel);
		menu_event_label_exists=TRUE;
		menu_event_branch=branchsym;
		break;

      case mousesym:
		strcpy(mouse_event_label,theLabel); 
		mouse_event_label_exists=TRUE;
		mouse_event_branch=branchsym;
		break;
		
      case timersym : strcpy(timer_event_label,theLabel); 
		timer_event_label_exists=TRUE;
		timer_event_branch=branchsym;
		break;

      case errorsym : strcpy(error_event_label,theLabel); 
		error_event_label_exists=TRUE;
		error_event_branch=branchsym;
		break;
		
      case windowsym : strcpy(wdw_event_label,theLabel); 
		wdw_event_label_exists=TRUE;
		wdw_event_branch=branchsym;
		break;

      case gadgetsym : strcpy(gad_event_label,theLabel); 
		gad_event_label_exists=TRUE;
		gad_event_branch=branchsym;
		break;
	  }
	  insymbol();
	}
  }
 }
}

int try_change_event_trapping_status(int lastsym) {
    if (sym == onsym || sym == offsym || sym == stopsym) {
        change_event_trapping_status(lastsym);
        return 1;
    }
    return 0;
}

void change_event_trapping_status(int event)
{
  /* <event> ON|OFF|STOP */
  int action;
  BOOL onoff;

  if (lastsym != windowsym && lastsym != gadgetsym &&
	  lastsym != menusym) insymbol();
  
  /* ON, OFF or STOP expected */
  if ((sym != onsym) && (sym != offsym) && (sym != stopsym)) {
	_error(58); 
	return;
  }

  action=sym; /* remember action symbol */
  insymbol();

  switch(event) {
  case breaksym: if (!break_event_label_exists) { _error(59); action = 0; } break;
  case menusym:  if (!menu_event_label_exists)  { _error(59); action = 0; } break;
  case mousesym: if (!mouse_event_label_exists) { _error(59); action = 0; } break;
  case timersym: if (!timer_event_label_exists) { _error(59); action = 0; } break;
  case errorsym: if (!error_event_label_exists) { _error(59); action = 0; } break;
  case windowsym: if (!wdw_event_label_exists)  { _error(59); action = 0; } break;
  case gadgetsym: if (!gad_event_label_exists)  { _error(59); action = 0; } break;
  }
	
  if (action == 0) { insymbol(); return; } /* error 59 */

  /* enable or disable event trapping for <event>. */
  onoff = (action == onsym);
  switch(event) {
  case breaksym:  break_event = onoff; break;
  case menusym:   menu_event  = onoff; break;
  case mousesym:  mouse_event = onoff; break;
  case timersym:  timer_event = onoff; if (onoff) ontimerused=TRUE; break;
  case errorsym:  error_event = onoff; break;
  case windowsym: wdw_event   = onoff; break;
  case gadgetsym: gad_event   = onoff; break;
  }
  
  if (action == offsym) /* disable event trapping for <event> and clear label. */
	switch(event) {
	case breaksym:  break_event_label_exists=FALSE; break;
	case menusym:   menu_event_label_exists=FALSE; break;
	case mousesym:  mouse_event_label_exists=FALSE; break;
	case timersym:  timer_event_label_exists=FALSE; break;
	case errorsym : error_event_label_exists=FALSE; break;
	case windowsym: wdw_event_label_exists=FALSE; break;
	case gadgetsym: gad_event_label_exists=FALSE; break;
	}
  
  insymbol();
}

void turn_event_off(char * eventHandler)
{
  /* 
  ** Turn event trapping off if this 
  ** subroutine/subprogram belongs to 
  ** an event.
  */
  if ((strcmp(eventHandler,break_event_label) == 0) && (break_event))
	{ break_event=FALSE; break_event_label_exists=FALSE; }
  else if ((strcmp(eventHandler,menu_event_label) == 0) && (menu_event))
	{ menu_event=FALSE; menu_event_label_exists=FALSE; }
  else if ((strcmp(eventHandler,mouse_event_label) == 0) && (mouse_event))
	{ mouse_event=FALSE; mouse_event_label_exists=FALSE; }
  else if ((strcmp(eventHandler,timer_event_label) == 0) && (timer_event))
	{ timer_event=FALSE; timer_event_label_exists=FALSE; }
  else if ((strcmp(eventHandler,error_event_label) == 0) && (error_event))
	{ error_event=FALSE; error_event_label_exists=FALSE; }
  else if ((strcmp(eventHandler,wdw_event_label) == 0) && (wdw_event))
	{ wdw_event=FALSE; wdw_event_label_exists=FALSE; }
  else if ((strcmp(eventHandler,gad_event_label) == 0) && (gad_event))
	{ gad_event=FALSE; gad_event_label_exists=FALSE; }
}

static void event_test(const char * test, const char * label, int branch)
{
  char lab[80],lablabel[80];
  make_label(lab,lablabel);
  
  gen_jsr(test);
  gen_test();
  gen_beq(lab);

  if (branch == callsym) gen_jsr(label);
  else if (branch  == gosubsym) gen_branch("jsr",label);
  else gen_branch("jmp",label);
  gen_label(lablabel);
}

static void ctrl_c_test() {
  event_test("_ctrl_c_test","_EXIT_PROG", gotosym);
}

static void break_event_test() {
  event_test("_ctrl_c_test",break_event_label, break_event_branch);
}

static void menu_event_test() {
  event_test("_menu_test",menu_event_label, menu_event_branch);
}

static void mouse_event_test() {
  event_test("_mouse", mouse_event_label, mouse_event_branch);
}

static void timer_event_test() {
  gen_load32d(ontimer_seconds,0);
  event_test("_ontimer", timer_event_label, timer_event_branch);
  enter_XREF("_MathBase");  /* timer routines need mathffp.library */
}

static void error_event_test() {
  event_test("_testerror",error_event_label, error_event_branch);
}

static void wdw_close_test()
{
  char lab[80],lablabel[80];
  
  /* Test for close-gadget selection 
	 in any window and exit program 
	 if close-gadget click detected.
	 
	 The clicked window will be closed
	 before the program exits.
  */
  make_label(lab,lablabel);

  gen_push32_val(1);
  gen_call_void("_wdw_close_test",4);
  gen_test();
  gen_beq(lab);
  gen_jmp("_EXIT_PROG");
  gen_label(lablabel);
}

static void wdw_event_test()
{
char lab[80],lablabel[80];

/* Test for close-gadget selection 
   in any window and transfer control
   to a user-defined subroutine if 
   close-gadget click detected.
   
   This may be extended in the future
   to accomodate other window events
   (eg: resizing).   
*/
   make_label(lab,lablabel);

   gen_push32_val(0);
   gen_call_void("_wdw_close_test",4);
   gen_test();
   gen_beq(lab);
 
   if (wdw_event_branch == callsym) gen_jsr(wdw_event_label);
   else if (wdw_event_branch == gosubsym) gen_branch("jsr",wdw_event_label);
   else gen_branch("jmp",wdw_event_label);
   gen_label(lablabel);
}

static void gad_event_test()
{
  char lab[80],lablabel[80];

/* Test for user-defined gadget selection 
   in current output window and transfer 
   control to a user-defined subroutine 
   if a gadget click is detected.
*/
   make_label(lab,lablabel);

   gen_push32_val(0);
   gen_call_void("_gadget_event_test",4);
   gen_test();
   gen_beq(lab);

   if (gad_event_branch == callsym) gen_jsr(gad_event_label);
   else if (gad_event_branch == gosubsym) gen_branch("jsr",gad_event_label);
   else gen_branch("jmp",gad_event_label);
   gen_label(lablabel);
}

void check_for_event()
{
/* produce code for event trapping */
 if (break_opt)   	ctrl_c_test();
 if (break_event) 	break_event_test();
 if (menu_event)  	menu_event_test();
 if (wdw_close_opt)	wdw_close_test(); 
 if (wdw_event) 	wdw_event_test();
 if (gad_event) 	gad_event_test();
 if (mouse_event) 	mouse_event_test();
 if (timer_event) 	timer_event_test();
 if (error_event) 	error_event_test();
}

