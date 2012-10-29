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

/*  IFF READ [#]channel[,screen-id] */
static void iff_read() {
  parse_channel();
  if (sym != comma) gen_push32_val(-1); /* no screen-id */
  else {
      /* screen-id */
      insymbol();
      make_sure_long();
      gen_call_void("_IFFPicRead",8);
  }
}

/* IFF OPEN #channel, filename
       READ | CLOSE #channel 
 */
void iff() {
  insymbol();
  switch(sym) {
  case opensym  : gen_call_sargs("_IFFPicOpen","#l,s",8); break;
  case readsym  : iff_read(); break;
  case closesym : gen_call_sargs("_IFFPicClose","#l",4); break;
  }
  /* We need to tell ACE to create/delete ILBM.library. */
  iffused = TRUE;
}
