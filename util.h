
#ifndef UTIL
#define UTIL

void u_strcpy (char *src, int sStart, char *dest, int dStart, int len);
char* u_strcpy_n (char *src, int start, int len);
int u_strcpy_0 (char *src, int sStart, char *dest, int dStart);
char* u_strcpy_n0 (char *src);
char* u_trim (char *src, int start, int len);

int* u_intcpy (int src);

#endif