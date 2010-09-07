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

extern	int  	addr[2]; 
extern	int	lev;

void m68k_amiga_startup(FILE * dest)
{
  char bytes[40],buf[40];

   /* 
   ** Check for Wb start BEFORE DOING ANYTHING ELSE! 
   ** This also always opens dos.library and stores 
   ** CLI argument data. 
   */
   fprintf(dest,"\tjsr\t_startup\n");
   fprintf(dest,"\tcmpi.b\t#1,_starterr\n"); /* see _startup in startup.lib */
   fprintf(dest,"\tbne.s\t_START_PROG\n");
   fprintf(dest,"\trts\n");
   fprintf(dest,"_START_PROG:\n");

   /* storage for initial stack pointer */
   enter_BSS("_initialSP:","ds.l 1");
   fprintf(dest,"\tmove.l\tsp,_initialSP\n"); /* save task's stack pointer */

   fprintf(dest,"\tmovem.l\td1-d7/a0-a6,-(sp)\n"); /* save initial registers */

   if (cli_args) 
      fprintf(dest,"\tjsr\t_parse_cli_args\n"); /* get CLI arguments */

   if (translateused) 
   {
    fprintf(dest,"\tjsr\t_opentranslator\n");
    fprintf(dest,"\tcmpi.b\t#1,_starterr\n");
    fprintf(dest,"\tbne.s\t_translate_ok\n");
    fprintf(dest,"\tjmp\t_ABORT_PROG\n");
    fprintf(dest,"_translate_ok:\n");
   }   

   if (mathffpused) 
   {
    fprintf(dest,"\tjsr\t_openmathffp\n");
    fprintf(dest,"\tcmpi.b\t#1,_starterr\n");
    fprintf(dest,"\tbne.s\t_mathffp_ok\n");
    fprintf(dest,"\tjmp\t_ABORT_PROG\n");
    fprintf(dest,"_mathffp_ok:\n");
   }   

   if (mathtransused) 
   {
    fprintf(dest,"\tjsr\t_openmathtrans\n"); 
    fprintf(dest,"\tcmpi.b\t#1,_starterr\n");
    fprintf(dest,"\tbne.s\t_mathtrans_ok\n");
    fprintf(dest,"\tjmp\t_ABORT_PROG\n");
    fprintf(dest,"_mathtrans_ok:\n");
   }   

   if (intuitionused && !gfxused) 
   {
    fprintf(dest,"\tjsr\t_openintuition\n");   
    fprintf(dest,"\tcmpi.b\t#1,_starterr\n");
    fprintf(dest,"\tbne.s\t_intuition_ok\n");
    fprintf(dest,"\tjmp\t_ABORT_PROG\n");
    fprintf(dest,"_intuition_ok:\n");
   }   

   if (gfxused)
   {
    /* open intuition.library */
    fprintf(dest,"\tjsr\t_openintuition\n");    
    fprintf(dest,"\tcmpi.b\t#1,_starterr\n");
    fprintf(dest,"\tbne.s\t_intuition_ok\n");
    fprintf(dest,"\tjmp\t_ABORT_PROG\n");
    fprintf(dest,"_intuition_ok:\n");

    /* open graphics.library */
    fprintf(dest,"\tjsr\t_opengfx\n");  
    fprintf(dest,"\tcmpi.b\t#1,_starterr\n");
    fprintf(dest,"\tbne.s\t_gfx_ok\n");
    fprintf(dest,"\tjmp\t_ABORT_PROG\n");
    fprintf(dest,"_gfx_ok:\n");
   }   

   /* create temporary ILBM.library */
   if (iffused) fprintf(dest,"\tjsr\t_create_ILBMLib\n");

   /* get timer event trapping start time */
   if (ontimerused) fprintf(dest,"\tjsr\t_ontimerstart\n");

   /* size of stack frame */ 
   if (addr[lev] == 0)
      strcpy(bytes,"#\0");
   else
      strcpy(bytes,"#-");     
   itoa(addr[lev],buf,10);   
   strcat(bytes,buf);     
 
   /* create stack frame */
   fprintf(dest,"\tlink\ta4,%s\n\n",bytes); 

   /* initialise global DATA pointer */
   if (basdatapresent) fprintf(dest,"\tmove.l\t#_BASICdata,_dataptr\n");
}
