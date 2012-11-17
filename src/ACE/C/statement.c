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
extern	BOOL	narratorused;
extern	BOOL	end_of_source;
extern	char 	exit_sub_name[80];

int   statetype; /* Used by randomize and wave */

static void say() {
  char addrbuf[80];
  insymbol();
  if (expr() != stringtype) _error(4);  /* phoneme string on stack */
  
  if (try_comma()) {
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
   if (eat_comma()) {
	 if (sym == sinsym) {
	   gen_clear_addr(); /* SIN wave = 0 -> flag for _wave */
	   insymbol();
	 } else {
	   /* now expect an address -> pointer to a block of bytes */
	   if (expr() != longtype) _error(4);
	   
	   /* number of bytes? */
	   if (eat_comma()) {
           long_expr();
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
		
		if (opsym == increment) gen_incr_indir(inc_item->type);
		else gen_decr_indir(inc_item->type);
	  }
	}
	insymbol();
  } 
}

void call_shared_lib_func(SYM * func_item)
{
    char func_address[MAXIDSIZE + 9];
    BYTE libnum;

    /* call shared library function */
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
    gen_restore_regs();
}

static void call_statement() {
  char  func_name[MAXIDSIZE];
  SYM * mc_item;
  char sub_name[80],buf[50];
  char addrbuf[80];
  char  ext_name[MAXIDSIZE+1];
  BOOL  need_symbol=TRUE;
  SHORT popcount;
  int i;

	 check_for_event();
	 insymbol();
	 if (sym != ident)  _error(32);
	 else {
	   /* function? */
	   strcpy(func_name,id);
	   remove_qualifier(func_name);
	   
	   if (exist(func_name,function)) { 
           call_shared_lib_func(curr_item);
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
}


/* ------*/
/* sound */
/* ------*/

static void sound() {
/* make a tone of given period, duration and volume through 
   the specified channel */

    BOOL voice=FALSE;
    BOOL volume=FALSE;

    insymbol();
    make_sure_short(expr());  /* period (short) 0..32767 */ 

    if (eat_comma()) {
        gen_Flt(expr());  /* duration (single) 0..77 */
    }

    if (try_comma()) {
        if (sym != comma) {  /* if comma -> skip volume */
            make_sure_short(expr());  /* volume (short) 0..64 */
            volume=TRUE;
        } else gen_load32d_val(64,2);   /* default volume = 64 */
    } else gen_load32d_val(64,2);   /* default volume = 64 */

    if (try_comma()) {
        make_sure_short(expr());  /* voice (short) 0..3 */
        voice=TRUE;
    } else gen_load32d_val(0,3);   /* default voice = 0 */
    
    if (voice)  gen_pop16d(3);  /* pop voice */
    if (volume) gen_pop16d(2);  /* pop volume */

    gen_call_args("_sound","d1,d0.w",0);
    enter_XREF("_MathBase");
}   

static void handle_label(char * label_name)
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
  } else _error(6);  /* duplicate label */
  lev=oldlevel;
}

/*-----------*/
/* statement */
/*-----------*/

struct Function f_font     = {"is,l",{},"_change_text_font", 8};
struct Function f_style    = {"il",  {},"_change_text_style",4}; 
struct Function f_bevelbox = {"ir,l",{},"_BevelBox",20};
void statement()
{
char  buf[50],idholder[50],sub_name[80];
char  func_name[MAXIDSIZE];
char  ext_name[MAXIDSIZE+1];
int   commandsym;
int   oldobj,oldtyp,stype;
int   oldlevel;
SYM   *sub_item;
BOOL  need_symbol=TRUE;

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
    } else if (exist(func_name,function)) {
        call_shared_lib_func(curr_item);
    } else {
      /* call SUB */
      if (sub_item->no_of_params != 0)  { load_params(sub_item); insymbol(); }
      gen_jsr(sub_name);
    }
  } else {
   /* label? */
   if (sym == colon) handle_label(idholder);
   else _error(24);   /* colon expected */
   insymbol();
  }
 } else if ((sym == wendsym) && (lastsym == colon)) {
   lastsym=undefined;
   return; /* eg: while i>2:wend */
 } else if ((sym == untilsym) && (lastsym == colon)) {
   lastsym=undefined;
   return; /* eg: repeat:..:until i>10 */
 } else if (sym == sharedsym && lev == ZERO) { _error(69); insymbol(); } /* shared */
 else if ((sym == nextsym) &&
		  (lastsym == colon))  /* next */ {
   lastsym=undefined;
   return;  /* eg: for i=1 to 10:next */
 } else { 

   switch (sym) {
   case shortconst:
   case longconst:  /* line number? */
	 make_label_from_linenum(sym,idholder);
	 handle_label(idholder);
	 insymbol();
	 break;
   case letsym: /* assign with LET ? */ 
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
	 break;
   case colon: /* multi-statement? */
       while (eat(colon)) { statement(); }
	 break;
   case assemsym: assem(); break;
   case areasym:  area(); break;
   case areafillsym: areafill(); break;
	 
	 /* event trapping activation/deactivation? */
   case breaksym:
   case mousesym:
   case timersym:
   case errorsym:
	 change_event_trapping_status(sym);
	 break;

   case defintsym:   change_id_type(shorttype); break;
   case deflngsym:   change_id_type(longtype); break;
   case defsngsym:   change_id_type(singletype); break;
   case defdblsym:   change_id_type(singletype); break;
   case defstrsym:   change_id_type(stringtype); break;

   case casesym: check_for_event(); case_statement(); break;
   case ifsym:   check_for_event(); if_statement(); break;
   case serialsym: check_for_event(); serial_command(); break;
   case printssym: check_for_event(); prints_statement(); break;
   case readsym: check_for_event(); read_data(); break;
   case writesym:  check_for_event(); write_to_file(); break;

   case screensym: screen(); check_for_event(); break;
   case windowsym:  window(); check_for_event(); break;

   case blocksym:    block_statement(); break;
   case callsym:     call_statement(); break; 
   case chdirsym:    chdir(); break;
   case circlesym:   circle(); break;
   case closesym:    close_a_file(); break;
   case colorsym:    color(); break;
   case commonsym:   define_common_or_global_variable(sym); break;
   case constsym:    define_constant(); break;
   case datasym:     get_data(); break;
   case declaresym:  declare(); break;
   case dimsym:      dim(); break;
   case iffsym:      iff(); break;
   case killsym:     kill(); break;
   case externalsym: define_external_object(); break;
   case filessym:    files(); break;
   case forsym:      for_statement(); break;
   case gadgetsym:   gadget(); break;
   case globalsym:   define_common_or_global_variable(sym); break;
   case librarysym: library(); break;
   case menusym: menu(); break;
   case messagesym: message(); break;
   case msgboxsym: MsgBox(); break;
   case namesym: ace_rename(); break;
   case pokesym:    poke(); break;
   case pokewsym: pokew(); break;
   case pokelsym: pokel(); break;
   case opensym: open_a_file(); break;
   case optionsym: parse_option_list(); break;
   case paintsym: paint(); break;
   case patternsym: pattern(); break;
   case psetsym: pset(); break;
   case repeatsym: repeat_statement(); break;
   case stringsym: declare_variable(stringtype); break;
   case soundsym: sound(); break;
   case structsym: define_structure(); break;
   case swapsym: swap(); break;
   case wavesym: wave(); break;
   case whilesym: while_statement(); break;
   case saysym: say(); break;

   case clssym:       gen_jsr("_cls"); insymbol(); break;
   case homesym:      gen_jsr("_home"); insymbol(); break;
   case pendownsym:   gen_jsr("_pendown"); insymbol(); break;
   case penupsym:     gen_jsr("_penup"); insymbol(); break;

   case fontsym:      parse_call_func(&f_font); break;
   case stylesym:     parse_call_func(&f_style); break;
   case bevelboxsym:  parse_call_func(&f_bevelbox); break;

   case scrollsym:    if (parse_gen_params(0,"ir,w,w")) gen_scrollraster(); break; 

   case palettesym:   insymbol(); gen_fcall("_palette",expr(),"w,f,f,f",notype,"d3,d2,d1,d0.w",0); break;
   case setxysym:     insymbol(); gen_fcall("_setxy",expr(),"w,w",shorttype,"d1.w,d0.w",0); break; 
   case turnsym:      insymbol(); gen_fcall("_turn",expr(),"w",shorttype,"d0.w",0); break;
   case turnleftsym:  insymbol(); gen_fcall("_turnleft",expr(),"w",shorttype,"d0.w",0); break;
   case turnrightsym: insymbol(); gen_fcall("_turnright",expr(),"w",shorttype,"d0.w",0); break;
   case fixsym:       insymbol(); gen_fcall("_fix",expr(),"l",longtype,"d0",0); break;

   case backsym: 
	 insymbol();
	 gen_Flt(expr());
	 gen_call_args("_back","d0",0);
	 enter_XREF("_MathTransBase");
	 break;

   case forwardsym:
	 insymbol();
	 gen_Flt(expr());
	 gen_call_args("_forward","d0",0);
	 enter_XREF("_MathTransBase");
	 break;

   case beepsym: 
	 gen_jsr("_beep");
	 enter_XREF("_MathBase");  /* _sound needs mathffp.library */
	 insymbol();
	 break;

   case clearsym:
	 insymbol(); 
	 if (sym == allocsym) { gen_jsr("_clear_alloc"); }
	 insymbol();
	 break;
	 
   case endsym:
   case stopsym: 
	 gen_jmp("_EXIT_PROG");
	 insymbol();
	 break;

   case exitsym:  /* exit (ie: EXIT SUB/FOR) */
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
	 break;

   case getsym: 
	 insymbol();
	 if (sym == lparen) {
	   /* Graphics GET */
	 } else {
	   /* Random File GET */
	   random_file_get();
	 }
	 break;

   case gotosym:
   case gosubsym:
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
	 break;

   case inputsym: 
	 check_for_event(); 
	 insymbol();
	 if (sym == hash) input_from_file(); else input(); 
	 break;

   case linesym:
	 insymbol();
	 if (sym == inputsym) line_input(); else draw_line();
	 break;

   case locatesym: 
	 insymbol();
	 make_sure_short(expr());  /* ROW */
	 if (try_comma()) {
	   make_sure_short(expr());
	 } else gen_push16_val(1); /* COLUMN */
	 gen_call_args("_locate","d1.w,d0.w",0);
	 break;

   case longintsym:
   case addresssym:
	 declare_variable(longtype);
	 break;

   case onsym:
	 /* on <event> | <integer-expression> */
	 insymbol();
	 if (sym==breaksym || sym==mousesym || sym==menusym || 
		 sym==timersym || sym==errorsym || sym==windowsym || 
		 sym==gadgetsym)
	   get_event_trap_label();
	 else
	   on_branch();    
	 break;

   case printsym:
   case question:
	 check_for_event(); 
	 print_statement(); 
	 if (sym == hash && (lastsym == printsym || lastsym == question)) 
	   print_to_file();
   break;

 case putsym: 
   insymbol();
   if (sym == stepsym || sym == lparen)  {
	 /* Graphics PUT */
   } else random_file_put();
   break;
 case remsym: 
   while ((sym != endofline) && (!end_of_source)) nextch();
   insymbol();
   break;
 case randomizesym:
   insymbol();
   statetype = expr();
   if ((statetype = make_integer(statetype)) == notype) _error(4);
   if (statetype == shorttype) make_long();
   gen_pop32d(0);
   gen_jsr("_randomise");
   enter_XREF("_MathBase");
   break;
 case restoresym: gen_move32("#_BASICdata","_dataptr"); insymbol(); break;
 case returnsym: 
   check_for_event();
   gen_rts();
   insymbol();
   break;
 case setheadingsym: 
   insymbol();
   gen_pop_as_short(expr(),0);
   gen_jsr("_setheading");
   break;

 case shortintsym: declare_variable(shorttype); break;
 case singlesym:   declare_variable(singletype); break;

 case sleepsym: 
   insymbol();
   if (sym != forsym) gen_jsr("_sleep"); 
   else { 
	 /* SLEEP FOR <seconds> */ 
     insymbol();
	 gen_fcall("_sleep_for_secs",expr(),"f",notype,"",4);
   }
   break;

 case systemsym:
   insymbol();
   stype = make_integer(expr());
   if (stype == shorttype || stype == longtype) {
     /* SYSTEM returncode */
     make_sure_long(stype);
     gen_pop32_var("_returncode");
     gen_jmp("_EXIT_PROG");
   } else {
     /* SYSTEM command-string */
     gen_call_void("_system_call",4);
   }
   break;

 case  increment:  /* ++ */
 case decrement:   /* -- */
   increment_decrement(sym);
   break;

 case shortpointer: /* *%<address> = <expr> */
   insymbol();
   if (expr() != longtype) _error(4); /* address */
   else {
	 if (sym != becomes) _error(5);
	 else {
	   insymbol();
	   gen_pop_as_short(expr(),0); /* expression */
	   gen_pop_addr(0);  /* pop address */
	   gen_save_indirect16();  /* store expression */
	 }
   }
   break;

 case longpointer:  /* *&<address> = <expr> */
  insymbol();
  if (expr() != longtype)  _error(4); /* address */
  else {
	if (sym != becomes) _error(5);
	else {
	  insymbol();
	  make_sure_long(expr());
	  gen_pop32d(0);  /* pop expression */ 
	  gen_pop_addr(0);  /* pop address */
	  gen_save_indirect32();  /* store expression */
	}
  }
  break;

 case singlepointer: /* *!<address> = <expr> */
   insymbol();
   if (expr() != longtype) _error(4); /* address */
   else {
	 if (sym != becomes) _error(5);
	 else {
	   insymbol();
	   gen_Flt(expr());
	   gen_pop32d(0);  /* pop expression */ 
	   gen_pop_addr(0);  /* pop address */
	   gen_save_indirect32();  /* store expression */
	 }
   }
   break;
 default:
   /* feature not implemented? */  
   if (obj == rsvd_word) { _error(68); insymbol(); }
   else
	 /* unknown */
	 insymbol();
   }
 }
}
