/* $Id$ */
/* Copyright (c) 2006-2016 Pierre Pronchery <khorben@defora.org> */
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
#include "player.h"
#include "../config.h"
#define _(string) gettext(string)

/* constants */
#ifndef PROGNAME_PLAYER
# define PROGNAME_PLAYER	"player"
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


/* private */
/* functions */
/* usage */
static int _usage(void)
{
	fprintf(stderr, _("Usage: %s [filename...]\n"), PROGNAME_PLAYER);
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int o;
	int i;
	Player * player;

	if(setlocale(LC_ALL, "") == NULL)
		player_error(NULL, "setlocale", 1);
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "")) != -1)
		switch(o)
		{
			default:
				return _usage();
		}
	if((player = player_new()) == NULL)
		return 2;
	if(optind < argc)
		/* FIXME might be an URL */
		player_open(player, argv[optind]);
	for(i = optind + 1; i < argc; i++)
		player_playlist_add(player, argv[i]);
	gtk_main();
	player_delete(player);
	return 0;
}
