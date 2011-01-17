
#include "codegen.h"

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


struct codegen_target i386_aros_target = {
  m68k_cmp,
  m68k_eor,
  x86_call,
  m68k_muls,
  m68k_neg,
  m68k_or,
  x86_pea,
  x86_push32d,
  generic_ret,
  
  x86_code_section,
  nop,

  0 /* write_xrefs? */
};


struct codegen_target x86_64_linux_target = {
  m68k_cmp,
  m68k_eor,
  x86_call,
  m68k_muls,
  m68k_neg,
  m68k_or,
  x86_pea,
  x86_push32d,
  generic_ret,

  x86_code_section,
  nop,

  0 /* write_xrefs? */
};

