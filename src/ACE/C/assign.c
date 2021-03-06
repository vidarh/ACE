/* << ACE >>

   -- Amiga BASIC Compiler --

   ** Parser: variable assignment code **
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
	   6th,22nd December 1992,
	   6th January 1993,
	   12th,15th February 1993,
	   12th,16th April 1993,
	   20th,30th June 1993,
	   11th October 1993,
	   5th,16th-18th December 1993,
	   2nd January 1994,
	   21st June 1994,
	   20th August 1994,
	   1st,10th September 1994,
	   1st October 1994,
	   11th March 1995
*/

#include "acedef.h"
#include "codegen.h"

#define QUN_CODE 3

/* locals */
static	char 	*frame_ptr[] = { "(a4)","(a5)" };

/* externals */
extern	int	sym;
extern	int	lastsym;
extern	int	obj;
extern	int	typ;
extern	char   	id[MAXIDSIZE]; 
extern	char   	ut_id[MAXIDSIZE];
extern	SHORT  	shortval;
extern	LONG   	longval; 
extern	float  	singleval;
extern	char   	stringval[MAXSTRLEN];
extern	SYM	*curr_item;
extern	SHORT	dimsize[MAXDIMS];
extern	char   	string_const_start[7];
extern	char   	string_const_end[4];
extern	int  	lev;
extern	int  	strstorecount;
extern	int	stringvarcount;
extern	char 	strstorename[80],strstorelabel[80];
extern	int	storetype;
extern	int  	arraycount;
extern	char 	tempstrname[80];
extern	BOOL 	readpresent;
extern	BOOL 	have_lparen;
extern	BOOL 	have_equal;

void assign_string_to_storage(SYM * storage, char * addrbuf);

/* functions */

int assign_coerce(int storetype,int exptype)
{
 /* coerce expression type to store type */

  if (((storetype == stringtype) && (exptype != stringtype)) ||
    ((storetype != stringtype) && (exptype == stringtype))) return(notype);
 else if (((storetype == shorttype) || (storetype == longtype))
		  && (exptype == singletype)) {
   gen_round(storetype);
 } else if ((storetype == singletype) &&
			((exptype == shorttype) || (exptype == longtype))) {
   gen_Flt(exptype);
 } else if ((storetype == longtype) && (exptype == shorttype)) {
   gen_pop16d(0);
   gen_ext16to32(0);
   push_result(longtype);
 } else if ((storetype == shorttype) && (exptype == longtype)) {
   gen_pop32d(0);
   push_result(shorttype);
 }

 return(storetype);  /* could be bytetype (for struct member) */
}

void make_string_store() {
  char numbuf[40];
  
  itoa(strstorecount++,numbuf,10);
  strcpy(strstorename,"_stringstore");
  strcat(strstorename,numbuf);
  strcpy(strstorelabel,strstorename);
  strcat(strstorelabel,":");
}

void create_string_variable(SYM * string_item,int string_size)
{
  /* creates a unique BSS object for a string variable 
	 and stores its address in the string variable
	 pointer in the stack frame. 
  */
  char stringvarname[40],stringvarlabel[40],bss_size[20];
  char numbuf[10],addrbuf[20];

  /* keep a record of "stringvar" number which is
	 about to be created (for simple string variables
	 and string SUBs only) for future reference. 
  */
  string_item->numconst.longnum = stringvarcount;

  /* make name of BSS object */
  strcpy(stringvarname,"_stringvar");
  itoa(stringvarcount++,numbuf,10);
  /* name */
  strcat(stringvarname,numbuf);
  /* label */
  strcpy(stringvarlabel,stringvarname);
  strcat(stringvarlabel,":");
  
  /* size of BSS object */
  itoa(string_size,numbuf,10);
  strcpy(bss_size,"ds.b ");
  strcat(bss_size,numbuf);
  
  /* enter bss object */
  enter_BSS(stringvarlabel,bss_size);
  
  /* store bss object address in stack frame */
  itoa(-1*string_item->address,addrbuf,10);
  strcat(addrbuf,frame_ptr[lev]);
  
  gen_pea(stringvarname);
  gen_pop32_var(addrbuf); 
}

