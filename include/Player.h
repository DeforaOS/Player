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



#ifndef DESKTOP_PLAYER_H
# define DESKTOP_PLAYER_H


/* Player */
/* types */
typedef enum _PlayerMessage
{
	PLAYER_MESSAGE_PLAY = 0,
	PLAYER_MESSAGE_PAUSE,
	PLAYER_MESSAGE_STOP
} PlayerMessage;


/* constants */
# define PLAYER_CLIENT_MESSAGE	"DEFORAOS_DESKTOP_PLAYER_CLIENT"

#endif /* !DESKTOP_PLAYER_H */
