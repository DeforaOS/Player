/* $Id$ */
/* Copyright (c) 2016-2021 Pierre Pronchery <khorben@defora.org> */
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
