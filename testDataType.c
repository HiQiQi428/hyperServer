
#include <string.h>
#include <stdio.h>
#include "dataType.h"

void testBufList () {
    printf ("[Test BufList]\n");
    BufList *bufList = createBufList (20);
    char *buf = addBuf (bufList);
    memcpy (buf, "Hello block 1      \n", 20);
    buf = addBuf (bufList);
    memcpy (buf, "Hello block 2      \n", 20);
    buf = addBuf (bufList);
    memcpy (buf, "Hello block 3      \n", 20);
    BufBlockPos start, end;
    setBufBlockPos (&start, 0, 3);
    setBufBlockPos (&end, 2, 14);
    char *data = substractData (bufList, start, end);
    printf ("%s\n", data);
    destroyBufList (bufList);
}

void testStringMap () {
    printf ("[Test StringMap]\n");
    StringMap *map = createStringMap ();
    addPair (map, "name", "luncert");
    printf ("name: %s\n", getValue (map, "name"));
    addPair (map, "unknown", "13");
    printf ("unknown: %s\n", getValue (map, "unknown"));
    addPair (map, "age", "10");
    printf ("age: %s\n", getValue (map, "age"));
    removePair (map, "unknown");
    printf ("size: %d name: %s age: %s\n", map->size, getValue (map, "name"), getValue (map, "age"));
    destroyStringMap (map, FALSE);
}

int main () {
    testBufList ();
    testStringMap ();
    return 0;
}