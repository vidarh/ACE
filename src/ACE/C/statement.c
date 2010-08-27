/* << ACE >>

   -- Amiga BASIC Compiler --

   ** Parser: statement code **
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
	   6th,7th,12th,13th,27th-29th December 1992,
	   4th,6th,31st January 1993,
	   6th,12th,13th,15th,18th,28th February 1993,
	   1st,7th,24th March 1993,
	   9th May 1993,
	   12th,13th,30th June 1993,
	   1st July 1993,
	   5th,25th September 1993 
	   26th October 1993,
	   1st,2nd,8th,9th November 1993,
	   27th December 1993,
	   2nd,5th January 1994,
	   6th,15th,16th,27th February 1994,
	   4th April 1994,
	   14th,15th May 1994,
	   10th July 1994,
	   7th August 1994,
	   3rd,8th,17th September 1994,
	   5th March 1995,
	   8th August 1995,
	   10th March 1996,
	   11th,22nd June 1996,
	   4th September 1996
*/

#include "acedef.h"
#include "event.h"
#include "gfx.h"
#include "codegen.h"

/* locals */
static	char 	*frame_ptr[] = { "(a4)","(a5)" };

/* externals */
extern	int	sym;
extern	int	lastsym;
extern	int	obj;
extern	int	typ;
extern	int 	lev;
extern	char	ch;
extern	char   	id[MAXIDSIZE]; 
extern	char   	ut_id[MAXIDSIZE];
extern	SYM	*curr_item;
extern	CODE	*curr_code;
extern	CODE	*exit_for_cx;

extern	ACELIBS acelib[NUMACELIBS];

extern	char 	librarybase[MAXIDSIZE+6];
extern	BOOL 	have_lparen;
extern	BOOL 	have_equal;
extern	BOOL 	restore_a4;
extern	BOOL 	restore_a5;
extern	BOOL	narratorused;
extern	BOOL	end_of_source;
extern	char 	exit_sub_name[80];

int   statetype; /* Used by randomize and wave */

static void say() {
  char addrbuf[80];
  insymbol();
  if (expr() != stringtype) _error(4);  /* phoneme string on stack */
  
  if (sym == comma) {
	insymbol();
	if ((sym == ident) && (obj == variable)) {
	  if (!exist(id,array)) _error(28); 
	  else if (curr_item->type != shorttype) _error(28); 
	  else {
		/* get address of array from stack frame */
		itoa(-1*curr_item->address,addrbuf,10);
		strcat(addrbuf,frame_ptr[lev]);
		gen_push32_var(addrbuf); /* push address of mode-array */
		/* SIZE of array not checked here! (must be >= 9 elements) */
	  } 
	  insymbol();
	} else _error(28); 
  } else gen_push32_val(0);  /* no mode-array -> push NULL */
   
  gen_call_args("_say","",8);
  enter_XREF("_cleanup_async_speech");
  narratorused=TRUE;
}

static void wave() {
   /* voice */
   insymbol();
   make_sure_short(expr());  /* voice (short) 0..3 */
   
   /* wave definition */
   if (sym != comma) _error(16);
   else {
	 insymbol();
	 if (sym == sinsym) {
	   gen_load32a_val(0,0);   /* SIN wave = 0 -> flag for _wave */
	   insymbol();
	 } else {
	   /* now expect an address -> pointer to a block of bytes */
	   if (expr() != longtype) _error(4);
	   
	   /* number of bytes? */
	   if (sym != comma) _error(16);
	   else {
		 insymbol();
		 make_sure_long(expr());
		 if (statetype == notype) _error(4); /* string -> type mismatch */
	   }   

	   gen_pop32d(1);  /* pop # of bytes of waveform data */
	   gen_pop_addr(0);  /* pop address of waveform data */
	 }
   }
   gen_call_args("_wave","d0.w",0); 
}

