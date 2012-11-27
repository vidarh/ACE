
#include "acedef.h"
#include "codegen.h"

extern int sym;

void codegen_set_target(struct codegen_target * t)
{
  target = t;
}

extern BOOL   early_exit;
extern CODE * new_code, * curr_code;
extern	char	tempstrname[80];

void kill_all_lists();

void gen(const char * opcode,const char * srcopr,const char *destopr)
{
 /* allocate memory for a new node & each field */
 if ((new_code = (CODE *)alloc_code(opcode,srcopr,destopr)) == NULL) {
   printf("Can't allocate memory for code node!\n");
   early_exit=TRUE;
   kill_all_lists();
   cleanup();
 }

 /* fill code struct */
 strcpy(new_code->opcode,opcode);
 strcpy(new_code->srcopr,srcopr);
 strcpy(new_code->destopr,destopr);
 
 new_code->next = NULL;
 curr_code->next = new_code;
 curr_code = curr_code->next;
}

static char * dreg[] = {"d0","d1","d2","d3","d4","d5","d6","d7"};
static char * areg[] = {"a0","a1","a2","a3","a4","a5","a6","a7"};

static const char *cond_branch_op(int op) {
  switch(op) {
  case equal     : return("beq.s");
  case notequal  : return("bne.s");
  case lessthan  : return("blt.s");
  case gtrthan   : return("bgt.s");
  case ltorequal : return("ble.s");
  case gtorequal : return("bge.s");
  }
  return 0;
}


/******* Public interface ***************/

/** Low level
 *
 *  These should ideally mostly be "packaged up" into higher level
 *  code generation functions. Just translating the lower level 
 *  functions to a new architecture would likely result in inefficient
 *  code.
 *
 **/

char cur_libbase[200];

/* For inline ASM, and hence completely arch. specific */
void gen_asm(const char * line) { gen(line,"  ","  "); }
void gen_label(const char * label) { 
  gen(label,"  ","  "); 
 cur_libbase[0] = 0; 
}

static void gen_bxx(int op, const char * label) {
  gen(cond_branch_op(op),label,"  ");
  enter_XREF(label);
}

static void gen_libbase(const char * base) {
  char buf[200];
  strcpy(buf,"_");
  strncat(buf,base,sizeof(buf)-1);
  strncat(buf,"Base",sizeof(buf)-strlen(buf));
  if (strncmp(buf,cur_libbase,200) == 0) return;
  strncpy(cur_libbase,buf,199);
  cur_libbase[199] = 0;
  gen_load32a(buf,6);
  enter_XREF(buf);
}

static void gen_libcall(const char * lvo, const char * base) {
  char buf[200];
  gen_libbase(base);
  strcpy(buf,"_LVO");
  strncat(buf,lvo,sizeof(buf)-4);
  enter_XREF(buf);
  strncat(buf,"(a6)",sizeof(buf)-strlen(buf));
  gen("jsr",buf,"  ");
}

static void m68k_and(int localtype) {
  if (localtype == shorttype)  gen("and.w","d1","d0");
  else gen("and.l","d1","d0");
}

static void m68k_incr_indir(int type) {
  switch(type) {
  case shorttype  : gen("add.w","#1","(a0)");  break;
  case longtype   : gen("add.l","#1","(a0)");  break;
  case singletype : 
	gen("move.l","(a0)","d0");
	gen("move.l","#$80000041","d1");
	gen_libcall("SPAdd","Math");
	gen("move.l","d0","(a0)");
	break;
  }
}

static void m68k_decr_indir(int type) {
  switch(type) {
  case shorttype  : gen("sub.w","#1","(a0)");  break;
  case longtype   : gen("sub.l","#1","(a0)");  break;
  case singletype : 
	gen("move.l","(a0)","d0");
	gen("move.l","#$80000041","d1");
	gen_libcall("SPSub","Math");
	gen("move.l","d0","(a0)");
	break;
  }
}

void gen_branch(char * branch, char * labname) {
  char lablabel[MAXIDSIZE+1],destbuf[3];

  /* generate a jsr/jmp instruction */
  sprintf(lablabel,"%s:",labname);

  if (!exist(lablabel,label)) strcpy(destbuf,"* ");  /* for later check */
  else strcpy(destbuf,"  ");  /* label already defined */
  
  gen(branch,labname,destbuf);
}

void m68k_link() { gen("link","a5","  "); }
void m68k_unlk() { gen("unlk","a5","  "); }
void generic_rts() { gen("rts","  ","  "); }
void generic_ret() { gen("ret","  ","  "); }
void gen_bne(const char * label) { gen_bxx(notequal, label); }
void gen_beq(const char * label) { gen_bxx(equal, label); }
void gen_bge(const char * label) { gen_bxx(gtorequal, label); }
void gen_bgt(const char * label) { gen("bgt.s",label,"  "); }
void gen_blt(const char * label) { gen("blt.s",label,"  "); }
void gen_jmp(const char * label) { gen("jmp",label,"  "); }
void gen_jmp_fwd(const char * label,const char * flag) { gen("jmp",label,flag); }

void m68k_pea(const char * target) { gen("pea",target,"  "); }

void gen_pea(const char * label) { target->pea(label); }

/* Used to create ops to possibly cange later */
void gen_nop() { gen("nop","  ","  "); }

