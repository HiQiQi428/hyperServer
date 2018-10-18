
#ifndef HTTP
#define HTTP

#include "dataType.h"

#define M_GET 1
#define M_POST 2
#define M_UNKNOWN 3

typedef struct _request {
    int method;
    char *host;
    char *path;
    StringMap *parameters;
    StringMap *headers;
    char *content;
} Request;

Request* createRequest ();
Request* substractRequest (int sockFd);
void outputRequest (Request *request);
void destroyRequest (Request *request);

typedef struct _response {
    int headerSize; // not consider the description of status code
    int contentLength;
    int statusCode;
    StringMap *headers;
    char *content;
} Response;

Response* createResponse ();
void addHeader (Response *response, char *name, char *value);
void appendContent (Response *response, char *src, int len);
char* serialize (Response *response, int *len);
void destroyResponse (Response *response);

typedef struct _handleNode {
    void* (*handler) (Request *, Response *);
    struct _handleNode *next;
} HandleNode;

typedef struct _dispatcher {
    HandleNode *head;
} Dispatcher;

Dispatcher* createDispatcher ();
void registerHandler (Dispatcher *dispatcher, void* (*handler) (Request *, Response *));
void dispatch (Dispatcher *dispatcher, int sockFd);
void destroyDispatcher (Dispatcher *dispatcher);

#endif