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



#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <System.h>
#include <Desktop.h>
#if GTK_CHECK_VERSION(3, 0, 0)
# include <gtk/gtkx.h>
#endif
#include "../common.h"

/* constants */
#ifndef PROGNAME_MPLAYER
# define PROGNAME_MPLAYER	"mplayer"
#endif


/* types */
struct _PlayerBackend
{
	Player * player;

	/* widgets */
	GtkWidget * view;

	/* mplayer */
	pid_t pid;
	int fd[2][2];				/* mplayer pipes	*/
	GIOChannel * channel[2];
	char * buf;				/* pipe write buffer	*/
	size_t buf_len;

	/* callbacks */
	guint read_id;				/* pipe read source id	*/
	guint write_id;				/* pipe write source id	*/
	guint timeout_id;			/* timeout source id	*/

	/* metadata */
	int width;
	int height;
	int audio_bitrate;
	int audio_channels;
	char * audio_codec;
	int audio_rate;
	gdouble length;
	gdouble video_aspect;
	int video_bitrate;
	char * video_codec;
	gdouble video_fps;
	int video_rate;
	int album;
	int artist;
	int title;
};


/* functions */
static void _playerbackend_set_size(PlayerBackend * player, int width,
		int height);

/* callbacks */
static int _playerbackend_command(PlayerBackend * player, char const * cmd,
		size_t cmd_len);

static int _playerbackend_on_sigchld(PlayerBackend * player);

static gboolean _command_read(GIOChannel * source, GIOCondition condition,
		gpointer data);
static gboolean _command_timeout(gpointer data);
static gboolean _command_write(GIOChannel * source, GIOCondition condition,
		gpointer data);


/* variables */
/* XXX needed for the signal handler */
static PlayerBackend * _player;


/* public */
/* playerbackend_init */
static void _init_on_plug_removed(gpointer data);
static void _init_signal(void);
static void _init_signal_handler(int signum);

PlayerBackend * playerbackend_init(Player * player)
{
	PlayerBackend * backend;
#if GTK_CHECK_VERSION(3, 0, 0)
	const GdkRGBA black = { 0.0, 0.0, 0.0, 0.0 };
#else
	const GdkColor black = { 0, 0, 0, 0 };
#endif

	if((backend = object_new(sizeof(*backend))) == NULL)
		return NULL;
	_player = backend;
	backend->player = player;
	backend->view = gtk_socket_new();
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_widget_override_background_color(backend->view,
			GTK_STATE_FLAG_NORMAL, &black);
#else
	gtk_widget_modify_bg(backend->view, GTK_STATE_NORMAL, &black);
#endif
	g_signal_connect_swapped(backend->view, "plug-removed",
			G_CALLBACK(_init_on_plug_removed), backend);
	backend->pid = -1;
	memset(&backend->fd, 0, sizeof(backend->fd));
	backend->buf = NULL;
	backend->buf_len = 0;
	backend->read_id = 0;
	backend->write_id = 0;
	backend->timeout_id = 0;
	backend->audio_codec = NULL;
	backend->video_codec = NULL;
	playerbackend_reset(backend);
	_init_signal();
	return backend;
}

static void _init_on_plug_removed(gpointer data)
{
	/* FIXME implement */
}

static void _init_signal(void)
	/* handle mplayer death; should be done in Player as a callback but
	 * would potentially conflict with other Player instances */
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = _init_signal_handler;
	sigfillset(&sa.sa_mask);
	if(sigaction(SIGCHLD, &sa, NULL) == -1)
		fputs(PROGNAME_PLAYER ": SIGCHLD: Not handled\n", stderr);
}

static void _init_signal_handler(int signum)
{
	PlayerBackend * player = _player;
	pid_t pid;
	int status;

	if(signum != SIGCHLD)
		return;
	if(_playerbackend_on_sigchld(player) == 0)
		return;
	if((pid = waitpid(-1, &status, WNOHANG)) == -1)
	{
		player_error(NULL, "waitpid", 1);
		return;
	}
	if(pid == 0)
		return;
	fputs(PROGNAME_PLAYER ": ", stderr);
	if(WIFEXITED(status))
		fprintf(stderr, "%s%d%s%u\n", "child ", pid,
				": exited with code ", WEXITSTATUS(status));
	else
		fprintf(stderr, "%d%s", pid, ": Unknown state\n");
}


