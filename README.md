# shorkfetch

A SHORK utility for displaying basic system and environment information in a summarised format, similar to fastfetch, neofetch, etc. but without any art, graphics and perhaps with a limited amount of fields. It is primarily written for use with SHORK family operating systems like [SHORK 486](https://github.com/SharktasticA/SHORK-486), designed to be minimal, not taxing on 486-era hardware, and is statically linked. But it should work on modern Linux distributions just fine.



## Building

### Requirements

* C compiler (tested with GCC and i486-linux-musl-cross)
* `make`, `ar`, `ranlib` and `strip`

### Compilation

Simply run `make`.

### Installation

Run `make install` to install to `/usr/bin`. If you want to install it elsewhere, you can override the install location prefix like `make PREFIX=/usr/local install`.



## Example output

### On SHORK 486 with '90s hardware

    root@shork-486
    --------------
    OS:      SHORK 486 0.1
    Kernel:  6.14.11
    Uptime:  0 hours, 2 minutes
    Shell:   sh
    CPU:     486 (1C)
    GPU:     3Dfx Voodoo 3
    Memory:  1.8MiB / 3.1MiB (56%)
    Swap:    0B / 8MiB (0%)
    Root:    56.7MiB / 81.1MiB (69%)

### On Debian 13 with modern hardware

    kali@SN-MAIN
    ------------
    OS:      Debian GNU/Linux 13 (trixie)
    Kernel:  6.12.63+deb13-amd64
    Uptime:  4 hours, 18 minutes
    Shell:   bash
    CPU:     AMD Ryzen 9 5950X (16C/32T)
    GPU:     NVIDIA AD103 [GeForce RTX 4080]
    Memory:  9.6GiB / 62.7GiB (15%)
    Swap:    0B / 976MiB (0%)
    Root:    416.2GiB / 455.9GiB (91%)

