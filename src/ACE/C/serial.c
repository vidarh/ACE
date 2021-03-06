/* << ACE >>

   -- Amiga BASIC Compiler --

   ** Parser: serial port commands **
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
     Date: 5th,6th September 1993
*/

#include "acedef.h"
#include "codegen.h"

/* locals */
static	char 	*frame_ptr[] = { "(a4)","(a5)" };

/* externals */
extern	int	sym;
extern	int	lev;
extern	int	obj;
extern	int	typ;
extern	SYM	*curr_item;
extern	char   	id[MAXIDSIZE]; 

/* functions */
static void open_serial() {
/* open a channel to a serial port.
    
   SERIAL OPEN [#] channel,unit,baud,"N|E|O|M|S<d><s>[A][X]"[,size][,name]

   where: 	channel	= unique channel number 
		unit	= unit of serial device to use
		baud	= baud rate
		NEOMS 	= parity: None, Even, Odd, Mark, Space
		<d>	= # of data bits (1 digit)
		<s>	= # of stop bits (1 digit)
		A	= serial device will use ALL 7 port wires
		X 	= sets Xon/Xoff mode
		size	= size of serial buffer
		name	= name of serial device		
 */
  short tokens[] = {comma,16,longtype/*unit*/,0,comma,16,longtype/*bps*/,0,comma,16,stringtype,4,-1-1};

  if (!expect_token_sequence(tokens)) return;

  /* optional serial READ buffer size */
  insymbol();
  if (try_comma()) {
      if (!peek(comma)) {
          long_expr(); /* Read buffer size */ 
      } else gen_push32_val(512);		/* defaults to 512 bytes */	
  } else gen_push32_val(512);		/* defaults to 512 bytes */

  /* optional serial device name */
  if (try_comma()) {
      if (expr() != stringtype) _error(4);	/* serial device name */
  } else gen_push32_val(0);		/* defaults to NULL */
  
  /* call open_serial function */
  gen_call_void("_OpenSerial",24);
}

/* 
   SERIAL READ [#] channel,buffer,length
*/
static void read_serial()
{
  SYM  *storage;
  char addrbuf[40];

  if (!eat_comma()) return;
  if (sym == ident && obj == variable) /* buffer */ {
	/* if string variable/array doesn't exist, create a simple variable */
	if (!exist(id,variable) && !exist(id,array)) {
	  /* allocate a simple string variable */
	  enter(id,typ,obj,0);
	  enter_DATA("_nullstring:","dc.b 0");
	  gen_pea("_nullstring");
	  assign_to_string_variable(curr_item,MAXSTRLEN);
	}

	storage=curr_item;
	
	/* is it a string variable or array? */
	if (storage->type != stringtype) {_error(4);return;}

	/* get address of string pointed to by variable/array element */
	itoa(-1*storage->address,addrbuf,10);
	strcat(addrbuf,frame_ptr[lev]);
	
	/* pass string address to function (on stack) */
	if (storage->object == array) {
	  gen_push_deref_array(storage,addrbuf);
	} else gen_push32_var(addrbuf);
	
    gen_call_sargs("_ReadSerial","i,l",12);
  } else _error(19); /* variable (or array) expected */      
  insymbol();
}

/* SERIAL
     READ  [#] channel,buffer,length
          read a specified number of bytes into a buffer.
     WRITE [#] channel,buffer,length
          write a specified number of bytes from a buffer.
     CLOSE [#] channel
*/
void serial_command()
{
  /* parse a serial command */

  insymbol();
  parse_channel();

  switch(sym) {
  case opensym:	 open_serial(); break;
  case readsym:	 read_serial(); break;
  case closesym: gen_call_void("_CloseSerial",4); break;
  case writesym: gen_call_sargs("_WriteSerial",",s,l",12); break;
  default:	_error(75);	/* open,close etc expected */
	break;
  }
}

