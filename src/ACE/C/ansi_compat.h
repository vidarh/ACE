
/* Functions used by ACE that are NOT ANSI C compatible */

char * itoa(int n, char s[], int bufsize);
char * strupr(char * a); 

#define ltoa itoa

