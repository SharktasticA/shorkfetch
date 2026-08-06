# SHORKFETCH

A cute shark-themed Linux fetch tool made with speed, consistent output, and both vintage *and* modern hardware support in mind! It is similar to neofetch, fastfetch, etc., a little less comprehensive and less customisable, but can be faster and provides cleaner output by default. It is primarily written for use with SHORK Operating Systems like [SHORK 486](https://github.com/SharktasticA/SHORK-486), but it works on other modern Linux systems just fine.

SHORKFETCH also has a focus on providing clean and accurate CPU and GPU name reporting, especially for vintage hardware and Intel integrated graphics. Data sources available for such *can* have too generic or 'messy' names, thus SHORKFETCH is designed to help address this.

<p align="center"><img alt="A screenshot of SHORKFETCH running on SHORK 486" src="screenshots/86box_shork-486.png"></p>



## Help wanted!

SHORKFETCH is young, and I would love to hear from you if you have tried SHORKFETCH and found that (in particular) the **CPU, DE, WM and/or GPU fields** were incorrect or imprecise, or in your opinion, were overly verbose, containing marks like "(R)", "TM", etc. and could likely be shortened without compromising understanding. Feel free to create an issue here or contact me via [email](https://sharktastica.co.uk/contact), Discord (@sharktastica) or Reddit (u/sharktastica), and I will take your feedback on board! Please include a screenshot of your SHORKFETCH's output, some context about your system's real specifications and environment, and especially the following depending on the issue:

* CPU: `cat /proc/cpuinfo`
* DE/WM: `echo $XDG_CURRENT_DESKTOP`
* GPU: `lspci -nn | grep 0300`



## Building & installing

### Arch

SHORKFETCH is available on the [AUR](https://aur.archlinux.org/packages/shorkfetch).

    yay -S shorkfetch

### Nix

#### Run

    nix run github:SharktasticA/shorkfetch

#### Install

    nix profile install github:SharktasticA/shorkfetch

### Everything else

#### Requirements

You will need a C compiler, `make` and libc. SHORKFETCH is often tested with GCC + glibc or musl.

#### Quick/automated install

    curl -fsSL https://raw.githubusercontent.com/SharktasticA/shorkfetch/refs/heads/main/install.sh | bash

#### Manual install

Clone this repository by running `git clone https://github.com/SharktasticA/shorkfetch`, or downloading as a zip file and extracting it. When inside the new directory, run `make install` to install to `/usr/bin` (you may need `sudo` if not installing as root). If you want to install it elsewhere, you can override the install location prefix like `make PREFIX=/usr/local install` to install it to `/usr/local/bin`.



## Running

Usage: shorkfetch [OPTIONS]

### Options

* `-b`, `--bullet`: Specifies a custom character to use with bullet-point mode; no assignment returns the current character
* `-cl`, `--colour`: Specifies a custom accent colour; no assignment returns the current colour
* `-co`, `--compact`: Compacts field names and field values
* `-f`, `--fields`: Specifies a custom fields list and order; no assignment returns list of current fields
* `-fa`, `--force-art`: Forces the SHORK ASCII art to display no matter the setting, number of fields or terminal size
* `-h`, `--help`: Shows help information and exits
* `-m`, `--mode` : Select which view mode to use: [n]ormal, [b]ullets
* `-na`, `--no-art`: Disables the SHORK ASCII art
* `-r`, `--reset`: Resets to default, deletes configuration file and exits
* `-s`, `--save`: Saves chosen options to a configuration file
* `-v`, `--version`: Displays version number and exits

### Colours

Only one colour can be used at a time. "off" will use your system's/terminal emulator's text colour, and doesn't use ANSI escape codes (increasing compatibility with commands that change the output colour of another such as [gay](https://github.com/ms-jpq/gay)).

    black           blue            cyan            green  
    magenta         red             white           yellow
    grey            bold_blue       bold_cyan       bold_green
    bold_magenta    bold_red        bold_white      bold_yellow
    off

### Fields

These are possible field names you can use with the `--fields` argument. You enter then as a comma-separated list in double quotes. You can use any more than once and in any order, though there is a maximum of 50 fields.

| Field | Description | Lines |
| ----- | ----------- | ----- |
| (single blank space) | New line | 1 |
| `---` | Separator | 1 |
| `os` | Operating system | 1 |
| `krn` | Kernel | 1 |
| `upt` | Uptime | 1 |
| `pkgs` | Packages | 1 |
| `scn` | Screen(s) | 1-∞ |
| `de` | Desktop environment | 1 |
| `wm` | Window manager and/or Wayland compositor | 1 |
| `trm` | Terminal emulator/console size | 1 |
| `sh` | Shell | 1 |
| `cpu` | CPU | 1 |
| `gpu` | GPU(s) | 1-4 |
| `ram` | System memory | 1 |
| `swap` | Swap memory | 1 |
| `root` | Root partition size | 1 |
| `lip` | Local IP address | 1 |
| `clrs` | ANSI escape code 16-colour palette | 2 |

### Configuration

When customising SHORKFETCH with the options above, you can use the `--save` option to store your choices in a configuration file. Subsequent saves will append the new options or replace existing ones. The configuration file (it is not recommended to modify this manually):

    ~/.config/shorkutils/shorkfetch.conf

To reset SHORKFETCH to its default configuration, simply run with the `--reset` option.

### Notes

* By default, the SHORK ASCII art is automatically disabled if the terminal's width is less than 62 columns or if less than 7 fields are present. The `--force-art` parameter can be used to override this.

## Screenshots

### SHORKFETCH on real hardware + Debian 13

<p align="center"><img alt="A screenshot of four different SHORKFETCH configurations running on Debian 13" src="screenshots/sharktastica-desktop_debian-13_tmux.png"></p>

### SHORKFETCH on 86Box + SHORK 486

<p align="center"><img alt="A screenshot of four different SHORKFETCH configurations running on SHORK 486 running inside 86Box" src="screenshots/86box_shork-486_tmux.png"></p>

### SHORKFETCH on VMware Workstation + SHORK 486

<p align="center"><img alt="A screenshot of four different SHORKFETCH configurations running on SHORK 486 running inside VMware Workstation" src="screenshots/vmware_shork-486_tmux.png"></p>
