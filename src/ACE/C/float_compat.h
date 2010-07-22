#ifndef AMIGA

#include <math.h>
#define SPAdd(x,y) (x+y)
#define SPSub(x,y) (x-y)
#define SPFloor floor
#define SPCeil ceil
#define SPMul(x,y) (x*y)
#define SPDiv(x,y) (x/y)
#define SPPow pow

extern float SPFlt(int inum);
extern int SPFix(float fnum);
extern int SPCmp(float fum1, float fnum2);
#endif
