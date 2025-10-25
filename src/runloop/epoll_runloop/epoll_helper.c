#include "epoll_helper.h"
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <rbl/logger.h>
#include <rbl/macros.h>

static int eph_epoll_ctl(int epoll_fd, int subject_fd, int operation, uint32_t interest, void* data);

void eph_add(int epoll_fd, int subject_fd, uint32_t interest, void* data)
{
    struct epoll_event epev = {.events = interest, .data = {.ptr = data}};
    int status = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, subject_fd, &(epev));
    if (status != 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("runloop_epoll_ctl epoll_fd: %d fd: %d status : %d errno : %d %s", epoll_fd, subject_fd, status, errno_saved, strerror(errno_saved));
    }
    RBL_LOG_FMT("eph_del epoll_fd: %d status : %d errno : %d", epoll_fd, status, errno);
    RBL_ASSERT((status == 0), "epoll ctl call failed");
}
void eph_mod(int epoll_fd, int subject_fd, uint32_t interest, void* data)
{
    struct epoll_event epev = {.events = interest, .data = {.ptr = data}};
    int status = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, subject_fd, &(epev));
    if (status != 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("runloop_epoll_ctl epoll_fd: %d fd: %d status : %d errno : %d %s", epoll_fd, subject_fd, status, errno_saved, strerror(errno_saved));
    }
    RBL_LOG_FMT("eph_del epoll_fd: %d status : %d errno : %d", epoll_fd, status, errno);
    RBL_ASSERT((status == 0), "epoll ctl call failed");
}
void eph_del(int epoll_fd, int subject_fd, uint32_t interest, void* data)
{
    int status = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, subject_fd, NULL);
    if (status != 0) {
        int errno_saved = errno;
        RBL_LOG_ERROR("runloop_epoll_ctl epoll_fd: %d fd: %d status : %d errno : %d %s", epoll_fd, subject_fd, status, errno_saved, strerror(errno_saved));
    }
    RBL_LOG_FMT("eph_del epoll_fd: %d status : %d errno : %d", epoll_fd, status, errno);
    RBL_ASSERT((status == 0), "epoll ctl call failed");
}
uint32_t eph_interest_none()
{
    return (uint32_t)0;
}
uint32_t eph_interest_read(bool edge_triggered)
{
    return (uint32_t)(EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLRDHUP | ((edge_triggered)? EPOLLET : 0));
}
uint32_t eph_interest_write(bool edge_triggered)
{
    return (uint32_t)(EPOLLOUT | EPOLLHUP | EPOLLERR | EPOLLRDHUP | ((edge_triggered)? EPOLLET : 0));
}
uint32_t eph_interest_both(bool edge_triggered)
{
    return eph_interest_read(edge_triggered) & eph_interest_write(edge_triggered);
}
uint32_t eph_interest_remove_read(uint32_t current_interest)
{
    current_interest &= ~EPOLLIN;
    return current_interest;
}
uint32_t eph_interest_remove_write(uint32_t current_interest)
{
    current_interest &= ~EPOLLOUT;
    return current_interest;
}