void gen_jsr(const char * label) { 
  target->jsr(label);
}

static void m68k_jsr(const char * label) {
  gen("jsr",label,"  "); enter_XREF(label);
  cur_libbase[0] = 0;
}

void gen_move16(const char * src, const char * dest) { gen("move.w",src,dest); }
void gen_move32(const char * src, const char * dest) { gen("move.l",src,dest); }

void gen_move(const char * src, const char *dest, int type) {
    if (type == shorttype) gen_move16(src,dest);
    else gen_move32(src,dest);
}

void gen_dataptr_next()
{
	/* advance to next DATA item */
	gen_load32a("_dataptr",2);
	gen_jsr("_strlen");
	gen_add32d_val(1,0); /* include EOS in length */

	/* FIXME: Why does it do this rather than add,l d0, _dataptr ? */
	gen_load32d("_dataptr",1);
	gen_add32dd(0,1);
	gen_save32d(1,"_dataptr");
}

/* FIXME: Isn't tst.l d0 faster? Keeping for now to
 * allow using diff against "old" ace as a regression test
 */
void gen_test() { gen("cmpi.l","#0",dreg[0]); }
void gen_cmp16dd() { gen("cmp.w","d1","d0"); }
void gen_cmp32dd() { gen("cmp.l","d1","d0"); }

void m68k_neg(int type)
{
  switch(type) {
  case shorttype: gen("neg.w","(sp)","  "); break;
  case longtype:  gen("neg.l","(sp)","  "); break;
  case singletype: 
	gen_pop32d(0); 
	gen_libcall("SPNeg","Math");
	gen_push32d(0);
	break;
  }
}

void m68k_not(int localtype) {
	if (localtype == shorttype) gen("not.w","(sp)","  ");
	else gen("not.l","(sp)","  ");
}

void gen_push_indirect_indexed(int type)
{
	if (type == stringtype) {
        /* push start address of string within BSS object */
        gen("adda.l","d7","a0");
        gen_push_addr(0);
	} else if (type == shorttype) gen("move.w","0(a0,d7.L)","-(sp)");
	else gen("move.l","0(a0,d7.L)","-(sp)");
}

void gen_push_indirect(int type) {
    if (type == shorttype)   gen("move.w","(a0)","-(sp)");
    else gen("move.l","(a0)","-(sp)");
}

void gen_pop_indirect(int type) {
    if (type == shorttype) gen("move.w","(sp)+","(a0)");
    else gen("move.l","(sp)+","(a0)");
}

void gen_push_address(unsigned short indir, const char * addrbuf, int typ)
{
    if (indir && typ != stringtype) {
        gen_load32a(addrbuf,0);        /* address of structure variable */
        gen_push_indirect(typ);       /* start address of structure */
	} else {
        if (typ == shorttype) gen_push16_var(addrbuf);
        else gen_push32_var(addrbuf);
    }
}

void m68k_push32d(unsigned char reg) { gen("move.l",dreg[reg],"-(sp)"); }
void m68k_pop32d(unsigned char reg) { gen("move.l","(sp)+",dreg[reg]); }

void gen_push32d(unsigned char reg) { target->push32d(reg); }

void gen_load_indirect(unsigned char ar, unsigned char dr)
{
  char buf[5];
  sprintf(buf,"(a%c)", ar + '0');
  gen("move.b", buf, dreg[dr]);
}

void gen_strcpy() {
    gen_jsr("_strcpy");
}

void gen_load_indirect32() {
  gen("move.l", "(a0)", "d0");
}

void gen_pokeb(int t)
{
  gen_pop_as_short(t,0); /* data to be poked */
  gen_pop_addr(0);   /* address */
  gen_save_indirect8();  /* poke (a0),d0 */
}

void gen_pokew()
{
  gen_pop_as_short(expr(), 0); /* data to be poked */
  gen_pop_addr(0);   /* address */
  gen_save_indirect16();   /* pokew (a0),d0 */
}

void gen_pokel(int t)
{
  /* coerce data to long */
  if (t == shorttype) {
	gen_pop16d(0);
	gen_ext16to32(0);
  } else gen_pop32d(0);   /* data to be poked */
  gen_pop_addr(0);       /* address */
  gen_save_indirect32(); /* pokel (a0),d0 */
}

extern	BOOL 	restore_a4;
extern	BOOL 	restore_a5;

void gen_restore_regs() {
    if (restore_a4) { gen_load32a("_a4_temp",4); restore_a4=FALSE; }
    if (restore_a5) { gen_load32a("_a5_temp",5); restore_a5=FALSE; }
}

void gen_save_indirect16() {  gen("move.w", "d0","(a0)"); }
void gen_save_indirect8()  {  gen("move.w", "d0","(a0)"); }
void gen_save_indirect32() {  gen("move.l", "d0","(a0)"); }

void gen_save_indirect_indexed32(const char * label) {
  gen("move.l", label,"0(a2,d7.L)");
}

void gen_pop_indirect_indexed32(unsigned char ar, unsigned char dr) {
  char buf[20];
  sprintf(buf,"0(%s,%s.L)", areg[ar], dreg[dr]);
  gen("move.l", "(sp)+",buf);
}

void gen_pop_indirect_indexed16(unsigned char ar, unsigned char dr) {
  char buf[20];
  sprintf(buf,"0(%s,%s.L)", areg[ar], dreg[dr]);
  gen("move.w", "(sp)+",buf);
}

