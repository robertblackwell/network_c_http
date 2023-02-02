# h11c

## Name change
I am planning on changing the name of this project to __h11c__

## Original Plan
My original plan was implement a basic  multi-threaded synchronous http server and client in c.

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
./cmake-build-debug/app/simple_server -p 9001 -t 10
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