/* playerbackend_destroy */
void playerbackend_destroy(PlayerBackend * player)
{
	char const cmd[] = "\nquit\n";
	gsize written;
	size_t i;
	int status = 0;
	pid_t res;
	struct timespec ts = { 0, 500000 };

	if(player->read_id != 0)
		g_source_remove(player->read_id);
	if(player->write_id != 0)
		g_source_remove(player->write_id);
	if(player->timeout_id != 0)
		g_source_remove(player->timeout_id);
	g_io_channel_write_chars(player->channel[1], cmd, sizeof(cmd) - 1,
			&written, NULL);
	g_io_channel_shutdown(player->channel[1], FALSE, NULL);
	for(i = 0; i < 6; i++)
	{
		if(player->pid == -1)
			break;
		if((res = waitpid(player->pid, &status, WNOHANG)) == -1)
		{
			player_error(NULL, "waitpid", 0);
			break;
		}
		else if(res == 0)
			nanosleep(&ts, NULL);
		else if(WIFEXITED(status) || WIFSIGNALED(status))
			break;
		if(i == 4)
			kill(player->pid, SIGTERM);
	}
	object_delete(player);
}


/* playerbackend_get_widget */
GtkWidget * playerbackend_get_widget(PlayerBackend * player)
{
	return player->view;
}


/* useful */
/* playerbackend_message */
void playerbackend_message(PlayerBackend * player, char const * message,
		unsigned int duration)
{
	char const cmd[] = "pausing_keep osd_show_text";
	char buf[128];
	int len;

	if((len = snprintf(buf, sizeof(buf), "%s \"%s\" %u\n", cmd, message,
					duration)) >= (int)sizeof(buf))
		fputs(PROGNAME_PLAYER ": String too long\n", stderr);
	else
		_playerbackend_command(player, buf, len);
}


/* playerbackend_reset */
void playerbackend_reset(PlayerBackend * player)
{
	player->width = 0;
	player->height = 0;
	player->audio_bitrate = 0;
	player->audio_channels = 0;
	if(player->audio_codec != NULL)
		free(player->audio_codec);
	player->audio_codec = NULL;
	player->audio_rate = 0;
	player->video_aspect = 0.0;
	player->video_bitrate = 0;
	if(player->video_codec != NULL)
		free(player->video_codec);
	player->video_codec = NULL;
	player->video_fps = 0.0;
	player->video_rate = 0;
	player->album = -1;
	player->artist = -1;
	player->title = -1;
}


/* playback */
/* playerbackend_forward */
void playerbackend_forward(PlayerBackend * player)
{
	char const cmd[] = "seek 10 0\n";

	_playerbackend_command(player, cmd, sizeof(cmd) - 1);
}


/* playerbackend_mute */
void playerbackend_mute(PlayerBackend * player, PlayerMute mute)
{
	char cmd[8];
	int len;

	len = snprintf(cmd, sizeof(cmd), "%s%s%s\n", "mute",
			(mute != PLAYER_MUTE_TOGGLE) ? " " : "",
			(mute != PLAYER_MUTE_TOGGLE)
			? (mute == PLAYER_MUTE_UNMUTE ? "0" : "1") : "");
	if(len >= (int)sizeof(cmd))
		fputs(PROGNAME_PLAYER ": String too long\n", stderr);
	else
		_playerbackend_command(player, cmd, len);
}


/* playerbackend_open */
int playerbackend_open(PlayerBackend * player, char const * filename,
		gboolean autoplay)
{
	char cmd[512];
	size_t len;

	len = snprintf(cmd, sizeof(cmd), "%s%s%s%s%s",
			autoplay ? "" : "pausing ",
			"loadfile \"", filename, "\" 0\n",
			autoplay ? "" : "frame_step\n");
	if(len >= sizeof(cmd))
	{
		fputs(PROGNAME_PLAYER ": String too long\n", stderr);
		return -1;
	}
	if(_playerbackend_command(player, cmd, len) != 0)
		return -1;
	/* XXX avoid code duplication */
	player_set_paused(player->player, autoplay ? FALSE : TRUE);
	if(player_get_paused(player->player) == FALSE
			&& player->timeout_id == 0)
		player->timeout_id = g_timeout_add(500, _command_timeout,
				player);
	return 0;
}


