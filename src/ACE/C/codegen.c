
#include "acedef.h"
#include "codegen.h"

extern BOOL   early_exit;
extern CODE * new_code, * curr_code;

void kill_all_lists();

/* FIXME: Make static */
void gen(const char * opcode,const char * srcopr,const char *destopr)
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

/******* Public interface ***************/

/** Low level
 *
 *  These should ideally mostly be "packaged up" into higher level
 *  code generation functions. Just translating the lower level 
 *  functions to a new architecture would likely result in inefficient
 *  code.
 *
 **/

void gen_label(const char * label)
{
  gen(label,"  ","  ");
}

void gen_bne(const char * label)
{
  gen("bne.s",label,"  ");
}

void gen_beq(const char * label)
{
  gen("beq.s",label,"  ");
}

void gen_bgt(const char * label)
{
  gen("bgt.s",label,"  ");
}

void gen_blt(const char * label)
{
  gen("blt.s",label,"  ");
}


/* Used to create ops to possibly cange later */
void gen_nop()
{
  gen("nop","  ","  ");
}

void gen_jsr(const char * label)
{
  gen("jsr",label,"  ");
}

void gen_jmp(const char * label)
{
  gen("jmp",label,"  ");
}

void gen_tst32d(unsigned char reg)
{
  /* FIXME: Isn't tst.l d0 faster? Keeping for now to
   * allow using diff against "old" ace as a regression test
   */
  gen("cmpi.l","#0",dreg[reg]);
}

void gen_tst16d(unsigned char reg)
{
  /* FIXME: Isn't tst.w d0 faster? Keeping for now to
   * allow using diff against "old" ace as a regression test
   */
  gen("cmpi.w","#0",dreg[reg]);
}

void gen_push32d(unsigned char reg)
{
  gen("move.l",dreg[reg],"-(sp)");
}

void gen_push32_val(long val)
{
  char buf[16];
  sprintf(buf,"#%ld",val);
  gen("move.l",buf,"-(sp)");
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

/***** "Mid level" code generation functions *****/

void gen_gfxcall(const char * lvo)
{
  char buf[200];
  strcpy(buf,"_LVO");
  strncat(buf,lvo,sizeof(buf)-4);
  enter_XREF(buf);
  strncat(buf,"(a6)",sizeof(buf)-strlen(buf));
  gen_load32a("_RPort",1);
  gen_load32a("_GfxBase",6);
  enter_XREF("_GfxBase");
  enter_XREF("_RPort");
  gen_jsr(buf);
}
