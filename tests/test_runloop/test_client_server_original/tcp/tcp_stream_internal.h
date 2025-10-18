#ifndef H_tcp_stream_internal_h
#define H_tcp_stream_internal_h
#include <common/iobuffer.h>
#include <kqueue_runloop/runloop.h>
#include "tcp_stream.h"

#define RD_STATE_INITIAL 11
#define RD_STATE_EAGAIN 22
#define RD_STATE_STOPPED 33
#define RD_STATE_READY 44
#define RD_STATE_ERROR 55

#define WRT_STATE_EAGAIN 11
#define WRT_STATE_READY 22
#define WRT_STATE_ERROR 33
#define WRT_STATE_STOPPED 44
#define WRT_STATE_INITIAL 55


#endif