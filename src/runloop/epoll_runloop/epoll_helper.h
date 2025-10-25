#ifndef H_c_http_runloop_epoll_helper_H
#define H_c_http_runloop_epoll_helper_H
#include <stdint.h>
#include <stdbool.h>
/**
 * Add subject_fd to the epoll interest list of epoll_fd with an event mask
 * set to "interest". Will fail (assert - fatal ) if you try to do this twice for the same
 * subject_fd and epoll_fd.
 */
void eph_add(int epoll_fd, int subject_fd, uint32_t interest, void* data);
/**
 * When a subject_fd already in the interest list of epoll_fd update
 * the set of interests with the new set of "interest".
 * Will fail (assert fatal) if subject_fd has not already been added to the interest list of epoll_fd
 * by a call to eph_add.
 */
void eph_mod(int epoll_fd, int subject_fd, uint32_t interest, void* data);
/**
 * Remove subject_fd from the interest list of epoll_fd.
 * Will fail (assert - fatal) if subject_fd has not already been added to the
 * interest list of epoll_fd with a eph_add() call.
 */
void eph_del(int epoll_fd, int subject_fd, uint32_t interest, void* data);

uint32_t eph_interest_none();
uint32_t eph_interest_read(bool edge_triggered);
uint32_t eph_interest_write(bool edge_triggered);
uint32_t eph_interest_both(bool edge_triggered);
uint32_t eph_interest_remove_read(uint32_t current_interest);
uint32_t eph_interest_remove_write(uint32_t current_interest);
#endif
