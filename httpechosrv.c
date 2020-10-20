/*
 * tcpechosrv.c - A concurrent TCP echo server using threads
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* for fgets */
#include <strings.h>    /* for bzero, bcopy */
#include <unistd.h>     /* for read, write */
#include <sys/socket.h> /* for socket use */
#include <signal.h>     /* for signals */
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "headers/helper_functions.h"
#include "headers/http_errors.h"
#include "headers/request_handlers.h"
#include "headers/httpechosrv.h"

/**
 * Main function: Sets the signal handler, creates the listening port to get requests and opens new threads on request
 */
int main(int argc, char **argv)
{
    // Install signal handler for SIGINT
    struct sigaction custom_handler;
    custom_handler.sa_handler = sigint_handler;
    sigaction(SIGINT, &custom_handler, NULL);

    int listenfd, *connfdp, port, clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pthread_t tid;

    // There should be an arg detailing port number. ie: ./out [PORT]
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    port = atoi(argv[1]);

    // Opens new port on which program listens to
    listenfd = open_listenfd(port);
    while (1)
    {
        connfdp = malloc(sizeof(int));
        // Accepts client connection - Gets first connection on listenfd socket and creates a new dedicated socket for client.
        // If no connections then block until one comes
        // https://www.ibm.com/support/knowledgecenter/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxbd00/accept.htm
        *connfdp = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        pthread_create(&tid, NULL, thread, connfdp);
    }
}

/**
 * Main function for threads; Argument is the socket file descriptor. First parses the incoming request,
 * then goes to the corresponding request handler (GET or POST)
 */
void *thread(void *vargp)
{
    int connfd = *((int *)vargp);
    // Makes it so that when this specific thread exits system can free the used resources without waiting
    pthread_detach(pthread_self());
    free(vargp);
    request *req = malloc(sizeof(request));
    memset(req->path, 0, sizeof(req->path));
    memset(req->http_version, 0, sizeof(req->http_version));
    memset(req->connection, 0, sizeof(req->connection));
    req->connfd = connfd;
    parse_request(req);
    if (req->request_type == 1)
    {
        handle_get_request(req);
    }
    else if (req->request_type == 2)
    {
        handle_post_request(req);
    }
    else
    {
        send500(req);
    }
    free(req);
    close(connfd);
    return NULL;
}

/**
 *  SIGINT handler to exit gracefully
 */
void sigint_handler(int signal)
{
    printf("\nWebserver is shutting down.\n");
    exit(SIGINT);
}

/*
 * open_listenfd - open and return a listening socket on port
 * Returns -1 in case of failure
 */
int open_listenfd(int port)
{
    int listenfd, optval = 1;
    struct sockaddr_in serveraddr;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval, sizeof(int)) < 0)
        return -1;

    /* listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
} /* end open_listenfd */
