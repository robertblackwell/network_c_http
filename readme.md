# http1.1 in C

## This project

This project is an experiment in __'C'__. 

Specifically the intent is to implement two simple __http servers__
in the 'C' language without the use of external libraries (see below for the one exception to this goal)
in order to understand what is required at the socket and epoll level, and to get more experience with
structuring code and data structures in 'C'.

## The "no library" exception

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

## Http 

### Http Synchronous Server

The application whose main function source code is in  `apps_http/sync/http_sync_app` implements a 
multithreaded http server that uses synchronous or blocking IO.

It only responds to a small set of __GET__ requests and at the time of writing does not perform
any disk IO to make responses messages.

This app starts a fixed number of processes each with a fixed number of threads. Each thread performs synchronous IO 
and hence services a single request at a time. Thus the maximum number of request that can be serviced 
concurrently is n(number of processes) * (t) number of threads per process. The `n` and `t` parameters
can be set on the command line of this app. 

This app relies on a feature of Linux whereby if multiple processes/threads call listen() and accept() on a socket
bound to the same host/port combination the OS will only present one of those processes/threads with a socket for a
single newly connecting client. This is an efficient round robbin scheduling of accept() calls.

### Http Asynchronous Server

The app with source in `apps_http/async/http_async_app` implements an event based server that performs all IO
in a non blocking or async manner. Like the http sync server the -n and -t command line options control
the number of processes and threads that are started by this server. With each thread running identical server
code and with essentially no interaction between processes and threads.

In this server each thread can handle __multiple concurrent requests per thread_ with (currently) no limit on 
this number. 

The allocation of incoming connections to threads is achieved using the Linux facility described above in
the section of __http_sync_app__.

It uses a home grown event mechanism called a __runloop_ which is based on linux `epoll` and which can be found in the 
directory `<project>/http_in_c/runloop`.

At the time of writing the event mechanism has no thread pool or other mechanism for the server to perform
disk IO efficiently. 

That is a future enhancement; but the mechanism for doing this is already present in the runloop code
in the form of the types RunloopEventfdQueue and RunloopQueueWatcher and their associated functions.

### Multi processes Multi-threads

Both the servers described above sets its listening socket to __SO_REUSEADDR__ and __SO_REUSEPORT__.

The second of these is necessary to achieve the round robbin allocation of incoming connections described
in the previous sections.

### Http Verifier App

The source of an app christened a "verifier app" lives in `app_http/verify` and is a __C__ client app that can be 
used to exercise or stress both of the server apps described above. 

This app has a number of command line options that allow a user to control the total number of requests,
and how many are concurrent. Its main output is an analysis of respopnse times for a request/response cycle
averaged over a number of cycles.

## The Demo Protocol
 
Analogous to the 3 apps described above for the http1.1 protocol, three apps `demo_sync_app`, `demo_async_app`, and
`demo_verify_app` have been implemented for a very simple protocol that I named __demo__. These apps can be found
in the folder `<project_folder>/apps_demo`.

The `demo` protocol is simple a string of printable characters enclosed between a STX (0x02) character and a ETX(0x03)
character.

These 3 apps were implemented to delineate the issues related to the http1.1 protocol as compared to
issues related to asynchornous io. 

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

-   a static library    __cmake_debug_build/http_in_c/libc_http_library.a__
- 
-   a binary executable __cmake_debug_build/apps_http/async/http_async_app__ 
-   a binary executable __cmake_debug_build/apps_http/sync/http_sync_app__
-   a binary executable __cmake_debug_build/apps_http/verify/http_verify_app__
-   
-   a binary executable __cmake_debug_build/apps_demo/async/demo_async_app__ 
-   a binary executable __cmake_debug_build/apps_demo/sync/demo_sync_app__
-   a binary executable __cmake_debug_build/apps_demo/verify/demo_verify_app__
-   
-   a python3 script    __python_client/client.py__ 

## Usage 

By default the servers run on port 9001.

All executables have a simplistic getopts interface, so when in doubt try:

```
    <program> -h
```

The `verify` apps will track response times and print a brief report. 


## Contributing
Pull requests are welcome.

## License
[MIT](https://choosealicense.com/licenses/mit/)