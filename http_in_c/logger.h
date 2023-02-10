#ifndef c_http_logger_h
#define c_http_logger_h
#define _GNU_SOURCE
#include <stdio.h>
extern void log_function(char* level, const char* funcname, const char* filename, int line, char* message);

// CHLOG_GLOBAL - is a global switch to turn logging off globally
#define CHLOG_GLOBALX

// CHLOG_ON - each c/c++ file can turn logging on by defining CHLOG_ON before including logger.h
// this only turns logging on if the global switch is defined
//
// LOG_ERROR - error logging is always on
//
#define LOG_ERROR(f_, ...) do {\
		char* s; \
		int c  = asprintf(&s, f_, ##__VA_ARGS__); \
		log_function("ERR", __FUNCTION__, __FILE__, __LINE__, s); \
		free(s); \
	} while(0);


#if defined(CHLOG_ON) && defined(CHLOG_GLOBAL)
	#define LOG_PRINTF(f_, ...) printf((f_), ##__VA_ARGS__)
	#define LOG_FMT(f_, ...) do {\
		char* s; \
		int c  = asprintf(&s, f_, ##__VA_ARGS__); \
		log_function("LOG", __FUNCTION__, __FILE__, __LINE__, s); \
		free(s); \
	} while(0);

    #define LOG_ENTRY() do {\
		log_function("ENTR", __FUNCTION__, __FILE__, __LINE__, ""); \
	} while(0);

    #define LOG_MSG(m)  do {\
		log_function("MSG", __FUNCTION__, __FILE__, __LINE__, m); \
	} while(0);

#else
	#define LOG_PRINTF(f_, ...)
	#define LOG_FMT(f_, ...)
    #define LOG_ENTRY()
    #define LOG_MSG(m)
#endif
#endif