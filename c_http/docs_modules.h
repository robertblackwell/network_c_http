/*
 * This file is input to doxygen - if defines a group hierachy that sets out the project code in  layers of modules
 */
/**
 * @defgroup group_main Project c_http Modules
 * @brief Module structure.
 *
 * The first branch of the module hierachy implements an API for a synchronous multi-threaded server
 * and a synchronous client.
 *
 * @{
 */

///////////////////////////////////////////////////////////////////////////
// group API
//////////////////////////////////////////////////////////////////////////
/** @defgroup group_api API
 *  @brief Project top level API.
 *
 *  With these components one can build a multi-threaded synchronous HTTP/1.X server
 *  and a multi threaded client that verifies the correct behavious of a server.
 *
 *  @{
 */
    /** @defgroup group_server Server
     *  @brief A multi-threaded synchronous server skeleton.
     */
    /** @defgroup group_handler Handler
     *  @brief An interface for a function called by Server to service a request.
     *
     *  A handler function implements that actual http app that responds to specfic requests.
     *  An example handler is provided that responds to 2 requests "/" and "/echo".
     *
     */
    /** @defgroup group_reader Reader
     *  @brief Object that synchronously reads an entire HTTP/1.1 message.
     */
    /** @defgroup group_writer Writer
     *  @brief Object that synchronously writes an entire HTTP/1.X message.
     */
    /** @defgroup group_message Message
     *  @brief A container for a HTTP/1.X request or response message
     */
    /** @defgroup group_client Client
     *  @brief A simple convenients object that can perform a single HTTP/1.x request/response cycle to a server.
     */
    /** @defgroup group_bufferchain BufferChain
     *  @brief A multi-part buffer structure typically used to contain the body of an http/1.x message.
     */
    /** @defgroup group_iobuffer IOBuffer
     *  @brief A contiguous buffer. These are the components of a BufferChain. Have features that facilitate management of
     *  partial reads and writes.
     */
    /** @defgroup group_cbuffer CBuffer
     *  @brief A contiguous buffer that is intended as an alternative to cstrings. It carries its length with it.
     */
 /** @} */

///////////////////////////////////////////////////////////////////////////
// group AIO API
//////////////////////////////////////////////////////////////////////////
/** @defgroup group_aio_api API - Async IO
 *  @brief API to additional functionality for async-io.
 * @{ */

    /** @defgroup group_xrserver XrServer - Async Server
     *  @brief A multi-threaded asynchronous server
     */

    /** @defgroup group_xrhandler XrHandler - Async Handler
    *  @brief Interface for handler object invoked by XrServer to handle requeests.
    */

    /** @defgroup group_xrconn XrConn - Async Connection
     *  @brief A wrapper for a socket that implements async completion callback style data read and write and async http/1.X message read and write.
     */
/** @} */

///////////////////////////////////////////////////////////////////////////
// group Details
//////////////////////////////////////////////////////////////////////////
/** @defgroup group_details Details
 *  @brief Modules that are used in the implementation of the API but are not visible in the API headers files.
 * @{ */
    /** @defgroup group_worker Worker
     *  @brief A Worker instance impements one thread of the sync server.
     */
    /** @defgroup group_hdrlist HdrList
    *  @brief Provides the datastructure for the list of header lines within a message container.
    */
    /** @defgroup group_parser Parser
    *  @brief Wrapps github.com/nodejs/httpll to provide a parser for http/1.X messages.
    */
    /** @defgroup group_parser_test ParserTest
     *  @brief A harness for testing the parser in a unit test frame work.
     */
    /** @defgroup group_parser_types Parser Types - copied from http_parser
     *  @brief A copy of certaiin enum types and utility functions from nodejs/httpll.
     */
    /** @defgroup group_rdsocket RdSocket
    *  @brief A wrapper for a socket fd or a Datasource instance so that there is a common interface to octet streams for both live and unit test.
    */
    /** @defgroup group_datasource DataSource
     *  @brief An octet source that can be used for testing the parser.
     */
/** @} */

