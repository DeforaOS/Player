/* $Id$ */
/* Copyright (c) 2013-2021 Pierre Pronchery <khorben@defora.org> */
/* All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
/* This file is part of DeforaOS Desktop Player */



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
