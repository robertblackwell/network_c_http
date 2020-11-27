#ifndef c_http_xr_proactor_h
#define c_http_xr_proactor_h
#include <c_http/xr/timer_watcher.h>
#include <c_http/xr/socket_watcher.h>
#include <c_http/xr/queue_watcher.h>

void Xrsw_start_read(XrSocketWatcherRef sock, void* buffer, int length, XrSocketWatcherCallback cb, void* user_data);
void Xrsw_start_write(XrSocketWatcherRef sock, void* buffer, int length, XrSocketWatcherCallback cb, void* user_data);

void Xrtw_wait(XrTimerWatcherRef timer, long interval_ms, XrTimerWatcherCallback cb, void* user_data);

void Xrqw_read(XrQueueWatcherRef queue, XrQueueWatcherCallback cb, void* user_data);

#endif;