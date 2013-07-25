TARGETS	= player playerctl
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
SBINDIR	= $(PREFIX)/sbin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags libDesktop`
CFLAGS	= -Wall -g -pedantic
LDFLAGSF= `pkg-config --libs libDesktop` -lintl
LDFLAGS	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: $(TARGETS)

player_OBJS = callbacks.o player.o main.o
player_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
player_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

player: $(player_OBJS)
	$(CC) -o player $(player_OBJS) $(player_LDFLAGS)

playerctl_OBJS = playerctl.o
playerctl_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
playerctl_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

playerctl: $(playerctl_OBJS)
	$(CC) -o playerctl $(playerctl_OBJS) $(playerctl_LDFLAGS)

callbacks.o: callbacks.c callbacks.h player.h ../include/Player.h ../config.h
	$(CC) $(player_CFLAGS) -c callbacks.c

player.o: player.c callbacks.h player.h ../include/Player.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(player_CFLAGS) -c player.c

main.o: main.c player.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(player_CFLAGS) -c main.c

playerctl.o: playerctl.c ../include/Player.h ../config.h
	$(CC) $(playerctl_CFLAGS) -c playerctl.c

clean:
	$(RM) -- $(player_OBJS) $(playerctl_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 player $(DESTDIR)$(BINDIR)/player
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 playerctl $(DESTDIR)$(BINDIR)/playerctl

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/player
	$(RM) -- $(DESTDIR)$(BINDIR)/playerctl

.PHONY: all clean distclean install uninstall
