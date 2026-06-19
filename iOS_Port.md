# shorkfetch — iOS (jailbreak) port

This branch contains the changes needed to build and run [shorkfetch](https://github.com/SharktasticA/shorkfetch) on a jailbroken iOS device (tested on a rootless jailbreak, iPhone 8 / iOS 16.7 with Dopamine-style `/var/jb` layout).

All credit for shorkfetch itself goes to [SharktasticA](https://github.com/SharktasticA) — this is just a compatibility patch on top of their work.

## What had to change

iOS's SDK and sandboxing model differ from Linux in a few ways that broke the original build:

1. **`system()` is unavailable on iOS.**
   The only call to it (a fallback in `isProgramInstalled()` in `src/general.c`, used when `$PATH` isn't set) was removed. This is an edge case that essentially never triggers in normal use, so the function now just returns `0` (not found) in that scenario instead of shelling out.

2. **`linux/limits.h` doesn't exist on the iOS SDK.**
   It was only ever used for the `PATH_MAX` constant, which is also available from the standard `<limits.h>`. Swapped in 6 files: `general.c`, `gpu.c`, `packages.c`, `screen.c`, `shorkconf.c`, `testing.h`.

3. **Static linking isn't supported on Apple platforms.**
   The Makefile's `LDFLAGS += -static` was removed — there's no static libc to link against on iOS/Darwin, only dynamic linking.

## Known limitations 

shorkfetch pulls most of its system data (CPU, memory, uptime, GPU, display info) from Linux's `/proc` and `/sys` pseudo-filesystems, which don't exist on iOS/Darwin (which is the XNU kernel, not Linux if you didn't know). As a result, several fields will currently show as blank, "unknown," or generic defaults — this isn't something the patches above fix, since the underlying data source genuinely isn't there. OS, kernel version, shell, and disk usage do work correctly since they come from portable APIs.

Possible future work (this probably wont happen, this was kinda a random project in my freetime): read CPU/memory/uptime via `sysctl` (e.g. `kern.boottime` for uptime) instead of `/proc`.

## Building on-device (rootless jailbreak)

These notes assume OpenSSH + a Procursus-based package manager (Sileo/Zebra) and a `/var/jb` rootless layout.

1. Make sure you have a compiler and make:
   ```
   apt install clang make
   ```

2. Copy this source tree to the device and build, pointing `make` at the real shell (rootless jailbreaks often don't expose `/bin/sh` directly):
   ```
   make SHELL=/var/jb/bin/sh
   ```

3. If you don't have `strip` installed, either skip it (the binary still works without stripping) or `apt install binutils`.

4. **Ad-hoc sign the binary** — iOS will kill unsigned binaries on launch:
   ```
   apt install ldid   # if not already present
   ldid -S shorkfetch
   ```

5. **Run it from a trusted path.** Even after signing, binaries outside the jailbreak's trusted filesystem (`/var/jb/...`) may still be killed on launch. Move it into a trusted bin directory rather than running it from `/var/root`:
   ```
   cp shorkfetch /var/jb/usr/local/bin/shorkfetch
   chmod +x /var/jb/usr/local/bin/shorkfetch
   /var/jb/usr/local/bin/shorkfetch
   ```

6. To run it by just typing `shorkfetch` from anywhere, symlink it into a directory that's already on `$PATH` (e.g. wherever `clang`/`make` live):
   ```
   ln -s /var/jb/usr/local/bin/shorkfetch /var/jb/usr/bin/shorkfetch
   ```

## License

Unchanged from upstream — GNU General Public License v3.0. See `LICENSE`.