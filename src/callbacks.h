/* $Id$ */
/* Copyright (c) 2006-2021 Pierre Pronchery <khorben@defora.org> */
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



#ifndef PLAYER_CALLBACKS_H
# define PLAYER_CALLBACKS_H

# include <gtk/gtk.h>


/* player */
gboolean on_player_closex(gpointer data);
int on_player_message(void * data, uint32_t value1, uint32_t value2,
		uint32_t value3);

/* file menu */
void on_file_open(gpointer data);
void on_file_open_dvd(gpointer data);
void on_file_open_url(gpointer data);
void on_file_properties(gpointer data);
void on_file_close(gpointer data);

/* edit menu */
void on_edit_preferences(gpointer data);

/* view menu */
void on_view_fullscreen(gpointer data);
void on_view_playlist(gpointer data);
void on_view_switch_angle(gpointer data);
void on_view_switch_audio(gpointer data);
void on_view_switch_subtitles(gpointer data);

/* help menu */
void on_help_about(gpointer data);
void on_help_contents(gpointer data);

/* toolbar */
void on_close(gpointer data);
void on_open(gpointer data);
void on_properties(gpointer data);
void on_preferences(gpointer data);

/* playbar */
void on_previous(gpointer data);
void on_rewind(gpointer data);
void on_play(gpointer data);
void on_pause(gpointer data);
void on_stop(gpointer data);
void on_forward(gpointer data);
void on_next(gpointer data);
void on_progress_changed(gpointer data);
void on_volume_changed(gpointer data);
void on_fullscreen(gpointer data);

/* view */
/* playlist */
void on_playlist_activated(gpointer data);
void on_playlist_add(gpointer data);
gboolean on_playlist_closex(gpointer data);
void on_playlist_load(gpointer data);
void on_playlist_remove(gpointer data);
void on_playlist_row_deleted(gpointer data);
void on_playlist_row_inserted(gpointer data);
void on_playlist_save(gpointer data);

#endif /* !PLAYER_CALLBACKS_H */
