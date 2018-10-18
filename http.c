
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dataType.h"
#include "http.h"
#include "util.h"

// for test
/*
char *httpRequest = "GET /p/8fe93a14754c?name=luncert&age=13&t_id=1 HTTP/1.1\r\nHost: www.fishbay.cn\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/56.0.2924.87 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,/*;q=0.8\r\nAccept-Encoding: gzip, deflate, sdch\r\nAccept-Language: zh-CN,zh;q=0.8,en;q=0.6\r\nContent-Length: 25\r\n\r\nadoin109ccj12i0km120nasn2";
int tc = 0, tsize = 454;
int read (int sockFd, char *buf, int maxBufSize) {
    int i, j, limit = (tsize - tc > maxBufSize) ? maxBufSize : tsize - tc;
    for (i = tc, j = 0; i < limit; i++, j++)
        buf[j] = httpRequest[i];
    tc += limit;
    return limit;
}
*/

#define BUF_SIZE 2048

Request* createRequest () {
    Request *req = (Request*) malloc (sizeof (Request));
    bzero (req, sizeof (Request));
    req->parameters = createStringMap ();
    req->headers = createStringMap ();
    return req;
}

Request* substractRequest (int sockFd) {
    Request *request = createRequest ();
    // ssize_t n;
    int n, beFirstLine = 1;
    BufList *bufList = createBufList (BUF_SIZE);
    char *buf = addBuf (bufList), *tmpString;
    BufBlockPos cur, pre;
    setBufBlockPos (&cur, 0, 0);
    setBufBlockPos (&pre, 0, 0);
    // resolve http header
    while ((n = read(sockFd, buf, BUF_SIZE)) > 0) {
        for (cur.cursor = 0; cur.cursor < n; cur.cursor++) {
            if (buf[cur.cursor] == '\n') {
                if (distance (bufList, pre, cur) == 1) {// \r\n
                    // set host
                    tmpString = getValue (request->headers, "Host");
                    request->host = u_strcpy_n (tmpString, 0, strlen (tmpString));
                    goto resolveContent;
                }
                int lineSize = distance (bufList, pre, cur);
                char *line = substractData (bufList, pre, cur);
                if (beFirstLine) {
                    beFirstLine = 0;
                    // resolve method
                    int s = 0, e = 0;
                    while (line[e] != ' ') e++;
                    char *method = u_strcpy_n (line, s, e - s);
                    if (strcmp ("GET", method) == 0) request->method = M_GET;
                    else if (strcmp ("POST", method) == 1) request->method = M_POST;
                    else request->method = M_UNKNOWN;
                    // resolve url
                    e++;
                    s = e;
                    while (line[e] != ' ' && line[e] != '?' ) e++; // get url
                    request->path = u_strcpy_n (line, s, e - s);
                    if (line[e] == '?') {
                        s = e + 1;
                        while (line[e] != ' ') e++;
                        // reoslve parameters
                        char pair[e - s];
                        u_strcpy (line, s, pair, 0, e - s);
                        resolveMultiPairs (request->parameters, pair, e - s, '=', '&');
                    }
                }
                else resolvePair (request->headers, line, lineSize, ':');
                free (line);
                setBufBlockPos (&pre, cur.blockIndex, cur.cursor + 1);
            }
        }
        buf = addBuf (bufList);
        cur.blockIndex++; // reference to the new buf
    }
    resolveContent:
    tmpString = getValue (request->headers, "Content-Length");
    if (tmpString != NULL) {
        setBufBlockPos (&pre, cur.blockIndex, cur.cursor + 1);
        int contentLength = atoi (tmpString);
        int tmp = contentLength - (n - cur.cursor - 1);
        int timesToRead = (tmp > 0) ? (tmp / BUF_SIZE + (tmp % BUF_SIZE == 0) ? 0 : 1) : 0;
        cur.cursor = n;
        for (tmp = 0; tmp < timesToRead; tmp++) {
            buf = addBuf (bufList);
            setBufBlockPos (&cur, cur.blockIndex + 1, 0);
            n = read(sockFd, buf, BUF_SIZE);
            cur.cursor = n;
        }
        request->content = substractData (bufList, pre, cur);
    }
    destroyBufList (bufList);
    return request;
}

void outputRequest (Request *request) {
    printf ("method: %d\n", request->method);
    printf ("path: %s\n", request->path);
    printf ("host: %s\n", request->host);
    outputMap (request->parameters);
    outputMap (request->headers);
    printf ("content: %s\n", request->content);
}

void destroyRequest (Request *request) {
    free (request->host);
    free (request->path);
    destroyStringMap (request->parameters, TRUE);
    destroyStringMap (request->headers, TRUE);
    if (request->content != NULL)
        free (request->content);
}

