# httpServer

[![License: MIT](https://img.shields.io/badge/license-MIT-blue)](https://opensource.org/licenses/MIT)
![Build Status for macOS](https://app.travis-ci.com/bboydt/httpServer.svg?branch=master)


httpServer - a small and simple HTTP server written in C.

## Library

The point of this is to handle simple HTTP requests in a small C library.  
This project is inspired by [wsServer](https://github.com/Theldus/wsServer).

## Building 

The simple example has been tested on an M2 Macbook Air using LLVM and Safari.
  Compiled with LLVM (see version output below).

```console
$ clang -v
Homebrew clang version 15.0.7
Target: arm64-apple-darwin22.4.0
Thread model: posix
...
```

### How to build

```console
$ git clone https://github.com/bboydt/httpServer
$ cd httpServer
$ make
```

## A complete example

This example opens an HTTP server at http://localhost:8080 and serves a simple landing page.

```c
#include <http.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#define lengthof(x) (sizeof((x)) / sizeof((x)[0]))

static http_server_t server;

int handle_root(http_server_t *server, http_request_t *request)
{
    http_write_status(server, request, 302, "Found");
    http_write_header_field(server, request, "Server", "simple-http-server");
    http_write_header_field(server, request, "Location", "/index.html");
    http_write_header_field(server, request, "Connection", "close");
    write(request->client_sock, "\r\n", 2);

    http_finish_request(server, request);

    return 1;
}

int handle_index(http_server_t *server, http_request_t *request)
{
    char index_html[] = "<html><body><center>Hello sailor!</center></body></html>";
    char buffer[64];

    http_write_status(server, request, 200, "OK");
    http_write_header_field(server, request, "Server", "simple-http-server");
    http_write_header_field(server, request, "Content-Type", "text/html; charset=UTF-8");
    sprintf(buffer, "%lu", lengthof(index_html));
    http_write_header_field(server, request, "Content-Length", buffer);
    http_write_header_field(server, request, "Connection", "close");
    http_write_body(server, request, index_html, lengthof(index_html));

    http_finish_request(server, request);

    return 1;
}

void stop(int signal)
{
    http_stop(&server);
    printf("Goodbye!\n");
    exit(0);
}

int main(int argc, const char **argv)
{
    printf("Running simple HTTP server example...\n");

    http_init(&server, 8080);
    http_add_url_handler(&server, "/", handle_root);
    http_add_url_handler(&server, "/index.html", handle_index);
    http_start(&server);
    signal(SIGINT, stop);

    printf("ctrl-c to stop.\n");
    while (1);

    return 0;
}
```

# Planned Support

## HTTPS
The current use case for this library does not require encryption, but eventually it would be nice to have.
# Contributions

Feel free to make a pull request. 
Follow the style of [http.h](include/http.h) and [http.c](src/http.c).
