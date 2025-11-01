

# What is a runloop ?

A runloop is a mechanism that allows a program to:
-    wait for events associated with file descriptors,
-    run a callback function when such an event happens, and
-    schedule subsequent callbacks to run in the future
 
A runloop is the common method for structuring event driven programs.

"Runloop" is the name that Apple gives to the implementation underlying its user interface.

Other "runloops" have other names.

An "event loop" is what python amd nodejs call theirs.

 In addition to thier use in GUI libraries they are common in programs that perform event-driven or asynchronous
 I/O, such a webservers and more generally network programs.

This directory (http_in_c/runloop) implements a "runloop" as the basis for building a simple event driven webserver.

# How is a runloop used

A typical event-driven program looks like this:

```

int main() 
{
    runloop_t* = runloop_new(....);
    
    ... do some setup -- more details later
    ... and start something going like a no-wait read
    
    runloop_run(rl);
    // the call on the previous line only returns when the program is over
}
```

# Considerations

 The runloop presented here is based on the Linux __e-poll__ systems call and is hence not portable to
 other operating systems.

 A central tool in building and using a runloop is the concept of a callback or closure. 
 It is a function together with a context.

You the programmer must tell the runloop which "callback" to invoke for each of the events that can 
possibly be triggered; because __all__ events funnel through the `runloop`.

In the C language a callback is a pair consisting of a function pointer and an anonymous pointer (void*). 

The pointer points at an instance of a  `struct` which holds all the context data the function needs
to operate.

This is a somewhat rigid way of providing closures and is one of the "features" that making writing event driven
code C a bit tedious. 

Typically, callbacks should be short and fast, should not perform long calculations and should not perform 
any synchronous I/O (like calling `printf()` or reading from a file). The whole point of the runloop is that

However, some operations cannot be made short and/or fast, and one runloop solution to this is a thread pool.

I have implemented a simple thread associated with the runloop in this directory.

Some runloops can be used by multiple threads (the C++ Boost library for example) and some cannot (libuv which is 
the basis of nodejs).

The runloop in this project is limited to a single thread. If you want multiple threads you need a runloop
per thread. I might talk later about how to coordinate between threads and their separate runloops.


