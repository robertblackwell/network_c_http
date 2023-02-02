#ifndef c_http_sync_server_h
#define c_http_sync_server_h

#include <c_http/saved/sync_handler.h>


typedef struct  SyncServer_s Server, *SyncServerRef;
SyncServerRef SyncServer_new(int port, size_t read_buffer_size, int nbr_threads, SyncAppMessageHandler app_handler);
void SyncServer_dispose(SyncServerRef* srefptr);
void SyncServer_listen(SyncServerRef server);
void SyncServer_terminate(SyncServerRef this);

/** @} */
#endif