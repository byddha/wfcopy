#include "uri.h"

#include <stdio.h>
#include <stdlib.h>

int is_uri_safe(unsigned char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' ||
           c == '.' || c == '_' || c == '~' || c == '/';
}

char *uri_encode_path(const char *path)
{
    size_t len = 0;

    for (const char *p = path; *p; p++)
        len += is_uri_safe((unsigned char)*p) ? 1 : 3;

    char *out = malloc(len + 1);
    if (!out)
        return NULL;

    char *dst = out;
    for (const char *p = path; *p; p++) {
        unsigned char c = (unsigned char)*p;
        if (is_uri_safe(c)) {
            *dst++ = (char)c;
        } else {
            sprintf(dst, "%%%02X", c);
            dst += 3;
        }
    }
    *dst = '\0';
    return out;
}
