#ifndef MY_STRING_H
#define MY_STRING_H

#define NULL_CHAR       '\0'
#define ZERO_VALUE      0

unsigned int mystrlen(const char *s);
int mystrcmp(const char *s1, const char *s2);
char *mystrcpy(char *dest, const char *src);
char *mystrcat(char *dest, const char *src);
void myitoa(int n, char *buf);

#endif
