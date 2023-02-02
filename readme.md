# http1.1 in C

## This project

This project is an experiment in __'C'__. 

Specifically the intent is to implement two simple __http servers__
in the 'C' language without the use of external ibraries (see below for the one exception to this goal).

There are two implementations of a simple Http server.

-   `http_in_c/sync_app/sync_app.c` contains the `main()` function for a traditional 
    multi-threaded server that performs all I/O in synchronous mode. 

-   `http_in_c/async_app/async_app.c` contains the `main()` function for an event based
multi-threaded server that performs all I/O in async or no-wait mode. This server uses `e-poll` to
handle events via a home grown event loop.

Both servers respond to `GET` requests only and only for a small number of `targets`.

Those `targets` are 

- '/', 
- '/echo', 
- '/file'

The first two respond with html pages that are constructed on the fly (using 'C' code in the server) while
the last reads an entirely static page from a file.

There are a number of limitations and design choices for each implementation. I will discuss those below.

## Parsing HTTP/1.1 messages

This turned out to be the most technically demanding individual task in the project.

While the http1.1 format seems superficially simple the details quickly become onerous and the task large.

Hence this is the place where I used a library. In fact over time I used two different but related libraries.

Initially I used the library at [https://github.com/nodejs/http-parser](https://github.com/nodejs/http-parser).
This is a library consisting of a single hand crafted __.c__ file of 2575 lines and a single __.h__ files of 449 lines.

More recently this projects has been abandoned in favor of [https://github.com/nodejs/llhttp](https://github.com/nodejs/llhttp)

In many regards this is the same project with the important distinction that the parser 'C' code is generated from 
rules rather than hand-crafted.

## The parsers callback models

Both the parser libraries used a callback model for notifying a caller about important events. 

So for example when enough data has been read to constitute a complete request/response this fact is notified to
your code by calling a callback you provided before the parsing started. 

The various elements of the completed message, such a method, target path, header fields and body data
must be pulled from the parsers data structures before parsing any more data.

This is a very natural approach for event driven async code but a little unusual for 
the more traditional synchronous I/O approach. 

## Threading strategies.

Both the servers run multiple threads.

### Threading in the Synchronous server
- The synchronous server runs a thread pool of a fixed number of threads. 
- Each thread serves a single client connection at a time.
- A thread can server a number of requests on a connection for a single client before the connection is closed
and a new client is served.
- Once a request has been read from a connection, that request is processed and a response sent before the next request
will be read on that same connection.
- The main thread (which is not part of the thread pool) runs a loop, blocking on an `accept` call waiting for connection 
attempts from potential clients. The `accept` call returns when a new connection is established. That new connection 
is passed to the thread pool via a queue that is protected with a mutex and condition variable.

### Threading in the event server

- The event server runs a number of threads (the number is set at startup time).
- Each thread when idle is waiting on an e-poll call to signal an event of interest
- Each thread is always watching for an `accept` event to signal the arrival of a new client connection.  The linux
operating system guarentees to wake only one thread for each new connection. This is how work is allocated 
between the threads. This also allows a single thread to serve multiple client connections at the same time.
- Once a thread has a client connection it is always watching for incoming data from the client, assembling that
data into complete http messages, initiating processing of that complete message, writing the response but also 
waiting for and reading additional incoming data.
- Each thread is typically serving multiple client connections at the same time.
- The total number of client connections that can be served st the same time is not restricted by the number of threds.

## Pipelining, to pipeline or not to pipeline ?

The result of the above threading strategy is that:

-   the synchronous server will not read a second request until the first request has been processed and
the response has delivered to the OS for transmission.

- whereas the async server could possibly be parsing data for a 2nd request for the same client before the response 
to the first request has been completely delivered to the OS for transmission. 

## Reading files




I had planned on using this as a model and re-implementating both the server and client in Rust, Swift and Zig 
as a means of learning those languages.

However somewhere along the way my ambitions grew and I have expanded the goal to include creating my 
own __runloop__ implementation as a basis for also implementing a multi-thread async io http server.

The runloop implementation is based on epoll and hence limited to Linux.

I am not sure how much of the runloop part of the c version of this exercise I will seek to implement in those
other languages, time will tell.

## Http http_parser_t
The most intricate part of the client and server is parsing http/1.X messages. Until recently I was using/planning to use
the http-parser at [https://github.com/nodejs/http-parser](https://github.com/nodejs/http-parser) 
as a dependency. For Rust, Swift and Zig that will require integrating those languages with
a C dependency.


However I recently (Nov 2020) noticed that [https://github.com/nodejs/http-parser](https://github.com/nodejs/http-parser) is 
no longer being maintained and is being replaced by [https://github.com/nodejs/llhttp](https://github.com/nodejs/llhttp) 

I have completed the conversion of this repo to use [https://github.com/nodejs/llhttp](https://github.com/nodejs/llhttp),
and integrated those changes back into the `master` branch. 

## Rust Http http_parser_t

The Rust version is already partially implemented at [https://github.com/robertblackwell/rust_http](https://github.com/robertblackwell/rust_http) using 
[https://github.com/nodejs/http-parser](https://github.com/nodejs/http-parser) and will be an early target for
conversion to [https://github.com/nodejs/llhttp](https://github.com/nodejs/llhttp).

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
## Usage

There is a simple server app operating (by default on port 9001) that responds to all request with a simple page,
or responds to a "GET /echo" request by returning the entire request message as the response body. 

Run it on port 9001 with 10 threads as follows:

```bash
./cmake-build-debug/sync_app/simple_server -p 9001 -t 10
```

There is also a benchmark app called `verifier_client` which can be run in conjunction with the simple_server app.

The following command will run the `verify_client` on port 9001, will run 60 threads and 300 request/response 
per thread for a toal of 18000 request/response cycles.

```bash
./cmake-build-debug/verifier/verifier_client -p 9001 -t 60 -r 300
```

The `verify_client` app will track response times and print a brief report. 

On my desktop maching those 18000 request/response cycles take 338 ms total elapsed time, 
the average response time for a request/response cycle is 1.1ms with a 
stddev of 1.0.

Of course there is no real network overhead in those numbers as both processes are on the same machine.

## Contributing
Pull requests are welcome.

## License
[MIT](https://choosealicense.com/licenses/mit/)