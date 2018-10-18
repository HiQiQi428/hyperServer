
#include <stdio.h>
#include <stdlib.h>
#include "threadPool.h"

void* testProcess (void *arg) {
    printf ("threadId: 0x%x, taskId: %d\n", pthread_self (), *(int *) arg);
    sleep (1);
    return NULL;
}

int main () {
    ThreadPool *pool = createThreadPool (3);
    int *workNum = (int*) malloc (sizeof (int) * 10);
    int i;
    for (i = 0; i < 10; i++) {
        workNum[i] = i;;
        submitTask (pool, testProcess, &workNum[i]);
    }
    sleep (5);
    destroyThreadPool (pool);
    free (workNum);
    return 0;
}