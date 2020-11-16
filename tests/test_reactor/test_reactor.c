#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/epoll.h>
#include <c_http/list.h>
#include <c_http/operation.h>
#include <c_http/oprlist.h>
#include <c_http/unittest.h>
#include <c_http//utils.h>

typedef struct Ctx1_s {
    char* text;
    bool  flag;
    int counter;
} Ctx1;

void fop_1(void* ctx)
{
    Ctx1* c1 = (Ctx1*)ctx;
    c1->flag = true;
    printf("op_1 : %s\n", c1->text);
}

int test_operation()
{
    Ctx1* p1 = malloc(sizeof(Ctx1));
    p1->text = "This is text for ctx1 test";
    p1->flag = false;
    Operation* op1 = Opr_new(&(fop_1), (void*)p1);
    op1->op(op1->ctx);
    bool x = p1->flag;
    UT_TRUE((x));
    return 0;
}
#define GG
#ifdef GG
typedef struct Ctx2_s {
    char* text;
    bool  flag;
    int counter;
} Ctx2;

void fop_2(void* ctx, int* res)
{
    Ctx2* c2 = (Ctx2*)ctx;
    c2->flag = true;
    *res = c2->counter;
}

int test_multiple_ops()
{

    OprListRef olist = OprList_new();
    for(int i = 0; i < 10; i++) {
        Ctx2* p2 = malloc(sizeof(Ctx2));
        p2->flag = false;
        p2->counter = i;

        void(*f)(void*);
        f = (void(*)(void*))(&fop_2);

        Operation* op2 = Opr_new(f, (void*)p2);
        OprList_add_back(olist, (void*)op2);
    }
    int j = 0;
    OprListIter iter = OprList_iterator(olist);
    while (iter != NULL) {
        Operation* opr_ref = OprList_itr_unpack(olist, iter);
        int k;
        void(*f)(void*, int*);
        f = (void(*)(void*, int*))opr_ref->op;
        f(opr_ref->ctx, &k);
        UT_EQUAL_INT(k, j);
        j++;
        iter = OprList_itr_next(olist, iter);
    }
}
typedef struct Consumer_s {
    pthread_t       pthread;
    int             id;
    pthread_mutex_t queue_mutex;
    OprListRef      oplist;
    int             pipefds[2];
    int             readfd;
    int             writefd;
    int             epfd; // the epoll fd common to producer and all consumers
} Consumer, *ConsumerRef;

void Consumer_init(ConsumerRef this, int id)
{
    this->oplist = OprList_new();
    pthread_mutex_init(&(this->queue_mutex), NULL);
    pipe2(this->pipefds, O_NONBLOCK | O_CLOEXEC);
    this->readfd = this->pipefds[0];
    this->writefd = this->pipefds[1];
    this->epfd = epoll_create1(O_CLOEXEC);
    this->id = id;
}
ConsumerRef Consumer_new(int id)
{
    ConsumerRef tmp = malloc(sizeof(Consumer));
    Consumer_init(tmp, id);
    return tmp;
}

void Consumer_post(ConsumerRef this, Operation* op)
{
    pthread_mutex_lock(&(this->queue_mutex));
    OprList_add_back(this->oplist, op);

    printf("Queue_add: %d\n", OprList_size(this->oplist));
    pthread_mutex_unlock(&(this->queue_mutex));
    char txt[10];
    sprintf(txt, "%d", this->id);
    write(this->writefd, &txt, 1);
}
Operation* Consumer_pop(ConsumerRef this)
{
    pthread_mutex_lock(&(this->queue_mutex));
    Operation* op = OprList_remove_first(this->oplist);
    pthread_mutex_unlock(&(this->queue_mutex));

    printf("Queue_pop: socket is %ld\n", (long)op->ctx);
    return op;
}
static void* Consumer_main(void* data)
{
    int MAX_EVENTS = 100;
    int READ_SIZE = 512;
    int running = 1;
    int event_count, i;
    size_t bytes_read;
    char read_buffer[READ_SIZE + 1];
    ConsumerRef this = (ConsumerRef)data;
    struct epoll_event event, events[10];
    event.events = EPOLLIN;
    event.data.fd = this->readfd;
    if(epoll_ctl(this->epfd, EPOLL_CTL_ADD, this->readfd, &event))
    {
        fprintf(stderr, "Failed to add file descriptor to epoll\n");
        close(this->epfd);
        assert(false);
    }
    while(running) {
        printf("\nPolling for input...\n");
        event_count = epoll_wait(this->epfd, events, MAX_EVENTS, 30000);
        printf("%d ready events\n", event_count);
        for(int i = 0; i < event_count; i++)
        {
            if (events[i].data.fd == this->readfd) {
                printf("Its a queue event\n");
                bytes_read = read(events[i].data.fd, read_buffer, READ_SIZE);
                printf("%zd bytes read.\n", bytes_read);
                read_buffer[bytes_read] = '\0';
                printf("Read '%s'\n", read_buffer);
                Operation* opr = Consumer_pop(this);
            } else {
            }
            if(!strncmp(read_buffer, "stop\n", 5))
                running = 0;

        }
    }

    if(close(this->epfd))
    {
        fprintf(stderr, "Failed to close epoll file descriptor\n");
        assert(false);
    }
    return NULL;
}
int Consumer_start(ConsumerRef cref)
{
    ASSERT_NOT_NULL(cref);

    int rc = pthread_create(&(cref->pthread), NULL, &(Consumer_main), (void*) cref);
    if(rc) {
        return rc;
    } else {
        return rc;
    }
}

