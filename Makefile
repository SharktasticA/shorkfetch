CC ?= gcc
AR ?= ar
RANLIB ?= ranlib
STRIP ?= strip
VERSION = 0.7-wip

ifeq ($(wildcard shorkcommon/.git),)
$(shell git submodule update --init shorkcommon)
endif

CFLAGS += -DVERSION='"$(VERSION)"' -Wall -Wextra -D_GNU_SOURCE -std=gnu99 \
	-I. -O3 -flto=auto -fno-plt

ifdef SHORK_DISKETTE
	CFLAGS += -DSHORK_DISKETTE
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
SRC_CONF = $(wildcard src-conf/*.c) $(wildcard shorkcommon/*.c)

PROGS = shorkfetch
ifndef SHORK_DISKETTE
ifndef TESTS
	PROGS += shorkfetch-conf
endif
endif

all: $(PROGS)

shorkfetch: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o shorkfetch $(LDFLAGS)
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

.PHONY: all install uninstall clean
