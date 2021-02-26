/* $Id$ */
/* Copyright (c) 2006-2018 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Player */
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
