/* << ACE >>

   -- Amiga BASIC Compiler --

   ** Parser: graphics functions **
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
	   6th,7th,12th,13th December 1992,
	   4th January 1993,
	   1st July 1993,
	   6th February 1994,
	   14th July 1994,
	   28th August 1994
*/

#include "acedef.h"
#include "codegen.h"

/* locals */
static	char	*frame_ptr[] = { "(a4)","(a5)" };

/* externals */
extern	int	sym;
extern	int	lev;
extern	int	obj;
extern	SHORT	shortval;
extern	CODE	*curr_code;
extern	SYM	*curr_item;
extern	char	id[MAXIDSIZE];

/* (x,y) */
static short point_tokens[] = {lparen,14,shorttype /*x*/,0,comma,16,shorttype/*y*/,0,rparen,9,-1,-1};


int expect_token_sequence(short * seq)
{
  while (*seq != -1) {
	if (*seq == shorttype) {
	  make_sure_short(expr());
	} else if (*seq == longtype) {
	  make_sure_long(expr());
	} else if (*seq == stringtype) {
	  if (expr() != stringtype) {
		_error(seq[1]);
		return 0;
	  }
	} else  if (*seq >= 0) {
	  if (sym != seq[0]) {
		_error(seq[1]);
		return 0;
	  }
	}
	seq += 2;
	insymbol();
  }
  return 1;
}

BOOL parse_step()
{
  insymbol();
  if (sym == stepsym) {
	insymbol();
	return TRUE;
  }
  return FALSE;
}

static BOOL pen_color()
{
  insymbol();
  if (sym != comma) return FALSE;
  insymbol();
  if (sym == comma) return FALSE; /* Handle case where there are more parameters coming. */
  gen_pop_as_short(expr(),0);
  gen_gfxcall("SetAPen");
  return TRUE;
}

static void reset_pen(BOOL colorset)
{
  if (!colorset) return;
  gen_load16d("_fgdpen",0);
  gen_gfxcall("SetAPen");
  enter_XREF("_fgdpen");
}

/* functions */
void pset()
{
  BOOL relative;
  BOOL colorset=FALSE;

  relative = parse_step();
  if (!expect_token_sequence(point_tokens)) return;

  colorset = pen_color();

  gen_pop16d(1); /* pop y-coordinate */
  gen_pop16d(0); /* pop x-coordinate */

  if (!colorset) {
	gen_load32a("_RPort",1);
	enter_XREF("_RPort");
  }

  if (relative) {
	gen_rport_rel_xy();
	gen_libcall("Move","Gfx");
  }

  /* 
  ** Save appropriate registers for call to WritePixel()
  ** since Move() may clobber them.
  */ 
  gen_move32aa(1,3);
  gen_move16dd(0,3);
  gen_move16dd(1,4);
  gen_libcall("Move","Gfx");
  
  /* 
  ** Restore appropriate registers for call to WritePixel().
  */
  gen_move16dd(4,1);
  gen_move16dd(3,0);
  gen_move32aa(3,1);
  gen_libcall("WritePixel","Gfx");
  reset_pen(colorset);
}

void paint() {
  BOOL paintcolor=FALSE;
  BOOL bordercolor=FALSE;

  /* PAINT (X,Y)[,paintcolor-id[,bordercolor-id]] */

  if (!expect_token_sequence(point_tokens)) return;
  insymbol();
  if (sym == comma) {
	insymbol();
		  
	if (sym != comma) {
	  make_sure_short(expr()); /* paintcolor-id */
	  paintcolor=TRUE;
	}
	
	if (sym == comma) {
	  insymbol();
	  make_sure_short(expr()); /* bordercolor-id */
	  bordercolor=TRUE;
	}
  }

  /* pop parameters */
  if (bordercolor) gen_pop16d(3);
  else gen_load32d(-1,3);  /* flag no border color-id */
  
  if (paintcolor) gen_pop16d(2);
  else gen_load32d(-1,2);  /* flag no paint color-id */
  
  gen_call_args("_paint","d1.w,d0.w",0);
}

