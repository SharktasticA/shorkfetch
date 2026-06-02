CC ?= gcc
AR ?= ar
RANLIB ?= ranlib
STRIP ?= strip

CFLAGS += -I.

ifdef TESTS
	CFLAGS += -DTESTS
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
