
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "http.h"
#include "threadPool.h"
#include "util.h"

#define SERVER_IP "0.0.0.0"
#define SERVER_PORT 8000
#define CONCURRENT_NUM 10

// TODO 支持chunk传送
// TODO 去除'\r'

ThreadPool *threadPool;
Dispatcher *dispatcher;

void* handler (Request *req, Response *rep) {
    rep->statusCode = 200;
    appendContent (rep, "<h1>Hello, World!</h1>", 22);
    return NULL;
}

void* handlerAdapter (void *arg) {
    int sockFd = *(int*) arg;
    dispatch (dispatcher, sockFd);
    close (sockFd);
    free (arg); // u_intcpy use malloc to create copy from a integer
    return NULL;
}

int main(int argn, char **argv) {
    threadPool = createThreadPool (CONCURRENT_NUM);
    dispatcher = createDispatcher ();
    registerHandler (dispatcher, handler);

    int serverFd, connFd, ret;
    socklen_t len;
    char ip[40] = { 0 };
    struct sockaddr_in serverAddr, clientAddr;

    serverFd = socket (AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        printf ("[hyper-server] socket error: %s\n", strerror (errno));
        exit (-1);
    }
    bzero (&serverAddr, sizeof (clientAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons (SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    ret = bind (serverFd, (struct sockaddr*) &serverAddr, sizeof (serverAddr));
    if (ret != 0) {
        close (serverFd);
        printf ("[hyper-server] bind error: %s\n", strerror (errno));
        exit (-1);
    }

    ret = listen (serverFd, 5);
    if (ret != 0) {
        close (serverFd);
        printf ("[hyper-server] listen error: %s\n", strerror (errno));
        exit (-1);
    }
    printf ("[hyper-server] startup finished on %s:%d\n", SERVER_IP, SERVER_PORT);

    len = sizeof (clientAddr);
    bzero (&clientAddr, sizeof (clientAddr));
    while (TRUE) {
        len = sizeof (clientAddr);
        connFd = accept (serverFd, (struct sockaddr*) &clientAddr, &len);
        printf("[hyper-server] client ip: %s\n", inet_ntop(AF_INET, &clientAddr.sin_addr, ip, sizeof(ip)));
        if (connFd < 0) printf ("[hyper-server] accept error: %s\n", strerror (errno));
        else submitTask (threadPool, handlerAdapter, u_intcpy (connFd));
    }
    close (serverFd);

    destroyDispatcher (dispatcher);
    destroyThreadPool (threadPool);
    return 0;
}