void gen_load_indirect_addr(unsigned char ar, unsigned char dest) {
  char buf[5];
  sprintf(buf,"(a%c)", ar + '0');
  gen("movea.l", buf, areg[dest]);
}

void gen_ext8to16(BYTE d)  { gen("ext.w",dreg[d],"  "); }
void gen_ext16to32(BYTE d) { gen("ext.l",dreg[d],"  "); }

void gen_cmp32_val(long val, const char * target) {
  char buf[16];
  sprintf(buf,"#$%lx",val);
  gen("cmpi.l",buf,target);
}

void gen_cmp32sp_val(long val) { gen_cmp32_val(val,"(sp)"); }

void gen_test16(const char * target) {
  gen("cmpi.w","#0",target);
}

void gen_save32_val(long val, const char * target) {
  char buf[16];
  sprintf(buf,"#$%lx",val);
  gen("move.l",buf,target);
}

void gen_save32d_val(long val, BYTE r) { gen_save32_val(val,dreg[r]); }

void gen_lsl(unsigned int n) {
  char buf[20];
  sprintf(buf,"#%d",n);
  gen("lsl.l",buf,dreg[7]);
}

void gen_comment(const char * c) { gen("; *** ",c,"  "); }

void gen_push32_val(long val) {
    target->push32_val(val);
}

void m68k_push32_val(long val) {
  char buf[20];
  if (val == 0) gen("move.l","#0","-(sp)");
  else {
      sprintf(buf,"#$%lx",val);
      gen("move.l",buf,"-(sp)");
  }
}



void gen_load32d_val(signed long val, unsigned char reg) {
  char buf[16];
  sprintf(buf,"#%ld",val);
  if (val <= 255)   gen("moveq",buf,dreg[reg]);
  else gen("move.l",buf,dreg[reg]);
}


void gen_clear_addr()                { gen("move.l","#0","a0"); }
void gen_clear32(const char *label)  { gen("move.l","#0",label); }
void gen_clear16(const char * label) {  gen("move.w","#0",label); }

static void gen_swapstr(const char * tempstrname) {
  /* make copies of two addresses */
  gen("move.l","a1","d1"); /* first address */
  gen("move.l","a2","d2"); /* second address */
  
  make_temp_string();
  
  /* copy first to temp */
  gen("movea.l","d1","a1");    /* source */ 
  gen_load_addr(tempstrname,0); /* dest */ 
  gen_jsr("_strcpy");   /* temp = first */
  
  /* copy second to first */
  gen("movea.l","d2","a1");    /* source */ 
  gen("movea.l","d1","a0");    /* dest */ 
  gen_jsr("_strcpy");   /* first = second */ 
  
  /* copy temp to second */
  gen_load_addr(tempstrname,1); /* source */ 
  gen("movea.l","d2","a0");    /* dest */
  gen_jsr("_strcpy");   /* second = temp */
}

void gen_swap(int typ)
{
  gen_pop_addr(2); /* second address */
  gen_pop_addr(1); /* first address */
  
  if (typ == stringtype) gen_swapstr(tempstrname);
  else if (typ == shorttype) {
      gen("move.w","(a1)","d0");   /* temp = [first] */
      gen("move.w","(a2)","(a1)"); /* [first] = [second] */
      gen("move.w","d0","(a2)");   /* [second] =temp */     
  } else {
      gen("move.l","(a1)","d0");   /* temp = [first] */
      gen("move.l","(a2)","(a1)"); /* [first] = [second] */
      gen("move.l","d0","(a2)");   /* [second] =temp */     
  }
}

void gen_save_registers() { gen("movem.l","d1-d7/a0-a6","-(sp)"); }
void gen_restore_registers() { gen("movem.l","(sp)+","d1-d7/a0-a6"); }

void gen_add32dd(BYTE reg1, BYTE reg2) { gen("add.l",dreg[reg1],dreg[reg2]); }

void gen_rport_rel_x(unsigned char r) {
  gen("add.w","36(a1)",dreg[r]);   /* x + RPort->cp_x */
}

void gen_rport_rel_y(unsigned char r) {
  gen("add.w","38(a1)",dreg[r]);   /* y + RPort->cp_y */
}

void gen_rport_rel_xy() {
  gen_rport_rel_x(0);
  gen_rport_rel_y(1);
}

void gen_push1(int countertype) {
    switch(countertype)   /* default step = 1 */
        {
        case shorttype  : gen_push16_val(1); break;
        case longtype   : gen_push32_val(1); break;
        case singletype : gen_push32_val(0x80000041); break;
        }
}

static void m68k_or(int type)
{
  if (type == shorttype) gen("or.w","d1","d0");
  else gen("or.l","d1","d0");
}

static void m68k_eor(int type)
{
  if (type == shorttype) gen("eor.w","d1","d0");
  else gen("eor.l","d1","d0");
}

void gen_sub(int t)
{
  if (t == shorttype)     gen("sub.w","d1","d0");
  else if (t == longtype) gen("sub.l","d1","d0");
  else gen_libcall("SPSub","Math");
}

void gen_add(int t)
{
  if (t == shorttype)     gen("add.w","d1","d0");
  else if (t == longtype) gen("add.l","d1","d0");
  else gen_libcall("SPAdd","Math");
}

