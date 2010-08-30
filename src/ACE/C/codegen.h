#ifndef CODEGEN_H
#define CODEGEN_H

/* Low level code generation functions */

void gen_rts();
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

/* Mid level */

void gen_libbase(const char * base);
void gen_libcall(const char * lvo, const char * base);
void gen_gfxcall(const char * lvo);
void gen_call_args(const char * label, const char * args, unsigned int stack);
static inline void gen_call_void(const char * label, unsigned int stack) { gen_call_args(label,"",stack); }
static inline void gen_call(const char * label, unsigned int stack) { gen_call_args(label,":d0",stack); }
#endif
