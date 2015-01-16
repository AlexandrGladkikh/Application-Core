#include "WrapNet.h"
#include <stdlib.h>
#include "errno.h"
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include "Log.h"

namespace wrap {
////////////////////////////////////

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    int		n;

    if ((n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
        return -1;
    return(n);		/* can return 0 on timeout */
}

typedef	void	Sigfunc(int);

Sigfunc *signal(int signo, Sigfunc *func)
{
    struct sigaction	act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
#endif
    } else {
#ifdef	SA_RESTART
        act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
    }
    if (sigaction(signo, &act, &oact) < 0)
        return(SIG_ERR);
    return(oact.sa_handler);
}
/* end signal */

Sigfunc *Signal(int signo, Sigfunc *func)	/* for our signal() function */
{
    Sigfunc	*sigfunc;

    if ( (sigfunc = signal(signo, func)) == SIG_ERR)
        Log("BADSIGNAL", "signal");
    return(sigfunc);
}

int CreateSocket(const char *host, const char *serv, socklen_t *addrlenp)
{
    int				listenfd, n;
    const int		on = 1;
    struct addrinfo	hints, *res, *ressave;
    int val;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
        return -1;
    ressave = res;

    do {
        listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (listenfd < 0)
            continue;		/* error, try next one */

        if (!Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
            return -1;

        if ((val = Fcntl(listenfd, F_GETFL, 0)) == -1)
            return -1;
        if ((Fcntl(listenfd, F_SETFL, val | O_NONBLOCK)) == -1)
            return -1;

        if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
            break;			/* success */

        Close(listenfd);	/* bind error, close and try next one */
    } while ((res = res->ai_next) != NULL);

    if (res == NULL)	/* errno from final socket() or bind() */
        return -1;

    if (!Listen(listenfd, LISTENQ))
        return -1;

    if (addrlenp)
        *addrlenp = res->ai_addrlen;	/* return size of protocol address */

    freeaddrinfo(ressave);

    return(listenfd);
}

bool Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    if (setsockopt(fd, level, optname, optval, optlen) < 0)
        return false;
    return true;
}

bool Listen(int fd, int backlog)
{
    char	*ptr;

        /*4can override 2nd argument with environment variable */
    if ( (ptr = getenv("LISTENQ")) != NULL)
    {
        backlog = atoi(ptr);
    }

    if (listen(fd, backlog) < 0)
    {
        return false;
    }
    return true;
}

int Fcntl(int fd, int cmd, int arg)
{
    int	n;

    if ( (n = fcntl(fd, cmd, arg)) == -1)
        return -1;
    return(n);
}

bool Close(int fd)
{
    if (close(fd) == -1)
        return false;
    return true;
}

int Poll(struct pollfd *fdarray, unsigned long nfds, int timeout)
{
    int		n;

    if ( (n = poll(fdarray, nfds, timeout)) < 0)
        return -1;

    return(n);
}

////////////////////////////////////
}
