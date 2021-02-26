/* $Id$ */
/* Copyright (c) 2016-2018 Pierre Pronchery <khorben@defora.org> */
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



#ifndef PLAYER_COMMON_H
# define PLAYER_COMMON_H

# include "../config.h"

/* constants */
# ifndef PROGNAME_PLAYER
#  define PROGNAME_PLAYER	"player"
# endif
# ifndef PREFIX
#  define PREFIX		"/usr/local"
# endif
# ifndef BINDIR
#  define BINDIR		PREFIX "/bin"
# endif
# ifndef DATADIR
#  define DATADIR		PREFIX "/share"
# endif

# define PLAYER_SPLASH		DATADIR "/" PACKAGE "/splash.png"

#endif /* !PLAYER_COMMON_H */
