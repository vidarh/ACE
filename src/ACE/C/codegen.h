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
extern struct codegen_target m68k_target;
extern struct codegen_target i386_aros_target;

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
void gen_load_indirect_addr(unsigned char ar, unsigned char dest);
void gen_load_indirect32(unsigned char ar, unsigned char dr);
void gen_save32_val(long val, const char * target);
void gen_comment(const char * c);
void gen_rport_rel_x(unsigned char r);
void gen_rport_rel_y(unsigned char r);
void gen_rport_rel_xy();
void gen_add16dd(BYTE reg1, BYTE reg2);
void gen_and32dd(BYTE reg1, BYTE reg2);
void gen_and16dd(BYTE reg1, BYTE reg2);
void gen_or32dd(unsigned char reg1, unsigned char reg2);
void gen_or16dd(unsigned char reg1, unsigned char reg2);
void gen_not16d(unsigned char reg);
void gen_neg16d(unsigned char r);
void gen_not16sp();
void gen_not32sp();
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
void gen_ext8to16(BYTE d);
void gen_ext16to32(BYTE d);
void gen_lsl(unsigned int n);
int gen_pop_as_short(int type, unsigned char reg);
void gen_decr_indirect_float();
void gen_save_indirect16(unsigned char dr, unsigned char ar);
void gen_save_indirect8(unsigned char dr, unsigned char ar);
void gen_save_indirect32(unsigned char dr, unsigned char ar);
void gen_load16d_val(int val, unsigned char reg);
void gen_load32a_val(long val, BYTE reg);
void gen_add16d_val(long val, unsigned char reg);
void gen_add32dd(BYTE reg1, BYTE reg2);
void gen_add32da(unsigned char srcreg, unsigned char destreg);
void gen_add32_val(long val, const char * label);
void gen_add32a_val(long val, unsigned char reg);
void gen_save32ad(unsigned char reg, unsigned char dr);
void gen_sub32d_val(long val, unsigned char reg);
void gen_save8d(unsigned char reg, const char * label);
void gen_load8d(const char * label, unsigned char reg);
void gen_push_indirect_indexed16();
void gen_push_indirect_indexed32();
void gen_push_indirect32(unsigned char r);
void gen_cmp32sp_val(long val);
void gen_cmp32dd(BYTE r1, BYTE r2);
void gen_add16_val(long val, const char * label);
void gen_push_indirect16(unsigned char r);
void gen_pop_indirect_indexed32(unsigned char ar, unsigned char dr);
void gen_pop_indirect_indexed16(unsigned char ar, unsigned char dr);
void gen_save_indirect_indexed16(const char * label, unsigned char ar, unsigned char dr);
void gen_cmp16_val(long val, const char * target);
void gen_cmp16dd(BYTE r1, BYTE r2);
void gen_add32d_val(long val, unsigned char reg);
void gen_save_indirect_indexed32(const char * label, unsigned char ar, unsigned char dr);
void gen_cmp32_val(long val, const char * target);
void gen_add32dd(BYTE reg1, BYTE reg2);
void gen_jmp_fwd(const char * label,const char * flag);
void gen_pop_indirect32(unsigned char r);
void gen_pop_indirect16(unsigned char r);
void gen_move16(const char * src, const char * dest);

/* Mid level */

int gen_fcall(const char * funcname, int type, const char * params, int ret, const char * callargs, int stackadj);
void gen_libcall(const char * lvo, const char * base);
void gen_gfxcall(const char * lvo);
void gen_call_args(const char * label, const char * args, unsigned int stack);
static inline void gen_call_void(const char * label, unsigned int stack) { gen_call_args(label,"",stack); }
static inline void gen_call(const char * label, unsigned int stack) { gen_call_args(label,":d0",stack); }
#endif
