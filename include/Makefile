PREFIX	= /usr/local
DESTDIR	=
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install
INCLUDEDIR= $(PREFIX)/include


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/include/Desktop
	$(INSTALL) -m 0644 Player.h $(DESTDIR)$(PREFIX)/include/Desktop/Player.h

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/include/Desktop/Player.h

.PHONY: all clean distclean install uninstall
