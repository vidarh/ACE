
#include "acedef.h"

/* WINDOW wdw-id[,title],rectangle[,type][,screen-id] 
   WINDOW CLOSE wdw-id
   WINDOW OUTPUT wdw-id 
   WINDOW ON | OFF | STOP
*/

/* FIXME: Unsure if the '!' rule is needed. */
struct ParseSequence seq_window[] = {
    {closesym,  {"il", {0}, "_CloseWdw", 4, ""}},
    {outputsym, {"il", {0}, "_ChangeOutputWdw", 4, ""}},
    {-1,        {"l!?s,r?l?l",{0,-1,0}, "_OpenWdw", 32, ""}}
};

/* 
   GADGET ON | OFF | STOP
   GADGET (WAIT|CLOSE|OUTPUT) gadget-id
   GADGET MOD gadget-id,knob-pos[,max-notches]
   GADGET gadget-id,status[,gadget-value,(x1,y1)-(x2,y2),type[,style][,font,size,txtstyle]]
*/

struct ParseSequence seq_gadget[] = {
    {closesym,  {"il",     {0}, "_CloseGadget",      4, ""}},
    {outputsym, {"il",     {0}, "_SetCurrentGadget", 4, ""}},
    {waitsym,   {"il",     {0}, "_WaitGadget",       4, ""}},
    {modsym,    {"il,l?l", {-1}, "_modify_gad",     12, ""}},
};


/* IFF OPEN #channel, filename
       READ | CLOSE #channel 
 */

struct ParseSequence seq_iff[] = {
    {opensym,  {"i#l,s", {0},  "_IFFPicOpen", 8, ""}},
    {readsym,  {"i#l?l", {-1}, "_IFFPicRead", 8, ""}},
    {closesym, {"i#l",   {0},  "_IFFPicClose",4, ""}}
};

/* SCREEN CLOSE <screen-id> */

struct ParseSequence seq_screen[] = {
    {closesym, {"iw",    {0},  "_closescreen",0, "d0.w"}}
};
