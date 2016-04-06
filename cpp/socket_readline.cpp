//
// Created by arvin on 2016/4/5.
//

#include <unistd.h>
#include <sys/socket.h>

ssize_t readn(int fd, void *buf, size_t count) {
    size_t nleft = count;
    ssize_t nread;
    char *bufp = (char *)buf;

    while (nleft > 0) {
        if ((nread = read(fd, bufp, nleft)) < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        else if (nread == 0) {

            return count - nleft;
        }

        bufp += nread;
        nleft -= nread;
    }

    return count;
}

inline ssize_t recv_peek(int fd, void *buf, size_t len) {
    while (1) {
        int ret = recv(fd, buf, len, MSG_PEEK);
        if (ret == -1 && errno == EINTR)
            continue;

        return ret;
    }
}

ssize_t readline(int fd, void *buf, size_t max) {
    int ret;
    int nread;
    char *bufp = (char *)buf;
    int nleft = max;
    int count = 0;

    while (1) {
        ret = recv_peek(fd, bufp, nleft);
        if (ret < 0) {
            return ret;
        }
        else if (ret == 0) {
            return ret;
        }

        nread = ret;
        for (int i = 0; i < nread; ++i) {
            if (bufp[i] == '\n') {
                ret = readn(fd, bufp, i + 1);

                if (ret != i + 1)
                    exit(EXIT_FAILURE);
                return ret + count;
            }
        }

        if (nread > nleft)
            exit(EXIT_FAILURE);
        nleft -= nread;
        ret = readn(fd, bufp, nread);
        if (ret != nread)
            exit(EXIT_FAILURE);

        bufp += nread;
        count += nread;
    }
}