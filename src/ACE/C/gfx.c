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
	  insymbol();
	}
	seq += 2;
  }
  return 1;
}

BOOL parse_step()
{
  if (sym == stepsym) {
	insymbol();
	return TRUE;
  }
  return FALSE;
}

static BOOL pen_color()
{
  if (!try_comma()) return FALSE;
  if (sym == comma) return FALSE; /* Handle case where there are more parameters coming. */
  gen_pop_as_short(expr(),0);
  gen_setapen();
  return TRUE;
}

static void reset_pen(BOOL colorset)
{
  if (!colorset) return;
  gen_load16d("_fgdpen",0);
  gen_setapen();
  enter_XREF("_fgdpen");
}

/* functions */
void pset() {
  BOOL relative;
  BOOL colorset=FALSE;
  insymbol();
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
	gen_gfx_move();
  }

  /* 
  ** Save appropriate registers for call to WritePixel()
  ** since Move() may clobber them.
  */ 
  gen_move32aa(1,3);
  gen_move16dd(0,3);
  gen_move16dd(1,4);
  gen_gfx_move();
  
  /* 
  ** Restore appropriate registers for call to WritePixel().
  */
  gen_move16dd(4,1);
  gen_move16dd(3,0);
  gen_move32aa(3,1);
  gen_writepixel();
  reset_pen(colorset);
}

/* PAINT (X,Y)[,paintcolor-id[,bordercolor-id]] */
void paint() {
  BOOL paintcolor=FALSE;
  BOOL bordercolor=FALSE;

  insymbol();
  if (!expect_token_sequence(point_tokens)) return;

  if (try_comma()) {
	if (sym != comma) {
	  make_sure_short(expr()); /* paintcolor-id */
	  paintcolor=TRUE;
	}
	
	if (try_comma()) {
	  make_sure_short(expr()); /* bordercolor-id */
	  bordercolor=TRUE;
	}
  }

  /* pop parameters */
  if (bordercolor) gen_pop16d(3);
  else gen_load32d_val(-1,3);  /* flag no border color-id */
  
  if (paintcolor) gen_pop16d(2);
  else gen_load32d_val(-1,2);  /* flag no paint color-id */
  
  gen_paint();
}

void gen_Flt_expr_pop(int reg) {
    gen_Flt(expr());
    gen_pop32d(reg);
}