void circle()
{
  BOOL relative;
  BOOL colorset=FALSE;
  BOOL start_angle=FALSE;
  BOOL end_angle=FALSE;
  BOOL aspect=FALSE;
  
  relative = parse_step();
  if (!expect_token_sequence(point_tokens)) return;
  insymbol();
  if (sym != comma) { _error(29); return; } /* radius expected -> no point going on */
  /* radius */
  insymbol();
  gen_Flt(expr());
  
  colorset = pen_color();
  
  /* start & end angle in <<degrees>> */
  
  if (sym == comma) {
	insymbol();
	if (sym != comma) {  /* else skip to end angle */
	  gen_Flt(expr());
	  gen_pop32d(3); /* start angle */
	  start_angle=TRUE;
	}
	
	if (sym == comma) {
      insymbol();
      if (sym != comma) {  /* else skip to aspect */
		if (!start_angle) _error(30); /* no start angle! */
		gen_Flt(expr());
		gen_pop32d(4); /* end angle */
		end_angle=TRUE;
      }
	} else _error(16);  
	
	if (sym == comma) {
      /* aspect */
      insymbol();
      gen_Flt(expr());
      gen_pop32d(5); /* aspect */
      aspect=TRUE;
	}    
  }
  
  /* pop radius & (x,y) coordinates */
  gen_pop32d(2);      /* radius */
  gen_pop16_var("_shorty"); /* y */
  gen_pop16_var("_shortx"); /* x */
  
  if (relative) {
	gen_load32a("_RPort",1);
	
	gen_load16d("_shortx",0);
	gen_rport_rel_x(0);  /* x + RPort->cp_x */
	gen_save16d(0,"_shortx");
	
	gen_load16d("_shorty",0);
	gen_rport_rel_y(0);   /* y + RPort->cp_y */
	gen_save16d(0,"_shorty");
  }

  /* convert x & y values to floats */
  gen_load16d("_shortx",0);
  gen_ext16to32(0);
  gen_libcall("SPFlt","Math");
  gen_save32d(0,"_floatx");
  
  gen_load16d("_shorty",0);
  gen_ext16to32(0);
  gen_libcall("SPFlt","Math");
  gen_save32d(0,"_floaty");
  
  gen_load32d("_floatx",0);
  gen_load32d("_floaty",1);
  
  if (!start_angle) gen_load32d(0,3);   /* default is zero */
  if (!end_angle)  gen_save32d_val(0xb3800049,4);  /* default is 359 */
  if (!aspect) gen_save32d_val(0xe147af3f,5);  /* default is .44 */
  
  gen_jsr("_ellipse");
  enter_XREF("_MathTransBase");  /* need these 3 libs for _ellipse */
  
  enter_BSS("_shortx","ds.w 1");
  enter_BSS("_shorty","ds.w 1");
  enter_BSS("_floatx","ds.l 1");
  enter_BSS("_floaty","ds.l 1");
  reset_pen(colorset);
}