/* playerbackend_pause */
void playerbackend_pause(PlayerBackend * player)
{
	char const cmd[] = "pause\n";

	if(player_get_paused(player->player) != 0)
	{
		if(player->timeout_id == 0)
			player->timeout_id = g_timeout_add(500,
					_command_timeout, player);
	}
	else if(player->timeout_id != 0)
	{
		g_source_remove(player->timeout_id);
		player->timeout_id = 0;
	}
	_playerbackend_command(player, cmd, sizeof(cmd) - 1);
}


/* playerbackend_play */
void playerbackend_play(PlayerBackend * player)
{
	char cmd[512];
	size_t len;
	char * filename;

	if((filename = player_get_filename(player->player)) == NULL)
		return;
	/* FIXME escape double quotes in filename? */
	if(player_get_paused(player->player))
		len = snprintf(cmd, sizeof(cmd), "%s", "pause\n");
	else
		len = snprintf(cmd, sizeof(cmd), "%s%s%s", "loadfile \"",
				filename, "\" 0\n");
	if(len >= sizeof(cmd))
	{
		fputs(PROGNAME_PLAYER ": String too long\n", stderr);
		return;
	}
	else
		player_reset(player->player, filename);
	free(filename);
	if(_playerbackend_command(player, cmd, len) != 0)
		return;
	player_set_paused(player->player, 0);
	if(player->read_id == 0)
		player->read_id = g_io_add_watch(player->channel[0], G_IO_IN,
				_command_read, player);
	if(player->timeout_id == 0)
		player->timeout_id = g_timeout_add(500, _command_timeout,
				player);
}


/* playerbackend_properties */
void playerbackend_properties(PlayerBackend * player)
{
	char const buf[] = "get_meta_album\nget_meta_artist\nget_meta_comment\n"
		"get_meta_genre\nget_meta_title\nget_meta_track\n"
		"get_meta_year\n";

	_playerbackend_command(player, buf, sizeof(buf) - 1);
}


/* playerbackend_rewind */
void playerbackend_rewind(PlayerBackend * player)
{
	char const cmd[] = "seek -10 0\n";

	_playerbackend_command(player, cmd, sizeof(cmd) - 1);
}


/* playerbackend_seek */
void playerbackend_seek(PlayerBackend * player, gdouble position)
{
	char buf[48];
	int len;

	len = snprintf(buf, sizeof(buf), "%s %.1f %d\n", "pausing_keep seek",
			position, 1);
	_playerbackend_command(player, buf, len);
}


/* playerbackend_stop */
void playerbackend_stop(PlayerBackend * player)
{
	char const cmd[] = "stop\n";

	_playerbackend_command(player, cmd, sizeof(cmd) - 1);
	player_seek(player->player, 0);
	/* FIXME also needs a stopped state */
	player_set_paused(player->player, FALSE);
	if(player->read_id != 0)
	{
		g_source_remove(player->read_id);
		player->read_id = 0;
	}
	if(player->timeout_id != 0)
	{
		g_source_remove(player->timeout_id);
		player->timeout_id = 0;
	}
}


/* playerbackend_switch_angle */
void playerbackend_switch_angle(PlayerBackend * player)
{
	const char cmd[] = "switch_angle\n";

	_playerbackend_command(player, cmd, sizeof(cmd) - 1);
}


/* playerbackend_switch_audio */
void playerbackend_switch_audio(PlayerBackend * player)
{
	const char cmd[] = "switch_audio\n";

	_playerbackend_command(player, cmd, sizeof(cmd) - 1);
}


/* playerbackend_switch_subtitles */
void playerbackend_switch_subtitles(PlayerBackend * player)
{
	const char cmd[] = "sub_visibility\n";

	_playerbackend_command(player, cmd, sizeof(cmd) - 1);
}


/* playerbackend_volume */
void playerbackend_volume(PlayerBackend * player, gdouble volume)
{
	char buf[256];
	int len;

	len = snprintf(buf, sizeof(buf), "volume %u 1\n",
			(unsigned)(volume * 100.0));
	_playerbackend_command(player, buf, len);
}


/* private */
/* playerbackend_set_size */
static void _playerbackend_set_size(PlayerBackend * player, int width,
		int height)
{
	if(width < 0)
		width = player->width;
	if(height < 0)
		height = player->height;
	player_set_size(player->player, player->width, player->height);
	player->width = width;
	player->height = height;
}


/* playerbackend_command */
static gboolean _command_on_timeout(gpointer data);

