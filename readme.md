# http1.1 in C

## This project

This project is an experiment in __'C'__. 

Specifically the intent is to implement two simple __http servers__
in the 'C' language without the use of external libraries (see below for the one exception to this goal)
in order to understand what is required at the socket and epoll level, and to get more experience with
structuring code and data structures in 'C'.

## the library exception

As it turns out one of the more complicated aspects of http servers is the parsing of http messages
(requests and responses).

While the http1.1 format seems superficially simple the details quickly become onerous and the task large.

Hence this is the place where I used a library. In fact over time I used two different but related libraries.

Initially I used the library at [https://github.com/nodejs/http-parser](https://github.com/nodejs/http-parser).
This is a library consisting of a single hand crafted __.c__ file of 2575 lines and a single __.h__ files of 449 lines.

More recently this projects has been abandoned in favor of [https://github.com/nodejs/llhttp](https://github.com/nodejs/llhttp)

In many regards this is the same project with the important distinction that the parser 'C' code is generated from 
rules rather than hand-crafted.

As a user of these libraries I found the former library [https://github.com/nodejs/http-parser](https://github.com/nodejs/http-parser)
easier to deal with where as I find the more recent versions of [https://github.com/nodejs/llhttp](https://github.com/nodejs/llhttp)
less amenable to my use case.

## sync_server_app

The executable `sync_server_app` implements a multithreaded http server that uses synchronous or blocking IO.

It only responds to a small set of __GET__ requests and at the time of writing does not perform
any disk IO to make responses messages.

This is the classic way of implementing a server before the days of event loops similar technologies.

There are two versions of this server selected by the build options __SYNC_WORKER_QUEUE__.

When this option is added to the CCFLAGS as -DSYNC_WORKER_QUEUE in the top level CmakeList.txt
only the the main performs `accept()` calls and the client connections/sockets from such calls
are fed to worker threads by a queue.

When this option is absent each worker thread issues accept() calls on a common listening socket created by the main
thread before creating the worker threads. The Linux operating system allocates new incoming connections between the
different `accept()` callers in a manner that only wakes one `accept()` caller for each new connection.

## async_server_app

The executable `async_server_ap` implements an event based single threaded server that performs all IO
in a non blocking or async manner.

It uses a home grown event mechanism based on linux `epoll` which can be found in the directory `<project>/http_in_c/runloop`.

At the time of writing the event mechanism has no thread pool or other mechanism for the server to perform
disk IO efficiently. 

Thats a future enhancement.

This server has two modes of operating that are selected by a build options __ASYNC_SINGLE_THREAD:

When this option is set the server runs as a single thread.

When this option is not set the server will try to start multiple threads, sharing a common listening sockets
and having a run loop for each thread. 

WE WARNED - there is a bug that causes crashes in the multi-threaded version.

### Multi processes

The `async_server_app` sets its listening socket to __SO_REUSEADDR__ and __SO_REUSEPORT__.

As a result one can start any number of instances of `async_server_app` simultaniously to get the benefit
of multiple servers without having to solve the problem caused by the multi-thread bug mentioned above.

The only downside of this is how to kill all the processes that are started.

## verifier app and python_client

The `verifier_app` is a __C__ client app that can be used to exercise both of the server apps described above.

There is also a `python_client/client.py` app that is a second way to exercise the servers.

In the directory

## Installation

Clone the repo and run the cmake build, and run the tests.

```bash
git clone git@github.com:robertblackwell/c_http.git
mkdir cmake-build-debug
cd cmake-build-debug
cmake ..
make
ctest
```
## Building

The following commands will clone the repo and build all project deliverables:

```bash
git clone git@github:robertblackwell/c_http.git
cd c_http
mkdir cmake_debug_build
cd cmake_debug_build
cmake ..
make
```
## Dependencies

The only dependencies are the two http parsing libraries discussed above.

For simplicity I have included those in source form in the repo. They are in the __vendor__
directory.

## Build deliverables

The project uses CMake for building. The build outputs are:

-   a static library    __cmake_debug_build/c_http/libc_http_library.a__ 
-   a binary executable __cmake_debug_build/sync-server-app/c_http/sync-server-app__ 
-   a binary executable __cmake_debug_build/async_server_appc_http/async-server-app__ 
-   a binary executable __cmake_debug_build/verifier/verifier_client__ 
-   a python3 script    __python_client/client.py__ 

## Usage 

By default the servers run on port 9001.

All executables have a simplistic getopts interface, so when in doubt try:

```
    <program> -h
```

The `verify_client` app will track response times and print a brief report. 


## Contributing
Pull requests are welcome.

## License
[MIT](https://choosealicense.com/licenses/mit/)