void gen_add_addr_offset(long val) {
  char buf[16];
  sprintf(buf,"#%ld",val);
  gen("adda.l",buf,"a0");
}

void gen_add32d_val(long val, unsigned char reg) {
  char buf[16];
  sprintf(buf,"#%ld",val);
  if (val < 8) gen("addq",buf,dreg[reg]);
  else gen("add.l",buf,dreg[reg]);
}

void gen_counter_incr(int typ, const char * step, const char * counter) {
    gen_loadd(typ, step, 0);
    switch (typ) {
    case shorttype:
        gen("add.w","d0",counter);
        break;
    case longtype:
        gen("add.l","d0",counter);
        break;
    case singletype:
		gen_load32d(counter,1);
        gen_libcall("SPAdd","Math");
        gen_save32d(0,counter);
        break;
    }
}

void gen_sub32d_val(long val, unsigned char reg) {  
  char buf[16];
  sprintf(buf,"#%ld",val);
  gen("sub.l",buf,dreg[reg]);
}

void gen_load16d_val(int val, unsigned char reg)
{
  char buf[16];
  sprintf(buf,"#%d",val);
  gen("move.w",buf,dreg[reg]);
}

void gen_push16_val(int val) {
    target->push16_val(val);
}

void m68_pop32d(BYTE r) { gen("move.l","(sp)+",dreg[r]); }

void m68k_push16_val(int val) {
  char buf[16];
  sprintf(buf,"#%d",val);
  gen("move.w",buf,"-(sp)");
}

void gen_pop_ignore(unsigned int bytes) {
  char buf[16];
  sprintf(buf,"#%d",(int)bytes);
  if (bytes > 7) gen("add.l",buf,"sp");
  else gen("addq",buf,"sp");
}

/* Pop 4 bytes off the stack and copy into a variable. */
void gen_pop32_var(const char * label) {
  gen("move.l","(sp)+",label);
}

void gen_pop16_var(const char * label) {
  gen("move.w","(sp)+",label);
}

void gen_pop_var(const char * label, int type) {
    if (type == shorttype) gen_pop16_var(label);
    else gen_pop32_var(label);
}

static void gen_push16d(unsigned char reg) {
  gen("move.w",dreg[reg],"-(sp)");
}

int gen_push_var(const char * absbuf, int mbr_type, int member_offset) {
    if (mbr_type == bytetype) {
        gen("move.b",absbuf,"d0");
        gen_ext8to16(0);
        push_result(shorttype);
        mbr_type=shorttype;              /* byte */
    } else if (mbr_type == shorttype)
        gen("move.w",absbuf,"-(sp)");
    else if (mbr_type == stringtype) {
        gen_add_addr_offset(member_offset);
        gen_push_addr(0);  /* push string address */
    } else
        gen("move.l",absbuf,"-(sp)"); /* long, single */ 
    return mbr_type;
}

void gen_push16_var(const char *label) { gen_push_var(label, shorttype,0); }
void gen_push32_var(const char *label) { gen_push_var(label, longtype,0); }

void gen_pop16d(unsigned char reg) {
  gen("move.w","(sp)+",dreg[reg]);
}

void gen_push_addr(unsigned char reg) {
  gen("move.l",areg[reg],"-(sp)");
}

void gen_load_addr(const char * label,unsigned char reg) {
  gen("lea",label,areg[reg]);
}

void gen_pop_addr(unsigned char reg) {
  gen("move.l","(sp)+",areg[reg]);
}

void gen_load32a(const char * label, unsigned char reg) {
  gen("move.l",label,areg[reg]);
}

void gen_load32d(const char * label, unsigned char reg) {
  gen("move.l",label,dreg[reg]);
}

void gen_load16d(const char * label, unsigned char reg) {
  gen("move.w",label,dreg[reg]);
}

void gen_loadd(int typ, const char * label, unsigned char reg) {
    switch (typ) {
    case shorttype:
        gen_load16d(label,reg);
        break;
    case longtype:
    case singletype:
        gen_load32d(label, reg);
    }
}

void gen_pop8_var(const char * label) {
  gen_pop16d(0);
  gen("move.w","d0", label);
}

void gen_save16d(unsigned char reg, const char * label) {
  gen("move.w",dreg[reg],label);
}

void gen_save32d(unsigned char reg, const char * label) {
  gen("move.l",dreg[reg],label);
}

void gen_save32a(unsigned char reg, const char * label) {
  gen("move.l",areg[reg],label);
}

void gen_frame_offset_simple(int offset, int local_off)
{
    char buf1[40];
    itoa(local_off,buf1,10);
    strcat(buf1,"(a5)");
    gen("move.l","a4","d0");   /* frame pointer */
    gen_sub32d_val(offset,0);  /* offset from frame top */
    gen_save32d(0,buf1);       /* store address in level ONE frame */
}

void gen_frame_offset_addr(int offset, int local_off)
{
  char buf0[40], buf1[40];
  /* frame location of level ONE object */ 
  itoa(local_off,buf1,10);
  strcat(buf1,"(a5)");

  itoa(offset,buf0,10);
  strcat(buf0,"(a4)");
  gen_move32(buf0,buf1);
}

void gen_move16dd(unsigned char srcreg, unsigned char destreg) {
  gen("move.w",dreg[srcreg],dreg[destreg]);
}