static int _playerbackend_command(PlayerBackend * player, char const * cmd,
		size_t cmd_len)
{
	char * p;

	if(player->pid == -1)
	{
		fputs(PROGNAME_PLAYER ": " PROGNAME_MPLAYER " not running\n",
				stderr);
		if(player->timeout_id != 0)
			g_source_remove(player->timeout_id);
		player->timeout_id = g_timeout_add(1000, _command_on_timeout,
				player);
		return -1;
	}
#ifdef DEBUG
	fprintf(stderr, "%s%d%s\"%s\"\n", "DEBUG: pid ", player->pid,
			": write ", cmd);
#endif
	if((p = realloc(player->buf, player->buf_len + cmd_len)) == NULL)
		return -player_error(NULL, "malloc", 1);
	player->buf = p;
	memcpy(&p[player->buf_len], cmd, cmd_len);
	player->buf_len += cmd_len;
	if(player->write_id == 0)
		player->write_id = g_io_add_watch(player->channel[1], G_IO_OUT,
				_command_write, player);
	return 0;
}

static gboolean _command_on_timeout(gpointer data)
{
	PlayerBackend * player = data;

	if(playerbackend_start(player) != 0)
		return TRUE;
	player->timeout_id = 0;
	return FALSE;
}


/* playerbackend_start */
int playerbackend_start(PlayerBackend * player)
{
	int ret;
	char const buf[] = "pausing loadfile " PLAYER_SPLASH " 0\nframe_step\n";
	char wid[32];
	/* FIXME configuration value for the path to the binary? */
	char * argv[] = { BINDIR "/" PROGNAME_MPLAYER, PROGNAME_MPLAYER,
		"-slave", "-wid", NULL, "-quiet", "-idle", "-framedrop",
		"-softvol", "-softvol-max", "200", "-identify",
		"-noconsolecontrols", "-nomouseinput", NULL };
	GError * error = NULL;

	argv[4] = wid;
	/* XXX not portable */
	snprintf(wid, sizeof(wid), "%lu", (unsigned long)gtk_socket_get_id(
				GTK_SOCKET(player->view)));
	if(pipe(player->fd[0]) != 0 || pipe(player->fd[1]) != 0)
		return -player_error(player->player, strerror(errno), 1);
	if((player->pid = fork()) == -1)
		return -player_error(player->player, strerror(errno), 1);
	if(player->pid == 0) /* child */
	{
		close(player->fd[0][0]);
		close(player->fd[1][1]);
		if(dup2(player->fd[1][0], 0) == -1)
			exit(player_error(NULL, "dup2", 2));
		if(dup2(player->fd[0][1], 1) == -1)
			exit(player_error(NULL, "dup2", 2));
		if((ret = execv(argv[0], &argv[1])) == 0)
			ret = 2;
		exit(player_error(NULL, argv[0], ret));
	}
	close(player->fd[0][1]);
	close(player->fd[1][0]);
	player->channel[0] = g_io_channel_unix_new(player->fd[0][0]);
	if(g_io_channel_set_encoding(player->channel[0], NULL, &error)
			!= G_IO_STATUS_NORMAL)
	{
		player_error(player->player, error->message, 1);
		g_error_free(error);
		error = NULL;
	}
	g_io_channel_set_buffered(player->channel[0], FALSE);
	player->read_id = g_io_add_watch(player->channel[0], G_IO_IN,
			_command_read, player);
	player->channel[1] = g_io_channel_unix_new(player->fd[1][1]);
	if(g_io_channel_set_encoding(player->channel[1], NULL, &error)
			!= G_IO_STATUS_NORMAL)
	{
		player_error(player->player, error->message, 1);
		g_error_free(error);
	}
	g_io_channel_set_buffered(player->channel[1], FALSE);
	ret = _playerbackend_command(player, buf, sizeof(buf) - 1);
	player_set_paused(player->player, TRUE);
	return ret;
}


/* callbacks */
/* playerbackend_on_sigchld */
static int _playerbackend_on_sigchld(PlayerBackend * player)
{
	pid_t pid;
	int status;
	char buf[80];

	if(player->pid == -1)
		return 1;
	if((pid = waitpid(player->pid, &status, WNOHANG)) == -1)
		return player_error(player->player, "waitpid", 1);
	if(pid == 0)
		return 1;
	if(WIFEXITED(status))
	{
		snprintf(buf, sizeof(buf), "%s %d: exited with code %u",
				PROGNAME_MPLAYER, pid, WEXITSTATUS(status));
		player_error(player->player, buf, 1);
	}
	else
		fprintf(stderr, "%s: %s %d: Unknown state\n", PROGNAME_PLAYER,
				PROGNAME_MPLAYER, pid);
	player->pid = -1;
	return 0;
}


