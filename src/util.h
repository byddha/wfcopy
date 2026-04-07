#pragma once

#include <stddef.h>

void  die(const char *msg);
char *xmalloc(size_t size);
char *xstrdup(const char *s);
int   write_all(int fd, const char *buf, size_t len);
