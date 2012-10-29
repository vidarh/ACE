
#include "codegen.h"

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

/* Note: Arguments are listed in the order to be popped off the stack,
 * which is generally the opposite of the normal order */
void m68k_call_args(const char * label, const char * args, unsigned int stack) {
  while (*args && args[0] != ':') {
	args = gen_pop_arg(args);
  }
  gen_jsr(label);
  if (stack > 0) gen_pop_ignore(stack);
  gen_push_ret(args);
}
