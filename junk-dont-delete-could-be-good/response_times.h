//
// Created by robert on 11/10/24.
//

#ifndef C_HTTP_RESPONSE_TIMES_H
#define C_HTTP_RESPONSE_TIMES_H

typedef struct ResponseTimes_s ResposeTimes, *ResponseTimesRef;

ResponseTimesRef response_times_new(unsigned long size);

/**
 * Appends the content of b to a - expanding a as necessary
 */
void response_times_append(ResponseTimesRef a, ResponseTimesRef b);

#endif //C_HTTP_RESPONSE_TIMES_H