///////////////////////////////////////////////////////////////////////////
// group AIO Details
//////////////////////////////////////////////////////////////////////////
/** @defgroup group_runloop AIO Details
 *  @brief This group is the additional functionality required to provide async-io facilities.
 * @{ */

    /** @defgroup group_conn_list ConnList
     *  @brief A list of XrConn currently active within XrServer.
     */

    /** @defgroup group_reactor Reactor
     *  @brief The reactor provides a distribution mechanism for file descriptor related events, and a means of scheduling the execution of call back functions. It is the core of the ASIO mechanism.
     *  @{
     */

        /** @defgroup group-epollr e-poll Event dispatcher
         *  @brief Captures and distributes fd realted events using epoll.
         */
        /** @defgroup group_runlist Runlist
         *  @brief Provides a mechanism for scheduling the future execution of a function without growing the stack.
         */
        /** @defgroup group_fd_table FD Table
         *  @brief A structure internal to the reactor that keeps a list of active file descriptors.
         */

    /** @} */

    /** @defgroup watchers Watcher or FD related event sourrces
     *  @brief This group contains a list of classes that watch file descriptors for events and interpret those events to suite the needs of the specific watcher type.
     *  @{
     */
        /** @defgroup group_watcher Watcher Base
         *  @brief This is the base of the watcher classes and hold a small amount of common behaviour.
         */
        /** @defgroup group_fdevent_watcher WFdEvent - FD Event Watcher
         *  @brief Can be used as an arbitary kind of event. Uses Linux fdevent mechanism.
         */
        /** @defgroup group_listener_watcher WListener - Listener Watcher
         *  @brief Watches a socket for accept events.
         */
        /** @defgroup group_queue_watcher WQueue - Queue Watcher
         *  @brief Watches an inter-thread queue for arrival of messages.
         */
        /** @defgroup group_socket_watcher WSocket - Socket Watcher
         *  @brief Watches a socket (of readable/writeable fd) for readable and/or writeable events.
         */
        /** @defgroup group_timer_watcher WTimer - Timer Watcher
         *  @brief Uses the Linux timerfd mechanism to provide oneshot and repeat timers.
         */

    /** @} */

/** @}  */

///////////////////////////////////////////////////////////////////////////
// group Data Structure LIbrary
//////////////////////////////////////////////////////////////////////////
/** @defgroup group_data_structures Data Structures Library
 *  @brief This group is the additional functionality required to provide async-io facilities.
 * @{ */
    /** @defgroup group_kvp KVPair - Key Value Pair
     *  @brief A key/value pair structure where key and value are both strings of characters.
     */
    /** @defgroup group_list List - Generic Doubly Linked List
     *  @brief A doubly linked list where the data on each node is a void*. Used as a generic in multiple places.
     */
    /** @defgroup group_queue Inter Thread Generic Queue
     *  @brief An inter thread queue synchronized with mutex and condition variable. Holds void* entries.
     */
    /** @defgroup group_util Utils Functions
     *  @brief A collection of macros and function that I could not find a better place for.
     */
    /** @defgroup group_alloc Custom Allocator
     *  @brief Wraps malloc - so that latter we can generalize to any allocator.
     */
/** @} */

///////////////////////////////////////////////////////////////////////////
// group Other
//////////////////////////////////////////////////////////////////////////
/** @defgroup group_other Other
 *  @brief This group is the additional functionality required to provide async-io facilities.
 * @{ */
    /** @defgroup group_logger Logger
     *  @brief A simple multi threaded logger just for this project.
     */
    /** @defgroup group_unittest Unit Test
     *  @brief A simple unit test system just for this project.
     */
    /** @defgroup group_macros Utility macros
     *  @brief A collection of helpful macros.
     */
    /** @defgroup group_check_tags Check struct tags - struct integrity checking
    *  @brief Place a type specific tag at the start of each struct and check it at every reference.
    */
/** @} */

///////////////////////////////////////////////////////////////////////////
// group Demo Apps
//////////////////////////////////////////////////////////////////////////
/** @defgroup group_apps Demo Apps
 *  @brief Two demo apps are provided.
 * @{ */
    /** @defgroup group_simple_server Simple Server
     *  @brief A simple multi threaded synchronous server.
     *
     *  This server is simply a demonstration of how to add functionality to the server by providing a handler function.
     *
     *  This example implements two requests
     *
     *  -   GET /echo - return the serialized version of the request as the response body
     *  -   GET /     - return a short html page with current time and date.
     *
     *  Command line options provide the ability to vary:
     *  -   the port on which to receive requests (-p)
     *  -   the number of worker threads (-t)
     *
     */

    /** @defgroup group_verifier Verifier
    *  @brief A cli program that runs multiple http client instances each in a separate thread.
     *
     *  This app sends GET /echo requests to the server and verifies that the response is correct.
     *
     *  In addition it times each request/response cycle and provides summary stats at the end of its run
     *  showing average and stdev response time.
     *
     *  Command line options can be used to vary:
     *  -   the port (localhost:port) on which to send the requests, (-p)
     *  -   the number of threads (client instances) to start, (-t)
     *  -   the number of requests per thread (-r)
    *


/** @} */

/** @} */
