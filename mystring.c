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
    int i = INITIAL_INDEX;
    int is_negative = FALSE;

    if (n == ZERO_VALUE) {
        buf[INITIAL_INDEX] = ZERO_CHAR;
        buf[INITIAL_INDEX + 1] = NULL_CHAR;
        return;
    }

    if (n < ZERO_VALUE) {
        is_negative = TRUE;
        n = -n;
    }

    while (n > ZERO_VALUE) {
        buf[i++] = (n % DECIMAL_BASE) + ZERO_CHAR;
        n /= DECIMAL_BASE;
    }

    if (is_negative)
        buf[i++] = NEGATIVE_SIGN;

    buf[i] = NULL_CHAR;

    // Reverse string in place
    for (int j = 0; j < i / HALF; j++) {
        char temp = buf[j];
        buf[j] = buf[i - j - 1];
        buf[i - j - 1] = temp;
    }
}