void circle() {
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
  
  if (try_comma()) {
	if (sym != comma) {  /* else skip to end angle */
        gen_Flt_expr_pop(3);
        start_angle=TRUE;
	}
	
	if (eat_comma()) {
      if (sym != comma) {  /* else skip to aspect */
		if (!start_angle) _error(30); /* no start angle! */
		gen_Flt_expr_pop(4);
		end_angle=TRUE;
      }
	}
	
	if (try_comma()) {
        gen_Flt_expr_pop(5);
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
  gen_load16d("_shorty",0);
  gen_ext16to32(0);
  gen_flt();
  gen_push32d(0);

  gen_load16d("_shortx",0);
  gen_ext16to32(0);
  gen_flt(); /* x is now in D0 */
  
  gen_pop32d(1); /* y */
  
  if (!start_angle) gen_load32d(0,3);   /* default is zero */
  if (!end_angle)  gen_save32d_val(0xb3800049,4);  /* default is 359 */
  if (!aspect) gen_save32d_val(0xe147af3f,5);  /* default is .44 */
  
  gen_jsr("_ellipse");
  enter_XREF("_MathTransBase");  /* need these 3 libs for _ellipse */
  
  enter_BSS("_shortx","ds.w 1");
  enter_BSS("_shorty","ds.w 1");
  reset_pen(colorset);
}

void enter_min_BSS() {
	enter_BSS("_xmin","ds.w 1");
	enter_BSS("_ymin","ds.w 1");
}

void gen_move_pnt(short rx, short ry) {
    gen_move16dd(rx,0);
    gen_move16dd(ry,0);
}

void gen_line_to(short rx, short ry) {
    gen_move_pnt(rx,ry);
    gen_draw();
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
  enter_min_BSS();
		
  if (!relative) {
	/* move to x,y */
	gen_gfxcall("Move");
	cx=curr_code;  /* don't need this Move for boxfill */
	/* XREF declared by box or line (see below) */
  }
		
  /* second coordinate clause ? */
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
  if (try_comma()) {
	if (sym != comma) {	/* ",," -> no color-id */
	  colorset=TRUE;
	  gen_push16_var("_xmin");	/* save d0 & d1 */
	  cx3=curr_code;
	  gen_push16_var("_ymin");
	  cx4=curr_code;
	  gen_pop_as_short(expr(),0);
	  gen_setapen();
	  gen_pop16_var("_ymin");
	  cx5=curr_code;
	  gen_pop16_var("_xmin");	/* restore d0 & d1 */
	  cx6=curr_code;
	} else colorset=FALSE;
  }
	
  /* box or boxfill? */
  if (try_comma()) {
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

  /* FIXME: Isn't this better handled with some small runtime library functions? */
  /* draw the line, outline box, or filled box */
  if (box) {
	gen_pop16d(5);  /* y2 */
	gen_pop16d(4);  /* x2 */
	gen_load16d("_ymin",3); /* y1 */
	gen_load16d("_xmin",2); /* x1 */
	/* x1=d2; y1=d3 x2=d4; y2=d5 */
	/* already moved to x1,y1 */

	gen_line_to(4,3); /* x1,y1 - x2,y1 */
	gen_line_to(4,5); /* x2,y1 - x2,y2 */
    gen_line_to(2,5); /* x2,y2 - x1,y2 */
	gen_line_to(2,3); /* x1,y2 - x1,y1 */
    enter_min_BSS();
  }	else if (boxfill) {
	change(cx,"nop","  ","  ");   /* don't need Move */
	gen_load32a("_RPort",1);
	gen_pop16d(3);  /* ymax */
	gen_pop16d(2);  /* xmax */
	gen_load16d("_ymin",1); /* ymin */
	gen_load16d("_xmin",0); /* xmin */
	gen_rectfill();
    enter_min_BSS();
  } else {
	/* draw line */
	gen_load32a("_RPort",1);
	gen_pop16d(1);  /* y2 */
	gen_pop16d(0);  /* x2 */
	/* already moved to x1,y1 */
	gen_draw();
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
  gen_setapen();
  enter_XREF("_fgdpen");
  enter_BSS("_fg","ds.w 1");
  
  if (try_comma()) {
	/* background color */
	gen_pop_as_short(expr(),0);
	gen_save16d(0,"_bg");  /* background pen for text color change */
	gen_save16d(0,"_bgpen"); /* change global background pen color */
	gen_setbpen();
  } else {
	/* default to current background pen */
	gen_move16("_bgpen","_bg");
  }
  enter_XREF("_bgpen");
  enter_BSS("_bg","ds.w 1");
  
  /* call text color change routine */
  gen_load16d("_fg",0);
  gen_load16d("_bg",1);
  gen_jsr("_changetextcolor");
}

/* AREA [STEP](x,y) */
void area() {
  BOOL relative;
  relative = parse_step();
  if (!expect_token_sequence(point_tokens)) return;
  gen_area(relative);
}

/* AREAFILL [mode] */
void areafill() {
  insymbol();
  if ((sym == shortconst) && ((shortval == 0) || (shortval == 1))) {
	gen_load16d_val(shortval,0);
	insymbol();
  } else gen_load16d_val(0,0);
  gen_jsr("_areafill");
}

/* PATTERN [line-pattern][,area-pattern] | RESTORE */ 
void pattern() {
  char addrbuf[40];
  BOOL linepatterncalled;
  
  insymbol();
  if (eat(restoresym)) {
	/* restore default pattern */
	gen_load32d_val(1,1); 	/* RESTORE flag */
	gen_jsr("_linepattern");
	gen_load32d_val(1,1); 	/* RESTORE flag */
	gen_jsr("_areapattern");
  } else {
	if (sym != comma) {
	  /* get line-pattern */
	  gen_pop_as_short(expr(), 0); /* line-pattern */
	  gen_load32d_val(0,1); 	/* RESTORE flag */
	  gen_jsr("_linepattern");
	  linepatterncalled=TRUE;
	} else linepatterncalled=FALSE;
	
    if (try_comma()) {
        /* area-pattern */
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

