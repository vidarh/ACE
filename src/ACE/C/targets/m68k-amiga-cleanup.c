#include <stdio.h>
#include "acedef.h"

extern	BOOL	cli_args;
extern	BOOL 	mathffpused;
extern	BOOL 	mathtransused;
extern	BOOL 	translateused;
extern	BOOL	iffused;
extern	BOOL	ontimerused;
extern	BOOL 	dosused;
extern	BOOL 	gfxused;
extern	BOOL 	intuitionused;
extern	BOOL 	narratorused;
extern	BOOL 	basdatapresent;

void m68k_amiga_cleanup(FILE * dest)
{
   /* exiting code */
   fprintf(dest,"\n_EXIT_PROG:\n");

   fprintf(dest,"\tunlk\ta4\n");

   /* 
   ** Programs which abort should cleanup libraries, free allocated memory
   ** and possibly reply to a Wb startup message. 
   */
   if (intuitionused || gfxused || mathffpused || mathtransused ||
       translateused) 
      fprintf(dest,"_ABORT_PROG:\n");

   /* Free memory allocated via ALLOC and db.lib calls to alloc(). */
   fprintf(dest,"\tjsr\t_free_alloc\n");

   /* close libraries */
   if (gfxused) 
   {
    fprintf(dest,"\tjsr\t_closegfx\n");
    fprintf(dest,"\tjsr\t_closeintuition\n");
   }
   if (narratorused) fprintf(dest,"\tjsr\t_cleanup_async_speech\n");
   if (intuitionused && !gfxused) fprintf(dest,"\tjsr\t_closeintuition\n");
   if (mathtransused) fprintf(dest,"\tjsr\t_closemathtrans\n");
   if (mathffpused) fprintf(dest,"\tjsr\t_closemathffp\n");
   if (translateused) fprintf(dest,"\tjsr\t_closetranslator\n");

   /* delete temporary ILBM.library */
   if (iffused) fprintf(dest,"\tjsr\t_remove_ILBMLib\n");

   /* restore registers */
   fprintf(dest,"\tmovem.l\t(sp)+,d1-d7/a0-a6\n");

   /* restore initial stack pointer */
   fprintf(dest,"\tmove.l\t_initialSP,sp\n");

   /* 
   ** Close dos.library and reply to Wb message
   ** as the LAST THING DONE before rts'ing.
   */
   fprintf(dest,"\tjsr\t_cleanup\n");

   /* return */
   fprintf(dest,"\n\trts\n");
}
