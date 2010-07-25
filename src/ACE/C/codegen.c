
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


/******* Public interface ***************/

void gen_jsr(const char * label)
{
  gen("jsr",label,"  ");
}