void gen_move32dd(unsigned char srcreg, unsigned char destreg) {
  gen("move.l",dreg[srcreg],dreg[destreg]);
}

void gen_move32aa(unsigned char srcreg, unsigned char destreg) {
  gen("move.l",areg[srcreg],areg[destreg]);
}

void gen_move32da(unsigned char srcreg, unsigned char destreg) {
  gen("movea.l",dreg[srcreg],areg[destreg]);
}

void gen_load32a_plus_index(const char * addrbuf) {
    gen_load32a(addrbuf,0);
    gen("adda.l","d7","a0");
}

/***** "Mid level" code generation functions *****/

void gen_round(int type) {  
  /*
  ** Convert float to integer
  ** with rounding.
  */
  gen_call_args("_round","d0:d0",0);
  enter_XREF("_MathBase");

  /*
  ** Only relevant when called from
  ** assign_coerce() and STOREType=shorttype.
  */
  if (type == shorttype) {
   gen_pop32d(0);
   gen_push16d(0);
  }
}  
 

/* convert an integer to a single-precision float */
int gen_Flt(int typ) {
  if (typ == singletype) return singletype;  /* already a float! */
  if (typ == stringtype) {_error(4); return undefined; } /* can't do it */
  return target->Flt(typ);
}

int m68k_Flt(int typ) {
  if (typ == shorttype) gen_pop16d(0);
  else gen_pop32d(0);
  if (typ == shorttype) gen_ext16to32(0); /* extend sign */
  gen_libcall("SPFlt","Math");
  gen_push32d(0);
  return singletype;
}


void pop_operands(int typ) {
  if (typ == shorttype) {
	gen_pop16d(0);  /* 2nd operand */
	gen_pop16d(1);  /* 1st operand -> d0 = d1 op d0 */
  } else {
	gen_pop32d(0);  /* 2nd operand */
	gen_pop32d(1);  /* 1st operand -> d0 = d1 op d0 */
  } 
}

void push_result(int typ) {
 if (typ == shorttype) gen_push16d(0);
 else gen_push32d(0);
}

void change_Flt(int exptyp,CODE * cx[]) {
  /* convert an integer to a float */
  if (exptyp == shorttype) change(cx[0],"move.w","(sp)+","d0");
  else change(cx[0],"move.l","(sp)+","d0");
  if (exptyp == shorttype) change(cx[1],"ext.l","d0","  ");
  change(cx[2],"move.l","_MathBase","a6");
  change(cx[3],"jsr","_LVOSPFlt(a6)","  ");
  change(cx[4],"move.l","d0","-(sp)");
  enter_XREF("_LVOSPFlt");
  enter_XREF("_MathBase");
}

int make_integer(int oldtyp) {
  if (oldtyp == stringtype) return(notype); /* can't do it! */
  else if (oldtyp == singletype) { 
	gen_round(oldtyp);
	return(longtype); 
  }
  return(oldtyp);  /* already an integer */
}

int long_expr() {
    return make_sure_long(expr());
}

int in_long_expr() {
    insymbol();
    return long_expr();
}

int comma_long_expr() {
    if (sym != comma) {
        _error(16);
        return 0;
    } else {
        in_long_expr();
        return 1;
    }
}
 
void parse_gen_params_with_default(long tok, const char * params, long def) {
    if (!parse_gen_params(tok,params)) gen_push32_val(def);
}


int peek(int token) {
    return sym == token;
}

int eat(int token) {
    if (sym == token) {
        insymbol();
        return 1;
    }
    return 0;

}

int eat_token(int token, int err) {
    if (eat(token)) return 1;
    if (err != 0) _error(err);
    return 0;
}

/* More conventional name for the one that throws an error */
int expect(int token, int err) {
    return eat_token(token,err);
}


int eat_comma(void) {
    return eat_token(comma,16);
}
       
int try_comma(void) {
    return eat_token(comma, 0);
}

int nostring(int type) {
    if (type != stringtype) return 1;
    _error(4);
    return 0;
}


int make_sure_short(int type) {
 if (type == longtype) make_short();
 else if (type == singletype) { make_integer(type); make_short(); }
 else if (type == stringtype) { _error(4); return undefined; }
 return shorttype;
}

int gen_pop_as_short(int type, unsigned char reg) {
  if (type == singletype) type = make_integer(type);
  if (type == longtype) gen_pop32d(reg);
  else if (type == shorttype) { gen_pop16d(reg); }
  else if (type == stringtype) { _error(4); return undefined; }
  return shorttype;
}

int make_sure_long(int type) {
 if      (type == shorttype) make_long();
 else if (type == singletype) make_integer(type);
 else if (type == stringtype) { _error(4); return undefined; }
 return longtype;
}

void make_short() {
 gen_pop32d(0);
 gen_push16d(0);
}

void make_long() {
 gen_pop16d(0);
 gen_ext16to32(0);
 gen_push32d(0);
}


void gen_instr(BOOL offset_on_stack) {
    gen_pop_addr(1);		/* Y$ */
    gen_pop_addr(0);		/* X$ */
    if (offset_on_stack) gen_pop32d(0);	/* I */
    else gen_load32d_val(1,0); /* I = 1 */
    gen_call("_instr",0); /* returns posn of Y$ in X$ */
}

