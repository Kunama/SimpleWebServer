#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "request.h"

void parse_request(request *req);
char *get_file_path(request *req);
void get_content_type(char *file_path, char *content_type);
