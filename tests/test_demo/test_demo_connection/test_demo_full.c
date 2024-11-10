
//#define RBL_LOG_ENABLE
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <rbl/unittest.h>
#include <rbl/macros.h>
#include <http_in_c/runloop/runloop.h>
#include <rbl/logger.h>
#include "demo_read.h"
#include "demo_write.h"
void async_socket_set_nonblocking(int socket)
{
    int flags = fcntl(socket, F_GETFL, 0);
    int modFlags2 = flags | O_NONBLOCK;
    int result = fcntl(socket, F_SETFL, modFlags2);
    if(result != 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("set non blocking error socket: %d error %d %s", socket, errno_saved, strerror(errno_saved))
    }
    RBL_ASSERT((result == 0), "set socket non blocking");
}


int test_io()
{
    /**
     * Two threads each thread has a reader and writer that share appropriate ends of a pipe
     */
    // create two pipes
    int pipe_1[2]; pipe(pipe_1);
    int pipe_2[2]; pipe(pipe_2);
    int max_io = 2;

    ReaderTableRef rdr = ReaderTable_new();
    ReaderTable_add_fd(rdr, pipe_1[0], max_io);
    ReaderTable_add_fd(rdr, pipe_2[0], max_io);
    RBL_LOG_FMT("read fd %d %d \n", pipe_1[0], pipe_2[0]);
    RBL_LOG_FMT("write fd %d %d \n", pipe_1[1], pipe_2[1]);
    pthread_t rdr_thread;
    WriterTableRef wrtr = WriterTable_new();
    WriterTable_add_fd(wrtr, pipe_1[1], max_io, 1000);
    WriterTable_add_fd(wrtr, pipe_2[1], max_io, 1000);
    pthread_t wrtr_thread;

    int r_rdr = pthread_create(&rdr_thread, NULL, reader_thread_func, (void*)rdr);
    int r_wrtr = pthread_create(&wrtr_thread, NULL, writer_thread_func, (void*)wrtr);

    pthread_join(rdr_thread, NULL);
    pthread_join(wrtr_thread, NULL);
    UT_TRUE((rdr->ctx_table[0].read_count == 3))
    UT_TRUE((rdr->ctx_table[1].read_count == 3))
    RBL_LOG_MSG("Trace after join\n\n")
    printf("After join\n");
    return 0;
}

int main()
{
    UT_ADD(test_io);
    int rc = UT_RUN();
    return rc;
}
