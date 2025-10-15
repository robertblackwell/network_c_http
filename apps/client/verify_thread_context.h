#ifndef C_HTTP_Demo_VERIFY_THREAD_CONTEXT_H
#define C_HTTP_Demo_VERIFY_THREAD_CONTEXT_H
#include <src/common/verify_statistics.h>

struct VerifyThreadContext_s {
    char* url;
    int port;
    int max_rountrips_per_connection;
    int rountrips_count;
    int max_connections_per_thread;
    int ident;
    int roundtrip_per_connection_counter;
    int total_roundtrips;
    ResponseTimesArrayRef response_times_ref;
    char uid[100];
};

typedef struct VerifyThreadContext_s VerifyThreadContext, *VerifyThreadContextRef;

VerifyThreadContext* verify_ctx_new(int port, int id, int max_roundtrips, int max_connections_per_thread, int max_threads);
void verify_ctx_init(VerifyThreadContextRef ctx, int port, int id, int max_roundtrips, int max_connections_per_thread, int max_threads);
void verify_ctx_increment_total_round_trip_count(VerifyThreadContextRef ctx);
void verify_ctx_reset_connection_round_trip_count(VerifyThreadContextRef ctx);
void verify_ctx_increment_connection_round_trip_count(VerifyThreadContextRef ctx);
#endif //C_HTTP_DEMO_VERIFY_THREAD_CONTEXT_H
