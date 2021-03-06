#ifndef  FUNCS_MISC_FUNCS_H
#define  FUNCS_MISC_FUNCS_H

/*
**
** Function declarations for ACE Basic
**
** Note: Translated to ACE by Nils Sjoholm
**
** Date: 11-Oct-95 TO 14-Oct-95
**
*/              

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif

DECLARE FUNCTION ADDRESS AllocMiscResource( LONGINT unitNum, ADDRESS _NAME ) LIBRARY misc
DECLARE FUNCTION FreeMiscResource( LONGINT unitNum ) LIBRARY misc
#endif   /* FUNCS_MISC_FUNCS_H */
