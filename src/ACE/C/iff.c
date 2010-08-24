/*
** ACE IFF commands.
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
**   Date: 27th February 1994,
**	   10th,11th,19th August 1994
*/

#include "acedef.h"
#include "codegen.h"

/* externals */
extern	int	sym;
extern	BOOL	iffused;

void parse_channel() {
  insymbol();
  if (sym == hash) insymbol();
  make_sure_long(expr()); /* channel */
}


/* 
** IFF OPEN [#]channel,file-name
*/
void	iff_open() {
  parse_channel();
  if (sym != comma) _error(16);
  else {
	/* picture file name */
	insymbol();
	if (expr() != stringtype) _error(4);
	else gen_call_void("_IFFPicOpen",8);
  } 
}

/* 
** IFF READ [#]channel[,screen-id]
*/
void	iff_read() {
  parse_channel();
  if (sym != comma)
	/* no screen-id */
	gen("move.l","#-1","-(sp)");	
  else {
	/* screen-id */

	insymbol();
	make_sure_long();
	gen_call_void("_IFFPicRead",8);
  }
}

/* 
** IFF CLOSE [#]channel
*/
void  iff_close() {
  parse_channel();
  gen_call_void("_IFFPicClose",4);
}

void iff()
{
/* IFF OPEN | READ | CLOSE */

	insymbol();
	
	switch(sym)
	{
		case opensym  : iff_open(); break;
		case readsym  : iff_read(); break;
		case closesym : iff_close(); break;
	}

	/*
	** We need to tell ACE to create/delete ILBM.library.
	*/
	iffused = TRUE;
}