void gen_hexstr(long sftype) {
    sftype = make_integer(sftype);
    if (sftype == longtype) gen_call_args("_hexstrlong","t0,d0 : a0",0);
    else gen_call_args("_hexstrshort","t0,d0.w : a0",0);
}

void gen_gfxcall(const char * lvo) {
  gen_load32a("_RPort",1);
  enter_XREF("_RPort");
  gen_libcall(lvo,"Gfx");
}

void gen_area(BOOL relative) {
  gen_pop16d(1); /* pop y-coordinate */
  gen_pop16d(0); /* pop x-coordinate */
  
  /* include point in area info' */
  if (relative) {
	gen("add.w","_last_areaX","d0");   /* d0 = x + lastareaY */
	gen("add.w","_last_areaY","d1");   /* d1 = y + lastareaY */
	enter_XREF("_last_areaX");
	enter_XREF("_last_areaY");
  }
  
  gen_jsr("_area");
}

/* Pop an argument off the statck.
 * d[0-7] => data registers
 * a[0-7] => address registers
 * t[0-7] => Create a temporary string, load into address register
 */
static const char * gen_pop_arg(const char * args) {
  if (*args == 'd') {
	if (args[2] == '.') {
	  if (args[3] == 'w') gen_pop16d(args[1] - '0');
	  args += 4;
	} else {
	  gen_pop32d(args[1] - '0');
	  args += 2;
	}
  } else if (*args == 'a') {
	gen_pop_addr(args[1] - '0');
	args += 2;
  } else if (*args == 't') {
	load_temp_string(args[1] - '0');
	args += 2;
  } else args += 1;
  return args;
}

static const char * gen_push_ret(const char * args) {
  if (*args != ':') return args;
  args += 1;
  if (*args == 'd') {
	if (args[2] == '.') {
	  if (args[3] == 'w') gen_push16d(args[1] - '0');
	} else {
	  gen_push32d(args[1] - '0');
	}
  } else if (*args == 'a') {
	gen_push_addr(args[1] - '0');
  }
  return args;
}

void m68k_call_args(const char * label, const char * args, unsigned int stack) {
  while (*args && args[0] != ':') {
	args = gen_pop_arg(args);
  }
  gen_jsr(label);
  if (stack > 0) gen_pop_ignore(stack);
  gen_push_ret(args);
}


void gen_call_args(const char * label, const char * args, unsigned int stack) {
    target->call_args(label,args,stack);
}

int gen_fcall(const char * funcname, int type, const char * params, int ret, const char * callargs, int stackadj)
{
  if (parse_gen_params(type,params) == undefined) return undefined;
  gen_call_args(funcname, callargs, stackadj);
  return ret;
}

void gen_call_sargs(const char * funcname, const char * params, int stackadj) {
  if (parse_gen_params(0,params) == undefined) return;
  gen_call(funcname, stackadj);
}


void gen_call_indirect(const char * addr) {
  gen("move.l",addr,"a0");
  gen("jsr","(a0)","  ");
}

void gen_lmulu() {
    gen_call_void("lmulu",8);
}

static int m68k_muls(int type)
{
  switch(type) {
  case shorttype:  
	pop_operands(shorttype);
	gen("muls","d1","d0");
	return longtype;
	break;
  case longtype   :	/* args on stack */
	gen_call_void("lmul",8); 
	return longtype;
	break;
  case singletype :  
	pop_operands(shorttype);
	gen_libcall("SPMul","Math");
	return singletype;
	break;
  }
  return notype;
}

void m68k_cmp(int simptype, int op)
{
  /* compare on basis of type -> d5 = d0 op d1 */
  char labname[80],lablabel[80];
   switch(simptype) {
    case shorttype  : 	
	  gen_pop16d(1);  /* 2nd */
	  gen_pop16d(0);  /* 1st */
	  gen_load32d_val(-1,5); /* assume true */
	  gen_cmp16dd();
	  break;

    case longtype:
	  gen_pop32d(1);  /* 2nd */
	  gen_pop32d(0);  /* 1st */
	  gen_load32d_val(-1,5); /* assume true */
	  gen_cmp32dd();
	  break;

    case singletype : 	
	  gen_pop32d(1);  /* 2nd */
	  gen_pop32d(0);  /* 1st */
	  gen_load32d_val(-1,5); /* assume true */
	  gen_libcall("SPCmp","Math");
	  break;

    case stringtype :
	  switch(op) {
	  case equal     : gen_call_args("_streq","a1,a0:d0",0); break;
	  case notequal  : gen_call_args("_strne","a1,a0:d0",0); break;
	  case lessthan  : gen_call_args("_strlt","a1,a0:d0",0); break;
	  case gtrthan   : gen_call_args("_strgt","a1,a0:d0",0); break;
	  case ltorequal : gen_call_args("_strle","a1,a0:d0",0); break;
	  case gtorequal : gen_call_args("_strge","a1,a0:d0",0); break;
	  }
	  break;
    }
   /* leave result on stack according to operator (-1 = true, 0 = false) */
   /* (this code for short,long & single comparisons only) */
   if (simptype != stringtype) {	
     make_label(labname,lablabel);
	 gen_bxx(op,labname);
     gen_load32d_val(0,5);  /* not true */
     gen_label(lablabel);
     gen_push32d(5); /* boolean result on stack */
   }
}