void assign_to_string_variable(SYM * string_item,int string_size)
{
  /* assigns a string on the stack 
	 to the specified string variable 
  */
  char addrbuf[20],buf[80];

  /* get stack frame address holder */
  itoa(-1*string_item->address,addrbuf,10);
  strcat(addrbuf,frame_ptr[lev]);
  
  if (string_item->new_string_var) {
	/* create a BSS object for new string variable */
	create_string_variable(string_item,string_size);
	string_item->new_string_var=FALSE;  
  } else if (string_item->decl != declared && !string_item->shared) {
	/* Make sure there is a valid address in the 
	   variable's stack-frame address holder EACH
	   time the variable is to be assigned a value.
	   
	   The reason is that unlike a declared string
	   variable or array, an undeclared string variable 
	   might not have a valid address at the time of 
	   assignment since the first occurrence of said 
	   variable may be as part of a case statement which 
	   might NEVER be reached.
	   
	   However, we still need string variable address
	   in stack frame for other purposes (eg: passing
	   to SUBs, use in factor() etc). 
	*/
	sprintf(buf,"#_stringvar%ld",(long)string_item->numconst.longnum);
	gen_move32(buf,addrbuf);
 }

  /* copy string on stack to variable */
  gen_pop_addr(1);  /* source */
  gen_load32a(addrbuf,0);  /* destination */
  gen_strcpy();  /* copy source to destination */
}

void assign_to_string_array(char * addrbuf)
{
  /* - assigns a string on the stack 
     to the specified string array element.
	 - assumes absolute index is in d7.
  */
  
  gen_pop_addr(1); /* source */
  gen_load32a_plus_index(addrbuf); /* destination */
  gen_strcpy();  /* copy source to destination */
}

void assign_to_struct(SYM * item)
{
  /* assign either an address to 
	 a structure variable or a
	 value to one of its members.
  */
  SYM    *structype;
  char   addrbuf[40],absbuf[40];
  STRUCM *member;
  BOOL   found=FALSE;
  int    exprtype,storetype;
  
  if (sym == memberpointer) {
	/* assign value to a member */
	
	/* get pointer to structure 
	   type definition. 
	*/
	structype = item->other; 
	
	insymbol();
	
	if (sym != ident) _error(7);
	else {
	  /* does member exist? */
	  member = structype->structmem->next;
	  while ((member != NULL) && (!found)) {
		if (strcmp(member->name,id) == 0) found=TRUE;
		else member = member->next;
	  }
   
	  /* dereference it? */
	  if (!found) { _error(67); insymbol(); }  /* not a member! */
	  else {
		/* assign value */
		insymbol();
		if (sym != equal) _error(5);
		else {
		  insymbol();
		  exprtype=expr();
		  
		  /* treat byte type as a SHORT when coercing */ 
		  if (member->type == bytetype) storetype=shorttype;
		  else storetype=member->type;  /* short, long, single */

		  storetype = assign_coerce(storetype,exprtype);
		  if (storetype == notype) _error(4);   /* type mismatch */
		  else {
			/* address of structure */
			ltoa(-1*item->address,addrbuf,10);
			strcat(addrbuf,frame_ptr[lev]);

			gen_load32a(addrbuf,0);
			if (item->shared && lev == ONE)
			  gen_load_indirect32();   /* start address of structure */

			/* offset from struct start */ 
			if (member->type != stringtype) {
			  ltoa(member->offset,absbuf,10);
			  strcat(absbuf,"(a0)");
			}

			if (member->type == bytetype) {
			  gen_pop8_var(absbuf);
			} else if (member->type == stringtype) {  /* string */
			  gen_pop_addr(1);  /* source */
			  gen_add_addr_offset(member->offset);  /* destination = struct address + offset */
			  gen_strcpy();  /* copy source to destination */
			} else if (member->type == shorttype) gen_pop16_var(absbuf);  /* short */
			else gen_pop32_var(absbuf);  /* long, single */
		  }
		} 
	  } 
	}
  } else {
	/* assign address of structure */
	if (sym != equal) _error(5); 
	else {
	  insymbol();
	  if (expr() != longtype) _error(4);
	  else {
		/* address of structure */
		ltoa(-1*item->address,addrbuf,10);
		strcat(addrbuf,frame_ptr[lev]);
    
		if (item->shared && lev == ONE) {
		  gen_load32a(addrbuf,0);  /* address of structure variable */
		  gen_pop_indirect(longtype);   /* store new address in variable */
		} else
		  gen_pop32_var(addrbuf);  /* store new address in variable */
	  }
	}
  }
}

