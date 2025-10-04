
#define RBL_LOG_ENABLE
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
             /* See feature_test_macros(7) */
#include <string.h>
#include <rbl/unittest.h>
#include <rbl/macros.h>
#include <src/runloop/runloop.h>
#include <rbl/logger.h>
#include "io_read.h"
#include "io_write.h"
/**
Tests Socket watcher - WSocket

 Creates two threads, one will write and one will read.

 Writer thread
    initiate a timer for each of the fd - this will start the folloing repeating sequence for each fd

    wait for timer to expire
    wait for fd to become writable
    write a message to fd
    set timer to start cycle again

Reader thread

    setup a POLLIN fd watcher for each of the FDs

    wait for fd to become readable
    read and print message

  @TODO - would be worth seeing how to d this in a single thread

 */
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
    int max_io = 5;

    Reader* rdr = Reader_new();
    Reader_add_fd(rdr, pipe_1[0], max_io);
    Reader_add_fd(rdr, pipe_2[0], max_io);
    RBL_LOG_FMT("read fd %d %d \n", pipe_1[0], pipe_2[0]);
    RBL_LOG_FMT("write fd %d %d \n", pipe_1[1], pipe_2[1]);
    pthread_t rdr_thread;
    Writer* wrtr = Writer_new();
    Writer_add_fd(wrtr, pipe_1[1], max_io, 500);
    Writer_add_fd(wrtr, pipe_2[1], max_io, 500);
    pthread_t wrtr_thread;

    int r_rdr = pthread_create(&rdr_thread, NULL, reader_thread_func, (void*)rdr);
    int r_wrtr = pthread_create(&wrtr_thread, NULL, writer_thread_func, (void*)wrtr);

    pthread_join(rdr_thread, NULL);
    pthread_join(wrtr_thread, NULL);
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
