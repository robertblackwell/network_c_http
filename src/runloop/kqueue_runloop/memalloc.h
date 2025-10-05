#ifndef runloop_malloc_h
#define runloop_malloc_h

#define runloop_MALLOC_TRACE
void* runloop_mem_alloc(int size, char* file, int line);
void  runloop_mem_free(void* p, char* file, int line);
#ifdef runloop_MALLOC_TRACE
    #define runloop_malloc(size) runloop_mem_alloc(size, __FILE__, __LINE__)
    #define runloop_free(p)      free(p, __FILE__, __LINE__)
#else
    #define runloop_malloc(size) malloc(size)
    #define runloop_free(p)      free(p)
#endif
#endif