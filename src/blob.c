#include "blob.h"
#include "uri.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *mime_types[BLOB_COUNT] = {
    [BLOB_URI_LIST]           = MIME_URI_LIST,
    [BLOB_GNOME_COPIED_FILES] = MIME_GNOME_COPIED_FILES,
    [BLOB_TEXT_UTF8]          = MIME_TEXT_UTF8,
    [BLOB_UTF8_STRING]        = MIME_UTF8_STRING,
};

void build_blobs(struct blob_set *blobs, char **paths, int count)
{
    size_t *uri_len;
    char  **uris;
    char   *dst;
    size_t  total;
    int     i;

    uris    = (char **)xmalloc(count * sizeof(char *));
    uri_len = (size_t *)xmalloc(count * sizeof(size_t));

    for (i = 0; i < count; i++) {
        char *encoded = uri_encode_path(paths[i]);
        if (!encoded)
            die("malloc");
        uri_len[i] = strlen("file://") + strlen(encoded);
        uris[i]    = xmalloc(uri_len[i] + 1);
        sprintf(uris[i], "file://%s", encoded);
        free(encoded);
    }

    /* text/uri-list — file:// URIs joined with \r\n */
    total = 0;
    for (i = 0; i < count; i++)
        total += uri_len[i] + 2;
    dst                        = xmalloc(total + 1);
    blobs->data[BLOB_URI_LIST] = dst;
    blobs->len[BLOB_URI_LIST]  = total;
    for (i = 0; i < count; i++) {
        memcpy(dst, uris[i], uri_len[i]);
        dst += uri_len[i];
        *dst++ = '\r';
        *dst++ = '\n';
    }
    *dst = '\0';

    /* x-special/gnome-copied-files — "copy\n" + URIs joined with \n */
    total = 5;
    for (i = 0; i < count; i++) {
        total += uri_len[i];
        if (i < count - 1)
            total++;
    }
    dst                                  = xmalloc(total + 1);
    blobs->data[BLOB_GNOME_COPIED_FILES] = dst;
    blobs->len[BLOB_GNOME_COPIED_FILES]  = total;
    memcpy(dst, "copy\n", 5);
    dst += 5;
    for (i = 0; i < count; i++) {
        memcpy(dst, uris[i], uri_len[i]);
        dst += uri_len[i];
        if (i < count - 1)
            *dst++ = '\n';
    }
    *dst = '\0';

    /* text/plain;charset=utf-8 — absolute paths joined with \n */
    total = 0;
    for (i = 0; i < count; i++) {
        total += strlen(paths[i]);
        if (i < count - 1)
            total++;
    }
    dst                         = xmalloc(total + 1);
    blobs->data[BLOB_TEXT_UTF8] = dst;
    blobs->len[BLOB_TEXT_UTF8]  = total;
    for (i = 0; i < count; i++) {
        size_t plen = strlen(paths[i]);
        memcpy(dst, paths[i], plen);
        dst += plen;
        if (i < count - 1)
            *dst++ = '\n';
    }
    *dst = '\0';

    /* UTF8_STRING — same as text/plain */
    blobs->data[BLOB_UTF8_STRING] = xstrdup(blobs->data[BLOB_TEXT_UTF8]);
    blobs->len[BLOB_UTF8_STRING]  = blobs->len[BLOB_TEXT_UTF8];

    for (i = 0; i < count; i++)
        free(uris[i]);
    free(uris);
    free(uri_len);
}

void free_blobs(struct blob_set *blobs)
{
    int i;

    for (i = 0; i < BLOB_COUNT; i++)
        free(blobs->data[i]);
}
