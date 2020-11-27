#define _GNU_SOURCE
#include <c_http/xr/xr_worker.h>
#include <c_http/constants.h>
#include <c_http/alloc.h>
#include <c_http/utils.h>
#include <c_http/socket_functions.h>
#include <c_http/xr/types.h>

#include <c_http/xr/evfd_queue.h>
#include <c_http/xr/reactor.h>
#include <c_http/xr/queue_watcher.h>
#include <c_http/xr/socket_watcher.h>
#include <c_http/xr/conn.h>
#include <c_http/xr/conn_list.h>
#include <c_http/ll_parser_types.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/epoll.h>

#include <pthread.h>

#define WRK_MAX_CONN 5

struct XrWorker_s {
    bool              active;
    int               active_socket;
    EvfdQueueRef      qref;
    pthread_t         pthread;
    int               id;
    XrHandlerFunction handler;
    XrReactorRef      reactor_ref;
    XrQueueWatcherRef qwatcher;
    uint              conn_count;
    XrConnListRef     conn_list;
};

void wkrk_state_machine(XrSocketWatcherRef sw, void* arg, uint64_t event)
{
    XrConnectionRef conn_ref = arg;
    printf("XrWorker::wrkr_state_machine fd: %d\n", conn_ref->fd);
    bool pollin = (event | EPOLLIN);
    bool pollout = (event | EPOLLOUT);
    bool pollerr = (event | EPOLLERR);
    bool pollhup = (event | EPOLLHUP);
    bool pollrdhup = (event | EPOLLRDHUP);

    switch(conn_ref->state) {
        case XRCONN_STATE_READ: {
            char* p = IOBuffer_space (conn_ref->io_buf_ref);
            int len = IOBuffer_space_len(conn_ref->io_buf_ref);
            int nread = read(conn_ref->fd, p, len);
            printf("Read %d \n", nread);
        }
        break;

        case XRCONN_STATE_WRITE:
        // process output data
        break;

        case XRCONN_STATE_HANDLE:
            assert(false);
        break;

        case XRCONN_STATE_UNINIT:
            assert(false);
        break;
    }
}
void queue_handler(XrQueueWatcherRef qw, void* ctx, uint64_t event)
{
    XrWorkerRef wrkr = (XrWorkerRef)ctx;
    XrReactorRef rtor = qw->runloop;
    EvfdQueueRef queue = wrkr->qref;
    void* queue_data = Evfdq_remove(queue);
    long fd = (long)queue_data;
    if(fd == -1L) {
        // terminate
        return;
    }
    printf("XrWorker::queue_handler fd: %ld\n", fd);
    XrSocketWatcherRef sw = Xrsw_new (rtor, fd);
//    XrConnectionRef conn_ref = XrConnection_new(fd, sw);
    uint64_t interest = EPOLLERR | EPOLLIN;
//    Xrsw_register(sw, &wkrk_state_machine,(void*)conn_ref, interest);
    printf("Q Handler received %p \n", queue_data);

}

XrWorkerRef XrWorker_new(int _id, XrHandlerFunction handler)
{
    XrWorkerRef wref = (XrWorkerRef)eg_alloc(sizeof(XrWorker));
    if(wref == NULL)
        return NULL;
    wref->active_socket = 0;
    wref->active  = false;
    wref->id = _id;
    wref->qref = Evfdq_new();
    wref->handler = handler;
    wref->reactor_ref = XrReactor_new();
    wref->qwatcher = Xrqw_new(wref->reactor_ref, wref->qref);
    wref->conn_count = 0;
    wref->conn_list = XrConnList_new();
    return wref;
}
void XrWorker_free(XrWorkerRef wref)
{
    Evfdq_free(wref->qref);
    free((void*)wref);
}

void handle_parse_error(MessageRef requestref, void* wrtr)
{
    char* reply = "HTTP/1.1 400 BAD REQUEST \r\nContent-length: 0\r\n\r\n";
//    Writer_write_chunk(wrtr, (void*)reply, strlen(reply));
}
EvfdQueueRef XrWorker_get_queue(XrWorkerRef this)
{
    return this->qref;
}
static void* XrWorker_main(void* data)
{
    ASSERT_NOT_NULL(data);
    XrWorkerRef wref = (XrWorkerRef)data;
    uint64_t interest = EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLHUP;
    Xrqw_register(wref->qwatcher, queue_handler, (void*)wref, interest);
    XrReactor_run(wref->reactor_ref, -1);
    finalize:
    return NULL;
}
// start a pthread - returns 0 on success errno on fila
int XrWorker_start(XrWorkerRef wref)
{
    ASSERT_NOT_NULL(wref);

    int rc = pthread_create(&(wref->pthread), NULL, &(XrWorker_main), (void*) wref);
    if(rc) {
        return rc;
    } else {
        return rc;
    }
}
// void XrWorker_set_pthread(XrWorkerref wref, pthread_t* pthread)
// {
//     wref->pthread = pthread;
// }
pthread_t* XrWorker_pthread(XrWorkerRef wref)
{
    ASSERT_NOT_NULL(wref);
    return &(wref->pthread);
}
void XrWorker_join(XrWorkerRef wref)
{
    int retvalue;
    pthread_join(wref->pthread, NULL);
}
