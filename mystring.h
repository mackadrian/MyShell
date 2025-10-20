#ifndef MY_STRING_H
#define MY_STRING_H

/* BOOLEAN CONSTANTS */
#define TRUE                1
#define FALSE               0

/* CHARACTER CONSTANTS */
#define NULL_CHAR           '\0'
#define NEGATIVE_SIGN       '-'
#define ZERO_CHAR           '0'

/* NUMERIC CONSTANTS */
#define ZERO_VALUE          0
#define INITIAL_INDEX       0
#define DECIMAL_BASE        10
#define HALF                2

/* FUNCTION DECLARATIONS */
unsigned int mystrlen(const char *s);
int mystrcmp(const char *s1, const char *s2);
char *mystrcpy(char *dest, const char *src);
char *mystrcat(char *dest, const char *src);
void myitoa(int n, char *buf);

#endif