void assign() {
  char addrbuf[80],sub_name[80];
  char ext_name[MAXIDSIZE],buf[MAXIDSIZE];
  SYM  *storage_item;
  int  oldlevel;
  int  exprtype;
  
  /* in case it's a subprogram */
  strcpy(sub_name,"_SUB_");
  strcat(sub_name,id);
  
  /* make external variable name by removing qualifier and  
	 adding an underscore prefix if one is not present. 
  */
  strcpy(buf,ut_id);
  remove_qualifier(buf);
  if (buf[0] != '_') {
	strcpy(ext_name,"_");
	strcat(ext_name,buf);
  } else strcpy(ext_name,buf);

  /* does it exist? */
  if (exist(id,constant)) { _error(53); return; }
  else if (exist(id,array)) obj=array;
  else if (exist(id,structure)) { assign_to_struct(curr_item); return; }
  else if (exist(sub_name,subprogram)) obj=subprogram;
  else if (exist(ext_name,extvar)) obj=extvar;
  else if (!exist(id,obj)) enter(id,typ,obj,0); /* create a simple variable */

  storage_item = curr_item;

  if (obj == array) push_indices(storage_item); /* parse indices first! */
  
  /* assign it */
  if (!have_equal) insymbol();
  if (sym == equal) { 
	if (storage_item->object != array) { /* get expression later! */
	  insymbol(); 
	  exprtype = expr();
	  if (exprtype == undefined) _error(0);   /* illegal syms? */
	  storetype = assign_coerce(storage_item->type,exprtype);
	  if (storetype == notype) _error(4);   /* type mismatch */
	}

	if (obj != extvar) {
	  /* get address of object */
	  if (storage_item->object == subprogram) { oldlevel=lev; lev=ZERO; }
	  
	  itoa(-1*storage_item->address,addrbuf,10);
	  strcat(addrbuf,frame_ptr[lev]);
	  
	  if (storage_item->object == subprogram) lev=oldlevel; 
	}

	switch(storage_item->object) {
	case variable: 
	  if ((storage_item->shared) && (lev == ONE) 
		  && (storage_item->type != stringtype)) {
		gen_load32a(addrbuf,0);  /* absolute address of store */
        gen_pop_indirect(storage_item->type);
	  } else if (storage_item->type == stringtype) 
		/* ordinary variable or shared string variable */
		assign_to_string_variable(storage_item,MAXSTRLEN);
	  else if (storage_item->type == shorttype) gen_pop16_var(addrbuf);
	  else gen_pop32_var(addrbuf); /* longtype or singletype */
	  break;

	case subprogram :   
	  if (storage_item->address != extfunc) {
		if (storage_item->type == stringtype) {
		  oldlevel=lev;
		  lev=ZERO; 
		  assign_to_string_variable(storage_item,MAXSTRLEN);
		  lev=oldlevel;
		} else if (storage_item->type == shorttype) gen_pop16_var(addrbuf);
		else
		  /* longtype or singletype */
		  gen_pop32_var(addrbuf);
	  } else {
          /* External subprogram being assigned a value */
          gen_pop(storage_item->type);
	  }
	  break;
		 
	case extvar : 
	  if (storage_item->type == shorttype) gen_pop16_var(ext_name);
	  else if (storage_item->type == stringtype) {
		gen_pop_addr(1);
		gen_load_addr(ext_name,0);
		gen_strcpy();
	  } else
		  /* long integer, single-precision */
		  gen_pop32_var(ext_name);
	  break;
	  
	case array : 	get_abs_ndx(storage_item);
			
			/* save storage info in case it gets clobbered
			   by other arrays in expr()!! */
			gen_save32d(7,"_tmpelement");
			enter_BSS("_tmpelement","ds.l 1");

			/*if (storage_item->type == stringtype)
			{
			 gen("move.l","_stroffset","_tmpstroffset");
			 enter_BSS("_tmpstroffset","ds.l 1");
			}*/

			/* get expression */
   			insymbol(); 
			have_lparen=FALSE; /* may encounter another array */
   			exprtype = expr();
   			if (exprtype == undefined) 
			   _error(0);  /* illegal syms? */
   			storetype = assign_coerce(storage_item->type,exprtype);
   			if (storetype == notype) 
       			   _error(4);   /* type mismatch */
			
			/* restore storage item info */
			gen_load32d("_tmpelement",7);

			if (storage_item->type == stringtype) 
			  assign_to_string_array(addrbuf);
			else {
			  gen_load32a(addrbuf,0);
			  if (storage_item->type == shorttype)
				gen_pop_indirect_indexed16(0,7);
			  else
				/* long or single */
				gen_pop_indirect_indexed32(0,7);
			}
			break;
     }
 } else _error(5); /* '=' expected */
}  

