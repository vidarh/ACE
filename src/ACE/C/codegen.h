#ifndef CODEGEN_H
#define CODEGEN_H

/* Low level code generation functions */

void gen_jsr(const char * label);
void gen_push32d(unsigned char reg);
void gen_pop32d(unsigned char reg);

#endif