static void m68k_code_section(FILE * dest)
{
  fprintf(dest,"\n\tSECTION code,CODE\n\n");
}

static void m68k_end_program(FILE * dest)
{
  fprintf(dest,"\n\tEND\n");
}

/* The m68k target */
void m68k_amiga_startup(FILE * dest);

struct codegen_target m68k_target = {
  m68k_and,
  m68k_cmp,
  m68k_decr_indir,
  m68k_eor,
  m68k_incr_indir,
  m68k_jsr,
  m68k_link,
  m68k_muls,
  m68k_neg,
  m68k_not,
  m68k_or,
  m68k_pea,
  m68k_pop32d,
  m68k_push16_val,
  m68k_push32d,
  m68k_push32_val,
  generic_rts,
  m68k_unlk,

  m68k_call_args,
  m68k_Flt,
  
  m68k_code_section,
  m68k_end_program,
  m68k_amiga_startup,

  1 /* write_xrefs? */
};

void nop()
{
}


void gen_lmod() { gen_call("ace_lrem",8); }
void gen_ldiv() { gen_call("ace_ldiv",8); }
void gen_fcmp() { gen_libcall("SPCmp","Math"); }

void gen_fdiv() { 
  gen_pop32d(1);  /* 2nd operand */
  gen_pop32d(0);  /* 1st operand */
  gen_libcall("SPDiv","Math"); 
}

void gen_power()
{
  gen_call("_power",8);	/* - Call exponentiation function. */
  enter_XREF("_MathTransBase"); /* opens FFP+IEEE SP transcendental libraries */
}


void gen_str_concat() 
{
  gen_pop_addr(2); /* 2nd */
  gen_call_args("_strcpy","a1,t0",0);
  /* prepare for strcat */
  gen_load_addr(tempstrname,0);
  gen_move32aa(2,1);
  gen_jsr("_strcat");
  gen_pea(tempstrname);
}

void gen_fmod() 
{
  /* single MOD */
  gen_call("_modffp",0);
  enter_XREF("_MathBase");
}

void gen_translate() { 
  gen_pop_addr(0); /* instr */
  load_temp_string(1); /* outstr */
  gen_move32aa(0,2);
  gen_jsr("_strlen"); /* inlen in d0 */
  gen_load32d_val(MAXSTRLEN,1); /* outlen = MAXSTRLEN */
  gen_libcall("Translate","Trans"); 
  gen_pea(tempstrname); /* outstr on stack */
}

void gen_val()
{
  gen_call("_val",4); /* string is on the stack */
  enter_XREF("_MathBase");  /* _val needs math libs */
  enter_XREF("_MathTransBase");
}

void gen_peek(int nftype)
{
  char labname[80];
  char lablabel[80];

  /* get address */
  if (nftype == shorttype) gen_pop_short_addr(0,0);
  else gen_pop_addr(0); 
  /* get value */
  gen_load_indirect(0,0);
  gen_ext8to16(0);
  /* if n<0 n=255-not(n) */
  gen("cmp.w","#0","d0");
  make_label(labname,lablabel);
  gen_bge(labname);
  gen("not.w","d0","  ");
  gen_load16d_val(255,1);
  gen("sub.w","d0","d1");
  gen_move16dd(1,0);
  gen_label(lablabel);
  push_result(shorttype);
}

void gen_roundl() 
{ 
  gen_pop32d(0);
  gen_libcall("SPFloor","Math");
  gen_libcall("SPFix","Math");
  push_result(longtype);
}

void gen_shl()
{
  gen_pop32d(1);
  gen_pop32d(0);
  gen("asl.l",dreg[1],dreg[0]); 
  push_result(longtype);
}

void gen_shr()
{
  gen_pop32d(1);
  gen_pop32d(0);
  gen("asr.l",dreg[1],dreg[0]);
  push_result(longtype);
}

void gen_point()
{
  gen_pop16d(1);  /* y */
  gen_pop16d(0);  /* x */
  gen_gfxcall("ReadPixel");
  push_result(longtype);
}

void gen_gfx_move() { gen_libcall("Move", "Gfx"); }
void gen_writepixel() { gen_libcall("WritePixel", "Gfx"); }
void gen_paint() { gen_call_args("_paint","d1.w,d0.w",0); }
void gen_flt() { gen_libcall("SPFlt","Math"); }
void gen_draw() { gen_libcall("Draw","Gfx"); }
void gen_rectfill() { gen_libcall("RectFill","Gfx"); }
void gen_scrollraster() { 
  /* pop parameters */
  gen_pop16d(1);		/* delta-y */
  gen("neg.w","d1","   ");
  gen_pop16d(0);		/* delta-x */
  gen("neg.w","d0","   ");
  gen_pop16d(5);  		/* ymax */
  gen_pop16d(4);  		/* xmax */
  gen_pop16d(3);  		/* ymin */
  gen_pop16d(2);  		/* xmin */
  
  gen_gfxcall("ScrollRaster"); 
}


void gen_setapen() { gen_gfxcall("SetAPen"); }
void gen_setbpen() { gen_gfxcall("SetBPen"); }
void gen_forbid() { gen_libcall("Forbid","AbsExec"); }
void gen_permit() { gen_libcall("Permit","AbsExec"); }
void gen_fix()
{
  gen_pop32d(0);
  gen_libcall("SPFix","Math"); 
  push_result(longtype);
}

