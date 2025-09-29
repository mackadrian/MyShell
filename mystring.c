#include "mystring.h"


unsigned int mystrlen(const char *s)
{
  int length = 0;
  while (s[len] != '\0')
    {
    len++;
    }
  return len;
}


int mystrcmp(const char *s1, const char *s2)
{
  while (*s1 && (*s1 == *s2))
    {
      s1++;
      s2++;
    }
  return (unsigned char)*s1 - (unsigned char)*s2;
}


char *mystrcpy(char *dest, const char *src)
{
  char *ptr = dest;
  while (*src)
    {
      *ptr++ = *src++;
    }
  *ptr = '\0';
  return dest;
}
