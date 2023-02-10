
#define CHLOG_ON
#include <http_in_c//async//connection_internal.h>
/////////////////////////////////////////////////////////////////////////////////////
// cleanup sequence - functions called when connection is terminating
/////////////////////////////////////////////////////////////////////////////////////

/**
 * This must be the last async connection function to run and it should only run once.
 */
void postable_cleanup(ReactorRef reactor, void* cref)
{
    AsyncConnectionRef connection_ref = cref;
    LOG_FMT("postable_cleanup entered");
    CHTTP_ASSERT((connection_ref->cleanup_done_flag == false), "cleanup should not run more than once");
    CHECK_TAG(AsyncConnection_TAG, connection_ref)
    rtor_stream_deregister(connection_ref->socket_stream_ref);
    connection_ref->handler_ref->handle_connection_done(connection_ref->handler_ref);
}
