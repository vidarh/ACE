/* << ACE >>

   -- Amiga BASIC Compiler --

   ** Parser: file functions **
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
	   4th,5th,6th January 1993,
	   12th,14th,15th February 1993,
	   1st March 1993,
	   9th,17th,18th May 1993,
	   15th December 1993,
	   2nd January 1994,
	   11th March 1995,
	   10th March 1996
*/

#include "acedef.h"
#include "codegen.h"

/* locals */
static 	char	*frame_ptr[] = { "(a4)", "(a5)" };
	
/* externals */
extern	int	lev;
extern	int	sym;
extern	int	obj;
extern	int	typ;
extern	BOOL	end_of_source;
extern	SYM	*curr_item;
extern	char	id[MAXIDSIZE];
extern	char	tempstrname[80];
	
/* functions */
/* OPEN mode,[#]filenumber,filespec */
void open_a_file() {
    if (parse_call_func_args(expr(),"Eis,#l,s")) gen_openfile();
}

/* CLOSE [#]filenumber[,[#]filenumber..] */
void close_a_file() {
 check_for_event();
 do {
     parse_channel();
     gen_closefile();
 } while (sym == comma);
}

void assign_string_to_storage(SYM * storage, char * addrbuf) {
  if (storage->object == variable)
	assign_to_string_variable(storage,MAXSTRLEN);
  else if (storage->object == array) {
	point_to_array(storage,addrbuf);
	assign_to_string_array(addrbuf);
  }
}

void line_input() {
  char addrbuf[40];
  SYM  *storage;

 /* LINE INPUT  [;][prompt-string;]string-variable
    LINE INPUT# filenumber;string-variable

    Note: only the latter is currently implemented.
 */

  if (!parse_gen_params(0,"Ei#l,")) return;

  if (sym == ident && obj == variable) {
      /* if string variable/array doesn't exist, create a simple variable */
      if (!exist(id,variable) && !exist(id,array)) {
          /* allocate a simple string variable */
          enter(id,typ,obj,0);
          gen_push_nullstring();
          assign_to_string_variable(curr_item,MAXSTRLEN);
      }
      
      storage=curr_item;

   /* is it a string variable or array? */
   if (storage->type != stringtype) { _error(4); return; }
   /* get address of string pointed to by variable/array element */
   itoa(-1*storage->address,addrbuf,10);
   strcat(addrbuf,frame_ptr[lev]);
	 
   /* pass filenumber (d0) and string address (a0) to function */
   if (storage->object == array) {
	 point_to_array(storage,addrbuf);
     gen_load32a_plus_index(addrbuf);
   } else gen_load32a(addrbuf,0);	/* string address */
	 
   gen_line_input();
	 
   insymbol();
   if (sym == lparen && storage->object != array) 
	 _error(71);  /* undeclared array */
 } else _error(19); /* variable (or array) expected */
}

void write_to_file() {
  int wtype;

  /* WRITE #filenumber,expression-list */
  
  if (!parse_gen_params(0,"Ei#il")) return;

  gen_pop_filenumber();
  
  if (!eat_comma()) return;
  /* get expressions */
  do {
	insymbol();
	wtype=expr(); 
	
	switch(wtype) {
	case undefined : _error(0);  break; /* expression expected */ 
	case shorttype : gen_writeshort(); break;
	case longtype :  gen_writelong(); break;
	case singletype : gen_writesingle(); break;
	case stringtype : gen_writestring();
	  break;
	}
	
	/* need a delimiter? */
	if (sym == comma) gen_writecomma();
  } while (sym == comma);  
	
  gen_write_eoln();

  enter_BSS("_seq_filenumber:","ds.l 1");
}

void gen_writecode(int code)
{
 /* write special character sequence to a file */

 check_for_event();

 switch(code) {
 case LF_CODE:    gen_write_eoln(); break;
 case TAB_CODE:   gen_writetab(); break;
 case SPACE_CODE: gen_writespc(); break;
 }
}

void print_to_file() {
  int exprtype,arguments=0;

 /* PRINT #filenumber,expression-list */

 check_for_event();
 insymbol();
 make_sure_long(expr());

 gen_pop_filenumber();

 if (sym != comma) _error(16);
 else {
   do {
   if (sym != ident && !strfunc() && !numfunc() && !factorfunc() && 
       obj != constant) 
      insymbol(); /* ident/func/literal after a space or as first parameter */

   /* end of line, multi-statement, ";", "," ELSE or comment 
      after "PRINT #filenumber," ? -> don't proceed to expr! */

      if ((sym == endofline) || (sym == colon) || (sym == apostrophe) || 
          (sym == semicolon) || (sym == comma) || (sym == elsesym) ||
		  (end_of_source)) {
		if (sym == comma) gen_writecode(TAB_CODE);
		else
		  if ((arguments == 0) && (sym != semicolon)) 
			gen_writecode(LF_CODE);  /* "PRINT #filenumber," with no args */ 
		
		if (sym != colon && sym != elsesym) 
		  insymbol();  /* leave colon for multi-statement 
						  in statement() or leave ELSE for if_statement() */
		return;
      }
	  
      /* get an expression */
      exprtype = expr();
	  
      if (exprtype == undefined) { _error(0); return; } /* illegal syms? */
	  
	  gen_fwrite(exprtype);

      if (exprtype != stringtype) 
		gen_writecode(SPACE_CODE); /* trailing space for any number */

      arguments++;

      if (sym == comma) gen_writecode(TAB_CODE);
   
  }
  while ((sym == comma) || (sym == semicolon) || (sym == ident) ||
 	  strfunc() || numfunc() || factorfunc() || obj == constant);  

  /* no comma or semicolon at end of PRINT# -> LF */
  gen_writecode(LF_CODE);
 }
}

