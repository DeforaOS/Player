TARGETS	= $(OBJDIR)player $(OBJDIR)playerctl
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
SBINDIR	= $(PREFIX)/sbin
CC	= cc
CPPFLAGSF= -I ../include
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

player_OBJS = $(OBJDIR)callbacks.o $(OBJDIR)player.o $(OBJDIR)main.o
player_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
player_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

$(OBJDIR)player: $(player_OBJS)
	$(CC) -o $(OBJDIR)player $(player_OBJS) $(player_LDFLAGS)

playerctl_OBJS = $(OBJDIR)playerctl.o
playerctl_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
playerctl_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

$(OBJDIR)playerctl: $(playerctl_OBJS)
	$(CC) -o $(OBJDIR)playerctl $(playerctl_OBJS) $(playerctl_LDFLAGS)

$(OBJDIR)callbacks.o: callbacks.c callbacks.h player.h ../include/Player.h ../config.h
	$(CC) $(player_CFLAGS) -o $(OBJDIR)callbacks.o -c callbacks.c

$(OBJDIR)player.o: player.c callbacks.h player.h ../include/Player.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(player_CFLAGS) -o $(OBJDIR)player.o -c player.c

$(OBJDIR)main.o: main.c player.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(player_CFLAGS) -o $(OBJDIR)main.o -c main.c

$(OBJDIR)playerctl.o: playerctl.c ../include/Player.h ../config.h
	$(CC) $(playerctl_CFLAGS) -o $(OBJDIR)playerctl.o -c playerctl.c

clean:
	$(RM) -- $(player_OBJS) $(playerctl_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 $(OBJDIR)player $(DESTDIR)$(BINDIR)/player
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 $(OBJDIR)playerctl $(DESTDIR)$(BINDIR)/playerctl

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/player
	$(RM) -- $(DESTDIR)$(BINDIR)/playerctl

.PHONY: all clean distclean install uninstall
