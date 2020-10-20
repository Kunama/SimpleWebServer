# Akhil Kunam - Simple HTTP Web Server

## What I Did

This is a very rudimentery web server that supports the POST and GET methods for HTTP/1.1 and HTTP/1.0. A lot of the code comes from starter file given as well as the batch sending done in the custom UDP we implemented for PA1.

---

The program starts by creating a socket in which the program listens to. When an incoming request is detected the program creates a socket for that request and pushes all the work off to a new thread so it can listen for new requests.

In the thread, it firsts parses the request to get the relevant information such as path, request method, connection. After parsing it, it flows to the correct handler based on request method where it forms the response and sends the file. Along the way the corresponding error codes are sent back to the client if any occur.

## How to run:

Assuming you are in the submission directory:

```
make clean; make; ./webserver 8888
```

This will compile all the files into an executable called `webserver` which takes an argument as the port number. When it is running you will see the outputs by the server such as the requests, response headers, and any errors.

**NOTE: All files are searched for in the www folder when requested by the client**

## Tests

To test some of the functionality there are a few NodeJS scripts. They all assume that the command was executed in the main submission directory and the server is running already. NodeJS should be installed and the node modules should be installed through

```
npm i
```

### Multiple GET Requests test

Does 5 asynchronous GET requests to the base index.html file and prints the first few characters for each response.

```
node webserver_tests/multiple_get.js
```

### Invalid Request test

Does a PATCH request which is currently not something the server accepts so it should respond with a `400 Bad Request` error.

```
node webserver_tests/invalid_request.js
```

### POST test

Does a GET request followed by a POST request to the same URL and prints the first 100 characters of each response.

```
node webserver_tests/post.js
```

Also can be tested through:

```
(echo -en "POST / HTTP/1.1\r\nHost: localhost\r\nConnection: Keep-alive\r\n\r\nPOSTDATA") | nc 127.0.0.1 8888
```