static void increment_decrement(int opsym) {
  char buf[80],ext_name[80];
  SYM * inc_item;
  
  insymbol();
  if (sym != ident) _error(7);
  else {
	/* it may be an external variable */
	strcpy(buf,ut_id);
	remove_qualifier(buf);
	if (buf[0] != '_') sprintf(ext_name,"_%s",buf);
	else strcpy(ext_name,buf);

	if ((!exist(id,variable)) && (!exist(ext_name,extvar))) _error(19); /* simple variable expected */
	else {
	  inc_item=curr_item;
	  if (inc_item->type == stringtype) _error(4);
	  else {
		/* get address of variable */
		address_of_object();
		gen_pop_addr(0);
		
		if (opsym == increment) {
		  switch(inc_item->type) {
		  case shorttype  : gen_incr_indirect16(); break;
		  case longtype   : gen_incr_indirect32(); break;
		  case singletype : gen_incr_indirect_float(); break;
		  }
		} else {
		  switch(inc_item->type)
			{
			case shorttype: gen_decr_indirect16(); break;
			case longtype:  gen_decr_indirect32();  break;
			case singletype : gen_decr_indirect_float(); break;
			}
		}
	  }
	}
	insymbol();
  } 
}



/* ------*/
/* sound */
/* ------*/

void sound()
{
/* make a tone of given period, duration and volume through 
   the specified channel */

BOOL voice=FALSE;
BOOL volume=FALSE;

 insymbol();
 make_sure_short(expr());  /* period (short) 0..32767 */ 

 if (sym != comma) _error(16);
 else {
  insymbol();
  gen_Flt(expr());  /* duration (single) 0..77 */
 }

 if (sym == comma) {
   insymbol();
   if (sym != comma) {  /* if comma -> skip volume */
	 make_sure_short(expr());  /* volume (short) 0..64 */
	 volume=TRUE;
   } else gen_load32d_val(64,2);   /* default volume = 64 */
 } else gen_load32d_val(64,2);   /* default volume = 64 */

 if (sym == comma) {
  insymbol();
  make_sure_short(expr());  /* voice (short) 0..3 */
  voice=TRUE;
 } else gen_load32d_val(0,3);   /* default voice = 0 */

 if (voice)  gen_pop16d(3);  /* pop voice */
 if (volume) gen_pop16d(2);  /* pop volume */

 gen_call_args("_sound","d1,d0.w",0);
 enter_XREF("_MathBase");
}   

void handle_label(label_name)
char *label_name;
{
int  oldlevel;
char label_lab[50];

 /* create a new label */

    oldlevel=lev;  /* make all labels global -> level ZERO */ 
    lev=ZERO;

    /* does label already exist? */
    strcpy(label_lab,label_name);
    strcat(label_lab,":\0");

    if (!exist(label_lab,label))  { 
     /* no, so create it */
     enter(label_lab,notype,label,0); 
     gen_label(label_lab);
     turn_event_off(label_name);  /* see event.c */
    }
    else _error(6);  /* duplicate label */
    lev=oldlevel;
}

/*-----------*/
/* statement */
/*-----------*/
 
