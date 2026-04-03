CC ?= gcc
AR ?= ar
RANLIB ?= ranlib
STRIP ?= strip

CFLAGS += -I.
LDFLAGS += -static

ifdef COL
    CFLAGS += -DCOL=$(COL)
endif

ifdef NO_ART
	CFLAGS += -DNO_ART
endif

SRC = main.c

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

clean:
	rm -f shorkfetch

.PHONY: install uninstall clean
