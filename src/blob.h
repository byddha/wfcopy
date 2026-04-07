#pragma once

#include <stddef.h>

#define MIME_URI_LIST           "text/uri-list"
#define MIME_GNOME_COPIED_FILES "x-special/gnome-copied-files"
#define MIME_TEXT_UTF8          "text/plain;charset=utf-8"
#define MIME_UTF8_STRING        "UTF8_STRING"

enum {
    BLOB_URI_LIST,
    BLOB_GNOME_COPIED_FILES,
    BLOB_TEXT_UTF8,
    BLOB_UTF8_STRING,
    BLOB_COUNT,
};

extern const char *mime_types[BLOB_COUNT];

struct blob_set {
    char  *data[BLOB_COUNT];
    size_t len[BLOB_COUNT];
};

void build_blobs(struct blob_set *blobs, char **paths, int count);
void free_blobs(struct blob_set *blobs);
