
#include "codegen.h"

/* FIXME: Need proper register allocation... */
static char * x86reg[] = {"%eax","%ebx","%ecx",0,0,"%edx",0,"%edx"};

void x86_push32d(unsigned char reg) { gen("push",x86reg[reg],"  "); }

void x86_pea(const char * label) { 
  char buf[100];
  strcpy(buf,"$");
  strncat(buf,label,sizeof(buf));
  buf[sizeof(buf)-1] = 0;
  gen("pushl",buf,"  "); 
}

void x86_call(const char * label) { 
  gen("call",label,"  "); enter_XREF(label);
  cur_libbase[0] = 0;
}

void x86_startup(FILE * dest) {
  fprintf(stderr,"; FIXME: x86 startup code\n");
}

void x86_eor(int x) {
  gen_comment("EOR not implemented");
}

void x86_cmp(int x,int y) {
  gen_comment("EOR not implemented");
}

void x86_muls(int x) {
  gen_comment("MULS not implemented");
}

void x86_neg(int x) {
  gen_comment("NEG not implemented");
}

void x86_or(int x) {
  gen_comment("OR not implemented");
}

void x86_code_section(FILE * dest)
{
  fprintf(dest,"\t.text\n\n");
}

struct codegen_target i386_aros_target = {
  x86_cmp,
  x86_eor,
  x86_call,
  x86_muls,
  x86_neg,
  x86_or,
  x86_pea,
  x86_push32d,
  generic_ret,
  
  x86_code_section,
  nop,
  x86_startup,

  0 /* write_xrefs? */
};


struct codegen_target x86_64_linux_target = {
  x86_cmp,
  x86_eor,
  x86_call,
  x86_muls,
  x86_neg,
  x86_or,
  x86_pea,
  x86_push32d,
  generic_ret,

  x86_code_section,
  nop,
  x86_startup,

  0 /* write_xrefs? */
};

