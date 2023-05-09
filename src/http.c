#include <http.h>

#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>

some junk

#define lengthof(x) (sizeof((x)) / sizeof((x)[0]))

void http_init(http_server_t *server, int port)
{
    memset(server, 0, sizeof(http_server_t));

    server->port = port;

    server->addr.sin_family = AF_INET;
    server->addr.sin_port = htons(server->port);
    server->addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

static inline void* listen_proc(void *arg)
{
    http_server_t *server = (http_server_t*)arg;

    // Make socket
    server->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server->sock == -1)
    {
        perror("Failed to create socket");
    }
    {
        bool val = true;
        setsockopt(server->sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));
    }

    if (bind(server->sock, (struct sockaddr*)&server->addr, sizeof(server->addr)) != 0)
    {
        perror("Failed to bind socket");
    }

    // Start listening
    assert(listen(server->sock, SOMAXCONN) != -1);
    printf("Listening for connections...\n");

    struct sockaddr_in addr;
    int addr_len = sizeof(addr), host_addr_len = sizeof(server->addr);
    char buffer[1024];

    while (1)
    {
        http_request_t request;

        // Accept connection
        request.client_sock = accept(server->sock, (struct sockaddr*)&server->addr, (socklen_t*)&host_addr_len);
        if (request.client_sock < 0)
        {
            perror("Failed to accept connection");
            continue;
        }

        // Report connection
        assert(getsockname(request.client_sock, (struct sockaddr *)&addr, (socklen_t *)&addr_len) == 0);
        printf("New connection: [%s:%d]\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        int length = read(request.client_sock, buffer, lengthof(buffer) - 1);
        if (length <= 0)
        {
            perror("Failed to read from socket");
            continue;
        }
        buffer[length] = '\0';

        sscanf(buffer, "%*s %s %*s\r\n", request.path);

        bool handled = false;
        for (int i = 0; i < server->url_proc_count; i++)
        {
            http_url_proc_node_t *proc_node = &server->url_procs[i];
            if (strcmp(request.path, proc_node->pattern) == 0)
            {
                handled = proc_node->proc(server, &request);
                break;
            }
        }

        if (!handled)
        {
            char index_html[] = "<html><body><center>404: Not Found</center></body></html>";
            char length_buffer[64];

            http_write_status(server, &request, 404, "Not Found");
            http_write_header_field(server, &request, "Server", "simple-http-server");
            http_write_header_field(server, &request, "Content-Type", "text/html; charset=UTF-8");
            sprintf(buffer, "%lu", lengthof(index_html));
            http_write_header_field(server, &request, "Content-Length", buffer);
            http_write_header_field(server, &request, "Connection", "close");
            http_write_body(server, &request, index_html, lengthof(index_html));

            http_finish_request(server, &request);
        }
    }
}

int http_start(http_server_t *server)
{
    pthread_create(&server->listen_thread, NULL, listen_proc, server);
    return 1;
}

void http_stop(http_server_t *server)
{
    pthread_cancel(server->listen_thread);
    close(server->sock);
}

int http_add_url_handler(http_server_t *server, const char *pattern, http_url_proc_t *proc)
{
    assert(server->url_proc_count <= HTTP_MAX_URL_PROCS);
    
    http_url_proc_node_t *node = &server->url_procs[server->url_proc_count++];
    
    node->proc = proc;
    strcpy(node->pattern, pattern);

    return 1;
}

int http_write_status(http_server_t *server, http_request_t *request, int code, const char *phrase)
{
    char buffer[64];
    size_t length;
    
    length = sprintf(buffer, "HTTP/1.1 %d %s\r\n", code, phrase);
    assert(length > 0);

    length = write(request->client_sock, buffer, length);
    assert(length > 0);

    return length;
}

int http_write_header_field(http_server_t *server, http_request_t *request, const char *field, const char *value)
{
    char buffer[128];
    size_t length;
    
    length = sprintf(buffer, "%s: %s\r\n", field, value);
    assert(length > 0);

    length = write(request->client_sock, buffer, length);
    assert(length > 0);

    return length;
}

int http_write_body(http_server_t *server, http_request_t *request, const char *body, size_t length)
{
    assert(write(request->client_sock, "\r\n", 2) == 2);

    length = write(request->client_sock, body, length);
    assert(length > 0);
    
    assert(write(request->client_sock, "\0", 1)); // this is kinda dumb @todo make better

    return length;
}

int http_finish_request(http_server_t *server, http_request_t *request)
{
    return close(request->client_sock);
}
