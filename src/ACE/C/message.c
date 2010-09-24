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

/* MESSAGE OPEN [#]channel,port-name,mode */
static void message_open() {
  parse_channel();
  if (sym != comma) _error(16);
  else {
	/* port-name */
	insymbol();
	if (expr() != stringtype) _error(4);
	else {
	  if (sym != comma) _error(16);
	  else {
		/* mode (r,R,w,W) */
		insymbol();
		if (expr() != stringtype) _error(4);
		else gen_call_void("_MessageOpen",12);
	  }
	}
  } 
}

/* MESSAGE READ [#]channel,message-string */
static void	message_read() {
  SYM	*storage;
  char	addrbuf[40];

  parse_channel();
  if (sym != comma) {
	_error(16);
	return;
  }

  /*
  ** Message string. 
  */
  insymbol();						
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
	
	/* 
	** Is it a string variable or array? 
	*/
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
		point_to_array(storage,addrbuf);
		gen_load32d(addrbuf,0);
		gen_add32dd(7,0);
		gen_push32d(0);
	  } else
		gen_push32_var(addrbuf);
	  
	  insymbol();
	  gen_call_void("_MessageRead",8);
	}
  } else _error(19);  /* variable or array expected */
}

/* MESSAGE WRITE [#]channel,message-string */
static void message_write() {
  parse_channel();
  if (sym != comma) _error(16);
  else gen_fcall("_MessageWrite",expr(),"s",stringtype,"",8);
}

/* MESSAGE WAIT [#]channel */
static void message_wait() {
  parse_channel();
  gen_call_void("_MessageWait",4);
}

/* MESSAGE CLEAR [#]channel */
static void message_clear() {
  parse_channel();
  gen_call_void("_MessageClear",4);
}

/* MESSAGE CLOSE [#]channel */
static void message_close() {
  parse_channel();
  gen_call_void("_MessageClose",4);
}

/*  MESSAGE OPEN|CLOSE|READ|WRITE|WAIT|CLEAR */
void message() {
	insymbol();
	switch(sym) {
		case opensym	: message_open(); break;
		case closesym	: message_close(); break;
		case readsym	: message_read(); break;
		case writesym	: message_write(); break;
		case waitsym	: message_wait(); break;
		case clearsym	: message_clear(); break;

		default		: _error(77); break;	
	}	
}
