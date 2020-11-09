# c_http

c_http is an implementation of a basic  multi-threaded http server and client in c.

I plan on using this as a model and re-implementating both the server and client in Rust, Swift and Zig 
as a means of learning those languages.

The most intricate part of the client and server is parsing http/1.X messages. In all cases I am using/planning to use
the http-parser at [https://github.com/nodejs/http-parser](https://github.com/nodejs/http-parser) 
as a dependecny. For Rust, Swift and Zig that will require integrating those languages with
a C dependency.

Since [https://github.com/nodejs/http-parser](https://github.com/nodejs/http-parser) is no longer maintained once 
I have completed the 4 languages I will search for, and maybe adapt, another C parser maybe 
[https://github.com/h2o/picohttpparser](https://github.com/h2o/picohttpparser) as the basis for these projects.

## Installation

Clone the repo and run the cmake build, and run the tests.

```bash
git clone git@github.com:robertblackwell/c_http.git
mkdir build
cd build
cmake ..
make
ctest
```

## Contributing
Pull requests are welcome.

## License
[MIT](https://choosealicense.com/licenses/mit/)