/* $Id$ */
/* Copyright (c) 2013-2021 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Player */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <Desktop.h>
#include "../include/Player.h"
#include "../config.h"
#define _(string) gettext(string)

/* constants */
#ifndef PROGNAME_PLAYERCTL
# define PROGNAME_PLAYERCTL	"playerctl"
#endif
#ifndef PREFIX
# define PREFIX			"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR		PREFIX "/share"
#endif
#ifndef LOCALEDIR
# define LOCALEDIR		DATADIR "/locale"
#endif


/* playerctl */
/* private */
/* prototypes */
static int _playerctl(PlayerMessage message, unsigned int arg1);

static int _error(char const * message, int ret);
static int _usage(void);


/* functions */
/* playerctl */
static int _playerctl(PlayerMessage message, unsigned int arg1)
{
	desktop_message_send(PLAYER_CLIENT_MESSAGE, message, arg1, 0);
	return 0;
}


/* error */
static int _error(char const * message, int ret)
{
	fputs(PROGNAME_PLAYERCTL ": ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, _("Usage: %s -FMmNPRpsu\n"), PROGNAME_PLAYERCTL);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int o;
	int message = -1;
	unsigned int arg1 = 0;

	if(setlocale(LC_ALL, "") == NULL)
		_error("setlocale", 1);
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "FMmNPRpsu")) != -1)
		switch(o)
		{
			case 'F':
				message = PLAYER_MESSAGE_FORWARD;
				arg1 = 0;
				break;
			case 'm':
				message = PLAYER_MESSAGE_MUTE;
				arg1 = PLAYER_MUTE_MUTE;
				break;
			case 'M':
				message = PLAYER_MESSAGE_PREVIOUS;
				arg1 = 0;
				break;
			case 'N':
				message = PLAYER_MESSAGE_NEXT;
				arg1 = 0;
				break;
			case 'P':
				message = PLAYER_MESSAGE_PAUSE;
				arg1 = 0;
				break;
			case 'R':
				message = PLAYER_MESSAGE_REWIND;
				arg1 = 0;
				break;
			case 'p':
				message = PLAYER_MESSAGE_PLAY;
				arg1 = 0;
				break;
			case 's':
				message = PLAYER_MESSAGE_STOP;
				arg1 = 0;
				break;
			case 'u':
				message = PLAYER_MESSAGE_MUTE;
				arg1 = PLAYER_MUTE_UNMUTE;
				break;
			default:
				return _usage();
		}
	if(argc != optind || message < 0)
		return _usage();
	return (_playerctl(message, arg1) == 0) ? 0 : 2;
}
