#define _GNU_SOURCE

#include "wayland.h"
#include "blob.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "wlr-data-control-unstable-v1-client-protocol.h"
#include <wayland-client.h>

static struct {
    struct wl_display                   *display;
    struct wl_registry                  *registry;
    struct wl_seat                      *seat;
    struct zwlr_data_control_manager_v1 *manager;
    struct zwlr_data_control_device_v1  *device;
    struct zwlr_data_control_source_v1  *source;

    struct blob_set                     *blobs;
    int                                  running;
} wl;

/* source listener */

static void source_send(void *data, struct zwlr_data_control_source_v1 *source,
                        const char *mime_type, int32_t fd)
{
    (void)data;
    (void)source;

    for (int i = 0; i < BLOB_COUNT; i++) {
        if (strcmp(mime_type, mime_types[i]) == 0) {
            write_all(fd, wl.blobs->data[i], wl.blobs->len[i]);
            break;
        }
    }
    close(fd);
}

static void source_cancelled(void *data, struct zwlr_data_control_source_v1 *source)
{
    (void)data;
    (void)source;
    wl.running = 0;
}

static const struct zwlr_data_control_source_v1_listener source_listener = {
    .send      = source_send,
    .cancelled = source_cancelled,
};

/* device listener */

static void device_data_offer(void *data, struct zwlr_data_control_device_v1 *device,
                              struct zwlr_data_control_offer_v1 *offer)
{
    (void)data;
    (void)device;
    zwlr_data_control_offer_v1_destroy(offer);
}

static void device_selection(void *data, struct zwlr_data_control_device_v1 *device,
                             struct zwlr_data_control_offer_v1 *offer)
{
    (void)data;
    (void)device;
    if (offer)
        zwlr_data_control_offer_v1_destroy(offer);
}

static void device_finished(void *data, struct zwlr_data_control_device_v1 *device)
{
    (void)data;
    (void)device;
    wl.running = 0;
}

static void device_primary_selection(void *data, struct zwlr_data_control_device_v1 *device,
                                     struct zwlr_data_control_offer_v1 *offer)
{
    (void)data;
    (void)device;
    if (offer)
        zwlr_data_control_offer_v1_destroy(offer);
}

static const struct zwlr_data_control_device_v1_listener device_listener = {
    .data_offer        = device_data_offer,
    .selection         = device_selection,
    .finished          = device_finished,
    .primary_selection = device_primary_selection,
};

/* registry listener */

static void registry_global(void *data, struct wl_registry *registry, uint32_t name,
                            const char *interface, uint32_t version)
{
    (void)data;
    (void)version;

    if (strcmp(interface, "wl_seat") == 0 && !wl.seat) {
        wl.seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
    } else if (strcmp(interface, "zwlr_data_control_manager_v1") == 0) {
        wl.manager = wl_registry_bind(registry, name, &zwlr_data_control_manager_v1_interface, 1);
    }
}

static void registry_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
    (void)data;
    (void)registry;
    (void)name;
}

static const struct wl_registry_listener registry_listener = {
    .global        = registry_global,
    .global_remove = registry_global_remove,
};

int clipboard_serve(struct blob_set *blobs)
{
    wl.blobs = blobs;

    wl.display = wl_display_connect(NULL);
    if (!wl.display) {
        fprintf(stderr, "wfcopy: cannot connect to wayland display\n");
        return -1;
    }

    wl.registry = wl_display_get_registry(wl.display);
    wl_registry_add_listener(wl.registry, &registry_listener, NULL);
    wl_display_roundtrip(wl.display);

    if (!wl.manager) {
        fprintf(stderr, "wfcopy: compositor does not support "
                        "wlr-data-control-unstable-v1\n");
        return -1;
    }
    if (!wl.seat) {
        fprintf(stderr, "wfcopy: no seat found\n");
        return -1;
    }

    wl.source = zwlr_data_control_manager_v1_create_data_source(wl.manager);
    zwlr_data_control_source_v1_add_listener(wl.source, &source_listener, NULL);

    for (int i = 0; i < BLOB_COUNT; i++)
        zwlr_data_control_source_v1_offer(wl.source, mime_types[i]);

    wl.device = zwlr_data_control_manager_v1_get_data_device(wl.manager, wl.seat);
    zwlr_data_control_device_v1_add_listener(wl.device, &device_listener, NULL);
    zwlr_data_control_device_v1_set_selection(wl.device, wl.source);

    wl_display_roundtrip(wl.display);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    if (pid > 0)
        return 0;

    wl.running = 1;
    while (wl.running) {
        if (wl_display_dispatch(wl.display) < 0)
            break;
    }

    zwlr_data_control_source_v1_destroy(wl.source);
    zwlr_data_control_device_v1_destroy(wl.device);
    zwlr_data_control_manager_v1_destroy(wl.manager);
    wl_registry_destroy(wl.registry);
    wl_display_disconnect(wl.display);

    return 0;
}
