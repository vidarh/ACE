
/* Functions used by ACE that are NOT ANSI C compatible */

char * itoa(int n, char s[], int bufsize);
char * strupr(char * a); 
long fsize(const char *filename);

#define ltoa itoa

