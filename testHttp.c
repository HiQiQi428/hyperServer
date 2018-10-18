
#include <stdio.h>
#include <stdlib.h>
#include "dataType.h"
#include "http.h"

void testRequest () {
    printf ("[Test Request]\n");
    Request *req = substractRequest (0);
    outputRequest (req);
    destroyRequest (req);
}

void testResponse () {
    printf ("[Test Response]\n");
    Response *rep = createResponse ();
    rep->statusCode = 200;
    addHeader (rep, "Server", "nginx");
    addHeader (rep, "Date", "Mon, 20 Feb 2017 09:13:59 GMT");
    addHeader (rep, "Content-Type", "text/plain;charset=UTF-8");
    addHeader (rep, "Vary", "Accept-Encoding");
    addHeader (rep, "Cache-Control", "no-store");
    addHeader (rep, "Progrma", "no-cache");
    addHeader (rep, "Cache-Control", "no-cache");
    addHeader (rep, "Expires", "Thu, 01 Jan 1970 00:00:00 GMT");
    addHeader (rep, "Content-Encoding", "gzip");
    addHeader (rep, "Transer-Encoding", "chunked");
    addHeader (rep, "Proxy-Connection", "keep-alive");
    appendContent (rep, "<h1>Hello, World!</h1>", 22);
    int len;
    char *repRaw = serialize (rep, &len);
    printf ("%s\n", repRaw);
    free (repRaw);
    destroyResponse (rep);
}

int main () {
    testRequest ();
    testResponse ();
    return 0;
}