void make_array_name(char * name,char * lab)
{
  char num[20];
 
  strcpy(name,"_array");
  itoa(arraycount++,num,10);
  strcat(name,num);
  strcpy(lab,name);
  strcat(lab,":");
} 

/* declare an array */ 
void dim() 
{
  BOOL  dimmed=TRUE;
  int   index;
  int   arraytype;
  char  arrayid[50];
  SYM   *array_item;
  char  buf[80],numbuf[80],addrbuf[80];
  char  arrayname[80],arraylabel[80];
  LONG  max_element,string_element_size;
 
  do {
	arraytype = undefined;

	insymbol();

	/* type identifiers */
	if (sym == shortintsym || sym == longintsym || sym == addresssym ||
		sym == singlesym || sym == stringsym) {
	  switch(sym) {
	  case shortintsym : arraytype = shorttype;  break;
	  case longintsym  : arraytype = longtype;   break;
	  case addresssym  : arraytype = longtype;   break;
	  case singlesym   : arraytype = singletype; break;
	  case stringsym   : arraytype = stringtype; break;
	  }
	  insymbol();
	}

	if (sym == ident) {
	  if (!exist(id,array)) { 
		dimmed=FALSE; 
		strcpy(arrayid,id); 
		if (arraytype == undefined) arraytype=typ; 
	  } else { _error(22); insymbol(); return; }  /* array already declared */
	  
	  insymbol();
	  
	  if (sym != lparen) _error(14);
	  else {
		index=0;
		do {
		  insymbol();
		  /* literal constant? */
		  if ((sym == shortconst) && (shortval > 0))
			dimsize[index++] = shortval+1;
		  else if ((sym == ident) && (exist(id,constant))) {
			/* defined constant? */
			
			if ((curr_item->type == shorttype) && (curr_item->numconst.shortnum > 0))
			  dimsize[index++] = curr_item->numconst.shortnum+1;
			else _error(23);
		  } else _error(23);  /* illegal array index */
		  insymbol();
		} while ((sym == comma) && (index < MAXDIMS));
		
		if (sym != rparen) _error(9);
		
		if (!dimmed) {
		  enter(arrayid,arraytype,array,index-1);
		  array_item = curr_item;
		  
		  max_element = max_array_ndx(array_item); /* number of linear elements */
		  
		  /* frame address to hold array pointer */  
		  itoa(-1*array_item->address,addrbuf,10);
		  strcat(addrbuf,frame_ptr[lev]);
		  
		  insymbol();
		  
		  /* specify size of string array elements with "SIZE"? */
		  if (sym == sizesym && array_item->type == stringtype) {
			insymbol();
			if (sym == shortconst) string_element_size=(LONG)shortval; 
			else if (sym == longconst) string_element_size=longval; 
			else if (sym == ident && exist(id,constant)) {
			  if (curr_item->type == shorttype) string_element_size=(LONG)curr_item->numconst.shortnum;
			  else if (curr_item->type == longtype) string_element_size=curr_item->numconst.longnum;
			  else _error(4);
			} else if (sym == singleconst) _error(4);
			else _error(27);  /* numeric constant expected */
			
			if (string_element_size <= 0L) _error(41); /* non-positive string size */
			
			insymbol();
		  } else string_element_size=MAXSTRLEN;
		  
		  /* record size of array in bytes (for SIZEOF) 
			 plus string element size */
		  if (array_item->type == stringtype) {
			array_item->size = max_element * string_element_size;
			/* size of each string array element */
			array_item->numconst.longnum = string_element_size;
		  } else if (array_item->type == shorttype)
			array_item->size = max_element*2;
		  else
			/* long or single */
			array_item->size = max_element*4; 
		  
		  /* specify ADDRESS? */
		  if (sym != addresssym) {
			/* set up BSS object for array */
			
			if (array_item->type == stringtype) strcpy(buf,"ds.b ");
			else if (array_item->type == shorttype) strcpy(buf,"ds.w ");
			else strcpy(buf,"ds.l "); /* long or single */ 
			
			if (array_item->type == stringtype)
			  ltoa(max_element*string_element_size,numbuf,10);
			else ltoa(max_element,numbuf,10);
			
			strcat(buf,numbuf);
			make_array_name(arrayname,arraylabel);
			
			/* create the BSS object */
			enter_BSS(arraylabel,buf);
			
			/* store address of array in stack frame */
			gen_pea(arrayname);
			gen_pop32_var(addrbuf);	    
		  } else {
			/* push specified array start address */
			insymbol();
			if (expr() != longtype) _error(4);
			else gen_pop32_var(addrbuf); /* store address of array in stack frame */
		  }
		}
	  }
	} else _error(7);
  } while (sym == comma);
}

