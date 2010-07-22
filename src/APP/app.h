
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef AMIGA

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>

#else

#include <inttypes.h>
#include <stdint.h>
typedef uint8_t BOOL;
typedef uint8_t UBYTE;
typedef int8_t BYTE;
typedef int16_t SHORT;
typedef int32_t LONG;
typedef uint32_t ULONG;
#define TRUE 1
#define FALSE 0
#define MEMF_ANY 0

struct Remember {
  struct Remember * next;
  void * mem;
};

#endif

