CC ?= gcc
AR ?= ar
RANLIB ?= ranlib
STRIP ?= strip

CFLAGS += -Wall -Wextra -D_GNU_SOURCE -std=gnu99 -I.

ifdef EMBEDDED
	CFLAGS += -DEMBEDDED
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

SRC = src/*.c

shorkfetch: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o shorkfetch $(LDFLAGS)
	$(STRIP) shorkfetch

PREFIX ?= /usr
BINDIR = $(PREFIX)/bin

install: shorkfetch
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 shorkfetch $(DESTDIR)$(BINDIR)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/shorkfetch
	rm -f $(HOME)/.config/shorkutils/shorkfetch.conf
	rm -f /home/$(SUDO_USER)/.config/shorkutils/shorkfetch.conf

clean:
	rm -f shorkfetch

.PHONY: install uninstall clean
