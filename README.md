# wfcopy

CLI tool to copy files to the Wayland clipboard.

`wl-copy` only advertises a single MIME type per invocation, so apps that expect a different one won't handle the paste correctly. File managers like Thunar get around this by advertising multiple MIME types at once; `wfcopy` does the same thing.

```
wfcopy photo.png archive.zip
```

Requires `libwayland-client` and `wayland-scanner`. Works on any compositor that supports `wlr-data-control-unstable-v1` (Hyprland, Sway, etc).
