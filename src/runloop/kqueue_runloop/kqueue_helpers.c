#include <kqueue_runloop/runloop_internal.h>
#include <sys/event.h>

//////////////////////////////////////////////////////////////////////////////
/// fd reader & writer
///////////////////////////////////////////////////////////////////////////////
int kqh_readerwriter_register(RunloopEventRef rlevent)
{
    struct kevent change[2];
    int nev;
    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)rlevent->stream.fd;
        RunloopRef rl = rlevent->runloop;
        EV_SET(&change[0], id, EVFILT_READ, EV_ADD | EV_ENABLE | EV_DISPATCH | EV_RECEIPT, 0, 0, rlevent);
        EV_SET(&change[1], id, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_DISPATCH | EV_RECEIPT, 0, 0, rlevent);
        nev = kevent(rl->kqueue_fd, change, 2, NULL, 0, NULL);
    #endif

    // check the data field of both change and event
    return 0;
}
int kqh_readerwriter_cancel(RunloopEventRef rlevent)
{
    int flags = EV_DELETE | EV_RECEIPT; 
    struct kevent change[2];
    int nev;

    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)rlevent->stream.fd;
        RunloopRef rl = rlevent->runloop;
        EV_SET(&change[0], id, EVFILT_READ, EV_DELETE | EV_RECEIPT, 0, 0, 0);
        EV_SET(&change[1], id, EVFILT_WRITE, EV_DELETE | EV_RECEIPT, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, change, 2, NULL, 0, NULL);
    #endif

    return 0;
}
int kqh_readerwriter_pause_reader(RunloopEventRef rlevent)
{
    int flags = EV_ADD | EV_DISABLE | EV_RECEIPT ; 
    struct kevent change[2];
    int nev;

    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)rlevent->stream.fd;
        RunloopRef rl = rlevent->runloop;
        EV_SET(&change[0], id, EVFILT_READ, EV_ADD | EV_DISABLE | EV_RECEIPT, 0, 0, 0);
        EV_SET(&change[1], id, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_RECEIPT, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, change, 2, NULL, 0, NULL);
    #endif

    return 0;
}
int kqh_readerwriter_pause(RunloopEventRef rlevent)
{
    int flags = EV_ADD | EV_DISABLE | EV_RECEIPT ; 
    struct kevent change[2];
    int nev;

    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)rlevent->stream.fd;
        RunloopRef rl = rlevent->runloop;
        EV_SET(&change[0], id, EVFILT_READ, EV_ADD | EV_ENABLE | EV_RECEIPT, 0, 0, 0);
        EV_SET(&change[1], id, EVFILT_WRITE, EV_ADD | EV_DISABLE | EV_RECEIPT, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, change, 2, NULL, 0, NULL);
    #endif

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// fd writer
///////////////////////////////////////////////////////////////////////////////
int kqh_writer_register(RunloopEventRef rlevent)
{
    int flags = EV_ADD | EV_ENABLE |EV_DISPATCH | EV_RECEIPT; 
    struct kevent change;
    struct kevent* change_ptr;
    int nev;
    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)rlevent->stream.fd;
        RunloopRef rl = rlevent->runloop;
        change_ptr = &change;
        EV_SET(&change, id, EVFILT_WRITE, flags, 0, 0, rlevent);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    // check the data field of both change and event
    return 0;
}
int kqh_writer_cancel(RunloopEventRef rlevent)
{
    int flags = EV_DELETE | EV_RECEIPT; 
    struct kevent change;
    int nev;
    // int flags = EV_DELETE | EV_RECEIPT; 
    // EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
    // nev = kevent(kq, &change, 1, NULL, 0, NULL);

    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)rlevent->stream.fd;
        RunloopRef rl = rlevent->runloop;
        EV_SET(&change, id, EVFILT_WRITE, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}