/* command_read */
static void _read_parse(PlayerBackend * player, char const * buf);

static gboolean _command_read(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	PlayerBackend * player = data;
	static char buf[512];
	static size_t buf_len = 0;
	gsize read;
	size_t i;
	size_t j;
	GIOStatus status;
	GError * error = NULL;

	if(condition != G_IO_IN)
	{
		player_error(player->player, "", 0); /* FIXME */
		gtk_main_quit();
		return FALSE; /* FIXME report error */
	}
	status = g_io_channel_read_chars(source, &buf[buf_len],
			sizeof(buf) - buf_len, &read, &error);
	if(status == G_IO_STATUS_EOF || read == 0)
	{
		player->read_id = 0;
		return FALSE; /* FIXME end of file? */
	}
	else if(status != G_IO_STATUS_NORMAL)
	{
		player_error(player->player, error->message, 1);
		g_error_free(error);
		/* FIXME recover somehow */
		gtk_main_quit();
		return FALSE;
	}
	buf_len += read;
	j = 0;
	for(i = 0; i < buf_len; i++)
	{
		if(buf[i] != '\n')
			continue;
		buf[i] = '\0';
		_read_parse(player, &buf[j]);
		j = i + 1;
	}
	buf_len -= j;
	memmove(buf, &buf[j], buf_len);
	return TRUE;
}

