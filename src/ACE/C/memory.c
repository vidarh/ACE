/* << ACE >>

   -- Amiga BASIC Compiler --

   ** Parser: memory functions **
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
     Date: 13th,14th June 1993,
	   3rd July 1993
*/

#include "acedef.h"
#include "codegen.h"

/* locals */
int	struct_member_type;  /* for SWAP -> filled by address_of_object() */

/* externals */
extern	int	sym;
extern	int	lev;
extern 	SYM	*curr_item;
extern	char	id[MAXIDSIZE];
extern	char	tempstrname[80];

/* -------------- */
/* POKE functions */
/* -------------- */

void poke() {
  if (in_long_expr() != undefined && try_comma()) gen_pokeb(expr());
}

void pokew() {
    if (in_long_expr() != undefined && try_comma()) gen_pokew();
}

void pokel() {
  int datatype;
  if (in_long_expr()!= undefined) {  
      if (try_comma()) {
	  /* get value */
	  datatype=make_integer(expr());
	  if ((datatype == shorttype) || (datatype == longtype)) gen_pokel(expr());
	  else _error(4);
	}
  }
}

/* ---- */
/* SWAP */
/* ---- */
void get_obj_info(char * objname,int * object,int * objtype) {
  /* get info about a variable, extvar, array or structure for SWAP */

  BOOL found = 
	exist(objname,variable) ||
	exist(objname,extvar) ||
	exist(objname,array) ||
	exist(objname,structure);

  if (found) {
	*object  = curr_item->object;
	*objtype = curr_item->type;
  } else {
	*object  = undefined;
	*objtype = undefined;
  }
}

/* SWAP <object>,<object> */
void swap() {
  char first[MAXIDSIZE],second[MAXIDSIZE];
  int  typ1,typ2,dataobj1,dataobj2;
  
  insymbol();
  
  if (sym != ident) _error(7);
  else {
	strcpy(first,id);
	address_of_object();
	get_obj_info(first,&dataobj1,&typ1);

	/* get_obj_info() won't tell us about structure member type */
	if (dataobj1 == structure) typ1 = struct_member_type;
	if (dataobj1 != structure && dataobj1 != array) insymbol();

	if (sym != comma) _error(16);
	else {
	  insymbol();
	  if (eat_token(ident,7)) {
		strcpy(second,id);
		address_of_object();
		get_obj_info(second,&dataobj2,&typ2);

		/* get_obj_info() won't tell us about structure member type */
		if (dataobj2 == structure) typ2 = struct_member_type;
		if (dataobj2 != structure && dataobj2 != array) insymbol();

		/* if two objects are of same data type -> swap them */
		if (typ1 != typ2) _error(4);
		else gen_swap(typ1);
	  }
	}
  } 
}
