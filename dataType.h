
#ifndef DATA_TYPE
#define DATA_TYPE

#define TRUE 1
#define FALSE 0

typedef struct _pair {
    char *key;
    char *value;
    struct _pair *next;
} StringPair;

typedef struct {
    int size;
    StringPair *head;
} StringMap;

StringMap* createStringMap ();
void resolvePair (StringMap *map, char *raw, int len, char splitCharacter);
void resolveMultiPairs (StringMap *map, char *raw, int len, char splitCharacter, char pairSplitCharacter);
void addPair (StringMap *map, char *key, char *value);
char* getValue (StringMap *map, char *key);
void removePair (StringMap *map, char *key);
void outputMap (StringMap *map);
void destroyStringMap (StringMap *map, int freeCompletely);


typedef struct _bufBlockPos {
    int blockIndex;
    int cursor;
} BufBlockPos;

typedef struct _bufBlock {
    char *buf;
    struct _bufBlock *next;
} BufBlock;

typedef struct _bufList {
    int size;
    int blockSize;
    BufBlock *head, *tail;
} BufList;

BufList* createBufList (int blockSize);
char* addBuf (BufList *bufList);
char* getBuf (BufList *bufList, int index);
void removeBuf (BufList *bufList, int index);
void destroyBufList (BufList *bufList);
void setBufBlockPos (BufBlockPos *pos, int blockIndex, int cursor);
int compareBufBlockPos (BufBlockPos a, BufBlockPos b);
char* substractData (BufList *bufList, BufBlockPos start, BufBlockPos end);
int distance (BufList *bufList, BufBlockPos start, BufBlockPos end);

#endif