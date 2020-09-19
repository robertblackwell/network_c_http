#ifndef c_ceg_server_h
#define c_ceg_server_h

struct Server_s;
typedef struct  Server_s Server, *ServerRef;

ServerRef Server_new(int port);
void Server_free(ServerRef* srefptr);
void Server_listen(ServerRef server);

#endif