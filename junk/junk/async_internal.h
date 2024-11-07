#ifndef http_in_c_async_async_socket_h
#define http_in_c_async_async_socket_h

int async_create_shareable_socket();
int async_socket_create();
void async_socket_set_nonblocking(int socket);
void async_socket_set_reuseaddr(int socket);
void async_socket_set_reuseport(int socket);
void async_socket_set_nonblocking(int socket);
void async_socket_listen(int socket);
void async_socket_bind(int socket, int port, const char* host);
int async_bind_and_listen_socket(int socket, int port, const char *host);
#endif