#ifndef CODEGEN_H
#define CODEGEN_H

/* Low level code generation functions */

void gen_jsr(const char * label);
void gen_push32d(unsigned char reg);
void gen_pop32d(unsigned char reg);
void gen_push16d(unsigned char reg);
void gen_pop16d(unsigned char reg);
void gen_push_addr(unsigned char reg);
void gen_pop_addr(unsigned char reg);
void gen_pop_ignore(unsigned int bytes);
void gen_pop32_var(const char * label)
void gen_pop16_var(const char * label)
#endif
