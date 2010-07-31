
/* Functions used by ACE that are NOT ANSI C compatible */

char * itoa(int n, char s[], int bufsize);
char * strupr(char * a); 
long fsize(const char *filename);

#define ltoa itoa

/* In ANSI99 but not ANSI89 */
double round(double);
