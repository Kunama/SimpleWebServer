#include "headers/helper_functions.h"
#include "headers/http_errors.h"

/**
 * Parses the request from a socket and returns request object
 * Parse the connection type for pipelining, request type, HTTP version, path requested, as well as POSTDATA if POST request
 */
void parse_request(request *req)
{
    char request[MAXBUF];
    memset(request, 0, sizeof(request));
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
    if (req->request_type == 2)
    {
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

/**
 * Returns the correct file path accounting for default index.html if directories.
 */
char *get_file_path(request *req)
{
    // Get file
    if (req->path[strlen(req->path) - 1] == '/')
    {
        strcat(req->path, "index.html");
    }
    int path_length = strlen(req->path) + 6;
    char *file_path = malloc(path_length);
    memset(file_path, 0, path_length);
    strcat(file_path, "./www");
    strcat(file_path, req->path);
    return file_path;
}

/**
 * Returns the content type of the file based on the file extension
 */
void get_content_type(char *file_path, char *content_type)
{
    char *extension = strrchr(file_path, '.') + 1;
    free(file_path);
    if (!strcasecmp(extension, "html"))
    {
        strcpy(content_type, "text/html");
    }
    else if (!strcasecmp(extension, "txt"))
    {
        strcpy(content_type, "text/plain");
    }
    else if (!strcasecmp(extension, "png"))
    {
        strcpy(content_type, "image/png");
    }
    else if (!strcasecmp(extension, "gif"))
    {
        strcpy(content_type, "image/gif");
    }
    else if (!strcasecmp(extension, "jpg"))
    {
        // In the writeup it said to use jpg, but Safari was downloading the file instead of displaying it so using the supported jpeg instead
        strcpy(content_type, "image/jpeg");
    }
    else if (!strcasecmp(extension, "css"))
    {
        strcpy(content_type, "text/css");
    }
    else if (!strcasecmp(extension, "js"))
    {
        strcpy(content_type, "application/javascript");
    }
}
