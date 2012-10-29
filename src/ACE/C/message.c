/*
** ACE Message Ports.
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
**
** Author: David J Benn
**   Date: 15th February 1994,
**	   22nd August 1994,
**	   1st September 1994
*/

#include "acedef.h"
#include "codegen.h"

/* locals */
static 	char	*frame_ptr[] = { "(a4)","(a5)" };

/* externals */
extern	int	sym;
extern	int	typ;
extern	int	obj;
extern	int	lev;
extern	char	id[MAXIDSIZE];
extern	SYM	*curr_item;

/* MESSAGE READ [#]channel,message-string */
static void	message_read() {
  SYM	*storage;
  char	addrbuf[40];

  if (!eat_comma()) return;

  /*
  ** Message string. 
  */
  if (sym == ident && obj == variable) {
	/* 
	** If string variable/array doesn't exist, 
	** create a simple variable.
	*/
	if (!exist(id,variable) && !exist(id,array)) {
	  /* 
	  ** Allocate a simple string variable.
	  */
	  enter(id,typ,obj,0);
	  enter_DATA("_nullstring:","dc.b 0");
	  gen_pea("_nullstring");
	  assign_to_string_variable(curr_item,
								MAXSTRLEN);
	}
	
	storage = curr_item;
	
	/* Is it a string variable or array? */
	if (storage->type != stringtype) _error(4);
	else {
	  /* 
	  ** Get address of string pointed to 
	  ** by variable/array element.
	  */
	  itoa(-1*storage->address,addrbuf,10);
	  strcat(addrbuf,frame_ptr[lev]);
	  
	  /*
	  ** Pass string address to function 
	  ** (on stack).
	  */
	  if (storage->object == array) {
		gen_push_deref_array(storage,addrbuf);
	  } else
		gen_push32_var(addrbuf);
	  
	  insymbol();
	  gen_call_void("_MessageRead",8);
	}
  } else _error(19);  /* variable or array expected */
}

/* MESSAGE CLOSE|WAIT|CLEAR [#]channel */
/* MESSAGE OPEN [#]channel,port-name,mode */
/* MESSAGE READ|WAIT [#]channel ... */
/* MESSAGE WRITE [#]channel,message-string */
void message() {
	insymbol();
    parse_channel();
	switch(sym) {
    case readsym	: message_read(); break;
    case writesym	: gen_fcall("_MessageWrite",expr(),",s",stringtype,"",8);
    case opensym	: gen_call_sargs("_MessageOpen",",s,s",12); break;
    case closesym	: gen_call_void("_MessageClose",4); break;
    case waitsym	: gen_call_void("_MessageWait",4);  break;
    case clearsym	: gen_call_void("_MessageClear",4); break;
    default		    : _error(77); break;	
	}	
}