void draw_line()
{
  BOOL relative=FALSE;
  BOOL colorset=FALSE;
  BOOL box=FALSE;
  BOOL boxfill=FALSE;
  CODE *cx,*cx1,*cx2,*cx3,*cx4,*cx5,*cx6;

  relative = parse_step();
  if (!expect_token_sequence(point_tokens)) return;

  gen_pop16d(1);   /* ymin */
  gen_pop16d(0);   /* xmin */
    
  /* save x1 & y1 since they may be changed by expr() calls below. */
  gen_save16d(0,"_xmin");    cx1=curr_code;
  gen_save16d(1,"_ymin");    cx2=curr_code;
  enter_BSS("_xmin","ds.w 1");
  enter_BSS("_ymin","ds.w 1");
		
  if (!relative) {
	/* move to x,y */
	gen_gfxcall("Move");
	cx=curr_code;  /* don't need this Move for boxfill */
	/* XREF declared by box or line (see below) */
  }
		
  /* second coordinate clause ? */
  insymbol();
  if (sym != minus) {
	if (relative) {
	  /* no second coordinate clause */
	  /* draw line from last pen position to x,y */
	  gen_gfxcall("Draw");
	} else _error(21);
	return;
  }

  insymbol();
  if (!expect_token_sequence(point_tokens)) return;

  /* pen color ? */
  insymbol();
  if (sym == comma) {
	insymbol();
	if (sym != comma) {	/* ",," -> no color-id */
	  colorset=TRUE;
	  gen_push16_var("_xmin");	/* save d0 & d1 */
	  cx3=curr_code;
	  gen_push16_var("_ymin");
	  cx4=curr_code;
	  gen_pop_as_short(expr(),0);
	  gen_gfxcall("SetAPen");
	  gen_pop16_var("_ymin");
	  cx5=curr_code;
	  gen_pop16_var("_xmin");	/* restore d0 & d1 */
	  cx6=curr_code;
	} else colorset=FALSE;
  }
	
  /* box or boxfill? */
  if (sym == comma) {
	insymbol();
	if (sym == ident) {
	  if ((id[0]=='B') && ((id[1]=='\0') || (id[1]==':'))) box=TRUE;
	  else
		if (((id[0]=='B') && (id[1]=='F'))
			&& ((id[2]=='\0') || (id[2]==':'))) boxfill=TRUE;
		else
		  _error(20);
	  insymbol();
	}
  }
  
  /* draw the line, outline box, or filled box */
  if (box) {
	gen_pop16d(5);  /* y2 */
	gen_pop16d(4);  /* x2 */
	gen_load16d("_ymin",3); /* y1 */
	gen_load16d("_xmin",2); /* x1 */
	/* x1=d2; y1=d3 x2=d4; y2=d5 */
	/* already moved to x1,y1 */

	gen_move16dd(4,0);
	gen_move16dd(3,1);
	gen_gfxcall("Draw");     /* x1,y1 - x2,y1 */
	gen_move16dd(4,0);
	gen_move16dd(5,1);
	gen_libcall("Draw","Gfx"); /* x2,y1 - x2,y2 */
	gen_move16dd(2,0);
	gen_move16dd(5,1);
	gen_libcall("Draw","Gfx"); /* x2,y2 - x1,y2 */
	gen_move16dd(2,0);
	gen_move16dd(3,1);
	gen_libcall("Draw","Gfx"); /* x1,y2 - x1,y1 */
	enter_BSS("_xmin","ds.w 1");
	enter_BSS("_ymin","ds.w 1");
  }	else if (boxfill) {
	change(cx,"nop","  ","  ");   /* don't need Move */
	gen_load32a("_RPort",1);
	gen_pop16d(3);  /* ymax */
	gen_pop16d(2);  /* xmax */
	gen_load16d("_ymin",1); /* ymin */
	gen_load16d("_xmin",0); /* xmin */
	gen_libcall("RectFill","Gfx");
	enter_BSS("_xmin","ds.w 1");
	enter_BSS("_ymin","ds.w 1");
  } else {
	/* draw line */
	gen_load32a("_RPort",1);
	gen_pop16d(1);  /* y2 */
	gen_pop16d(0);  /* x2 */
	/* already moved to x1,y1 */
	gen_libcall("Draw","Gfx");
	/* don't need to save x1 & x2 in _xmin & _ymin */
	change(cx1,"nop","  ","  ");
	change(cx2,"nop","  ","  ");
	if (colorset) {
	  change(cx3,"nop","  ","  ");
	  change(cx4,"nop","  ","  ");
	  change(cx5,"nop","  ","  ");
	  change(cx6,"nop","  ","  ");
	}
  }
  reset_pen(colorset);
}



void color() {
  /* foreground color */
  insymbol();
  gen_pop_as_short(expr(),0);
  gen_save16d(0,"_fg");    /* foreground pen for text color change */
  gen_save16d(0,"_fgdpen");  /* change global foreground color holder */
  gen_gfxcall("SetAPen");
  enter_XREF("_fgdpen");
  enter_BSS("_fg","ds.w 1");
  
  if (sym == comma) {
	/* background color */
	insymbol();
	gen_pop_as_short(expr(),0);
	gen_save16d(0,"_bg");  /* background pen for text color change */
	gen_save16d(0,"_bgpen"); /* change global background pen color */
	gen_gfxcall("SetBPen");
	enter_XREF("_bgpen");
	enter_BSS("_bg","ds.w 1");
  } else {
	/* default to current background pen */
	gen_move16("_bgpen","_bg");
	enter_XREF("_bgpen");
	enter_BSS("_bg","ds.w 1");
  }
  
  /* call text color change routine */
  gen_load16d("_fg",0);
  gen_load16d("_bg",0);
  gen_jsr("_changetextcolor");
}