/* --------------- */
/* INPUT functions */
/* --------------- */

void input() {
  int  inptype;
  char addrbuf[80];
  SYM  *storage;
  
  if ((sym != comma) && (sym != semicolon) && (sym != ident)) {
	/* print a string constant? */
	inptype=expr();
	if ((inptype == stringtype) && (lastsym == stringconst)) {
	  gen_call_void("_Ustringprint",4);
	} else _error(18); 
  }

  do { 
	/* ";" or "," -> "?" */
	if ((sym == comma) || (sym == semicolon)) {
	  if (sym == semicolon) 
		{ gen_printcode(QUN_CODE); gen_printcode(SPACE_CODE); }
	  insymbol();
	} else { gen_printcode(QUN_CODE); gen_printcode(SPACE_CODE); }
	
	/* allocate variable storage, call _input* and store value in variable */
	if ((sym == ident) && (obj == variable)) {
	  if ((!exist(id,obj)) && (!exist(id,array)))
		enter(id,typ,obj,0);  /* allocate storage for a simple variable */
	  
	  storage = curr_item;
	  
	  itoa(-1*storage->address,addrbuf,10);
	  strcat(addrbuf,frame_ptr[lev]); 
	  
	  /* ALL data types need a temporary string */
	  make_temp_string();
	  if (storage->type != stringtype) gen_load_addr(tempstrname,1);
	  else gen_pea(tempstrname);
	  
	  /* When storing an input value into an array element, must save
		 value (d0) first, since array index calculation may be corrupted
		 if index has to be coerced from ffp to short.
	  */
	  
	  switch(storage->type) {
	  case shorttype  : input_short(storage,"_inputshort",addrbuf,lev); break;
	  case longtype   : input_long(storage,"_inputlong",addrbuf,lev); break;
	  case singletype : input_single(storage,"_inputsingle",addrbuf,lev); break;
	  case stringtype : 
		gen_jsr("_Ustringinput");
		assign_string_to_storage(storage,addrbuf);
		break;
	  }
	} else _error(19);
	
	insymbol();
	if (sym == lparen && storage->object != array) _error(71);  /* undeclared array */
  } while ((sym==comma) || (sym==semicolon) || (sym==ident));
}

