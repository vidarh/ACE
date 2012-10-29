
#include "codegen.h"

/* FIXME: Need proper register allocation... */
static char * x86reg[] = {"%eax","%ebx","%ecx","FIXMEREG1","FIXMEREG2","%edx","FIXMEREG3","%edx"};

void x86_push32d(unsigned char reg) { gen("push",x86reg[reg],"  "); }
void x86_pop32d(unsigned char reg)  { gen("popl",x86reg[reg],"  "); }

void x86_pea(const char * label) { 
  char buf[100];
  strcpy(buf,"$");
  strncat(buf,label,sizeof(buf));
  buf[sizeof(buf)-1] = 0;
  gen("pushl",buf,"  "); 
}

void i386_Flt(int typ) {
    gen("fildl","(%esp)","  ");
}

void x86_call(const char * label) { 
  gen("call",label,"  "); enter_XREF(label);
  cur_libbase[0] = 0;
}

void x86_startup(FILE * dest) {
  fprintf(stderr,"; FIXME: x86 startup code\n");
}

void x86_and(int x) {
  gen_comment("AND not implemented");
}

void x86_decr_indir(int x) {
  gen_comment("decr_indir not implemented");
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

void i386_call_args(const char * label, const char * args, unsigned int stack) {
  gen_jsr(label);
  if (stack > 0) gen_pop_ignore(stack);
}

void x86_push32_val(long val) {
  char buf[16];
  sprintf(buf,"#$%lx",val);
  gen("push",buf,"  ");
}


/* FIXME Not up to date - check x86_64-linux */
struct codegen_target i386_aros_target = {
    nop,
    x86_cmp,
    nop,
    x86_eor,
    nop,
    x86_call,
    nop,
    x86_muls,
    x86_neg,
    nop,
    x86_or,
    x86_pea,
    x86_push32_val, // push16
    x86_push32d,
    x86_push32_val,
    generic_ret,
    nop,

    i386_call_args,
    i386_Flt,
  
    x86_code_section,
    nop,
    x86_startup,
    
    0 /* write_xrefs? */
};


struct codegen_target x86_64_linux_target = {
    nop,
    x86_cmp,
    nop,
    x86_eor,
    nop,
    x86_call,
    nop,
    x86_muls,
    x86_neg,
    nop,
    x86_or,
    x86_pea,
    x86_pop32d,
    x86_push32_val,
    x86_push32d,
    x86_push32_val,
    generic_ret,
    nop,

    nop, /* call_args */
    nop, /* Flt */

    x86_code_section,
    nop,
    x86_startup,
    
    0 /* write_xrefs? */
};

/* FIXME Not up to date - check x86_64 */
struct codegen_target i386_linux_target = {
    x86_and,
    x86_cmp,
    x86_decr_indir,
    x86_eor,
    nop,
    x86_call,
    nop,
    x86_muls,
    x86_neg,
    nop,
    x86_or,
    x86_pea,
    x86_pop32d,
    x86_push32_val,
    x86_push32d,
    x86_push32_val,
    generic_ret,
    nop,

    i386_call_args,
    i386_Flt,

    x86_code_section,
    nop,
    x86_startup,

    0 /* write_xrefs? */
};

