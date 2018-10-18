
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include "dataType.h"
#include "threadPool.h"

void *threadRoutine (void *arg) {
    ThreadPool *pool = (ThreadPool*) arg;
    while (TRUE) {
        pthread_mutex_lock (&(pool->queueLock)); // lock
        // waiting for task
        while (pool->curQueueSize == 0 && !pool->shutdown)
            pthread_cond_wait (&(pool->queueReady), &(pool->queueLock));
        // exit thread if pool->shutdown is true
        if (pool->shutdown) {
            pthread_mutex_unlock (&(pool->queueLock));
            pthread_exit (NULL);
        }
        assert (pool->curQueueSize != 0);
        assert (pool->queueHead != NULL);
        // get the head in queue
        pool->curQueueSize--;
        ThreadExecutor *executor = pool->queueHead;
        pool->queueHead = executor->next;
        pthread_mutex_unlock (&(pool->queueLock));
        // execute
        (*(executor->task)) (executor->arg);
        free (executor);
        executor = NULL;
    }
    pthread_exit (NULL); // this line should be unreachable
}

ThreadPool* createThreadPool (int maxThreadNum) {
    ThreadPool *pool = (ThreadPool*) malloc (sizeof (ThreadPool));
    bzero (pool, sizeof (ThreadPool));
    pthread_mutex_init (&(pool->queueLock), NULL);
    pthread_cond_init (&(pool->queueReady), NULL);
    pool->maxThreadNum = maxThreadNum;
    pool->threadId = (pthread_t*) malloc (maxThreadNum * sizeof (pthread_t));
    int i;
    for (i = 0; i < maxThreadNum; i++)
        pthread_create (&(pool->threadId[i]), NULL, threadRoutine, pool);
    return pool;
}

void submitTask (ThreadPool *pool, void *(*task) (void *arg), void *arg) {
    ThreadExecutor *newExecutor = (ThreadExecutor*) malloc (sizeof (ThreadExecutor));
    newExecutor->task = task;
    newExecutor->arg = arg;
    newExecutor->next = NULL;

    pthread_mutex_lock (&(pool->queueLock)); // lock
    ThreadExecutor *executor = pool->queueHead;
    if (executor != NULL) {
        while (executor->next != NULL)
            executor = executor->next;
        executor->next = newExecutor;
    }
    else pool->queueHead = newExecutor;
    pool->curQueueSize++;
    pthread_mutex_unlock (&(pool->queueLock)); // unlock
    pthread_cond_signal (&(pool->queueReady)); // notify thread blocked
}

int destroyThreadPool (ThreadPool *pool) {
    if (pool != NULL) {
        if (pool->shutdown)
            return -1;
        pool->shutdown = TRUE;
        pthread_cond_broadcast (&(pool->queueReady)); // notify all sleeping thread
        // blocking to wait for threads
        int i;
        for (i = 0; i < pool->maxThreadNum; i++)
            pthread_join (pool->threadId[i], NULL);
        free(pool->threadId);
        // destroy executor queue
        ThreadExecutor *head = NULL;
        while (pool->queueHead != NULL) {
            head = pool->queueHead;
            pool->queueHead = pool->queueHead->next;
            free(head);
        }
        pthread_mutex_destroy (&(pool->queueLock));
        pthread_cond_destroy (&(pool->queueReady));
        free(pool);
        return 0;
    }
    return -1;
}