int kqh_writer_pause(RunloopEventRef rlevent)
{
    int flags = EV_ADD | EV_DISABLE | EV_RECEIPT ; 
    struct kevent change;
    int nev;
    // int flags = EV_DELETE | EV_RECEIPT; 
    // EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
    // nev = kevent(kq, &change, 1, NULL, 0, NULL);

    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)rlevent->stream.fd;
        RunloopRef rl = rlevent->runloop;
        EV_SET(&change, id, EVFILT_WRITE, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// fd reader
///////////////////////////////////////////////////////////////////////////////
int kqh_reader_register(RunloopEventRef rlevent)
{
    int flags = EV_ADD | EV_ENABLE | EV_RECEIPT | EV_DISPATCH; 
    struct kevent change;
    int nev;
    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)rlevent->stream.fd;
        RunloopRef rl = rlevent->runloop;
        EV_SET(&change, id, EVFILT_READ, EV_ADD | EV_ENABLE | EV_RECEIPT, 0, 0, rlevent);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    // check the data field of both change and event
    return 0;
}
int kqh_reader_cancel(RunloopEventRef rlevent)
{
    int flags = EV_DELETE | EV_RECEIPT; 
    struct kevent change;
    int nev;
    // int flags = EV_DELETE | EV_RECEIPT; 
    // EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
    // nev = kevent(kq, &change, 1, NULL, 0, NULL);

    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)rlevent->stream.fd;
        RunloopRef rl = rlevent->runloop;
        EV_SET(&change, id, EVFILT_READ, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}
int kqh_reader_pause(RunloopEventRef rlevent)
{
    int flags = EV_ADD | EV_DISABLE | EV_RECEIPT ; 
    struct kevent change;
    int nev;
    // int flags = EV_DELETE | EV_RECEIPT; 
    // EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
    // nev = kevent(kq, &change, 1, NULL, 0, NULL);

    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)rlevent->stream.fd;
        RunloopRef rl = rlevent->runloop;
        EV_SET(&change, id, EVFILT_READ, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// signal
///////////////////////////////////////////////////////////////////////////////
int kqh_signal_register(RunloopEventRef rlevent)
{
    int flags = EV_ADD | EV_ENABLE | EV_RECEIPT ; 
    struct kevent change;
    int nev;
    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)rlevent;
        RunloopRef rl = rlevent->runloop;
        EV_SET(&change, id, EVFILT_SIGNAL, flags, 0, 0, rlevent);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    // check the data field of both change and event
    return 0;
}
int kqh_signal_cancel(RunloopEventRef rlevent)
{
    int flags = EV_DELETE | EV_RECEIPT; 
    struct kevent change;
    int nev;
    // int flags = EV_DELETE | EV_RECEIPT; 
    // EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
    // nev = kevent(kq, &change, 1, NULL, 0, NULL);

    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)rlevent;
        RunloopRef rl = rlevent->runloop;
        EV_SET(&change, id, EVFILT_SIGNAL, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}
int kqh_signal_pause(RunloopEventRef rlevent)
{
    int flags = EV_ADD | EV_DISABLE | EV_RECEIPT ; 
    struct kevent change;
    int nev;
    // int flags = EV_DELETE | EV_RECEIPT; 
    // EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
    // nev = kevent(kq, &change, 1, NULL, 0, NULL);

    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)rlevent;
        RunloopRef rl = rlevent->runloop;
        EV_SET(&change, id, EVFILT_SIGNAL, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// timers
///////////////////////////////////////////////////////////////////////////////
int kqh_timer_register(RunloopEventRef rlevent, bool one_shot, uint64_t milli_secs)
{
    int flags = EV_ADD | EV_ENABLE | EV_RECEIPT | (one_shot ? EV_ONESHOT : 0); 
    struct kevent change;
    int nev;
    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)rlevent;
        RunloopRef rl = rlevent->runloop;
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, rlevent);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    // check the data field of both change and event
    return 0;
}
int kqh_timer_cancel(RunloopEventRef rlevent)
{
    int flags = EV_DELETE | EV_RECEIPT; 
    struct kevent change;
    int nev;
    // int flags = EV_DELETE | EV_RECEIPT; 
    // EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
    // nev = kevent(kq, &change, 1, NULL, 0, NULL);

    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)rlevent;
        RunloopRef rl = rlevent->runloop;
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}
int kqh_timer_pause(RunloopEventRef rlevent)
{
    int flags = EV_ADD | EV_DISABLE | EV_RECEIPT ; 
    struct kevent change;
    int nev;
    // int flags = EV_DELETE | EV_RECEIPT; 
    // EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
    // nev = kevent(kq, &change, 1, NULL, 0, NULL);

    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)rlevent;
        RunloopRef rl = rlevent->runloop;
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// user events
///////////////////////////////////////////////////////////////////////////////

