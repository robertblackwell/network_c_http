#ifndef C_HTTP_EPOLL_RUNLOOP_H
#define C_HTTP_EPOLL_RUNLOOP_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
/** \defgroup runloop Runloop
 * @{
 * ## Runloop
 * A Runloop is a device that:
 * -    uses Linux epoll to allow client code to watch for events on file descriptors, and
 * -    to schedule callback functions (via runloop_post())  to be run at some point in the future
 *
 * The key feature of the device is that many file descriptors can be monitored simultaiously,
 * and many callbacks can be waiting for execution. In this regard a reactor is like
 * a lightweight task scheduler
 *
 * To watch or observe a file descriptor for events an object of type RunloopWatcherBase (or derived from RunloopWatcherBase)
 * must be created and passed to the reactor.
 *
 * The RunloopWatcherBase holds at least 2 pieces of information:
 * -    a function to be called when an event of interest happens
 * -    optionally a context pointer for the callback function
 * in this sense the various watchers are generalizations of a callback closure
 *
 * For convenience a number of special purposes watchers/observers have been provided.
 */
#pragma "Defining RunloopRef"
struct Runloop_s;
typedef struct Runloop_s Runloop, *RunloopRef;

struct RunloopWatcherBase_s;
/**
 * There are 5 types of events that can be waited for. They are:
 * -    timer, wait for it to expire
 * -    socket/pipe,  ready for read, ready for write
 * -    socket, ready for accept() call
 * -    a linux eventfd, wait for the fd to be triggered
 * -    interthread queue, wait for an entry to be added
 *
 * To use each type of event a specific type of opaque object must be created in order to perform such an event wait.
 *
 * There are some common elements shared between these 5 types of events and those
 * common elements are represented by the type RunloopWatcherBase.
 *
 * In other languages RunloopWatcherBase would be the base class with the other event objects inheriting from
 * this base object.
 *
* Since C does not have inheritence the approach adopted is embedding. Thus a subclass is defined as follows:
*
*```
* typedef struct SubClassOfEvent_s {
    *      RunloopWatcherBase;
    *      .....
    *      other fields as required for the specific subclass
            *      .....
    * } SubClassOfEvent, *SubClassOfEventRef;
*```
* This means that a pointer to a SubClassOfEvent is also a pointer to a RunloopWatcherBase instance.
*
* For this to work the code must be compiled with the microsoft extension __-fms-extensions__.
*/
typedef struct RunloopWatcherBase_s RunloopWatcherBase, *RunloopWatcherBaseRef;       // Base object for objects that wait for an fd event

struct Functor_s;
typedef struct Functor_s Functor;

typedef uint64_t EventMask, RunloopTimerEvent;

/**
 * PostableFunction defines the call signature of functions that can be added to a runloops queue of
 * functions to be called. As such they represent the next step in an ongoing computation of a lightweight
 * "thread".
 */
typedef void (*PostableFunction) (RunloopRef runloop_ref, void* arg);

typedef void(*AsioReadcallback)(void* arg, long length, int error_number);
typedef void(*AsioWritecallback)(void* arg, long length, int error_number);
RunloopRef runloop_get_threads_reactor();
/**
 * Create a new instance of a Runloop.
 *
 * @return a pointer to a dynamically allocated Runloop object that has been initialized.
 */
RunloopRef runloop_new(void);
/**
 * Free the memory associated with an instance of a Runloop object, including any associated other objects
 * that are owned by the subject Runloop.
 * @param athis RunloopRef
 * @throws in athis is NULL
 */
void       runloop_free(RunloopRef athis);

/**
 * Initializes the memory pointed to by __athis__ to be a valid instance of a Runloop.
 *
 * @param athis Must point to a memory area of sufficient size.
 * @throws if athis is NULL
 */
void       runloop_init(RunloopRef athis);
/**
 * Frees the memory of all objects that the Runloop pointed to by athis holds ownership of.
 * @param athis a valid non NULL RunloopRef
 * @throws if athis is NULL
 */
void       runloop_deinit(RunloopRef athis);

void       runloop_close(RunloopRef athis);
int        runloop_register(RunloopRef athis, int fd, uint32_t interest, RunloopWatcherBaseRef wref);
int        runloop_deregister(RunloopRef athis, int fd);
int        runloop_reregister(RunloopRef athis, int fd, uint32_t interest, RunloopWatcherBaseRef wref);
int        runloop_run(RunloopRef athis, time_t timeout);
void       runloop_post(RunloopRef athis, PostableFunction cb, void* arg);
void       runloop_delete(RunloopRef athis, int fd);
void       runloop_verify(RunloopRef r);
/** @} */

/**
 * The following include files provide the API for their specific type of event.
 *
 * The first of these RunloopWatcherBase in w_watcher_base.h is meant to be the common element of all event
 * objects and in other languages wold be the base class with the other event objects inheriting from
 * this base object.
 *
 * Since C does not have inheritence the approach adopted is embedding. Thus a subclass is defined as follows:
 *
 * ```
 * typedef struct SubClassOfEvent_s {
 *      RunloopWatcherBase;
 *      .....
 *      other fields as required for the specific subclass
 *      .....
 * } SubClassOfEvent, *SubClassOfEventRef;
 *
 * This means that a pointer to a SubClassOfEvent is also a pointer to a RunloopWatcherBase instance.
 *
 * For this to work the code must be compiled with the microsoft extension __-fms-extensions__.
 *
 * Note that these include files only hold forward decleration of their data structures
 */
#include "w_watcher_base.h"
#include "w_timer.h"
#include "w_listener.h"
#include "w_stream.h"
#include "w_eventfd.h"
#include "w_eventfd_queue.h"
#include "w_interthread_queue.h"

/**
 * The next include file provide details struct definitions for all the structured forward declared in
 * the previous group of header files.
 *
 * The details are all in one file because of the dependencies between the data structures which include struct
 * definitions not required to give the event object API.
 */
#include "rl_internal.h"
/**
 * When coding in the C language, particularly using callback style it is very easy to misinterpret the data type
 * at the end of a pointer. In order to have runtime checking that points reference the type of object I think they
 * do, and to be able to see during debugging what type of object is at the end of a pointer. I have apllied a
 * technique I learned many years ago.
 *
 * The in-memory image of every object is bracketed by an known byte pattern that is different for each type of
 * object.
 *
 * Every time a pointer is interpreted as pointiing to a specific type of object the barckets are tested to
 * confirm that the object is the type that was expected. This is done with 6 macros which may be found in
 *
 * ```
 * rbl/checktags.h
 * ```
 * The macros are:
 * ```
 *  RBL_DECLARE_TAGE
 *  RBL_DECLARE_END_TAG
 *  RBL_SET_TAG
 *  RBL_SET_END_TAG
 *  RBL_CHECK_TAG
 *  RBL_CHECK_END_TAG
 * ```
 *
 * The last two macros abort the program is the correct tag is not found.
 *
 * In addition to aiding in the correct interpretation of pointers the bracketing nature of the tags
 * catches a lot of situations where the code writes off the end of an object or an object gets corrupted.
 *
 */
#include "rl_checktag.h"
#endif