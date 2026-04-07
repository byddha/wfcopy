#define _GNU_SOURCE

#include "blob.h"
#include "util.h"
#include "wayland.h"

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    struct blob_set blobs;
    int             count, i, ret;
    char          **paths;

    if (argc < 2) {
        fprintf(stderr, "usage: wfcopy <file> [file ...]\n");
        return 1;
    }

    count = argc - 1;
    paths = (char **)xmalloc(count * sizeof(char *));

    for (i = 0; i < count; i++) {
        char resolved[PATH_MAX];
        if (!realpath(argv[i + 1], resolved)) {
            fprintf(stderr, "wfcopy: %s: %s\n", argv[i + 1], strerror(errno));
            return 1;
        }
        paths[i] = xstrdup(resolved);
    }

    build_blobs(&blobs, paths, count);

    signal(SIGPIPE, SIG_IGN);

    ret = clipboard_serve(&blobs);

    free_blobs(&blobs);
    for (i = 0; i < count; i++)
        free(paths[i]);
    free(paths);

    return ret < 0 ? 1 : 0;
}
