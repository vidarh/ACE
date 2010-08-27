
#include "acedef.h"
#include "codegen.h"

extern BOOL   early_exit;
extern CODE * new_code, * curr_code;

void kill_all_lists();

static void gen(const char * opcode,const char * srcopr,const char *destopr)
{
 /* allocate memory for a new node & each field */
 if ((new_code = (CODE *)alloc_code(opcode,srcopr,destopr)) == NULL)
 {
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



static const char *cond_branch_op(int op)
{
 switch(op)
 {
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

/* For inline ASM, and hence completely arch. specific */
void gen_asm(const char * line) { gen(line,"  ","  "); }
void gen_label(const char * label) { gen(label,"  ","  "); }

void gen_bxx(int op, const char * label) {
  gen(cond_branch_op(op),label,"  ");
  enter_XREF(label);
}

void gen_incr_indirect16() { gen("add.w","#1","(a0)"); }
void gen_incr_indirect32() { gen("add.l","#1","(a0)"); }

void gen_incr_indirect_float() {
  gen_libbase("Math");
  gen("move.l","(a0)","d0");
  gen("move.l","#$80000041","d1");
  gen_libcall("SPAdd","Math");
  gen("move.l","d0","(a0)");
}

void gen_decr_indirect16() { gen("sub.w","#1","(a0)"); }
void gen_decr_indirect32() { gen("sub.l","#1","(a0)"); }

void gen_decr_indirect_float() {
  gen_libbase("Math");
  gen("move.l","(a0)","d0");
  gen("move.l","#$80000041","d1");
  gen_libcall("SPSub","Math");
  gen("move.l","d0","(a0)");
}

void gen_branch(char * branch, char * labname) {
  char lablabel[MAXIDSIZE+1],destbuf[3];

 /* generate a jsr/jmp instruction */

 sprintf(lablabel,"%s:",labname);

 if (!exist(lablabel,label)) strcpy(destbuf,"* ");  /* for later check */
 else strcpy(destbuf,"  ");  /* label already defined */

 gen(branch,labname,destbuf);
}

void gen_link() { gen("link","a5","  "); }
void gen_unlk() { gen("unlk","a5","  "); }
void gen_rts() { gen("rts","  ","  "); }
void gen_bne(const char * label) { gen_bxx(notequal, label); }
void gen_beq(const char * label) { gen_bxx(equal, label); }
void gen_bge(const char * label) { gen_bxx(gtorequal, label); }
void gen_bgt(const char * label) { gen("bgt.s",label,"  "); }
void gen_blt(const char * label) { gen("blt.s",label,"  "); }
void gen_jmp(const char * label) { gen("jmp",label,"  "); }
void gen_jmp_fwd(const char * label,const char * flag) { gen("jmp",label,flag); }

void gen_pea(const char * target) { gen("pea",target,"  "); }

/* Used to create ops to possibly cange later */
void gen_nop() { gen("nop","  ","  "); }

void gen_jsr(const char * label) {
  gen("jsr",label,"  ");
  enter_XREF(label);
}


void gen_move16(const char * src, const char * dest) { gen("move.w",src,dest); }
void gen_move32(const char * src, const char * dest) { gen("move.l",src,dest); }

/* FIXME: Isn't tst.l d0 faster? Keeping for now to
 * allow using diff against "old" ace as a regression test
 */
void gen_tst32d(unsigned char reg) { gen("cmpi.l","#0",dreg[reg]); }
void gen_tst16d(unsigned char reg) { gen("cmp.w","#0",dreg[reg]); }

void gen_cmp16dd(unsigned char r1, unsigned char r2) {
  gen("cmp.w",dreg[r1],dreg[r2]);
}

void gen_cmp32dd(unsigned char r1, unsigned char r2) {
  gen("cmp.l",dreg[r1],dreg[r2]);
}

void gen_not16d(unsigned char reg) { gen("not.w",dreg[reg],"  "); }
void gen_neg16d(unsigned char r) { gen("neg.w",dreg[r],"  "); }

void gen_neg16sp() { gen("neg.w","(sp)","  "); }
void gen_neg32sp() { gen("neg.l","(sp)","  "); }
void gen_not16sp() { gen("not.w","(sp)","  "); }
void gen_not32sp() { gen("not.l","(sp)","  "); }
void gen_muls()    { gen("muls","d1","d0"); }

void gen_push_indirect_indexed16() { gen("move.w","0(a0,d7.L)","-(sp)"); }
void gen_push_indirect_indexed32() { gen("move.l","0(a0,d7.L)","-(sp)"); }

void gen_push_indirect32(unsigned char r) {
  char buf[5];
  sprintf(buf,"(a%c)", r + '0');
  gen("move.l",buf,"-(sp)");
}

void gen_pop_indirect32(unsigned char r) {
  char buf[5];
  sprintf(buf,"(a%c)", r + '0');
  gen("move.l","(sp)+",buf);
}

void gen_pop_indirect16(unsigned char r) {
  char buf[5];
  sprintf(buf,"(a%c)", r + '0');
  gen("move.w","(sp)+",buf);
}


void gen_push_indirect16(unsigned char r) {
  char buf[5];
  sprintf(buf,"(a%c)", r + '0');
  gen("move.w",buf,"-(sp)");
}

void gen_push32d(unsigned char reg) { gen("move.l",dreg[reg],"-(sp)"); }

void gen_load_indirect(unsigned char ar, unsigned char dr)
{
  char buf[5];
  sprintf(buf,"(a%c)", ar + '0');
  gen("move.b", buf, dreg[dr]);
}

void gen_load_indirect32(unsigned char ar, unsigned char dr) {
  char buf[5];
  sprintf(buf,"(a%c)", ar + '0');
  gen("move.l", buf, dreg[dr]);
}

void gen_save_indirect16(unsigned char dr, unsigned char ar) {
  char buf[5];
  sprintf(buf,"(a%c)", ar + '0');
  gen("move.w", dreg[dr],buf);
}


void gen_save_indirect8(unsigned char dr, unsigned char ar) {
  char buf[5];
  sprintf(buf,"(a%c)", ar + '0');
  gen("move.w", dreg[dr],buf);
}


void gen_save_indirect32(unsigned char dr, unsigned char ar) {
  char buf[5];
  sprintf(buf,"(a%c)", ar + '0');
  gen("move.l", dreg[dr],buf);
}


void gen_save_indirect_indexed32(const char * label, unsigned char ar, unsigned char dr) {
  char buf[20];
  sprintf(buf,"0(%s,%s.L)", areg[ar], dreg[dr]);
  gen("move.l", label,buf);
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

void gen_save_indirect_indexed16(const char * label, unsigned char ar, unsigned char dr) {
  char buf[20];
  sprintf(buf,"0(%s,%s.L)", areg[ar], dreg[dr]);
  gen("move.w", label,buf);
}

void gen_load_indirect_addr(unsigned char ar, unsigned char dest) {
  char buf[5];
  sprintf(buf,"(a%c)", ar + '0');
  gen("movea.l", buf, areg[dest]);
}

void gen_ext8to16(unsigned char d)  { gen("ext.w",dreg[d],"  "); }
void gen_ext16to32(unsigned char d) { gen("ext.l",dreg[d],"  "); }

void gen_ext8to32(unsigned char d) {
  gen_ext8to16(d);
  gen_ext16to32(d);
}

void gen_cmp32_val(long val, const char * target) {
  char buf[16];
  sprintf(buf,"#$%lx",val);
  gen("cmpi.l",buf,target);
}

void gen_cmp32sp_val(long val) {
  gen_cmp32_val(val,"(sp)");
}


void gen_cmp16_val(long val, const char * target) {
  char buf[16];
  sprintf(buf,"#$%lx",val);
  gen("cmpi.w",buf,target);
}

void gen_save32_val(long val, const char * target) {
  char buf[16];
  sprintf(buf,"#$%lx",val);
  gen("move.l",buf,target);
}


void gen_save32d_val(long val, unsigned char r) {
  gen_save32_val(val,dreg[r]);
}

void gen_lsl(unsigned int n) {
  char buf[20];
  sprintf(buf,"#%d",n);
  gen("lsl.l",buf,dreg[7]);
}

void gen_comment(const char * c) { gen("; *** ",c,"  "); }

void gen_push32_val(long val) {
  char buf[16];
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

void gen_load32a_val(long val, unsigned char reg) {
  gen_load32d_val(val,areg[reg]);
}

void gen_load32_val(long val, const char * label) {
  char buf[16];
  sprintf(buf,"#%ld",val);
  gen("move.l",buf,label);
}

void gen_load16_val(long val, const char * label) {
  char buf[16];
  sprintf(buf,"#%ld",val);
  gen("move.w",buf,label);
}

void gen_swapstr(const char * tempstrname) {
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

void gen_swap16() {
  gen("move.w","(a1)","d0");   /* temp = [first] */
  gen("move.w","(a2)","(a1)"); /* [first] = [second] */
  gen("move.w","d0","(a2)");   /* [second] =temp */     
}

void gen_swap32() {
      gen("move.l","(a1)","d0");   /* temp = [first] */
      gen("move.l","(a2)","(a1)"); /* [first] = [second] */
      gen("move.l","d0","(a2)");   /* [second] =temp */     
}


void gen_save_registers() { gen("movem.l","d1-d7/a0-a6","-(sp)"); }
void gen_restore_registers() { gen("movem.l","(sp)+","d1-d7/a0-a6"); }

void gen_add32dd(unsigned char reg1, unsigned char reg2) {
  gen("add.l",dreg[reg1],dreg[reg2]);
}

void gen_add16d_var(const char * var, unsigned char r) {
  gen("add.w",var,dreg[r]);
}

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

void gen_add16dd(unsigned char reg1, unsigned char reg2) {
  gen("add.w",dreg[reg1],dreg[reg2]);
}

void gen_and32dd(unsigned char reg1, unsigned char reg2) {
  gen("and.l",dreg[reg1],dreg[reg2]);
}

void gen_and16dd(unsigned char reg1, unsigned char reg2) {
  gen("and.w",dreg[reg1],dreg[reg2]);
}

void gen_or32dd(unsigned char reg1, unsigned char reg2) { gen("or.l",dreg[reg1],dreg[reg2]); }
void gen_or16dd(unsigned char reg1, unsigned char reg2) { gen("or.w",dreg[reg1],dreg[reg2]); }
void gen_eor32dd(unsigned char reg1, unsigned char reg2) { gen("eor.l",dreg[reg1],dreg[reg2]); }
void gen_eor16dd(unsigned char reg1, unsigned char reg2) { gen("eor.w",dreg[reg1],dreg[reg2]); }
void gen_asr32dd(unsigned char reg1, unsigned char reg2) { gen("asr.l",dreg[reg1],dreg[reg2]); }
void gen_asl32dd(unsigned char reg1, unsigned char reg2) {
  gen("asl.l",dreg[reg1],dreg[reg2]);
}

void gen_sub16dd(unsigned char reg1, unsigned char reg2) {
  gen("sub.w",dreg[reg1],dreg[reg2]);
}

void gen_sub32dd(unsigned char reg1, unsigned char reg2) {
  gen("sub.l",dreg[reg1],dreg[reg2]);
}

void gen_add32a_val(long val, unsigned char reg) {
  char buf[16];
  sprintf(buf,"#%ld",val);
  gen("adda.l",buf,areg[reg]);
}


void gen_add32d_val(long val, unsigned char reg) {
  char buf[16];
  sprintf(buf,"#%ld",val);
  if (val < 8) gen("addq",buf,dreg[reg]);
  else gen("add.l",buf,dreg[reg]);
}

void gen_add16d_val(long val, unsigned char reg) {
  char buf[16];
  sprintf(buf,"#%ld",val);
  gen("add.w",buf,dreg[reg]);
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
  char buf[16];
  sprintf(buf,"#%d",val);
  gen("move.w",buf,"-(sp)");
}

void gen_pop_ignore(unsigned int bytes)
{
  char buf[16];
  sprintf(buf,"#%d",(int)bytes);
  if (bytes > 7) {
	gen("add.l",buf,"sp");
  } else {
	gen("addq",buf,"sp");
  }
}

void gen_pop32d(unsigned char reg)
{
  gen("move.l","(sp)+",dreg[reg]);
}

/* Pop 4 bytes off the stack and copy into a variable. */
void gen_pop32_var(const char * label)
{
  gen("move.l","(sp)+",label);
}

void gen_pop16_var(const char * label)
{
  gen("move.w","(sp)+",label);
}

void gen_push16d(unsigned char reg)
{
  gen("move.w",dreg[reg],"-(sp)");
}

void gen_push16_var(const char *label)
{
  gen("move.w",label,"-(sp)");
}

void gen_push32_var(const char *label)
{
  gen("move.l",label,"-(sp)");
}

void gen_pop16d(unsigned char reg)
{
  gen("move.w","(sp)+",dreg[reg]);
}

void gen_push_addr(unsigned char reg)
{
  gen("move.l",areg[reg],"-(sp)");
}

void gen_load_addr(const char * label,unsigned char reg)
{
  gen("lea",label,areg[reg]);
}

void gen_pop_addr(unsigned char reg)
{
  gen("move.l","(sp)+",areg[reg]);
}

void gen_load32a(const char * label, unsigned char reg)
{
  gen("move.l",label,areg[reg]);
}

void gen_load32d(const char * label, unsigned char reg)
{
  gen("move.l",label,dreg[reg]);
}

void gen_load16d(const char * label, unsigned char reg)
{
  gen("move.w",label,dreg[reg]);
}

void gen_load8d(const char * label, unsigned char reg)
{
  gen("move.b",label,dreg[reg]);
}


void gen_save8d(unsigned char reg, const char * label)
{
  gen("move.w",dreg[reg],label);
}

void gen_save16d(unsigned char reg, const char * label)
{
  gen("move.w",dreg[reg],label);
}

void gen_save32d(unsigned char reg, const char * label)
{
  gen("move.l",dreg[reg],label);
}

void gen_save32a(unsigned char reg, const char * label)
{
  gen("move.l",areg[reg],label);
}

void gen_save32ad(unsigned char reg, unsigned char dr)
{
  gen("move.l",areg[reg],dreg[dr]);
}

void gen_move16dd(unsigned char srcreg, unsigned char destreg)
{
  gen("move.w",dreg[srcreg],dreg[destreg]);
}

void gen_move32dd(unsigned char srcreg, unsigned char destreg)
{
  gen("move.l",dreg[srcreg],dreg[destreg]);
}

void gen_move32aa(unsigned char srcreg, unsigned char destreg)
{
  gen("move.l",areg[srcreg],areg[destreg]);
}

void gen_move32da(unsigned char srcreg, unsigned char destreg)
{
  gen("movea.l",dreg[srcreg],areg[destreg]);
}

void gen_add32da(unsigned char srcreg, unsigned char destreg)
{
  gen("adda.l",dreg[srcreg],areg[destreg]);
}

/***** "Mid level" code generation functions *****/

void gen_libbase(const char * base)
{
  char buf[200];
  strcpy(buf,"_");
  strncat(buf,base,sizeof(buf)-1);
  strncat(buf,"Base",sizeof(buf)-strlen(buf));
  gen_load32a(buf,6);
  enter_XREF(buf);
}

void gen_libcall(const char * lvo, const char * base)
{
  char buf[200];
  strcpy(buf,"_LVO");
  strncat(buf,lvo,sizeof(buf)-4);
  enter_XREF(buf);
  strncat(buf,"(a6)",sizeof(buf)-strlen(buf));
  gen("jsr",buf,"  ");
}

void gen_gfxcall(const char * lvo)
{
  gen_load32a("_RPort",1);
  enter_XREF("_RPort");
  gen_libbase("Gfx");
  gen_libcall(lvo,"Gfx");
}

/* Pop an argument off the statck.
 * d[0-7] => data registers
 * a[0-7] => address registers
 * t[0-7] => Create a temporary string, load into address register
 */
static char * gen_pop_arg(char * args)
{
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

static char * gen_push_ret(char * args)
{
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

/* Note: Arguments are listed in the order to be popped off the stack,
 * which is generally the opposite of the normal order */
void gen_call_args(const char * label, const char * args, unsigned int stack)
{
  while (*args && args[0] != ':') {
	args = gen_pop_arg(args);
  }
  gen_jsr(label);
  if (stack > 0) gen_pop_ignore(stack);
  gen_push_ret(args);
}

void gen_call_indirect(const char * addr) {
  gen("move.l",addr,"a0");
  gen("jsr","(a0)","  ");
}
