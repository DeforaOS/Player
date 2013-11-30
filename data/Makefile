PREFIX	= /usr/local
DESTDIR	=
MKDIR	= mkdir -m 0755 -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/applications
	$(INSTALL) -m 0644 deforaos-player.desktop $(DESTDIR)$(PREFIX)/share/applications/deforaos-player.desktop
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/Player
	$(INSTALL) -m 0644 splash.png $(DESTDIR)$(PREFIX)/share/Player/splash.png

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/applications/deforaos-player.desktop
	$(RM) -- $(DESTDIR)$(PREFIX)/share/Player/splash.png

.PHONY: all clean distclean install uninstall
