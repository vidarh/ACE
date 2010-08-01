#ifndef CODEGEN_H
#define CODEGEN_H

/* Low level code generation functions */

void gen_jsr(const char * label);
void gen_push32d(unsigned char reg);
void gen_push32_val(long val);
void gen_pop32d(unsigned char reg);
void gen_push16d(unsigned char reg);
void gen_pop16d(unsigned char reg);
void gen_push_addr(unsigned char reg);
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
void gen_tst32d(unsigned char reg);
void gen_tst16d(unsigned char reg);
void gen_load32a(const char * label, unsigned char reg);
void gen_load32d(const char * label, unsigned char reg);

void gen_gfxcall(const char * lvo);

#endif
