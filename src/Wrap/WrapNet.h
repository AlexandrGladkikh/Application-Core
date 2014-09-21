#ifndef WRAPNET_H
#define WRAPNET_H

#pragma once

#include <sys/select.h>

namespace wrap {
////////////////////////////////////

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

////////////////////////////////////
}

#endif // WRAPNET_H
