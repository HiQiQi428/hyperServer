
#include <stdlib.h>
#include "util.h"

void u_strcpy (char *src, int sStart, char *dest, int dStart, int len) {
    int i;
    for (i = 0; i < len; i++)
        dest[i + dStart] = src[i + sStart];
    return ;
}

// make a copy from source
char* u_strcpy_n (char *src, int start, int len) {
    char *ret = (char*) malloc (sizeof (char) * (len + 1));
    u_strcpy (src, start, ret, 0, len);
    ret[len] = 0;
    return ret;
}

// copy to dest from source which ends with 0
int u_strcpy_0 (char *src, int sStart, char *dest, int dStart) {
    int i;
    for (i = 0; src[i + sStart] != 0; i++)
        dest[i + dStart] = src[i + sStart];
    return i;
}

// make copy from source which ends with 0
char* u_strcpy_n0 (char *src) {
    int len, i;
    for (len = 0; src[len] != 0; len++);
    char *ret = (char*) malloc (sizeof (char) * (len + 1));
    for (i = 0; i < len; i++)
        ret[i] = src[i];
    ret[len] = 0;
    return ret;
}

char* u_trim (char *src, int start, int len) {
    char *ret;
    int s = start, e = start + len - 1;
    while (src[s] == ' ') s++;
    while (src[e] == ' ') e--;
    return u_strcpy_n (src, s, e - s + 1);
}

int* u_intcpy (int src) {
    int *ret = (int*) malloc (sizeof (int));
    *ret = src;
    return ret;
}