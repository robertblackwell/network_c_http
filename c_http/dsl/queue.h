#ifndef c_http_queue_h
#define c_http_queue_h
#include <stdint.h>
#include <stddef.h>
/**
 * @addtogroup group_queue
 * @{
 *
 * This is a queue of integers (socket fd) with a fixed max size.
 * Consumers wait on empty
 * producers wait on full
 */
typedef int32_t SocketFD;
/**
 * @brief A multi-thread queue using mutex and condition variables
 */
struct Queue_s;
typedef struct Queue_s Queue, *QueueRef;
/**
 * @brief Create a new queue with a given capacity
 * @param capacity size_t Cannot be 0
 * @return QueueRef
 */
QueueRef Queue_new_with_capacity(size_t capacity);
/**
 * @brief Create a new queue with a default size
 * @return QueueRef
 */
QueueRef Queue_new();
/**
 * @brief Free a Queue and its resources.
 *
 * Set the arg to NULL on completion.
 * @param q_p QueueRef *
 */
void Queue_free(QueueRef* q_p);
/**
 * @brief Get the top entry from the Queue. Wait if Queue is empty
 * @param q QueueRef
 * @return SockFD
 */
SocketFD Queue_remove(QueueRef q);
/**
 * @brief add an entry to the queue. Wait if Queue is full until there is space.
 * @param q      QueueRef
 * @param socket SockFd
 */
void Queue_add(QueueRef q, SocketFD socket);
/**
 * @brief Return the current size (number of entries) of the queue
 * @param this QueueRef
 * @return size_t
 */
size_t Queue_size(QueueRef this);
/**
 * @brief Return the max capacity of the queue
 * @param this QueueRef
 * @return size_t
 */
size_t Queue_capacity(QueueRef this);
/** @} */
#endif