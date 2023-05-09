#ifndef HTTP_H
#define HTTP_H

#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// Defines
//

#define HTTP_URL_LEN 128
#define HTTP_URL_PATH_LEN 64
#define HTTP_URL_QUERY_LEN 64
#define HTTP_MAX_URL_PROCS 8

// Types
//

typedef struct http_status_t http_status_t;
typedef struct http_request_t http_request_t;
typedef struct http_url_proc_node_t http_url_proc_node_t;
typedef struct http_server_t http_server_t;

enum
{
    HTTP_REQUEST_GET,
    HTTP_REQUEST_POST,
    HTTP_REQUEST_OTHER
};

struct http_status_t
{
    int code;
    const char *phrase; 
};

struct http_request_t
{
    int client_sock;
    int method; // GET, POST, etc.
    char ip[INET6_ADDRSTRLEN];
    char path[HTTP_URL_PATH_LEN];
    char query[HTTP_URL_QUERY_LEN];
};

typedef int (http_url_proc_t)(struct http_server_t*, struct http_request_t*);

struct http_url_proc_node_t
{
    http_url_proc_t *proc;
    char pattern[HTTP_URL_LEN]; 
};

struct http_server_t
{
    int port;
    struct sockaddr_in addr;

    http_url_proc_node_t url_procs[HTTP_MAX_URL_PROCS];
    int url_proc_count;

    pthread_t listen_thread;

    int sock;
};

// Interface
//

extern void http_init(http_server_t *server, int port);
extern int http_start(http_server_t *server);
extern void http_stop(http_server_t *server);
extern int http_add_url_handler(http_server_t *server, const char *pattern, http_url_proc_t *proc);
extern int http_write_status(http_server_t *server, http_request_t *request, int code, const char *phrase);
extern int http_write_header_field(http_server_t *server, http_request_t *request, const char *field, const char *value);
extern int http_write_body(http_server_t *server, http_request_t *request, const char *body, size_t length);
extern int http_finish_request(http_server_t *server, http_request_t *request);

#ifdef __cplusplus
}
#endif

#endif // HTTP_H