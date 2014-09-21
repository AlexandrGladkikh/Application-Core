#include "WrapNet.h"
#include <stdlib.h>
#include "errno.h"

namespace wrap {
////////////////////////////////////

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    int		n;

    if ((n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
        return -1;
    return(n);		/* can return 0 on timeout */
}

////////////////////////////////////
}