int kqh_user_event_register(RunloopEventRef rlevent)
{
    int flags = EV_ADD | EV_ENABLE | EV_RECEIPT;
    struct kevent change;
    int nev;
#ifdef RL_KQ_BATCH_CHANGES
    change_ptr = runloop_change_next(rl)
    EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
#else
    uint64_t id = (uint64_t)rlevent;
    RunloopRef rl = rlevent->runloop;
    EV_SET(&change, id, EVFILT_USER, flags, 0, 0, rlevent);
    nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
#endif
    return 0;
}
int kqh_user_event_trigger(RunloopEventRef rlevent, void* data)
{
    int flags = EV_ADD | EV_ENABLE | EV_RECEIPT;
    struct kevent change;
    int nev;
#ifdef RL_KQ_BATCH_CHANGES
    change_ptr = runloop_change_next(rl)
    EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
#else
    uint64_t id = (uint64_t)rlevent;
    RunloopRef rl = rlevent->runloop;
    EV_SET(&change, id, EVFILT_USER, flags, NOTE_TRIGGER, (intptr_t)data, rlevent);
    nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
#endif
    return 0;
}
int kqh_user_event_cancel(RunloopEventRef rlevent)
{
    int flags = EV_DELETE | EV_RECEIPT;
    struct kevent change;
    int nev;
    // int flags = EV_DELETE | EV_RECEIPT;
    // EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
    // nev = kevent(kq, &change, 1, NULL, 0, NULL);

#ifdef RL_KQ_BATCH_CHANGES
    change_ptr = runloop_change_next(rl)
    EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
#else
    uint64_t id = (uint64_t)rlevent;
    RunloopRef rl = rlevent->runloop;
    EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
    nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
#endif

    return 0;
}
int kqh_user_event_pause (RunloopEventRef rlevent)
{
    int flags = EV_ADD | EV_DISABLE | EV_RECEIPT ;
    struct kevent change;
    int nev;
    // int flags = EV_DELETE | EV_RECEIPT;
    // EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
    // nev = kevent(kq, &change, 1, NULL, 0, NULL);

#ifdef RL_KQ_BATCH_CHANGES
    change_ptr = runloop_change_next(rl)
    EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
#else
    uint64_t id = (uint64_t)rlevent;
    RunloopRef rl = rlevent->runloop;
    EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
    nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
#endif

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// listener
///////////////////////////////////////////////////////////////////////////////
int kqh_listener_register(RunloopEventRef rlevent)
{
    int flags = EV_ADD | EV_ENABLE | EV_RECEIPT;
    struct kevent change;
    int nev;
#ifdef RL_KQ_BATCH_CHANGES
    change_ptr = runloop_change_next(rl)
    EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
#else
    int fd = rlevent->listener.fd;
    EV_SET(&change, fd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_RECEIPT | EV_DISPATCH, 0, 0, rlevent);
    nev = kevent(rlevent->runloop->kqueue_fd, &change, 1, NULL, 0, NULL);
#endif
    return 0;
}
int kqh_listener_cancel(RunloopEventRef rlevent)
{
    int flags = EV_DELETE | EV_RECEIPT;
    struct kevent change;
    int nev;
    // int flags = EV_DELETE | EV_RECEIPT;
    // EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
    // nev = kevent(kq, &change, 1, NULL, 0, NULL);

#ifdef RL_KQ_BATCH_CHANGES
    change_ptr = runloop_change_next(rl)
    EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
#else
    int fd = rlevent->listener.fd;
    EV_SET(&change, fd, EVFILT_TIMER, flags, 0, 0, 0);
    nev = kevent(rlevent->runloop->kqueue_fd, &change, 1, NULL, 0, NULL);
#endif

    return 0;
}
int kqh_listener_pause (RunloopEventRef rlevent)
{
    int flags = EV_ADD | EV_DISABLE | EV_RECEIPT ;
    struct kevent change;
    int nev;
    // int flags = EV_DELETE | EV_RECEIPT;
    // EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
    // nev = kevent(kq, &change, 1, NULL, 0, NULL);

#ifdef RL_KQ_BATCH_CHANGES
    change_ptr = runloop_change_next(rl)
    EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
#else
    int fd = rlevent->listener.fd;
    EV_SET(&change, fd, EVFILT_READ, flags, 0, 0, 0);
    nev = kevent(rlevent->runloop->kqueue_fd, &change, 1, NULL, 0, NULL);
#endif

    return 0;
}
