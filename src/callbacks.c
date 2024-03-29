/* $Id$ */
/* Copyright (c) 2006-2023 Pierre Pronchery <khorben@defora.org> */
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



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include <Desktop.h>
#include "../include/Player.h"
#include "player.h"
#include "callbacks.h"
#include "common.h"
#include "../config.h"


/* macros */
#define max(a, b) (((a) > (b)) ? (a) : (b))


/* functions */
/* callbacks */
/* on_close */
void on_close(gpointer data)
{
	Player * player = data;

	on_player_closex(player);
}


/* on_player_closex */
gboolean on_player_closex(gpointer data)
{
	Player * player = data;

	player_stop(player);
	gtk_main_quit();
	return TRUE;
}


/* on_player_message */
int on_player_message(void * data, uint32_t value1, uint32_t value2,
		uint32_t value3)
{
	Player * player = data;
	PlayerMessage message = value1;
	PlayerMute mute;

	switch(message)
	{
		case PLAYER_MESSAGE_FORWARD:
			player_forward(player);
			break;
		case PLAYER_MESSAGE_MUTE:
			mute = value2;
			player_mute(player, mute);
			break;
		case PLAYER_MESSAGE_NEXT:
			player_next(player);
			break;
		case PLAYER_MESSAGE_PAUSE:
			player_pause(player);
			break;
		case PLAYER_MESSAGE_PLAY:
			player_play(player);
			break;
		case PLAYER_MESSAGE_PREVIOUS:
			player_previous(player);
			break;
		case PLAYER_MESSAGE_REWIND:
			player_rewind(player);
			break;
		case PLAYER_MESSAGE_STOP:
			player_stop(player);
			break;
	}
	return 0;
}


/* on_file_open */
void on_file_open(gpointer data)
{
	Player * player = data;

	player_open_dialog(player);
}


/* on_file_open_dvd */
void on_file_open_dvd(gpointer data)
{
	Player * player = data;

	player_open_dvd(player);
}


/* on_file_open_url */
void on_file_open_url(gpointer data)
{
	Player * player = data;

	player_open_url_dialog(player);
}


/* on_file_properties */
void on_file_properties(gpointer data)
{
	Player * player = data;

	player_show_properties(player, TRUE);
}


/* on_file_close */
void on_file_close(gpointer data)
{
	Player * player = data;

	on_player_closex(player);
}


/* on_edit_preferences */
void on_edit_preferences(gpointer data)
{
	Player * player = data;

	player_show_preferences(player, TRUE);
}


/* on_view_fullscreen */
void on_view_fullscreen(gpointer data)
{
	Player * player = data;

	player_set_fullscreen(player, !player_get_fullscreen(player));
}


/* on_view_playlist */
void on_view_playlist(gpointer data)
{
	Player * player = data;

	player_show_playlist(player, TRUE);
}


/* on_view_switch_angle */
void on_view_switch_angle(gpointer data)
{
	Player * player = data;

	player_switch_angle(player);
}


/* on_view_switch_audio */
void on_view_switch_audio(gpointer data)
{
	Player * player = data;

	player_switch_audio(player);
}


/* on_view_switch_subtitles */
void on_view_switch_subtitles(gpointer data)
{
	Player * player = data;

	player_switch_subtitles(player);
}


/* on_help_about */
void on_help_about(gpointer data)
{
	Player * player = data;

	player_about(player);
}


/* on_help_contents */
void on_help_contents(gpointer data)
{
	(void) data;

	desktop_help_contents(PACKAGE, PROGNAME_PLAYER);
}


/* on_forward */
void on_forward(gpointer data)
{
	Player * player = data;

	player_forward(player);
}


/* on_fullscreen */
void on_fullscreen(gpointer data)
{
	Player * player = data;

	player_set_fullscreen(player, !player_get_fullscreen(player));
}


/* on_next */
void on_next(gpointer data)
{
	Player * player = data;

	player_next(player);
}


/* on_open */
void on_open(gpointer data)
{
	Player * player = data;

	player_open_dialog(player);
}


/* on_preferences */
void on_preferences(gpointer data)
{
	Player * player = data;

	player_show_preferences(player, TRUE);
}


/* on_properties */
void on_properties(gpointer data)
{
	Player * player = data;

	player_show_properties(player, TRUE);
}


/* on_pause */
void on_pause(gpointer data)
{
	Player * player = data;

	player_pause(player);
}


/* on_play */
void on_play(gpointer data)
{
	Player * player = data;

	player_play(player);
}


/* on_previous */
void on_previous(gpointer data)
{
	Player * player = data;

	player_previous(player);
}


/* on_progress_changed */
void on_progress_changed(gpointer data)
{
	Player * player = data;

	/* XXX hack */
	player_seek(player, -1.0);
}


/* on_rewind */
void on_rewind(gpointer data)
{
	Player * player = data;

	player_rewind(player);
}


/* on_stop */
void on_stop(gpointer data)
{
	Player * player = data;

	player_stop(player);
}


/* on_volume_changed */
void on_volume_changed(gpointer data)
{
	Player * player = data;

	/* XXX ugly hack */
	player_set_volume(player, 0.0 / 0.0);
}


/* view */
/* playlist */
/* on_playlist_activated */
void on_playlist_activated(gpointer data)
{
	Player * player = data;

	player_playlist_play_selected(player);
}


/* on_playlist_add */
void on_playlist_add(gpointer data)
{
	Player * player = data;

	player_playlist_add_dialog(player);
}


/* on_playlist_closex */
gboolean on_playlist_closex(gpointer data)
{
	Player * player = data;

	player_show_playlist(player, FALSE);
	return TRUE;
}


/* on_playlist_load */
void on_playlist_load(gpointer data)
{
	Player * player = data;

	player_playlist_open_dialog(player);
}


/* on_playlist_remove */
void on_playlist_remove(gpointer data)
{
	Player * player = data;

	player_playlist_remove_selection(player);
}


/* on_playlist_row_deleted */
void on_playlist_row_deleted(gpointer data)
{
	Player * player = data;

	player_playlist_reordered(player);
}


/* on_playlist_row_inserted */
void on_playlist_row_inserted(gpointer data)
{
	Player * player = data;

	player_playlist_reordered(player);
}


/* on_playlist_save */
void on_playlist_save(gpointer data)
{
	Player * player = data;

	player_playlist_save_as_dialog(player);
}