void area()
{
  BOOL relative;
  /* AREA [STEP](x,y) */

  relative = parse_step();
  if (!expect_token_sequence(point_tokens)) return;
  
  gen_pop16d(1); /* pop y-coordinate */
  gen_pop16d(0); /* pop x-coordinate */
  
  /* include point in area info' */
  if (relative) {
	gen_add16d_var("_last_areaX",0);   /* d0 = x + lastareaY */
	gen_add16d_var("_last_areaY",1);   /* d1 = y + lastareaY */
	enter_XREF("_last_areaX");
	enter_XREF("_last_areaY");
  }
  
  gen_jsr("_area");
}

void areafill()
{
  /* AREAFILL [mode] */
  
  insymbol();
  
  if ((sym == shortconst) && ((shortval == 0) || (shortval == 1))) {
	switch(shortval) {
	case 0 : gen_load16d(0,0); break;
	case 1 : gen_load16d(1,0); break;
	}
	insymbol();
  } else gen_load16d(0,0);
  
  gen_jsr("_areafill");
}

void pattern()
{
  char addrbuf[40];
  BOOL linepatterncalled;
  
  /* PATTERN [line-pattern][,area-pattern] | RESTORE */ 
  
  insymbol();
  
  if (sym == restoresym) {
	/* restore default pattern */
	gen_load32d_val(1,1); 	/* RESTORE flag */
	gen_jsr("_linepattern");
	gen_load32d_val(1,1); 	/* RESTORE flag */
	gen_jsr("_areapattern");
	insymbol();
  } else {
	if (sym != comma) {
	  /* get line-pattern */
	  gen_pop_as_short(expr(), 0); /* line-pattern */
	  gen_load32d_val(0,1); 	/* RESTORE flag */
	  gen_jsr("_linepattern");
	  linepatterncalled=TRUE;
	} else linepatterncalled=FALSE;
	
  if (sym == comma) {
	/* area-pattern */
	insymbol();
	if ((sym == ident) && (obj == variable) && (exist(id,array))) {
      if (curr_item->type != shorttype) _error(28);
      else {
        if (!linepatterncalled) {
		  /* line-pattern must be set
			 to $FFFF if none specified,
			 otherwise area-pattern doesn't
			 seem to work! 
		  */
		  gen_load32d_val(1,1); 	/* set line-pattern to $FFFF */
		  gen_jsr("_linepattern");
		}
		
       	/* get address of array */
       	itoa(-1*curr_item->address,addrbuf,10);
       	strcat(addrbuf,frame_ptr[lev]);
       	gen_load32a(addrbuf,0);	/* start address of array */
		
       	/* size of array? */
		gen_load32d_val(curr_item->size / 2, 0); 	/* size of array */

		gen_load32d_val(0,1); 	/* RESTORE flag */
		gen_jsr("_areapattern");
		enter_XREF("_MathBase");
		enter_XREF("_MathTransBase");	/* need to find Log2(size) */
      }

	  insymbol();
	}
	else _error(28);  /* short integer array expected */ 
  }
  }
}

void scroll()
{
  /* SCROLL (xmin,ymin)-(xmax,ymax),delta-x,delta-y */
  short tokens[] =
	{comma, 16, shorttype /* delta-x */,0, comma, shorttype /*delta-y*/,0,-1,-1};

  insymbol();
  if (!parse_rect()) return;
  if (!expect_token_sequence(tokens)) return;
  
  /* pop parameters */
  gen_pop16d(1);		/* delta-y */
  gen_neg16d(1);
  gen_pop16d(0);		/* delta-x */
  gen_neg16d(0);
  gen_pop16d(5);  		/* ymax */
  gen_pop16d(4);  		/* xmax */
  gen_pop16d(3);  		/* ymin */
  gen_pop16d(2);  		/* xmin */
  
  /* call ScrollRaster function */
  gen_gfxcall("ScrollRaster");
} 



/* STYLE n */
void text_style() {
  insymbol();
  make_sure_long();
  gen_call_void("_change_text_style",4);
}

/* FONT name,size */
void text_font() {
  short tokens[] = {stringtype,4,comma,16,longtype,0,-1,-1};
  insymbol();
  if (expect_token_sequence(tokens)) gen_call_void("_change_text_font",8);
}

/* GET */
void gfx_get() { insymbol(); }

/* PUT */
void gfx_put() { insymbol(); }
