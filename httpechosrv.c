/* 
 * tcpechosrv.c - A concurrent TCP echo server using threads
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* for fgets */
#include <strings.h>    /* for bzero, bcopy */
#include <unistd.h>     /* for read, write */
#include <sys/socket.h> /* for socket use */
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAXLINE 8192 /* max text line length */
#define MAXBUF 8192  /* max I/O buffer size */
#define LISTENQ 1024 /* second argument to listen() */

// Struct for holding all request info
typedef struct Request
{
    int connfd;
    int request_type;
    char path[MAXLINE];
    char http_version[9];
    char connection[15];
    char post_data[MAXLINE];
} request;

// Define structure of methods so they can be called before implementation
int open_listenfd(int port);
void *thread(void *vargp);
void parse_request(request *req);
void handle_get_request(request *req);
void handle_post_request(request *req);
// https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
void send500(request* req); // Internal Server error
void send400(request* req); // Bad Request error
void send404(request* req); // File not found error


int main(int argc, char **argv)
{
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

/* thread routine */
void *thread(void *vargp)
{
    int connfd = *((int *)vargp);
    // Makes it so that when this specific thread exits system can free the used resources without waiting
    pthread_detach(pthread_self());
    free(vargp);
    request *req = malloc(sizeof(request));
    memset(req->path,0,sizeof(req->path));
    memset(req->http_version,0,sizeof(req->http_version));
    memset(req->connection,0,sizeof(req->connection));
    req->connfd = connfd;
    parse_request(req);
    if (req->request_type == 1)
    {
        handle_get_request(req);
    } else if(req->request_type == 2){
        handle_post_request(req);
    } else{
        send500(req);
    }
    free(req);
    close(connfd);
    return NULL;
}

/**
 * Parses the request from a socket and returns 0 if GET and 1 if POST
 */
void parse_request(request *req)
{
    char request[MAXBUF];
    memset(request,0,sizeof(request));
    read(req->connfd, request, MAXLINE);
    printf("Server received the following request:\n%s\n", request);

    char *current = &request;
    // Parse request type
    if (strncmp(request, "GET", 3) == 0)
    {
        req->request_type = 1;
        current = current + 4;
    }
    else if (strncmp(request, "POST", 4) == 0)
    {
        req->request_type = 2;
        current = current + 5;
    }
    else
    {
        send400(req);
    }

    // Parse path
    // https://www.geeksforgeeks.org/how-to-append-a-character-to-a-string-in-c/
    char *current_char[2];
    current_char[0] = *current;
    current_char[1] = '\0';
    while (current_char[0] != ' ' && current_char[0] != '\0')
    {
        strcat(req->path, current_char);
        current++;
        current_char[0] = *current;
    }

    current++;
    current_char[0] = *current;

    // Parse HTTP version
    while (current_char[0] != '\r' && current_char[0] != '\0')
    {
        strcat(req->http_version, current_char);
        current++;
        current_char[0] = *current;
    }
    if (strcmp(req->http_version, "HTTP/1.1") != 0 && strcmp(req->http_version, "HTTP/1.0") != 0)
    {
        send400(req);
    }

    // Parse connection
    current = strstr(request, "Connection: ");
    if (current == NULL)
    {
        send400(req);
    }
    current = current + 12;
    current_char[0] = *current;
    while (current_char[0] != '\r' && current_char[0] != '\0')
    {
        strcat(req->connection, current_char);
        current++;
        current_char[0] = *current;
    }

    // Parse POST DATA
    if(req->request_type == 2){
        current = strstr(request, "\r\n\r\n");
        if (current == NULL)
        {
            send400(req);
        }
        current = current + 4;
        current_char[0] = *current;
        while (current_char[0] != '\r' && current_char[0] != '\0')
        {
            strcat(req->post_data, current_char);
            current++;
            current_char[0] = *current;
        }
    }
}

char* get_file_path(request *req){
    // Get file
    if(req->path[strlen(req->path)-1] == '/'){
        strcat(req->path, "index.html");
    }
    int path_length = strlen(req->path)+6;
    char* file_path = malloc(path_length);
    memset(file_path,0,path_length);
    strcat(file_path, "./www");
    strcat(file_path, req->path);
    return file_path;
}

void get_content_type(char* file_path, char* content_type){
    char* extension = strrchr(file_path, '.') + 1;
    free(file_path);
    if(!strcasecmp(extension, "html")){
        strcpy(content_type, "text/html");
    } else if(!strcasecmp(extension, "txt")){
        strcpy(content_type, "text/plain");
    } else if(!strcasecmp(extension, "png")){
        strcpy(content_type, "image/png");
    } else if(!strcasecmp(extension, "gif")){
        strcpy(content_type, "image/gif");
    } else if(!strcasecmp(extension, "jpg")){
        // In the writeup it said to use jpg, but Safari was downloading the file instead of displaying it so using the supported jpeg instead
        strcpy(content_type, "image/jpeg");
    } else if(!strcasecmp(extension, "css")){
        strcpy(content_type, "text/css");
    } else if(!strcasecmp(extension, "js")){
        strcpy(content_type, "application/javascript");
    }
}

// Send to client for GET requests
void handle_get_request(request *req)
{
    char* file_path = get_file_path(req);
    FILE* requested_file = fopen(file_path, "r");
    if(!requested_file){
        send404(req);
    }

    // Get file type
    char content_type[23];
    get_content_type(file_path, content_type);

    // Get content-length
    fseek(requested_file, 0, SEEK_END);
    int content_length = ftell(requested_file);
    fseek(requested_file, 0, SEEK_SET);

    // Send header:
    // https://stackoverflow.com/questions/4881937/building-strings-from-variables-in-c
    char send_buffer[MAXLINE];
    memset(send_buffer,0,sizeof(send_buffer));
    snprintf(send_buffer, sizeof(send_buffer), "%s 200 OK\r\nContent-Type:%s\r\nContent-Length:%d\r\n\r\n", req->http_version, content_type, content_length);
    printf("Returning header:\n%s\n", send_buffer);
    write(req->connfd, send_buffer, strlen(send_buffer));

    int n;
    memset(send_buffer,0,sizeof(send_buffer));
    while ((n = fread(send_buffer, 1, sizeof(send_buffer)-1, requested_file)) > 0) {
        write(req->connfd, send_buffer, n);
        memset(send_buffer,0,sizeof(send_buffer));
    }

    fclose(requested_file);
}

void handle_post_request(request *req){
    char* file_path = get_file_path(req);
    FILE* requested_file = fopen(file_path, "r");
    if(!requested_file){
        send404(req);
    }

    // Get file type
    char content_type[23];
    get_content_type(file_path, content_type);
    if(strcmp(content_type, "text/html") != 0){
        // POST only returns for html as said in write up.
        send400(req);
    }

    // Get content-length
    fseek(requested_file, 0, SEEK_END);
    int content_length = ftell(requested_file);
    fseek(requested_file, 0, SEEK_SET);

    // Add post data length
    content_length = content_length + strlen(req->post_data) + 20; // +20 for the <pre> and <h1> tags

    // Send header:
    // https://stackoverflow.com/questions/4881937/building-strings-from-variables-in-c
    char send_buffer[MAXLINE];
    memset(send_buffer,0,sizeof(send_buffer));
    snprintf(send_buffer, sizeof(send_buffer), "%s 200 OK\r\nContent-Type:%s\r\nContent-Length:%d\r\n\r\n<pre><h1>%s</h1></pre>", req->http_version, content_type, content_length, req->post_data);
    printf("Returning header:\n%s\n", send_buffer);
    write(req->connfd, send_buffer, strlen(send_buffer));

    int n;
    memset(send_buffer,0,sizeof(send_buffer));
    while ((n = fread(send_buffer, 1, sizeof(send_buffer)-1, requested_file)) > 0) {
        write(req->connfd, send_buffer, n);
        memset(send_buffer,0,sizeof(send_buffer));
    }

    fclose(requested_file);
}

void send500(request* req)
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

void send400(request* req)
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

void send404(request* req)
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
