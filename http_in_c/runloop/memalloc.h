#ifndef rtor_malloc_h
#define rtor_malloc_h

#define RTOR_MALLOC_TRACE
void* rtor_mem_alloc(int size, char* file, int line);
void  rtor_mem_free(void* p, char* file, int line);
#ifdef RTOR_MALLOC_TRACE
    #define rtor_malloc(size) rtor_mem_alloc(size, __FILE__, __LINE__)
    #define rtor_free(p)      free(p, __FILE__, __LINE__)
#else
    #define rtor_malloc(size) malloc(size)
    #define rtor_free(p)      free(p)
#endif
#endif