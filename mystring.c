#include "mystring.h"

/* ---
Function Name: mystrlen

Purpose: 
  Returns the length of a string

Input:
  s - string

Output:
  len - 
--- */
unsigned int mystrlen(const char *s)
{
  int length = 0;
  while (s[len] != '\0')
    {
    len++;
    }
  return len;
}

/* ---
Function Name: mystrcmp

Purpose:
  Compares two strings

Input:
  s1 - string

Output:
  str_compare - returns
--- */
int mystrcmp(const char *s1, const char *s2)
{
  while (*s1 && (*s1 == *s2))
    {
      s1++;
      s2++;
    }

  int str_compare = (unsigned char)*s1 - (unsigned char)*s2;
  
  return str_compare
}

/* ---
Function Name: mystrcpy

Purpose: 
  Copies the string from source into destination.

Input:
  dest - 
  src - 

Output:
  dest - destination is returned
--- */
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
