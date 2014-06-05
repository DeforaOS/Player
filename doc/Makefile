TARGETS	= index.html player.1 player.html playerctl.1 playerctl.html
PREFIX	= /usr/local
DESTDIR	=
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: $(TARGETS)

index.html: index.xml index.xsl player.xml playerctl.xml
	./docbook.sh -P "$(PREFIX)" -- "index.html"

player.1: player.xml
	./docbook.sh -P "$(PREFIX)" -- "player.1"

player.html: manual.css.xml player.css.xml player.xml
	./docbook.sh -P "$(PREFIX)" -- "player.html"

playerctl.1: playerctl.xml
	./docbook.sh -P "$(PREFIX)" -- "playerctl.1"

playerctl.html: manual.css.xml playerctl.css.xml playerctl.xml
	./docbook.sh -P "$(PREFIX)" -- "playerctl.html"

clean:
	$(RM) -- $(index.html_OBJS) $(player.1_OBJS) $(player.html_OBJS) $(playerctl.1_OBJS) $(playerctl.html_OBJS)
	./docbook.sh -c -P "$(PREFIX)" -- "playerctl.html"

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "player.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "player.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "playerctl.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "playerctl.html"

uninstall:
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "player.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "player.html"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "playerctl.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "playerctl.html"

.PHONY: all clean distclean install uninstall
