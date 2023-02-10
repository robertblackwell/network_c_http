
#define CHLOG_ON
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
             /* See feature_test_macros(7) */
#include <string.h>
#include <http_in_c/unittest.h>
#include <http_in_c/runloop/runloop.h>
#include <http_in_c/logger.h>
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


int test_io()
{
    // create two pipes
    int pipe_1[2]; pipe(pipe_1);
    int pipe_2[2]; pipe(pipe_2);
    int max_io = 5;

    Reader* rdr = Reader_new();
    Reader_add_fd(rdr, pipe_1[0], max_io);
    Reader_add_fd(rdr, pipe_2[0], max_io);
    LOG_FMT("read fd %d %d \n", pipe_1[0], pipe_2[0]);
    LOG_FMT("write fd %d %d \n", pipe_1[1], pipe_2[1]);
    pthread_t rdr_thread;
    Writer* wrtr = Writer_new();
    Writer_add_fd(wrtr, pipe_1[1], max_io, 500);
    Writer_add_fd(wrtr, pipe_2[1], max_io, 500);
    pthread_t wrtr_thread;

    int r_rdr = pthread_create(&rdr_thread, NULL, reader_thread_func, (void*)rdr);
    int r_wrtr = pthread_create(&wrtr_thread, NULL, writer_thread_func, (void*)wrtr);

    pthread_join(rdr_thread, NULL);
    pthread_join(wrtr_thread, NULL);
    LOG_MSG("Trace after join\n\n")
    printf("After join\n");
    return 0;
}

int main()
{
    UT_ADD(test_io);
    int rc = UT_RUN();
    return rc;
}
