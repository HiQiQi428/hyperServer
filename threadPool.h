
#ifndef _ThreadPool
#define _ThreadPool

#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>

typedef struct executor {
    void *(*task) (void *arg);
    void *arg;
    struct executor *next;
} ThreadExecutor;

typedef struct {
    pthread_mutex_t queueLock;
    pthread_cond_t queueReady;
    ThreadExecutor *queueHead; // all tasks waiting to be executed
    int shutdown;
    pthread_t *threadId;
    int maxThreadNum; // number of active thread in pool
    int curQueueSize; // number of task wait in queue
} ThreadPool;

ThreadPool* createThreadPool (int maxThreadNum);

void submitTask (ThreadPool *pool, void *(*task) (void *arg), void *arg);

int destroyThreadPool (ThreadPool *pool);

#endif