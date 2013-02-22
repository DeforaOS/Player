/* $Id$ */
/* Copyright (c) 2013 Pierre Pronchery <khorben@defora.org> */
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
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif
#ifndef LOCALEDIR
# define LOCALEDIR	DATADIR "/locale"
#endif


/* playerctl */
/* private */
/* prototypes */
static int _playerctl(PlayerMessage message);

static int _error(char const * message, int ret);
static int _usage(void);


/* functions */
/* playerctl */
static int _playerctl(PlayerMessage message)
{
	desktop_message_send(PLAYER_CLIENT_MESSAGE, message, 0, 0);
	return 0;
}


/* error */
static int _error(char const * message, int ret)
{
	fputs("playerctl: ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fputs(_("Usage: playerctl -Pps\n"), stderr);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int o;
	int message = -1;

	if(setlocale(LC_ALL, "") == NULL)
		_error("setlocale", 1);
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "FPRps")) != -1)
		switch(o)
		{
			case 'F':
				message = PLAYER_MESSAGE_FORWARD;
				break;
			case 'P':
				message = PLAYER_MESSAGE_PAUSE;
				break;
			case 'R':
				message = PLAYER_MESSAGE_REWIND;
				break;
			case 'p':
				message = PLAYER_MESSAGE_PLAY;
				break;
			case 's':
				message = PLAYER_MESSAGE_STOP;
				break;
			default:
				return _usage();
		}
	if(argc != optind || message < 0)
		return _usage();
	return (_playerctl(message) == 0) ? 0 : 2;
}