void input_from_file() {
  char addrbuf[80];
  SYM  *storage;
  
  /* INPUT #filenumber,variable-list */
  
  check_for_event();
  insymbol();

  if (make_integer(expr()) == shorttype) make_long();
  
  gen_pop_filenumber();
  
  if (sym != comma) _error(16);
  else {
	do { 
	  /* allocate variable storage, call _input* and store value in variable */

	  insymbol();

	  if ((sym == ident) && (obj == variable)) {
		if ((!exist(id,obj)) && (!exist(id,array)))
		  enter(id,typ,obj,0);  /* allocate storage for a simple variable */

		storage = curr_item;

		itoa(-1*storage->address,addrbuf,10);
		strcat(addrbuf,frame_ptr[lev]); 
  
		/* ALL data types need a temporary string pointer in a0 */
		load_temp_string(0);

		/* when storing an input value into an array element, must save
		   value (d0) first, since array index calculation may be corrupted
		   if index has to be coerced from ffp to short.
		*/

		/* pass file number */
		gen_load32d("_seq_filenumber",0);
		switch(storage->type) {
		case shorttype:  input_short(storage,"_finputshort",addrbuf,lev); break;
		case longtype:   input_long(storage,"_finputlong",addrbuf,lev); break;
		case singletype: input_single(storage,"_finputsingle",addrbuf,lev); break;
		case stringtype : 
		  gen_jsr("_finputstring");
		  gen_push_addr(0); 
		  assign_string_to_storage(storage,addrbuf);
		  break;
		}
	  } else _error(19);
	  
	  insymbol();
	  if (sym == lparen && storage->object != array) _error(71); 	  /* undeclared array */
	} while (sym == comma);
  }
}

/* KILL <filespec> */
void kill() {
  check_for_event();
  insymbol();
  if (expr() != stringtype) _error(4);
  else gen_kill();
}

/* NAME <filespec1> AS <filespec2> */
void ace_rename() {
  check_for_event();
  insymbol();
  if (expr() != stringtype) { _error(4); return; }
  if (sym != assym) { _error(72); return; }
  if (expr() != stringtype) { _error(4); return; }

  gen_rename();
}

/* CHDIR <dirname> */
void chdir() {
 check_for_event();
 insymbol();
 gen_chdir(expr());
}

/* FILES [TO <storefile>] [,<target>] */
void files() {
  check_for_event();
  insymbol();

  /* storage file specified? */
  if (sym == tosym) {
	insymbol();
	if (expr() != stringtype) _error(4);
  } else gen_push32_val(0);  /* NULL for storage file name */
      
  /* target file or directory specified? */
  opt_arg(stringtype,0); /* NULL for target name */
  gen_files();
}

void push_struct_var_info(SYM * structVar) {
  char addrbuf[40];

  /* Push address held by structure variable. */
  sprintf(addrbuf,"%d%s",-1*structVar->address, frame_ptr[lev]);
  gen_push_address(structVar->shared && lev == ONE, addrbuf, longtype);
  /* Push size of structure in bytes. */
  gen_push32_val(structVar->other->size);
}

/* Put/Get data to/from a random file */
void random_file_action() {
  SYM *structVar;

  check_for_event(); 
	
  /* We already have the first symbol. */
  if (sym == hash) insymbol(); /* skip '#' if present */
  make_sure_long(expr()); /* filenum */

  if (eat_comma()) return;
  /* Structure variable address and size. */
  if (!exist(id,structure)) { _error(79); return; }

  structVar = curr_item;
  push_struct_var_info(structVar);
  
  insymbol();
  opt_arg(longtype,0); /* Optional record numer; 0 default = don't seek before read/write */
}

/* SYNTAX: GET [#]fileNum, structVar [, recordNum] */
void random_file_get() {
  random_file_action();
  gen_getrecord();
}

/* SYNTAX: PUT [#]fileNum, structVar [, recordNum] */
void random_file_put() {
  random_file_action();
  gen_putrecord();
}
