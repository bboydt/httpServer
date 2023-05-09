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