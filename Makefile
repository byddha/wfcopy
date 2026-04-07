PROTO_XML = protocol/wlr-data-control-unstable-v1.xml
SCANNER   = wayland-scanner

BUILD     = build
PROTO_H   = $(BUILD)/wlr-data-control-unstable-v1-client-protocol.h
PROTO_C   = $(BUILD)/wlr-data-control-unstable-v1-protocol.c
PROTO_O   = $(BUILD)/wlr-data-control-unstable-v1-protocol.o

SRCS = src/main.c src/util.c src/uri.c src/blob.c src/wayland.c
OBJS = $(patsubst src/%.c,$(BUILD)/%.o,$(SRCS)) $(PROTO_O)

CC      ?= cc
CFLAGS  ?= -Wall -Wextra -O2
LDFLAGS  = -lwayland-client

PREFIX  ?= /usr/local

wfcopy: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(BUILD)/%.o: src/%.c $(PROTO_H) | $(BUILD)
	$(CC) $(CFLAGS) -I$(BUILD) -c -o $@ $<

$(PROTO_H): $(PROTO_XML) | $(BUILD)
	$(SCANNER) client-header $< $@

$(PROTO_C): $(PROTO_XML) | $(BUILD)
	$(SCANNER) private-code $< $@

$(PROTO_O): $(PROTO_C) | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD):
	mkdir -p $(BUILD)

install: wfcopy
	install -Dm755 wfcopy $(DESTDIR)$(PREFIX)/bin/wfcopy

clean:
	rm -rf wfcopy $(BUILD)

.PHONY: clean install
