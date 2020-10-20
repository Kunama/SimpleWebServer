#include "request.h"

// https://developer.mozilla.org/en-US/docs/Web/HTTP/Status
void send500(request *req); // Internal Server error
void send400(request *req); // Bad Request error
void send404(request *req); // File not found error