
#define CHLOG_ON
#include <src//async//connection_internal.h>
/////////////////////////////////////////////////////////////////////////////////////
// cleanup sequence - functions called when connection is terminating
/////////////////////////////////////////////////////////////////////////////////////

/**
 * This must be the last async connection function to run and it should only run once.
 */
void async_postable_cleanup(RunloopRef reactor, void* cref)
{
    AsyncConnectionRef connection_ref = cref;
    RBL_LOG_FMT("postable_cleanup entered");
    RBL_ASSERT((connection_ref->cleanup_done_flag == false), "cleanup should not run more than once");
    RBL_CHECK_TAG(AsyncConnection_TAG, connection_ref)
    runloop_stream_deregister(connection_ref->socket_stream_ref);
    connection_ref->handler_ref->handle_close_connection(connection_ref->handler_ref);
}
