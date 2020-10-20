#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>

#include "headers/http_errors.h"

/**
 * Sends 500 error code to client when server fails internally (Ctrl+C)
 */
void send500(request *req)
{
    printf("Server failed for this request\n");

    char httpmsg[] = "HTTP/1.1 500 Internal Server Error\r\nContent-Type:text/html\r\nContent-Length:132\r\n\r\n";
    char httpfile[] = "<html><h1>These messages in the exact format as shown above should be sent back to the client if any of the above error occurs.</h1>";
    char buf[MAXLINE];
    strcpy(buf, httpmsg);
    write(req->connfd, buf, strlen(httpmsg));
    strcpy(buf, httpfile);
    write(req->connfd, buf, strlen(httpfile));
    free(req);
    close(req->connfd);
    pthread_exit(pthread_self());
}

/**
 * Sends 400 error code to client when any part of the request is invalid
 */
void send400(request *req)
{
    printf("Server received invalid request\n");

    char httpmsg[] = "HTTP/1.1 400 Bad Request\r\nContent-Type:text/html\r\nContent-Length:44\r\n\r\n";
    char httpfile[] = "<html><h1>The request sent was invalid.</h1>";
    char buf[MAXLINE];
    strcpy(buf, httpmsg);
    write(req->connfd, buf, strlen(httpmsg));
    strcpy(buf, httpfile);
    write(req->connfd, buf, strlen(httpfile));
    free(req);
    close(req->connfd);
    pthread_exit(pthread_self());
}

/**
 * Sends 404 error code to client when requested file does not exist on server or directory does not contain index.html
 */
void send404(request *req)
{
    printf("Client requested resource that is not found\n");

    char httpmsg[] = "HTTP/1.1 404 File Not Found\r\nContent-Type:text/html\r\nContent-Length:150\r\n\r\n";
    char httpfile[] = "<html><h1>The requested file could not be found. Please try again with a different file. If it is a directory, make sure it has a trailing slash!</h1>";
    char buf[MAXLINE];
    strcpy(buf, httpmsg);
    write(req->connfd, buf, strlen(httpmsg));
    strcpy(buf, httpfile);
    write(req->connfd, buf, strlen(httpfile));
    free(req);
    close(req->connfd);
    pthread_exit(pthread_self());
}