void point_to_array(SYM * storage,char * addrbuf) {

    /* get absolute index of array element */
    have_lparen=FALSE;
    push_indices(storage);
    get_abs_ndx(storage);

    if (storage->type != stringtype)
       gen_load32a(addrbuf,2);  /* --> pointer to start of array <-- */
}

/* -------------- */
/* DATA functions */
/* -------------- */

void make_data_const(char * string) {
char *strbuf;

 /* actual string constant */
 strbuf=(char *)alloc(strlen(string)+10); 
                             /* +10 is for string_const_start/end (9) & '\0' */ 
 strcpy(strbuf,string_const_start);
 strcat(strbuf,string);
 strcat(strbuf,string_const_end);
 enter_BASDATA(strbuf);
 /*FreeMem(strbuf,strlen(string)+10);*/
}

void get_data() {
  /* parse a line of BASIC DATA */
  char  fnumbuf[40];
  float fnum,sign;

  do {
	sign=1.0;
	
	insymbol();
	
	/* arithmetic sign? */
	if ((sym == minus) || (sym == plus))  { 
	  if (sym == minus) sign=-1.0; 
	  insymbol();
	  if ((sym == ident) || (sym == stringconst)) _error(27);
	}

	if (sym == ident) make_data_const(ut_id);
	else if (sym == stringconst) make_data_const(stringval);
	else if (sym == singleconst) {
	  sprintf(fnumbuf,"%lx",(unsigned long)(singleval * sign));
	  make_data_const(fnumbuf);
    } else if (sym == longconst) {
      fnum=(float)longval * sign;
      sprintf(fnumbuf,"%lx",(unsigned long)fnum);
      make_data_const(fnumbuf);
	} else if (sym == shortconst) {
      fnum= singleval * sign;
      sprintf(fnumbuf,"%lx",(unsigned long)fnum);
      make_data_const(fnumbuf);
	} else _error(26);  /* constant expected */

   insymbol(); 
  } while (sym == comma);  
}

void read_data() { 
  char addrbuf[80];
  SYM  *storage;
 
  /* read a value from the DATA list into a variable or array element */

  readpresent=TRUE;
  
  do {
	insymbol();
  
	if ((sym == ident) && (obj == variable)) {
	  if ((!exist(id,obj)) && (!exist(id,array)))
      enter(id,typ,obj,0);  /* allocate storage */

	  storage=curr_item;  /* save storage item information */

	  itoa(-1*storage->address,addrbuf,10);
	  strcat(addrbuf,frame_ptr[lev]); 
  
	  /* is it an array? (this must already have been dimensioned!) */
	  if (storage->object == array) point_to_array(storage, addrbuf);


	  if (storage->type == stringtype) {
          gen_dataptr_push(); /* addr of source */
          if (storage->object == variable) assign_to_string_variable(storage,MAXSTRLEN);
          else if (storage->object == array) assign_to_string_array(addrbuf);
	  } else {
          /* get next item from DATA list and call htol */
          gen_dataptr_htol(); /* return LONG from (a1) */

          if (storage->type == longtype) {
              push_result(longtype);
              make_integer(singletype);
          } else if (storage->type == shorttype) {
              push_result(longtype);
              make_sure_short(singletype);
          }

          if (storage->object == variable) {
              if ((storage->shared) && (lev == ONE)) {
                  gen_load32a(addrbuf,0);   /* abs addr of store */
                  if (storage->type == singletype)     gen_save_indirect32();
                  else gen_pop_indirect(storage->type);
              } else {
                  if (storage->type == singletype)     gen_save32d(0,addrbuf);
                  else if (storage->type == longtype)  gen_pop32_var(addrbuf);
                  else if (storage->type == shorttype) gen_pop16_var(addrbuf);
              }
          } else if (storage->object == array) {
              if (storage->type == singletype)       gen_save_indirect_indexed32("d0");
              else if (storage->type == longtype)    gen_pop_indirect_indexed32(2,7);
              else if (storage->type == shorttype)   gen_pop_indirect_indexed16(2,7);
          }
	  }
	} else _error(19);  /* variable expected */ 			

    gen_dataptr_next();

	insymbol();
	if (sym == lparen && storage->object != array) _error(71);  /* undeclared array */  
  } while (sym == comma);
}
