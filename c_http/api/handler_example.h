#ifndef c_http_handler_example_h
#define c_http_handler_example_h

#include <c_http/api/message.h>
#include <c_http/api/writer.h>

/**
 * This is the prototype for a functions that is called by the synchronous Server to service a single request.
 *
 * The full request message is passed in together with a Writer object that the handler function should
 * use to write the response.
 *
 * @param request MessageRef
 * @param wrtr    WriterRef
 * @return
 */
int handler_example(MessageRef request, WriterRef wrtr);

#endif