void gen_atan() { gen_libcall("SPAtan","MathTrans"); }
void gen_tan() { gen_libcall("SPTan","MathTrans"); }
void gen_cos() { gen_libcall("SPCos","MathTrans"); }
void gen_exp() { gen_libcall("SPExp","MathTrans"); }
void gen_log() { gen_libcall("SPLog","MathTrans"); }
void gen_sqrt() { gen_libcall("SPSqrt","MathTrans"); }
void gen_sin() { gen_libcall("SPSin","MathTrans"); }

void gen_writeshort() {
  gen_pop16d(1);
  gen_load32d("_seq_filenumber",0);
  gen_jsr("_writeshort");
}

void gen_writelong() {
  gen_pop32d(1);
  gen_load32d("_seq_filenumber",0);
  gen_jsr("_writelong");
}

void gen_writesingle() {
  gen_pop32d(1);
  gen_load32d("_seq_filenumber",0);
  gen_jsr("_writesingle");
  enter_XREF("_MathBase");
}

void gen_writestring() {
  gen_load32d("_seq_filenumber",0);
  gen_jsr("_writequote");	
  gen_pop_addr(0);
  gen_load32d("_seq_filenumber",0);
  gen_jsr("_writestring");
  gen_load32d("_seq_filenumber",0);
  gen_jsr("_writequote");
}

void gen_writecomma() {
  gen_load32d("_seq_filenumber",0);
  gen_jsr("_writecomma"); 
}

void gen_write_eoln() {
  gen_load32d("_seq_filenumber",0);
  gen_jsr("_write_eoln");
}

void gen_pop_filenumber() {
  gen_pop32_var("_seq_filenumber");
  enter_BSS("_seq_filenumber:","ds.l 1");
}

void gen_writetab() { 
  gen_load32d("_seq_filenumber",0);
  gen_jsr("_writeTAB"); 
}

void gen_writespc() { 
  gen_load32d("_seq_filenumber",0);
  gen_jsr("_writeSPC");
}

void gen_rename()
{
  gen_pop32d(2);  /* <filespec2> */
  gen_pop32d(1);  /* <filespec1> */
  gen_jsr("_rename");
}

void gen_kill()
{
  gen_pop32d(1);
  gen_jsr("_kill");
}

void gen_fwrite(int exprtype)
{
    /* pass filenumber to write routine */
    gen_load32d("_seq_filenumber", exprtype == stringtype ? 0 : 1);
    
    switch(exprtype) {
    case shorttype : gen_fcall("_fprintshort",shorttype,"w",longtype,"d0",0); break;
    case longtype:   gen_fcall("_fprintlong",longtype,"l",longtype,"d0",0); break;
    case singletype: gen_fcall("_fprintsingle",singletype,"f",singletype,"d0",0); break;
    case stringtype: gen_fcall("_writestring",stringtype,"s",stringtype,"a0",0); break;
    }
}

void gen_push_deref_array(SYM * storage, char * addrbuf)
{
  point_to_array(storage,addrbuf);
  gen_load32d(addrbuf,0);
  gen("add.l","d7","d0");
  push_result(longtype);
}


void gen_openfile()   { gen_call_args("_openfile","a1,d0,a0",0); }
void gen_closefile()  { gen_call_args("_closefile","d0",0); }
void gen_line_input() { gen_call_args("_line_input","d0",0); }
void gen_files()      { gen_call_void("_files",4); } 
void gen_chdir(int e) { gen_fcall("_chdir",e,"s",stringtype,"d1",0); }
void gen_getrecord()  { gen_call_void("_GetRecord",16); }
void gen_putrecord()  { gen_call_void("_PutRecord",16); }

void gen_push_nullstring() {
	 enter_DATA("_nullstring:","dc.b 0");
	 gen_pea("_nullstring");
}



void input_short(SYM * storage,const char * func, char * addrbuf, int lev) {
  gen_jsr(func);
  if (storage->object == variable) {
	if ((storage->shared) && (lev == ONE)) {
	  gen_load32a(addrbuf,0); /* abs address of store */
	  gen_save_indirect16();
	} else gen_save16d(0,addrbuf); /* ordinary variable */
  } else if (storage->object == array) {
	gen_save16d(0,"_short_input_temp");
	point_to_array(storage,addrbuf);
    gen("move.w", "_short_input_temp","0(a2,d7.L)");
	enter_BSS("_short_input_temp:","ds.w 1");
  }
}

void input_long(SYM * storage, const char * func, char * addrbuf, int lev) {
 gen_jsr(func);
 if (storage->object == variable) {
   if ((storage->shared) && (lev == ONE)) {
	 gen_load32a(addrbuf,0);  /* abs address of store */
	 gen_save_indirect32();
   } else gen_save32d(0,addrbuf); /* ordinary variable */
 } else  if (storage->object == array) {
   gen_save32d(0,"_long_input_temp");
   point_to_array(storage,addrbuf);
   gen_save_indirect_indexed32("_long_input_temp");
   enter_BSS("_long_input_temp:","ds.l 1");
 }
}

void input_single(SYM * storage, const char * func, char * addrbuf, int lev) {
    input_long(storage,func,addrbuf, lev);
    enter_XREF("_MathBase"); /* need math libs */
    enter_XREF("_MathTransBase");
}

