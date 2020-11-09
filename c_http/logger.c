#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#include <c_http/logger.h>
// needed for multi threading
static bool needs_initialization = true;
static pthread_mutex_t lock;

void log_function(char* level, const char* funcname, const char* filename, int line, char* message)
{
	if (needs_initialization) {
		needs_initialization = false;
		pthread_mutex_init(&lock, NULL);
	}
	pthread_mutex_lock(&lock);
	// here put in locking for multi threaded applications
	printf("%s %s %s[%d] %s\n", level, funcname, filename, line, message);
	pthread_mutex_unlock(&lock);

}