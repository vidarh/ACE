
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

void gen_jsr(const char * label)
{
  gen("jsr",label,"  ");
}

void gen_push32d(unsigned char reg)
{
  gen("move.l",dreg[reg],"-(sp)");
}

void gen_pop_ignore(unsigned int bytes)
{
  char buf[16];
  sprintf(buf,"#%ld",bytes);
  gen("addq",buf,"sp");
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

void gen_push16_var(const char * label)
{
  gen("move.w",label,"-(sp)");
}

void gen_push32_var(const char * label)
{
  gen("move.l",label,"-(sp)");
}

void gen_push16d(unsigned char reg)
{
  gen("move.w",dreg[reg],"-(sp)");
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
