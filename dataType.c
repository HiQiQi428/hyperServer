
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "util.h"
#include "dataType.h"

/* StringMap */

StringMap* createStringMap () {
    StringMap* map = (StringMap*) malloc (sizeof (StringMap));
    bzero (map, sizeof (StringMap));
    return map; 
}

void resolvePair (StringMap *map, char *raw, int len, char splitCharacter) {
    int i;
    char *key, *value;
    // resolve key
    for (i = 0; raw[i] != splitCharacter; i++);
    key = u_trim (raw, 0, i);
    i++;
    // resolve value
    value = u_trim (raw, i, len - i);
    // add to map
    addPair (map, key, value);
}

void resolveMultiPairs (StringMap *map, char *raw, int len, char splitCharacter, char pairSplitCharacter) {
    int start = 0, end = 0;
    char *key, *value;
    if (raw[len - 1] != pairSplitCharacter) {
        char *tmp = (char*) malloc (sizeof (char) * (len + 1));
        u_strcpy (raw, 0, tmp, 0, len);
        tmp[len++] = pairSplitCharacter;
        raw = tmp;
    }
    for (; end < len; end++) {
        if (raw[end] == splitCharacter) {
            key = u_trim (raw, start, end - start);
            start = end + 1;
        }
        else if (raw[end] == pairSplitCharacter) {
            value = u_trim (raw, start, end - start);
            start = end + 1;
            // addPair
            addPair (map, key, value);
        }
    }
    // no need to free key and value
}

void addPair (StringMap *map, char *key, char *value) {
    StringPair *newPair = (StringPair*) malloc (sizeof (StringPair));
    newPair->key = key;
    newPair->value = value;
    newPair->next = NULL;
    if (map->head == NULL)
        map->head = newPair;
    else {
        newPair->next = map->head;
        map->head = newPair;
    }
    map->size++;
}

char* getValue (StringMap *map, char *key) {
    StringPair *pair = map->head;
    while (pair != NULL) {
        if (strcmp (pair->key, key) == 0)
            return pair->value;
        else
            pair = pair->next;
    }
    return NULL;
}

void removePair (StringMap *map, char *key) {
    StringPair *pair = map->head, *pre = NULL;
    while (pair != NULL) {
        if (strcmp (pair->key, key) == 0) {
            if (pre != NULL)
                pre->next = pair->next;
            else
                map->head = pair->next;
            free (pair);
            map->size--;
            break;
        }
        else {
            pre = pair;
            pair = pair->next;
        }
    }
}

void outputMap (StringMap *map) {
    if (map != NULL) {
        StringPair *pair = map->head;
        printf ("StringMap size=%d {\n", map->size);
        while (pair != NULL) {
            printf ("  key: %s, value: %s\n", pair->key, pair->value);
            pair = pair->next;
        }
        printf ("}\n");
    }
}

void destroyStringMap (StringMap *map, int freeCompletely) {
    if (map != NULL) {
        StringPair *pair = map->head, *next = NULL;
        while (pair != NULL) {
            next = pair->next;
            if (freeCompletely) {
                if (pair->key != NULL)
                    free (pair->key);
                if (pair->value != NULL)
                    free (pair->value);
            }
            free (pair);
            pair = next;
        }
        free (map);
    }
}

/* BufList */

BufList* createBufList (int blockSize) {
    BufList *bufList = (BufList*) malloc (sizeof (BufList));
    bzero (bufList, sizeof (BufList));
    bufList->blockSize = blockSize;
    return bufList;
}

char* addBuf (BufList *bufList) {
    BufBlock *newBlock = (BufBlock*) malloc (sizeof (BufBlock));
    bzero (newBlock, sizeof (BufBlock));
    newBlock->buf = (char*) malloc (sizeof (char) * bufList->blockSize);
    bzero (newBlock->buf, sizeof (char) * bufList->blockSize);
    if (bufList->head == NULL)
        bufList->tail = bufList->head = newBlock;
    else {
        bufList->tail->next = newBlock;
        bufList->tail = newBlock;
    }
    bufList->size++;
    return bufList->tail->buf;
}

char* getBuf (BufList *bufList, int index) {
    if (index > -1 && index < bufList->size) {
        int i;
        BufBlock *block = bufList->head;
        for (i = 0; i < index; i++)
            block = block->next;
        return block->buf;
    }
    else return NULL;
}

void removeBuf (BufList *bufList, int index) {
    if (index > -1 && index < bufList->size) {
        int i;
        BufBlock *block = bufList->head, *preBlock = NULL;
        for (i = 0; i < index; i++) {
            preBlock = block;
            block = block->next;
        }
        if (preBlock != NULL)
            preBlock->next = block->next;
        else
            bufList->head = block->next;
        free (block->buf);
        free (block);
        bufList->size--;
    }
}

void destroyBufList (BufList *bufList) {
    if (bufList != NULL) {
        BufBlock *block = bufList->head, *next = NULL;
        while (block != NULL) {
            next = block->next;
            if (block->buf != NULL)
                free (block->buf);
            free (block);
            block = next;
        }
        free (bufList);
    }
}

void setBufBlockPos (BufBlockPos *pos, int blockIndex, int cursor) {
    pos->blockIndex = blockIndex;
    pos->cursor = cursor;
}

int compareBufBlockPos (BufBlockPos a, BufBlockPos b) {
    if (a.blockIndex == b.blockIndex) {
        if (a.cursor == b.cursor)
            return 0;
        else return (a.cursor > b.cursor) ? 1 : -1;
    }
    else return (a.blockIndex > b.blockIndex) ? 1 : -1;
}


char* substractData (BufList *bufList, BufBlockPos start, BufBlockPos end) {
    char *ret, *buf = getBuf (bufList, start.blockIndex);
    if (start.blockIndex == end.blockIndex)
        ret = u_strcpy_n (buf, start.cursor, end.cursor - start.cursor);
    else {
        int n = 0, i, j;
        // calculate size
        int size = (bufList->blockSize - start.cursor) + (end.blockIndex - start.blockIndex - 1) * bufList->blockSize + end.cursor;
        ret = (char*) malloc (sizeof(char) * size);
        // read first block
        u_strcpy (buf, start.cursor, ret, n, bufList->blockSize - start.cursor);
        n += bufList->blockSize - start.cursor;
        // read middle block
        for (i = start.blockIndex + 1; i < end.blockIndex - start.blockIndex; i++) {
            buf = getBuf (bufList, i);
            u_strcpy (buf, 0, ret, n, bufList->blockSize);
            n += bufList->blockSize;
        }
        // read last block
        buf = getBuf (bufList, i);
        u_strcpy (buf, 0, ret, n, end.cursor);
        n += end.cursor;
        assert (n == size);
    }
    return ret;
}

int distance (BufList *bufList, BufBlockPos start, BufBlockPos end) {
    if (start.blockIndex == end.blockIndex)
        return end.cursor - start.cursor;
    else
        return (bufList->blockSize - start.cursor) + (end.blockIndex - start.blockIndex - 1) * bufList->blockSize + end.cursor;
}