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



#ifndef PLAYER_PLAYER_H
# define PLAYER_PLAYER_H

# include <gtk/gtk.h>
# include "Player.h"


/* Player */
/* types */
typedef struct _Player Player;

typedef enum _PlayerMetadata
{
	PM_ALBUM = 0,
	PM_ARTIST,
	PM_COMMENT,
	PM_GENRE,
	PM_LENGTH,
	PM_TITLE,
	PM_TRACK,
	PM_YEAR
} PlayerMetadata;


/* functions */
Player * player_new(void);
void player_delete(Player * player);

/* accessors */
char * player_get_filename(Player * player);
gboolean player_get_fullscreen(Player * player);
gboolean player_get_paused(Player * player);

void player_set_fullscreen(Player * player, gboolean fullscreen);
void player_set_metadata(Player * player, PlayerMetadata metadata,
		char const * value);
void player_set_paused(Player * player, gboolean paused);
void player_set_progress(Player * player, unsigned int progress);
void player_set_seekable(Player * player, gboolean seekable);
void player_set_size(Player * player, int width, int height);
void player_set_volume(Player * player, gdouble volume);

/* useful */
void player_about(Player * player);
int player_error(Player * player, char const * message, int ret);
void player_reset(Player * player, char const * filename);

/* playback */
void player_switch_angle(Player * player);
void player_switch_audio(Player * player);
void player_switch_subtitles(Player * player);

void player_mute(Player * player, PlayerMute mute);

/* playlist management */
int player_open(Player * player, char const * filename);
int player_open_dialog(Player * player);
int player_open_dvd(Player * player);
int player_open_url(Player * player, char const * url);
int player_open_url_dialog(Player * player);
void player_playlist_add(Player * player, char const * filename);
void player_playlist_add_dialog(Player * player);
void player_playlist_add_url(Player * player, char const * url);
void player_playlist_clear(Player * player);
void player_playlist_open(Player * player, char const * filename);
void player_playlist_open_dialog(Player * player);
void player_playlist_play_selected(Player * player);
void player_playlist_remove_selection(Player * player);
void player_playlist_reordered(Player * player);
void player_playlist_save_as_dialog(Player * player);

/* playback */
void player_forward(Player * player);
void player_next(Player * player);
void player_pause(Player * player);
void player_play(Player * player);
void player_previous(Player * player);
void player_seek(Player * player, gdouble position);
void player_stop(Player * player);
void player_rewind(Player * player);

/* user interface */
void player_show_playlist(Player * player, gboolean show);
void player_show_preferences(Player * player, gboolean show);
void player_show_properties(Player * player, gboolean show);

#endif /* !PLAYER_PLAYER_H */
