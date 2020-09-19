#ifndef c_c_eg_logger_h
#define c_c_eg_logger_h
#define GNU_SOURCE
#include <stdio.h>

extern void log_function(char* level, const char* funcname, const char* filename, int line, char* message);

#define ENABLE_LOG
#ifdef ENABLE_LOG
	#define LOG_PRINTF(f_, ...) printf((f_), ##__VA_ARGS__)
	#define LOG_FMT(f_, ...) do {\
		char* s; \
		int c  = asprintf(&s, f_, ##__VA_ARGS__); \
		log_function("LOG", __FUNCTION__, __FILE__, __LINE__, s); \
		free(s); \
	} while(0);
#else
	#define LOG_PRINTF(f_, ...)
	#define LOG_FMT(f_, ...) 
#endif

#endif