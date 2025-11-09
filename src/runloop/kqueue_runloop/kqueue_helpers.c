#include "runloop_internal.h"
#include <sys/event.h>

//////////////////////////////////////////////////////////////////////////////
/// fd reader & writer
///////////////////////////////////////////////////////////////////////////////
int kqh_readerwriter_register(RunloopStreamRef stream)
{
    struct kevent change[2];
    int nev;
    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)stream->fd;
        RunloopRef rl = stream->runloop;
        EV_SET(&change[0], id, EVFILT_READ, EV_ADD | EV_ENABLE | EV_DISPATCH | EV_RECEIPT, 0, 0, stream);
        EV_SET(&change[1], id, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_DISPATCH | EV_RECEIPT, 0, 0, stream);
        nev = kevent(rl->kqueue_fd, change, 2, NULL, 0, NULL);
    #endif

    // check the data field of both change and event
    return 0;
}
int kqh_readerwriter_cancel(RunloopStreamRef stream)
{
    int flags = EV_DELETE | EV_RECEIPT; 
    struct kevent change[2];
    int nev;

    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)stream->fd;
        RunloopRef rl = stream->runloop;
        EV_SET(&change[0], id, EVFILT_READ, EV_DELETE | EV_RECEIPT, 0, 0, 0);
        EV_SET(&change[1], id, EVFILT_WRITE, EV_DELETE | EV_RECEIPT, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, change, 2, NULL, 0, NULL);
    #endif

    return 0;
}
int kqh_readerwriter_pause_reader(RunloopStreamRef stream)
{
    int flags = EV_ADD | EV_DISABLE | EV_RECEIPT ; 
    struct kevent change[2];
    int nev;

    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)stream->fd;
        RunloopRef rl = stream->runloop;
        EV_SET(&change[0], id, EVFILT_READ, EV_ADD | EV_DISABLE | EV_RECEIPT, 0, 0, 0);
        EV_SET(&change[1], id, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_RECEIPT, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, change, 2, NULL, 0, NULL);
    #endif

    return 0;
}
int kqh_readerwriter_pause(RunloopStreamRef stream)
{
    int flags = EV_ADD | EV_DISABLE | EV_RECEIPT ; 
    struct kevent change[2];
    int nev;

    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)stream->fd;
        RunloopRef rl = stream->runloop;
        EV_SET(&change[0], id, EVFILT_READ, EV_ADD | EV_ENABLE | EV_RECEIPT, 0, 0, 0);
        EV_SET(&change[1], id, EVFILT_WRITE, EV_ADD | EV_DISABLE | EV_RECEIPT, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, change, 2, NULL, 0, NULL);
    #endif

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// fd writer
///////////////////////////////////////////////////////////////////////////////
int kqh_writer_register(RunloopStreamRef stream)
{
    int flags = EV_ADD | EV_ENABLE |EV_DISPATCH | EV_RECEIPT; 
    struct kevent change;
    struct kevent* change_ptr;
    int nev;
    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)stream->fd;
        RunloopRef rl = stream->runloop;
        change_ptr = &change;
        EV_SET(&change, id, EVFILT_WRITE, flags, 0, 0, stream);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    // check the data field of both change and event
    return 0;
}
int kqh_writer_cancel(RunloopStreamRef stream)
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
        uint64_t id = (uint64_t)stream->fd;
        RunloopRef rl = stream->runloop;
        EV_SET(&change, id, EVFILT_WRITE, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}
int kqh_writer_pause(RunloopStreamRef stream)
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
        uint64_t id = (uint64_t)stream->fd;
        RunloopRef rl = stream->runloop;
        EV_SET(&change, id, EVFILT_WRITE, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// fd reader
///////////////////////////////////////////////////////////////////////////////
int kqh_reader_register(RunloopStreamRef stream)
{
    int flags = EV_ADD | EV_ENABLE | EV_RECEIPT | EV_DISPATCH; 
    struct kevent change;
    int nev;
    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)stream->fd;
        RunloopRef rl = stream->runloop;
        EV_SET(&change, id, EVFILT_READ, EV_ADD | EV_ENABLE | EV_RECEIPT, 0, 0, stream);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    // check the data field of both change and event
    return 0;
}
int kqh_reader_cancel(RunloopStreamRef stream)
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
        uint64_t id = (uint64_t)stream->fd;
        RunloopRef rl = stream->runloop;
        EV_SET(&change, id, EVFILT_READ, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}
int kqh_reader_pause(RunloopStreamRef stream)
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
        uint64_t id = (uint64_t)stream->fd;
        RunloopRef rl = stream->runloop;
        EV_SET(&change, id, EVFILT_READ, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// signal
///////////////////////////////////////////////////////////////////////////////
int kqh_signal_register(RunloopSignalRef signal)
{
    int flags = EV_ADD | EV_ENABLE | EV_RECEIPT ; 
    struct kevent change;
    int nev;
    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)signal;
        RunloopRef rl = signal->runloop;
        EV_SET(&change, id, EVFILT_SIGNAL, flags, 0, 0, signal);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    // check the data field of both change and event
    return 0;
}
int kqh_signal_cancel(RunloopSignalRef signal)
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
        uint64_t id = (uint64_t)signal;
        RunloopRef rl = signal->runloop;
        EV_SET(&change, id, EVFILT_SIGNAL, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}
