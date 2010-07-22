/* 
** Unix-style hassle free memory allocation
** via Intuition's Alloc/FreeRemember functions.
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
** Call alloc for each memory allocation request.
**
** Call free_alloc once at the end of the program run.
**
** Author: David J Benn
**   Date: 30th June 1993, 
**	   1st July 1993, 
**    	   16th,24th,25th December 1993
*/

#include "app.h"

#ifdef AMIGA
extern struct Library	*IntuitionBase = NULL;

#else

static void * AllocRemember(struct Remember ** rem, long bytes, int flags)
{
  struct Remember * new_rem = malloc(sizeof(struct Remember));
  new_rem->next = *rem;
  new_rem->mem = malloc(bytes);
  *rem = new_rem;
  return new_rem->mem;
}

static void FreeRemember(struct Remember ** rem, BOOL unused_)
{
  struct Remember * node = *rem;
  while(node) {
	struct Remember * next = node->next;
	free(node->mem);
	free(node);
	node = next;
  }
  *rem = 0;
}

#endif

struct Remember *RememberList = NULL;

/* functions */

void *alloc(bytes,flags)
ULONG bytes,flags;
{
/* allocate memory as requested */
#ifdef AMIGA
/* open Intuition Library */
  if (!IntuitionBase) {
	IntuitionBase = (struct Library *)OpenLibrary("intuition.library",0L);
	if (IntuitionBase == NULL) 
	  { puts("Can't open intuition.library!"); exit(10); } 
  }
#endif

 return((void *)AllocRemember(&RememberList,bytes,flags));     
}

void free_alloc()
{
/* free all memory allocated by alloc 
   and close intuition library */

 if (RememberList) 
 {
	FreeRemember(&RememberList,TRUE);
	RememberList = NULL;
 }

#ifdef AMIGA
 if (IntuitionBase) CloseLibrary(IntuitionBase);
#endif
}
