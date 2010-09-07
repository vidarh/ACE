#ifndef CODEGEN_H
#define CODEGEN_H

/* A "vtable" for the code generators, to present a CPU independent interface.
   Once complete, retargeting to a new CPU should be a matter of copying an
   existing codegen_target structure that most closely resembles the new CPU,
   and replace the appropriate function pointers.
*/
struct codegen_target {
  /* Simple, low level functions that are reasonably common on many
	 architectures, in alphabetic order */

  void (* cmp)(int,int);
  void (* eor)(int);
  int (* muls)(int);
  void (* neg)(int type);
  void (* or)(int);
  void (* rts)();
};

struct codegen_target * target;

void codegen_set_target(struct codegen_target * t);

/* Current targets */
struct codegen_target m68k_target;

/* Low level code generation functions */

static inline void gen_rts() { target->rts(); }
static inline void gen_eor(int type) { target->eor(type); }
static inline void gen_or(int type) { target->or(type); }
static inline void gen_neg(int type) { target->neg(type); }
static inline int gen_muls(int type) { return target->muls(type); }
static inline void gen_cmp(int type,int op) { target->cmp(type,op); }

void gen_link();
void gen_unlk();
void gen_move32(const char * src, const char * dest);
void gen_incr_indirect16();
void gen_incr_indirect32();
void gen_incr_indirect_float();
void gen_decr_indirect16();
void gen_decr_indirect32();
void gen_jsr(const char * label);
void gen_push32d(unsigned char reg);
void gen_push32_val(long val);
void gen_pop32d(BYTE reg);
void gen_push16d(unsigned char reg);
void gen_pop16d(unsigned char reg);
void gen_push_addr(unsigned char reg);
void gen_load_addr(const char * label,unsigned char reg);
void gen_pop_addr(unsigned char reg);
void gen_pop_ignore(unsigned int bytes);
void gen_pop32_var(const char * label);
void gen_pop16_var(const char * label);
void gen_push32_var(const char * label);
void gen_push16_var(const char * label);
void gen_nop();
void gen_label(const char * label);
void gen_bne(const char * label);
void gen_beq(const char * label);
void gen_blt(const char * label);
void gen_bgt(const char * label);
void gen_bge(const char * label);
void gen_jmp(const char * label);
void gen_tst32d(BYTE reg);
void gen_tst16d(BYTE reg);
void gen_load32a(const char * label, unsigned char reg);
void gen_load32d(const char * label, unsigned char reg);
void gen_load16d(const char * label, unsigned char reg);
void gen_move16dd(unsigned char srcreg, unsigned char destreg);
void gen_move32dd(unsigned char srcreg, unsigned char destreg);
void gen_move32aa(unsigned char srcreg, unsigned char destreg);
void gen_save32a(unsigned char reg, const char * label);
void gen_save32d(unsigned char reg, const char * label);
void gen_load32d_val(signed long val, unsigned char reg);
void gen_save32a(unsigned char reg, const char * label);
void gen_not16d(unsigned char reg);
void gen_neg16d(unsigned char r);
void gen_pea(const char * target);
void gen_push16_val(int val);
void gen_swap(int typ);

/* Mid level */

int gen_fcall(const char * funcname, int type, const char * params, int ret, const char * callargs, int stackadj);
void gen_libcall(const char * lvo, const char * base);
void gen_gfxcall(const char * lvo);
void gen_call_args(const char * label, const char * args, unsigned int stack);
static inline void gen_call_void(const char * label, unsigned int stack) { gen_call_args(label,"",stack); }
static inline void gen_call(const char * label, unsigned int stack) { gen_call_args(label,":d0",stack); }
#endif
