#ifdef ANSI

#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
 
/* reverse:  reverse string s in place */
static void reverse(char s[])
{
  int i, j;
  char c;
 
  for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
	c = s[i];
	s[i] = s[j];
	s[j] = c;
  }
}

/* itoa:  convert n to characters in s */
char * itoa(int n, char s[], int bufsize)
{
  int i, sign;
 
  if ((sign = n) < 0)  /* record sign */
	n = -n;          /* make n positive */
  i = 0;
  do {       /* generate digits in reverse order */
	s[i++] = n % 10 + '0';   /* get next digit */
  } while ((n /= 10) > 0 && i < bufsize);     /* delete it */
  if (sign < 0)
	s[i++] = '-';
  s[i] = '\0';
  reverse(s);
  return s;
}

char * strupr (char * a)
{
  char *ret = a;

  while (*a != '\0')
    {
      if (islower (*a))
		*a = toupper (*a);
      ++a;
    }

  return ret;
}



long fsize(const char *filename) {
  struct stat st; 

  if (stat(filename, &st) == 0)
	return st.st_size;

  return -1; 
}

#endif