void run(ConsumerRef this)
{

}
void consumer(ConsumerRef params)
{
}
#ifdef HGHG
#define MAX_EVENTS_SIZE 5;
typedef struct thread_info {
    pthread_t thread_id;
    int rank;
    int epfd;
} thread_info_t;

static void *consumer_routine(void *data) {
    struct thread_info *c = (struct thread_info *)data;
    struct epoll_event *events;
    int epfd = c->epfd;
    int nfds = -1;
    int i = -1;
    int ret = -1;
    uint64_t v;
    int num_done = 0;

    events = calloc(MAX_EVENTS_SIZE, sizeof(struct epoll_event));
    assert(events != NULL);

    for (;;) {
        nfds = epoll_wait(epfd, events, MAX_EVENTS_SIZE, 1000);
        for (i = 0; i < nfds; i++) {
            if (events[i].events & EPOLLIN) {
                log_debug("[consumer-%d] got event from fd-%d",
                          c->rank, events[i].data.fd);
                ret = read(events[i].data.fd, &v, sizeof(v));
                if (ret < 0) {
                    log_error("[consumer-%d] failed to read eventfd", c->rank);
                    continue;
                }
                close(events[i].data.fd);
                do_task();
                log_debug("[consumer-%d] tasks done: %d", c->rank, ++num_done);
            }
        }
    }
}
#endif
void producer()
{

    for(int i = 0; i < 10; i++) {
        sleep(5);

    }
}

void post_f(void* data)
{
    ConsumerRef c = (ConsumerRef)data;
    int x = (int)(uint64_t) data;
    printf("post_f on consumer %d  socket_fd: %d\n", c->id, x);
}
#define  MAX_CONSUMERS 5
void pc_main()
{
    Consumer consumers[MAX_CONSUMERS];
    pthread_t threads[MAX_CONSUMERS];

    int epfd = epoll_create1(O_CLOEXEC);
    for(int i = 0; i < MAX_CONSUMERS; i++) {
        Consumer_init(&(consumers[i]), i);
        Consumer_start(&(consumers[i]));
    }
    sleep(1);
    uint64_t next_consumer = 0;
    uint64_t sock = 1;
    for(;;) {
        Operation* op = Opr_new(post_f, (void*)sock);
        Consumer_post(&(consumers[next_consumer]), op);
        sleep(2);
        sock++;
        next_consumer++;
        if (next_consumer == MAX_CONSUMERS)
            next_consumer = 0;
    }
}

#endif
int main()
{
    pc_main();
    UT_ADD(test_operation);
    UT_ADD(test_multiple_ops);
//    UT_ADD(test_hdrlist_add_back_get_content);
//    UT_ADD(test_hdrlist_find);
//    UT_ADD(test_hdr_add_many);
//    UT_ADD(test_list_remove_front);
//    UT_ADD(test_list_remove_back);
//    UT_ADD(test_iter);
//    UT_ADD(test_serialize_headers);
//    UT_ADD(test_serialize_headers_2);
    int rc = UT_RUN();
    return rc;
}
