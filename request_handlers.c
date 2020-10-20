#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "headers/request_handlers.h"
#include "headers/helper_functions.h"
#include "headers/http_errors.h"

/**
 * Handler to support GET requests
 */
void handle_get_request(request *req)
{
    char *file_path = get_file_path(req);
    FILE *requested_file = fopen(file_path, "r+");
    printf("%s\n", file_path);
    char cwd[400];
    getcwd(cwd, sizeof(cwd));
    printf("Current working dir: %s\n", cwd);

    if (!requested_file)
    {
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
    memset(send_buffer, 0, sizeof(send_buffer));
    snprintf(send_buffer, sizeof(send_buffer), "%s 200 OK\r\nContent-Type:%s\r\nContent-Length:%d\r\n\r\n", req->http_version, content_type, content_length);
    printf("Returning header:\n%s\n", send_buffer);
    write(req->connfd, send_buffer, strlen(send_buffer));

    // Send file
    int n;
    memset(send_buffer, 0, sizeof(send_buffer));
    while ((n = fread(send_buffer, 1, sizeof(send_buffer) - 1, requested_file)) > 0)
    {
        write(req->connfd, send_buffer, n);
        memset(send_buffer, 0, sizeof(send_buffer));
    }

    fclose(requested_file);
}

/**
 * Handler to support POST requests
 */
void handle_post_request(request *req)
{
    char *file_path = get_file_path(req);
    FILE *requested_file = fopen(file_path, "r");
    if (!requested_file)
    {
        send404(req);
    }

    // Get file type
    char content_type[23];
    get_content_type(file_path, content_type);
    if (strcmp(content_type, "text/html") != 0)
    {
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
    memset(send_buffer, 0, sizeof(send_buffer));
    snprintf(send_buffer, sizeof(send_buffer), "%s 200 OK\r\nContent-Type:%s\r\nContent-Length:%d\r\n\r\n<pre><h1>%s</h1></pre>", req->http_version, content_type, content_length, req->post_data);
    printf("Returning header:\n%s\n", send_buffer);
    write(req->connfd, send_buffer, strlen(send_buffer));

    // Send file
    int n;
    memset(send_buffer, 0, sizeof(send_buffer));
    while ((n = fread(send_buffer, 1, sizeof(send_buffer) - 1, requested_file)) > 0)
    {
        write(req->connfd, send_buffer, n);
        memset(send_buffer, 0, sizeof(send_buffer));
    }

    fclose(requested_file);
}
