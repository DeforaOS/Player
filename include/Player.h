/* $Id$ */
/* Copyright (c) 2013-2021 Pierre Pronchery <khorben@defora.org> */
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



#ifndef DESKTOP_PLAYER_H
# define DESKTOP_PLAYER_H


/* Player */
/* types */
typedef enum _PlayerMute
{
	PLAYER_MUTE_UNMUTE = 0,
	PLAYER_MUTE_MUTE,
	PLAYER_MUTE_TOGGLE
} PlayerMute;

typedef enum _PlayerMessage
{
	PLAYER_MESSAGE_FORWARD = 0,
	PLAYER_MESSAGE_MUTE,		/* PlayerMute */
	PLAYER_MESSAGE_NEXT,
	PLAYER_MESSAGE_PAUSE,
	PLAYER_MESSAGE_PLAY,
	PLAYER_MESSAGE_PREVIOUS,
	PLAYER_MESSAGE_REWIND,
	PLAYER_MESSAGE_STOP
} PlayerMessage;


/* constants */
# define PLAYER_CLIENT_MESSAGE	"DEFORAOS_DESKTOP_PLAYER_CLIENT"

#endif /* !DESKTOP_PLAYER_H */
