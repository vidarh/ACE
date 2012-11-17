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
extern	BOOL	iffused;

/* IFF OPEN #channel, filename
       READ | CLOSE #channel 
 */

struct ParseSequence seq_iff[] = {
    {opensym,  {"i#l,s", {0},  "_IFFPicOpen", 8}},
    {readsym,  {"i#l?l", {-1}, "_IFFPicRead", 8}},
    {closesym, {"i#l",   {0},  "_IFFPicClose",4}}
};

void iff() {
  insymbol();
  parse_alt_sequence(seq_iff,3);
  /* We need to tell ACE to create/delete ILBM.library. */
  iffused = TRUE;
}
