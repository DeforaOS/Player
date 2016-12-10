/* $Id$ */
/* Copyright (c) 2016 Pierre Pronchery <khorben@defora.org> */
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



#ifndef PLAYER_BACKEND_H
# define PLAYER_BACKEND_H

# include "player.h"


/* types */
typedef struct _PlayerBackend PlayerBackend;


PlayerBackend * playerbackend_init(Player * player);
void playerbackend_destroy(PlayerBackend * player);

GtkWidget * playerbackend_get_widget(PlayerBackend * player);

/* useful */
void playerbackend_message(PlayerBackend * player, char const * message,
		unsigned int duration);
void playerbackend_reset(PlayerBackend * player);

/* playback */
void playerbackend_forward(PlayerBackend * player);
void playerbackend_mute(PlayerBackend * player, PlayerMute mute);
int playerbackend_open(PlayerBackend * player, char const * filename,
		gboolean autoplay);
void playerbackend_pause(PlayerBackend * player);
void playerbackend_play(PlayerBackend * player);
void playerbackend_properties(PlayerBackend * player);
void playerbackend_rewind(PlayerBackend * player);
void playerbackend_seek(PlayerBackend * player, gdouble position);
int playerbackend_start(PlayerBackend * player);
void playerbackend_stop(PlayerBackend * player);
void playerbackend_switch_angle(PlayerBackend * player);
void playerbackend_switch_audio(PlayerBackend * player);
void playerbackend_switch_subtitles(PlayerBackend * player);
void playerbackend_volume(PlayerBackend * player, gdouble volume);

#endif /* !PLAYER_BACKEND_H */
