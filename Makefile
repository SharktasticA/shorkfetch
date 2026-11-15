CC ?= gcc
AR ?= ar
RANLIB ?= ranlib
STRIP ?= strip
LIBS ?=
VERSION = 0.7-wip

ifeq ($(SKIP_SUBMODULE_SYNC),)
    _ := $(shell sh ./sync-submodules.sh >&2)
endif

CFLAGS += -DVERSION='"$(VERSION)"' -Wall -Wextra -D_GNU_SOURCE -std=gnu99 \
    -I. -O3 -flto=auto -fno-plt

ifdef SHORK_DISKETTE
    CFLAGS += -DSHORK_DISKETTE
    LIBS += -ldl
endif

ifdef NO_STR_CLEANING
    CFLAGS += -DNO_STR_CLEANING
endif

ifdef TESTS
    CFLAGS += -DTESTS
endif

ifdef X86_ONLY
    CFLAGS += -DX86_ONLY
endif

SRC = $(wildcard src/*.c) $(wildcard shorkcommon/*.c)
SRC_CONF = $(wildcard src/conf.c) $(wildcard src-conf/*.c) \
    $(wildcard shorkcommon/*.c)

PROGS = shorkfetch
ifndef SHORK_DISKETTE
ifndef TESTS
    PROGS += shorkfetch-conf
endif
endif

all: $(PROGS)

shorkfetch: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o shorkfetch $(LDFLAGS) $(LIBS)
	$(STRIP) shorkfetch

shorkfetch-conf: $(SRC_CONF)
	$(CC) $(CFLAGS) $(SRC_CONF) -o shorkfetch-conf $(LDFLAGS)
	$(STRIP) shorkfetch-conf

PREFIX ?= /usr
BINDIR = $(PREFIX)/bin

install: $(PROGS)
	install -d $(DESTDIR)$(BINDIR)
	for p in $(PROGS); do install -m 755 $$p $(DESTDIR)$(BINDIR); done

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/shorkfetch
	rm -f $(DESTDIR)$(BINDIR)/shorkfetch-conf
	rm -f $(HOME)/.config/shorkutils/shorkfetch.conf
	rm -f /home/$(SUDO_USER)/.config/shorkutils/shorkfetch.conf

clean:
	rm -f shorkfetch shorkfetch-conf

distclean: clean
	rm -rf cpuinfo-ds shorkcommon

.PHONY: all install uninstall clean distclean