int kqh_signal_pause(RunloopSignalRef signal)
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
        uint64_t id = (uint64_t)signal;
        RunloopRef rl = signal->runloop;
        EV_SET(&change, id, EVFILT_SIGNAL, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// timers
///////////////////////////////////////////////////////////////////////////////
int kqh_timer_register(RunloopTimerRef timer, bool one_shot, uint64_t milli_secs)
{
    int flags = EV_ADD | EV_ENABLE | EV_RECEIPT | (one_shot ? EV_ONESHOT : 0); 
    struct kevent change;
    int nev;
    #ifdef RL_KQ_BATCH_CHANGES
        change_ptr = runloop_change_next(rl)
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
    #else
        uint64_t id = (uint64_t)timer;
        RunloopRef rl = timer->runloop;
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, timer);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    // check the data field of both change and event
    return 0;
}
int kqh_timer_cancel(RunloopTimerRef timer)
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
        uint64_t id = (uint64_t)timer;
        RunloopRef rl = timer->runloop;
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}
int kqh_timer_pause(RunloopTimerRef timer)
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
        uint64_t id = (uint64_t)timer;
        RunloopRef rl = timer->runloop;
        EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
        nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
    #endif

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// user events
///////////////////////////////////////////////////////////////////////////////

int kqh_user_event_arm(RunloopUserEventRef uevent)
{
    int flags = EV_ADD | EV_ENABLE | EV_RECEIPT | EV_DISPATCH | EV_ONESHOT;
    struct kevent change;
    int nev;
#ifdef RL_KQ_BATCH_CHANGES
    change_ptr = runloop_change_next(rl)
    EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
#else
    uint64_t id = (uint64_t)uevent;
    RunloopRef rl = uevent->runloop;
    EV_SET(&change, id, EVFILT_USER, flags, 0, 0, uevent);
    nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
#endif
    return 0;
}
int kqh_user_event_trigger(RunloopUserEventRef uevent, void* data)
{
    int flags = EV_ADD | EV_ENABLE | EV_RECEIPT;
    struct kevent change;
    int nev;
#ifdef RL_KQ_BATCH_CHANGES
    change_ptr = runloop_change_next(rl)
    EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
#else
    uint64_t id = (uint64_t)uevent;
    RunloopRef rl = uevent->runloop;
    int kqfd = (uevent->dup_fd == -1) ? uevent->runloop->kqueue_fd : uevent->dup_fd;
    EV_SET(&change, id, EVFILT_USER, flags, NOTE_TRIGGER, (intptr_t)data, uevent);
    nev = kevent(kqfd, &change, 1, NULL, 0, NULL);
#endif
    return 0;
}
int kqh_user_event_cancel(RunloopUserEventRef uevent)
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
    uint64_t id = (uint64_t)uevent;
    RunloopRef rl = uevent->runloop;
    EV_SET(&change, id, EVFILT_TIMER, flags, 0, 0, 0);
    nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
#endif

    return 0;
}
int kqh_user_event_pause (RunloopUserEventRef uevent)
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
    uint64_t id = (uint64_t)uevent;
    RunloopRef rl = uevent->runloop;
    EV_SET(&change, id, EVFILT_USER, flags, 0, 0, 0);
    nev = kevent(rl->kqueue_fd, &change, 1, NULL, 0, NULL);
#endif

    return 0;
}

//////////////////////////////////////////////////////////////////////////////
/// listener
///////////////////////////////////////////////////////////////////////////////
int kqh_listener_register(RunloopListenerRef listener)
{
    int flags = EV_ADD | EV_ENABLE | EV_RECEIPT;
    struct kevent change;
    int nev;
#ifdef RL_KQ_BATCH_CHANGES
    change_ptr = runloop_change_next(rl)
    EV_SET(&change, id, EVFILT_TIMER, flags, 0, milli_secs, 0);
#else
    int fd = listener->fd;
    EV_SET(&change, fd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_RECEIPT | EV_DISPATCH, 0, 0, listener);
    nev = kevent(listener->runloop->kqueue_fd, &change, 1, NULL, 0, NULL);
#endif
    return 0;
}
int kqh_listener_cancel(RunloopListenerRef listener)
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
    int fd = listener->fd;
    EV_SET(&change, fd, EVFILT_TIMER, flags, 0, 0, 0);
    nev = kevent(listener->runloop->kqueue_fd, &change, 1, NULL, 0, NULL);
#endif

    return 0;
}
int kqh_listener_pause (RunloopListenerRef listener)
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
    int fd = listener->fd;
    EV_SET(&change, fd, EVFILT_READ, flags, 0, 0, 0);
    nev = kevent(listener->runloop->kqueue_fd, &change, 1, NULL, 0, NULL);
#endif

    return 0;
}