// Response

Response* createResponse () {
    Response *response = (Response*) malloc (sizeof (Response));
    bzero (response, sizeof (Response));
    response->headerSize = 15; // 'HTTP/1.1 XXX \r\n'
    response->headers = createStringMap ();
    return response;
}

void addHeader (Response *response, char *name, char *value) {
    response->headerSize += strlen (name);
    response->headerSize += 2; // ': '
    response->headerSize += strlen (value);
    response->headerSize += 2; // '\r\n'
    addPair (response->headers, u_strcpy_n0 (name), u_strcpy_n0 (value));
}

void appendContent (Response *response, char *src, int len) {
    if (response->content == NULL) {
        response->contentLength = len;
        response->content = u_strcpy_n (src, 0, len);
    }
    else {
        char *tmp = (char*) malloc (sizeof (char) * (response->contentLength + len));
        u_strcpy (response->content, 0, tmp, 0, response->contentLength);
        u_strcpy (src, 0, tmp, response->contentLength, len);
        free (response->content);
        response->content = tmp;
        response->contentLength += len;
    }
}

char* serialize (Response *response, int *len) {
    char *buf;
    int cursor = 0;
    char tmpBuf[20] = {0};
    sprintf (tmpBuf, "%d", response->contentLength);
    addHeader (response, "Content-Length", tmpBuf);
    int size = response->headerSize + 2 + response->contentLength + 1;
    switch (response->statusCode) {
        case 200:
            u_strcpy ("OK", 0, tmpBuf, 0, 2);
            size += 2;
            break;
        case 404:
            u_strcpy ("NOT FOUND", 0, tmpBuf, 0, 9);
            size += 9;
            break;
        case 500:
            u_strcpy ("SERVER ERROR", 0, tmpBuf, 0, 12);
            size += 12;
            break;
        default:
            u_strcpy ("UNKNOWN", 0, tmpBuf, 0, 7);
            size += 7;
    }
    *len = size;
    buf = (char*) malloc (sizeof (char) * size);
    // add first line
    cursor += sprintf (buf, "HTTP/1.1 %d %s\r\n", response->statusCode, tmpBuf);
    // add the headers
    StringPair *pair = response->headers->head;
    while (pair != NULL) {
        cursor += u_strcpy_0 (pair->key, 0, buf, cursor);
        buf[cursor++] = ':';
        buf[cursor++] = ' ';
        cursor += u_strcpy_0 (pair->value, 0, buf, cursor);
        buf[cursor++] = '\r';
        buf[cursor++] = '\n';
        pair = pair->next;
    }
    // add content;
    buf[cursor++] = '\r';
    buf[cursor++] = '\n';
    u_strcpy (response->content, 0, buf, cursor, response->contentLength);
    cursor += response->contentLength;
    buf[cursor++] = 0;
    assert (cursor == size);
    return buf;
}

void destroyResponse (Response *response) {
    destroyStringMap (response->headers, TRUE);
    if (response->content != NULL)
        free (response->content);
}

// Dispatcher

Dispatcher* createDispatcher () {
    Dispatcher *dispatcher = (Dispatcher*) malloc (sizeof (Dispatcher));
    bzero (dispatcher, sizeof (dispatcher));
    return dispatcher;
}

void registerHandler (Dispatcher *dispatcher, void* (*handler) (Request *, Response *)) {
    HandleNode *newHandleNode = (HandleNode*) malloc (sizeof (HandleNode));
    newHandleNode->handler = handler;
    if (dispatcher->head == NULL) {
        newHandleNode->next = NULL;
        dispatcher->head = newHandleNode;
    }
    else {
        newHandleNode->next = dispatcher->head;
        dispatcher->head = newHandleNode;
    }
}

void dispatch (Dispatcher *dispatcher, int sockFd) {
    Request *req = substractRequest (sockFd);
    // outputRequest (req);
    Response *rep = createResponse ();
    HandleNode *handleNode = dispatcher->head;
    while (handleNode != NULL) {
        handleNode->handler (req, rep);
        handleNode = handleNode->next;
    }
    int repLen;
    char *repRaw = serialize (rep, &repLen);
    // printf ("%s\n", repRaw);
    write (sockFd, repRaw, repLen);
    destroyResponse (rep);
    destroyRequest (req);
}

void destroyDispatcher (Dispatcher *dispatcher) {
    if (dispatcher != NULL) {
        HandleNode *handleNode = dispatcher->head, *next;
        while (handleNode != NULL) {
            next = handleNode->next;
            free (handleNode);
            handleNode = next;
        }
        free (dispatcher);
    }
}
