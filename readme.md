# c_http

c_http is an implementation of a basic  multi-threaded http server and client in c.

I plan on using this as a model and re-implementating both the server and client in Rust, Swift and Zig 
as a means of learning those languages.

The most intricate part of the client and server is parsing http/1.X messages. In all cases I am using/planning to use
the http-parser at [https://github.com/nodejs/http-parser](https://github.com/nodejs/http-parser) 
as a dependecny. For Rust, Swift and Zig that will require integrating those languages with
a C dependency.

The Rust version is already partially implemented at []()

Since [https://github.com/nodejs/http-parser](https://github.com/nodejs/http-parser) is no longer maintained once 
I have completed the 4 languages I will search for, and maybe adapt, another C parser maybe 
[https://github.com/h2o/picohttpparser](https://github.com/h2o/picohttpparser) as the basis for these projects.

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

There is a simple server app operating on port 9001 that responds to all request with a simple page. Run it 
by using the following command.
```bash
./cmake-build-debug/app/simple_server
```
The server currently starts 50 threads to service requests. This number can be changed by editing `c_eg/constants.h`.


There is also a benchmark app called `verifier_client` which can be run in conjunction with the siimple_server app
with the command:

```bash
./cmake-build-debug/verifier/verifier_client
```

As currently configured the verifyier app runs 30 threads each of which make 100 GET requests to the server on 9001.

To change the configuration you will need to edit the verifier source code.

## Contributing
Pull requests are welcome.

## License
[MIT](https://choosealicense.com/licenses/mit/)