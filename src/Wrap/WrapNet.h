#ifndef WRAPNET_H
#define WRAPNET_H

#pragma once

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>

namespace wrap {
////////////////////////////////////

#ifndef HAVE_BZERO
#define bzero(ptr, n)   memset(ptr, 0, n)
#endif

#define LISTENQ 5

typedef	void	Sigfunc(int);

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

int CreateSocket(const char *host, const char *serv, socklen_t *addrlenp);

bool Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);

bool Listen(int fd, int backlog);

int Fcntl(int fd, int cmd, int arg);

bool Close(int fd);

int Poll(pollfd *fdarray, unsigned long nfds, int timeout);

Sigfunc *signal(int signo, Sigfunc *func);

Sigfunc *Signal(int signo, Sigfunc *func);

////////////////////////////////////
}

#endif // WRAPNET_H
