all:
	gcc -g -pthread httpechosrv.c http_errors.c request_handlers.c helper_functions.c -o webserver
clean:
	rm webserver; rm -rf webserver.dSYM