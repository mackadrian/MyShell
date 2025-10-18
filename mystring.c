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
    int len = ZERO_VALUE;
    while (s[len] != NULL_CHAR)
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
    *ptr = NULL_CHAR;
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

    while (*ptr != NULL_CHAR)
    {
        ptr++;
    }

    while (*src != NULL_CHAR)
    {
        *ptr = *src;
        ptr++;
        src++;
    }

    *ptr = NULL_CHAR;

    return dest;
}

/* ---
Function Name: myitoa

Purpose:
  Converts an integer value into its string representation.

Input:
  n   - integer number to convert
  buf - character buffer to hold resulting string (must be large enough)

Output:
  Populates buf with the integer as a string and null terminator.
--- */
void myitoa(int n, char *buf)
{
    int i = 0;
    int is_negative = 0;

    if (n == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    if (n < 0) {
        is_negative = 1;
        n = -n;
    }

    while (n > 0) {
        buf[i++] = (n % 10) + '0';
        n /= 10;
    }

    if (is_negative)
        buf[i++] = '-';

    buf[i] = '\0';

    // Reverse string in place
    for (int j = 0; j < i / 2; j++) {
        char temp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = temp;
    }
}