static void _read_parse(PlayerBackend * player, char const * buf)
{
	unsigned int u1;
	unsigned int u2;
	gdouble db;
	char str[256];
	time_t t;
	struct tm tm;

	if(sscanf(buf, "ANS_META_ALBUM='%255[^'\n]\n", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		string_rtrim(str, NULL);
		player_set_metadata(player->player, PM_ALBUM, str);
	}
	else if(sscanf(buf, "ANS_META_ARTIST='%255[^'\n]\n", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		string_rtrim(str, NULL);
		player_set_metadata(player->player, PM_ARTIST, str);
	}
	else if(sscanf(buf, "ANS_META_COMMENT='%255[^'\n]\n", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		string_rtrim(str, NULL);
		player_set_metadata(player->player, PM_COMMENT, str);
	}
	else if(sscanf(buf, "ANS_META_GENRE='%255[^'\n]\n", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		string_rtrim(str, NULL);
		player_set_metadata(player->player, PM_GENRE, str);
	}
	else if(sscanf(buf, "ANS_META_TITLE='%255[^'\n]\n", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		string_rtrim(str, NULL);
		player_set_metadata(player->player, PM_TITLE, str);
	}
	else if(sscanf(buf, "ANS_META_TRACK='%255[^'\n]\n", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		string_rtrim(str, NULL);
		player_set_metadata(player->player, PM_TRACK, str);
	}
	else if(sscanf(buf, "ANS_META_YEAR='%255[^'\n]\n", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		string_rtrim(str, NULL);
		player_set_metadata(player->player, PM_YEAR, str);
	}
	else if(sscanf(buf, "ANS_PERCENT_POSITION=%u\n", &u1) == 1)
	{
		player_set_progress(player->player, u1);
		if(u1 == 100)
			player_next(player->player);
	}
	else if(sscanf(buf, "ANS_TIME_POSITION=%lf\n", &db) == 1)
	{
		t = db;
		gmtime_r(&t, &tm);
		strftime(str, sizeof(str), "%H:%M:%S", &tm);
		player_set_metadata(player->player, PM_LENGTH, str);
	}
	else if(sscanf(buf, "ANS_VIDEO_RESOLUTION='%u x %u'\n", &u1, &u2) == 2)
		_playerbackend_set_size(player, u1, u2);
	else if(sscanf(buf, "ID_AUDIO_BITRATE=%u\n", &u1) == 1)
		player->audio_bitrate = u1;
	else if(sscanf(buf, "ID_AUDIO_CODEC=%255[^\n]", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		string_rtrim(str, NULL);
		if(player->audio_codec != NULL)
			free(player->audio_codec);
		player->audio_codec = strdup(str);
	}
	else if(sscanf(buf, "ID_AUDIO_NCH=%u\n", &u1) == 1)
		player->audio_channels = u1;
	else if(sscanf(buf, "ID_AUDIO_RATE=%u\n", &u1) == 1)
		player->audio_rate = u1;
	else if(sscanf(buf, "ID_CLIP_INFO_NAME%u=%255s", &u1, str) == 2)
	{
		str[sizeof(str) - 1] = '\0';
		string_rtrim(str, NULL);
		if(strcmp(str, "Album") == 0)
			player->album = u1;
		else if(strcmp(str, "Artist") == 0)
			player->artist = u1;
		else if(strcmp(str, "Title") == 0)
			player->title = u1;
	}
	else if(sscanf(buf, "ID_CLIP_INFO_VALUE%u=%255[^\n]", &u1, str) == 2)
	{
		str[sizeof(str) - 1] = '\0';
		string_rtrim(str, NULL);
		if(player->album >= 0 && (unsigned)player->album == u1)
			player_set_metadata(player->player, PM_ALBUM, str);
		else if(player->artist >= 0 && (unsigned)player->artist == u1)
			player_set_metadata(player->player, PM_ARTIST, str);
		else if(player->title >= 0 && (unsigned)player->title == u1)
			player_set_metadata(player->player, PM_TITLE, str);
		/* FIXME also update the duration */
	}
	else if(sscanf(buf, "ID_LENGTH=%lf\n", &db) == 1)
		player->length = db;
	else if(sscanf(buf, "ID_SEEKABLE=%u\n", &u1) == 1)
		player_set_seekable(player->player, u1 ? TRUE : FALSE);
	else if(sscanf(buf, "ID_VIDEO_ASPECT=%lf\n", &db) == 1)
		player->video_aspect = db;
	else if(sscanf(buf, "ID_VIDEO_BITRATE=%u\n", &u1) == 1)
		player->video_bitrate = u1;
	else if(sscanf(buf, "ID_VIDEO_CODEC=%255[^\n]", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		string_rtrim(str, NULL);
		if(player->video_codec != NULL)
			free(player->video_codec);
		player->video_codec = strdup(str);
	}
	else if(sscanf(buf, "ID_VIDEO_FPS=%lf\n", &db) == 1)
		player->video_fps = db;
	else if(sscanf(buf, "ID_VIDEO_HEIGHT=%u\n", &u1) == 1)
		_playerbackend_set_size(player, -1, u1);
	else if(sscanf(buf, "ID_VIDEO_RATE=%u\n", &u1) == 1)
		player->video_rate = u1;
	else if(sscanf(buf, "ID_VIDEO_WIDTH=%u\n", &u1) == 1)
		_playerbackend_set_size(player, u1, -1);
#ifdef DEBUG
	else
		fprintf(stderr, "DEBUG: unknown output \"%s\"\n", buf);
#endif
}


/* command_timeout */
static gboolean _command_timeout(gpointer data)
{
	PlayerBackend * player = data;
	static const char cmd[] = "pausing_keep get_time_pos\n"
		"pausing_keep get_percent_pos\n";

	_playerbackend_command(player, cmd, sizeof(cmd) - 1);
	return TRUE;
}


/* command_write */
static gboolean _command_write(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	PlayerBackend * player = data;
	gsize written;
	char * p;
	GError * error = NULL;

	if(condition != G_IO_OUT)
	{
		player_error(player->player, "", 0); /* FIXME */
		gtk_main_quit();
		player->write_id = 0;
		return FALSE; /* FIXME report error */
	}
	if(g_io_channel_write_chars(source, player->buf, player->buf_len,
				&written, &error) != G_IO_STATUS_NORMAL)
	{
		player_error(player->player, error->message, 1);
		g_error_free(error);
		/* FIXME recover somehow */
		gtk_main_quit();
		player->write_id = 0;
		return FALSE;
	}
#ifdef DEBUG
	fprintf(stderr, "DEBUG: wrote %zu bytes\n", written);
#endif
	player->buf_len -= written;
	memmove(player->buf, &player->buf[written], player->buf_len);
	if(player->buf_len == 0)
	{
		player->write_id = 0;
		return FALSE;
	}
	if((p = realloc(player->buf, player->buf_len)) != NULL)
		player->buf = p;
	return TRUE;
}
