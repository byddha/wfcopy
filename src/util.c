#define _GNU_SOURCE

#include "util.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void die(const char *msg)
{
    perror(msg);
    exit(1);
}

char *xmalloc(size_t size)
{
    char *p = malloc(size);
    if (!p)
        die("malloc");
    return p;
}

char *xstrdup(const char *s)
{
    char *p = strdup(s);
    if (!p)
        die("strdup");
    return p;
}

int write_all(int fd, const char *buf, size_t len)
{
    while (len > 0) {
        ssize_t n = write(fd, buf, len);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        buf += n;
        len -= (size_t)n;
    }
    return 0;
}