void statement()
{
char  buf[50],destbuf[3],idholder[50],addrbuf[80],sub_name[80],numbuf[40];
char  func_name[MAXIDSIZE],func_address[MAXIDSIZE+9];
char  ext_name[MAXIDSIZE+1];
int   commandsym;
int   oldobj,oldtyp,stype;
int   oldlevel;
SYM   *func_item,*sub_item,*mc_item,*inc_item,*dec_item;
BYTE  libnum;
BOOL  need_symbol=TRUE;
int   i;
SHORT popcount;

 /* data object assignment (variable, subprogram or array element), 
    label declaration or subprogram call without CALL? */
 if (sym == ident) {
  /* make subprogram name */
  strcpy(sub_name,"_SUB_");
  strcat(sub_name,id);

  /* store id in case it's a function */
  strcpy(func_name,id);
  remove_qualifier(func_name);

  /* make external variable name by removing qualifier and 
     adding an underscore prefix if one is not present. 
  */
  strcpy(buf,ut_id);
  remove_qualifier(buf);
  if (buf[0] != '_') {
   strcpy(ext_name,"_\0");
   strcat(ext_name,buf);
  } else strcpy(ext_name,buf);

  /* assignment? */
  strcpy(idholder,id);  /* save info for label or assign() */
  oldobj=obj;
  oldtyp=typ;

  insymbol();
  
  /* a variable/subprogram assignment or an array element assignment? */
  if ((sym == equal) || (sym == memberpointer) || ((sym == lparen) 
      && (!exist(sub_name,subprogram)) && (!exist(func_name,function))
      && (!exist(ext_name,extfunc)))) 
  {
     strcpy(id,idholder);  /* restore info */
     obj=oldobj;
     typ=oldtyp;
     if (sym == equal)  have_equal=TRUE;
     if (sym == lparen) {
         if (!exist(id,array)) { _error(71); insymbol(); return; } 
		 else
		   have_lparen=TRUE;
	 }
     assign(); 
     have_equal=FALSE;
     have_lparen=FALSE;
  }
  else
  /* implicit subprogram or function call (ie: without CALL command)? */
  if (exist(func_name,function) || exist(sub_name,subprogram) ||
      exist(ext_name,extfunc)) 
  {
    sub_item=curr_item; /* - store curr_item because the next call to exist()
			     will clobber it! (for use by SUB call)
			   - if sub_item points to a function item,
			     it makes no difference since func_item
			     will be used instead. */
    check_for_event();

    if (exist(ext_name,extfunc))
    {
     /* call external function */
     call_external_function(ext_name,&need_symbol);
     if (need_symbol) insymbol();
    }
    else
    if (exist(func_name,function)) {
     /* call shared library function */
     func_item=curr_item;
     if (func_item->no_of_params != 0)
        { load_func_params(func_item); insymbol(); }

     /* call the function */
     if ((libnum=check_for_ace_lib(func_item->libname)) == NEGATIVE) 
        make_library_base(func_item->libname);
     else strcpy(librarybase,acelib[libnum].base);
     gen_load32a(librarybase,6);
     itoa(func_item->address,func_address,10);
     strcat(func_address,"(a6)");
     gen_jsr(func_address);
     if (restore_a4) { gen_save32a("_a4_temp",4); restore_a4=FALSE; }
     if (restore_a5) { gen_save32a("_a5_temp",5); restore_a5=FALSE; }
    }
    else
    {
      /* call SUB */
      if (sub_item->no_of_params != 0) 
         { load_params(sub_item); insymbol(); }
      gen_jsr(sub_name);
    }
  }
  else {
   /* label? */
   if (sym == colon) handle_label(idholder);
   else _error(24);   /* colon expected */
   insymbol();
  }
 } else if (sym == shortconst || sym == longconst) { /* line number? */
  make_label_from_linenum(sym,idholder);
  handle_label(idholder);
  insymbol();
 } else if (sym == letsym) {  /* assign with LET ? */ 
  insymbol();

  if (sym == ident) {
   strcpy(idholder,id);  /* save info for assign() */
   oldobj=obj;
   oldtyp=typ;

   insymbol();

   if ((sym == equal) || (sym == lparen) || (sym == memberpointer)) {
     strcpy(id,idholder);  /* restore info */
     obj=oldobj;
     typ=oldtyp;
     if (sym == equal) have_equal=TRUE;
     if (sym == lparen) have_lparen=TRUE;
     assign(); 
     have_equal=FALSE;
     have_lparen=FALSE;
   }
  } else _error(7); 
 } else
 /* multi-statement? */
 if (sym == colon) {
  while (sym == colon) { insymbol(); statement(); }
 } else
   /* assem */
 if (sym == assemsym) assem();
 else
 /* area */
 if (sym == areasym) area();
 else
 /* areafill */
 if (sym == areafillsym) areafill();
 else
 /* back */
 if (sym == backsym) {
  insymbol();
  gen_Flt(expr());
  gen_call_args("_back","d0",0);
  enter_XREF("_MathTransBase");
 }
 else
 /* beep */
 if (sym == beepsym)  { 
  gen_jsr("_beep");
  enter_XREF("_MathBase");  /* _sound needs mathffp.library */
  insymbol();
 }  
 else
 /* bevelbox */
 if (sym == bevelboxsym) bevel_box();
 else
 /* event trapping activation/deactivation? */
 if ((sym == breaksym) || (sym == mousesym) || 
     (sym == timersym) || (sym == errorsym)) 
    change_event_trapping_status(sym);
 else
 /* block */
 if (sym == blocksym) block_statement();
 else
 /* call */
 if (sym == callsym) {
  check_for_event();

  insymbol();
  if (sym != ident)  _error(32);
  else {
   /* function? */
   strcpy(func_name,id);
   remove_qualifier(func_name);

   if (exist(func_name,function)) { 
    func_item=curr_item;
    if (func_item->no_of_params != 0)
    { insymbol(); load_func_params(func_item); }
    /* call it */
    if ((libnum=check_for_ace_lib(func_item->libname)) == NEGATIVE) 
       make_library_base(func_item->libname);
    else
       strcpy(librarybase,acelib[libnum].base);
    gen_load32a(librarybase,6);
    itoa(func_item->address,func_address,10);
    strcat(func_address,"(a6)");
    gen_jsr(func_address);
    if (restore_a4) { gen_load32a("_a4_temp",4); restore_a4=FALSE; }
    if (restore_a5) { gen_load32a("_a5_temp",5); restore_a5=FALSE; }
   } else {
    /* subprogram, machine code subroutine or external function? */
    strcpy(sub_name,"_SUB_");
    strcat(sub_name,id);

    if (!exist(sub_name,subprogram)) {
     /* make external variable name 
        by removing qualifier and 
        adding an underscore prefix 
        if one is not present. 
     */
     strcpy(buf,ut_id);
     remove_qualifier(buf);
     if (buf[0] != '_') {
	   strcpy(ext_name,"_");
	   strcat(ext_name,buf);
     } else 
	   strcpy(ext_name,buf);

      if (exist(ext_name,extfunc)) { 
       /* external function? */
       insymbol();
       call_external_function(ext_name,&need_symbol);
      } else { 
      /* machine code subroutine? */
      if (exist(id,variable) && (curr_item->type == longtype)) {
       mc_item=curr_item;
       insymbol();
       if (sym == lparen) { load_mc_params(mc_item); need_symbol=TRUE; } 
       else 
           { mc_item->no_of_params=0; need_symbol=FALSE; }

       /* call routine */
       itoa(-1*mc_item->address,addrbuf,10);
       strcat(addrbuf,frame_ptr[lev]);
       /* pop parameters? */
       if (mc_item->no_of_params != 0) {
         popcount=0;
         for (i=0;i<mc_item->no_of_params;i++) {
		   if (mc_item->p_type[i] == shorttype) popcount += 2;
		   else popcount += 4;
         }
		 gen_pop_ignore(popcount);
       }
      } else _error(37); /* undeclared subprogram */
     }
    } else {
	  /* user-defined subprogram */
	  if (curr_item->no_of_params != 0) 
		{ insymbol(); load_params(curr_item); }
	  gen_jsr(sub_name);
	}
   }
  }
  if (need_symbol) insymbol();
 } else
 /* case */
 if (sym == casesym) { check_for_event(); case_statement(); }
 else if (sym == chdirsym) chdir();
 else if (sym == circlesym) circle();
 else if (sym == clearsym) {
   insymbol(); 
   if (sym == allocsym) { gen_jsr("_clear_alloc"); }
   insymbol();
 } else if (sym == closesym) close_a_file();
 else if (sym == clssym) { 
   gen_jsr("_cls"); 
   insymbol(); 
 } else if (sym == colorsym) color();
 else if (sym == commonsym) define_common_or_global_variable(sym);
 else if (sym == constsym) define_constant();
 else if (sym == datasym) get_data();
 else if (sym == declaresym) declare();
 else if (sym == defintsym) change_id_type(shorttype);
 else if (sym == deflngsym) change_id_type(longtype);
 else if (sym == defsngsym) change_id_type(singletype);
 else if (sym == defdblsym) change_id_type(singletype);
 else if (sym == defstrsym) change_id_type(stringtype);
 else if (sym == dimsym) dim();
 else if ((sym == endsym) || (sym == stopsym)) {
   gen_jmp("_EXIT_PROG");
   insymbol();
 } else if (sym == exitsym) { /* exit (ie: EXIT SUB/FOR) */
  insymbol();
  if (sym == forsym) {
  	/* EXIT FOR */
	gen_nop();
	exit_for_cx = curr_code;
	insymbol();
  } else if (lev == ONE) {
	/* EXIT SUB */
   	if (sym != subsym) _error(35);
   	else gen_jmp(exit_sub_name);
   	insymbol();
  } else {
	_error(36); /* can only use EXIT SUB in a subprogram! */
	insymbol();
  }
 } else if (sym == externalsym) define_external_object();
 else if (sym == filessym) files();
 else if (sym == fixsym) {
	 insymbol();
	 make_sure_long(expr());
	 gen_call_args("_fix","d0",0);
 } else if (sym == fontsym) text_font();
 else if (sym == forsym) for_statement();
 else if (sym == forwardsym) {
   insymbol();
   gen_Flt(expr());
   gen_call_args("_forward","d0",0);
   enter_XREF("_MathTransBase");
 } else  if (sym == gadgetsym) gadget();
 else if (sym == getsym) {
 	insymbol();
	if (sym == lparen) {
		/* Graphics GET */
	} else {
	  /* Random File GET */
	  random_file_get();
	}
 } else if (sym == globalsym) define_common_or_global_variable(sym);
 else if ((sym == gotosym) || (sym == gosubsym)) {
  check_for_event();
  oldlevel=lev; /* labels are defined at level ZERO only */
  lev=ZERO;
  commandsym=sym;
  insymbol();
  if (sym == ident || sym == shortconst || sym==longconst) {
   if (sym != ident) make_label_from_linenum(sym,id);   
   /* generate approriate branch */
   switch(commandsym) {
	 case gotosym  : gen_branch("jmp",id); break;
	 case gosubsym : gen_branch("jsr",id); break;
	 }
  }
  lev=oldlevel;
  insymbol(); 
 } else if (sym == homesym) { gen_jsr("_home"); insymbol(); }
 else if (sym == ifsym) { check_for_event(); if_statement(); }
 else if (sym == iffsym) iff();
 else if (sym == killsym) kill();
 else if (sym == inputsym) { 
  check_for_event(); 
  insymbol();
  if (sym == hash) input_from_file(); else input(); 
 } else if (sym == librarysym) library();
 else if (sym == linesym) {
  insymbol();
  if (sym == inputsym) line_input(); else draw_line();
 } else if (sym == locatesym) {
  insymbol();
  make_sure_short(expr());  /* ROW */
  if (sym == comma) {
	insymbol();
	make_sure_short(expr());
  } else gen_push16_val(1); /* COLUMN */
  gen_call_args("_locate","d1.w,d0.w",0);
 } else if (sym == longintsym || sym == addresssym) declare_variable(longtype);
 else if (sym == menusym) menu();
 else if (sym == messagesym) message();
 else if (sym == msgboxsym) MsgBox();
 else if (sym == namesym) ace_rename();
 else if ((sym == nextsym) && (lastsym == colon))  /* next */ {
   lastsym=undefined;
   return;  /* eg: for i=1 to 10:next */
 } else  if (sym == onsym) {
   /* on <event> | <integer-expression> */
   insymbol();
   if (sym==breaksym || sym==mousesym || sym==menusym || 
	   sym==timersym || sym==errorsym || sym==windowsym || 
	   sym==gadgetsym)
     get_event_trap_label();
   else
     on_branch();    
 } else if (sym == opensym) open_a_file();
 else if (sym == optionsym) parse_option_list();
 else if (sym == paintsym) paint();
 else if (sym == palettesym) { insymbol(); gen_fcall("_palette",expr(),"w,f,f,f",notype,"d3,d2,d1,d0.w",0); }
 else if (sym == patternsym) pattern();
 else if (sym == pendownsym) {
  gen_jsr("_pendown");
  insymbol();
 } else if (sym == penupsym) {
  gen_jsr("_penup");
  insymbol();
 } else if (sym == pokesym) poke();
 else if (sym == pokewsym) pokew();
 else if (sym == pokelsym) pokel();
 else if ((sym == printsym) || (sym == question)) 
 { 
  check_for_event(); 
  print_statement(); 
  if (sym == hash && (lastsym == printsym || lastsym == question)) 
     print_to_file();
 }
 else if (sym == printssym) { check_for_event(); prints_statement(); }    
 else if (sym == psetsym) pset(); 
 else if (sym == putsym) {
 	insymbol();
	if (sym == stepsym || sym == lparen)  {
		/* Graphics PUT */
	} else {
	  /* Random File PUT */
	  random_file_put();
	}
 } else if (sym == readsym) { check_for_event(); read_data(); }
 else if (sym == remsym) {
  while ((sym != endofline) && (!end_of_source)) nextch();
  insymbol();
 } else if (sym == randomizesym)
 {
  insymbol();
  statetype = expr();
  if ((statetype = make_integer(statetype)) == notype) _error(4);
  if (statetype == shorttype) make_long();
  gen_pop32d(0);
  gen_jsr("_randomise");
  enter_XREF("_MathBase");
 }
 else if (sym == repeatsym) repeat_statement();
 else if (sym == restoresym) { gen_move32("#_BASICdata","_dataptr"); insymbol(); }
 else if (sym == returnsym) {
   check_for_event();
   gen_rts();
   insymbol();
 } else if (sym == saysym) say();
 else if (sym == screensym) { screen(); check_for_event(); }
 else if (sym == scrollsym) scroll();
 else if (sym == serialsym) { check_for_event(); serial_command(); }
 else if (sym == setheadingsym) {
   insymbol();
   gen_pop_as_short(expr(),0);
   gen_jsr("_setheading");
 } else if (sym == setxysym) {
   insymbol();
   make_sure_short(expr()); /* x */
   if (sym != comma) _error(16);
   else {
	 insymbol();
	 make_sure_short(expr()); /* y */
	 gen_call_args("_setxy","d1.w,d0.w",0);
   }
 }
 else if (sym == sharedsym && lev == ZERO) { _error(69); insymbol(); } /* shared */
 else if (sym == shortintsym) declare_variable(shorttype);
 else if (sym == singlesym) declare_variable(singletype);
 else if (sym == sleepsym)  {
    insymbol();
    if (sym != forsym) gen_jsr("_sleep"); 
    else { 
	  /* SLEEP FOR <seconds> */ 
	  insymbol();
	  gen_fcall("_sleep_for_secs",expr(),"s",notype,"",4);
    }
 }
 else if (sym == stringsym) declare_variable(stringtype);
 else if (sym == soundsym) sound();
 else if (sym == structsym) define_structure();
 else if (sym == stylesym) text_style();
 else if (sym == swapsym) swap();
 else if (sym == systemsym) {
   insymbol();
   stype = make_integer(expr());
   if (stype == shorttype || stype == longtype) {
     /* SYSTEM returncode */
     if (stype == shorttype) make_long()  ; /* get short integer exit value */
     gen_pop32_var("_returncode");
     gen_jmp("_EXIT_PROG");
   } else {
     /* SYSTEM command-string */
     gen_call_void("_system_call",4);
   }
 } else if (sym == turnsym) {
   insymbol();
   gen_pop_as_short(expr(),0);
   gen_jsr("_turn");
 } else if (sym == turnleftsym) {
   insymbol();
   gen_pop_as_short(expr(),0);
   gen_jsr("_turnleft");
 } else if (sym == turnrightsym) {
   gen_pop_as_short(expr(),0);
   gen_jsr("_turnright");
 } else if ((sym == untilsym) && (lastsym == colon)) {
   lastsym=undefined;
   return; /* eg: repeat:..:until i>10 */
 } else if (sym == wavesym) wave();
 else if (sym == whilesym) while_statement();
 else if ((sym == wendsym) && (lastsym == colon)) {
   lastsym=undefined;
   return; /* eg: while i>2:wend */
 } else if (sym == windowsym) { window(); check_for_event(); }
 else if (sym == writesym) { check_for_event(); write_to_file(); }
 else if (sym == increment || sym == decrement) /* ++ | -- */ increment_decrement(sym);
 else if (sym == shortpointer) { /* *%<address> = <expr> */
   insymbol();
   if (expr() != longtype)  /* address */
	 _error(4);
   else {
	 if (sym != becomes) _error(5);
	 else {
	   insymbol();
	   gen_pop_as_short(expr(),0); /* expression */
	   gen_pop_addr(0);  /* pop address */
	   gen_save_indirect16(0,0);  /* store expression */
	 }
   }
 } else if (sym == longpointer) { /* *&<address> = <expr> */
   insymbol();
   if (expr() != longtype)  /* address */
	 _error(4);
   else {
	 if (sym != becomes) _error(5);
	 else {
	   insymbol();
	   make_sure_long(expr());
	   gen_pop32d(0);  /* pop expression */ 
	   gen_pop_addr(0);  /* pop address */
	   gen_save_indirect32(0,0);  /* store expression */
	 }
   }
 } else if (sym == singlepointer) { /* *!<address> = <expr> */
   insymbol();
   if (expr() != longtype) _error(4); /* address */
   else {
	 if (sym != becomes) _error(5);
	 else {
	   insymbol();
	   gen_Flt(expr());
	   gen_pop32d(0);  /* pop expression */ 
	   gen_pop_addr(0);  /* pop address */
	   gen_save_indirect32(0,0);  /* store expression */
	 }
   }
 }
 else
   /* feature not implemented? */  
   if (obj == rsvd_word) { _error(68); insymbol(); }
   else
	 /* unknown */
	 insymbol();
}
