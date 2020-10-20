#define MAXLINE 8192 /* max text line length */
#define MAXBUF 8192  /* max I/O buffer size */
#define LISTENQ 1024 /* second argument to listen() */

// Struct for holding all request info
#ifndef REQUEST_DEFINED
#define REQUEST_DEFINED
typedef struct Request
{
    int connfd;
    int request_type;
    char path[MAXLINE];
    char http_version[9];
    char connection[15];
    char post_data[MAXLINE];
} request;
#endif