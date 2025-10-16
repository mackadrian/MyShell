#include "mystring.h"

/* ---
Function Name: mystrlen

Purpose: 
  Returns the length of a string excluding the null terminator.

Input:
  s - pointer to a null-terminated string
Output:
  len - number of characters in the string
--- */
unsigned int mystrlen(const char *s)
{
  int len = 0;
  while (s[len] != '\0')
    {
    len++;
    }
  return len;
}

/* ---
Function Name: mystrcmp

Purpose:
  Compares two strings character by character

Input:
  s1 - pointer to the first string
  s2 - pointer to the second string
Output:
  str_compare - 
    < 0 if s1 is less than s2
    = 0 if s1 is equal to s2
    > 0 if s1 is greater than s2
--- */
int mystrcmp(const char *s1, const char *s2)
{
  while (*s1 && (*s1 == *s2))
    {
      s1++;
      s2++;
    }

  int str_compare = (unsigned char)*s1 - (unsigned char)*s2;
  
  return str_compare;
}

/* ---
Function Name: mystrcpy

Purpose: 
  Copies the string from source into destination.

Input:
  dest - pointer to destination buffer
  src - pointer to source string
Output:
  dest - pointer to the destination string
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

/* ---
Function Name: mystrcat

Purpose: 
  Appends the source string 'src' to the end of the destination 'dest'.

Input:
  dest - pointer to destination buffer
  src - pointer to source string
Output:
  dest - pointer to the destination buffer containing the concatenated string
--- */
char *mystrcat(char *dest, const char *src)
{
    char *ptr = dest;

    while (*ptr != '\0') 
    {
        ptr++;
    }

    while (*src != '\0') 
    {
        *ptr = *src;
        ptr++;
        src++;
    }

    *ptr = '\0';

    return dest;
}
