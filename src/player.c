/* $Id$ */
static char const _copyright[] =
"Copyright © 2006-2014 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Player */
static char const _license[] =
"This program is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, version 3 of the License.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program. If not, see <http://www.gnu.org/licenses/>.\n";



#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <libgen.h>
#include <errno.h>
#include <libintl.h>
#include <System.h>
#include <Desktop.h>
#include <gdk/gdkkeysyms.h>
#if GTK_CHECK_VERSION(3, 0, 0)
# include <gtk/gtkx.h>
#endif
#include "../include/Player.h"
#include "callbacks.h"
#include "player.h"
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)

/* constants */
#ifndef PROGNAME
# define PROGNAME	"player"
#endif
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef BINDIR
# define BINDIR		PREFIX "/bin"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif


/* Player */
/* private */
/* types */
struct _Player
{
	Config * config;

	/* view */
	int paused;
	gboolean fullscreen;

	/* current file */
	GtkTreeRowReference * current;
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

	/* widgets */
	PangoFontDescription * bold;
	GtkWidget * window;
	GtkWidget * menubar;
#if GTK_CHECK_VERSION(2, 18, 0)
	GtkWidget * infobar;
	GtkWidget * infobar_label;
#endif
	GtkWidget * view;
	GtkWidget * view_window;
	GtkWidget * progress;
	int progress_ignore;
	GtkWidget * progress_length;
	GtkToolItem * tb_previous;
	GtkToolItem * tb_rewind;
	GtkToolItem * tb_play;
	GtkToolItem * tb_pause;
	GtkToolItem * tb_stop;
	GtkToolItem * tb_forward;
	GtkToolItem * tb_next;
#if GTK_CHECK_VERSION(2, 12, 0)
	GtkWidget * tb_volume;
#endif
	GtkToolItem * tb_fullscreen;

	/* about */
	GtkWidget * ab_window;

	/* preferences */
	GtkWidget * pr_window;
	GtkWidget * pr_autoplay;

	/* properties */
	GtkWidget * me_window;
	GtkWidget * me_album;
	GtkWidget * me_artist;
	GtkWidget * me_comment;
	GtkWidget * me_genre;
	GtkWidget * me_title;
	GtkWidget * me_track;
	GtkWidget * me_year;

	/* playlist */
	GtkWidget * pl_window;
	GtkListStore * pl_store;
	GtkWidget * pl_view;
};


/* constants */
#define PLAYER_CONFIG_FILE	".player"
#define PLAYER_ICON_NAME	"multimedia"
#define PLAYER_SPLASH		DATADIR "/" PACKAGE "/splash.png"

enum
{
	PL_COL_ENABLED = 0,
	PL_COL_ICON,
	PL_COL_FILENAME,
	PL_COL_TRACK,
	PL_COL_ARTIST,
	PL_COL_ALBUM,
	PL_COL_TITLE,
	PL_COL_DURATION,
	PL_NUM_COLS
};
# define PL_LAST PL_NUM_COLS

static const DesktopAccel _player_accel[] =
{
	{ G_CALLBACK(on_fullscreen), GDK_CONTROL_MASK, GDK_KEY_F },
	{ G_CALLBACK(on_fullscreen), 0, GDK_KEY_F11 },
#ifdef EMBEDDED
	{ G_CALLBACK(on_close), GDK_CONTROL_MASK, GDK_KEY_W },
#endif
	{ NULL, 0, 0 }
};

#ifndef EMBEDDED
static const DesktopMenu _player_menu_file[] =
{
	{ N_("_Open..."), G_CALLBACK(on_file_open), GTK_STOCK_OPEN,
		GDK_CONTROL_MASK, GDK_KEY_O },
	{ N_("Open _URL..."), G_CALLBACK(on_file_open_url), NULL, 0, 0 },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Properties"), G_CALLBACK(on_file_properties),
		GTK_STOCK_PROPERTIES, GDK_MOD1_MASK, GDK_KEY_Return },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Close"), G_CALLBACK(on_file_close), GTK_STOCK_CLOSE,
		GDK_CONTROL_MASK, GDK_KEY_W },
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _player_menu_edit[] =
{
	{ N_("_Preferences"), G_CALLBACK(on_edit_preferences),
		GTK_STOCK_PREFERENCES, GDK_CONTROL_MASK, GDK_KEY_P },
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _player_menu_view[] =
{
	{ N_("_Playlist"), G_CALLBACK(on_view_playlist), NULL, GDK_CONTROL_MASK,
		GDK_KEY_L },
# if GTK_CHECK_VERSION(2, 8, 0)
	{ N_("_Fullscreen"), G_CALLBACK(on_view_fullscreen),
		GTK_STOCK_FULLSCREEN, 0, GDK_KEY_F11 },
# else
	{ N_("_Fullscreen"), G_CALLBACK(on_view_fullscreen), NULL, 0,
		GDK_KEY_F11 },
# endif
	{ "", NULL, NULL, 0, 0 },
	{ N_("Switch angle"), G_CALLBACK(on_view_switch_angle), NULL, 0, 0 },
	{ N_("Switch audio"), G_CALLBACK(on_view_switch_audio), NULL, 0, 0 },
	{ N_("Switch subtitles"), G_CALLBACK(on_view_switch_subtitles), NULL, 0,
		0 },
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _player_menu_help[] =
{
	{ N_("_Contents"), G_CALLBACK(on_help_contents), "help-contents", 0,
		GDK_KEY_F1 },
# if GTK_CHECK_VERSION(2, 6, 0)
	{ N_("_About"), G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0, 0 },
# else
	{ N_("_About"), G_CALLBACK(on_help_about), NULL, 0, 0 },
# endif
	{ NULL, NULL, NULL, 0 ,0 }
};

static const DesktopMenubar _player_menubar[] =
{
	{ N_("_File"), _player_menu_file },
	{ N_("_Edit"), _player_menu_edit },
	{ N_("_View"), _player_menu_view },
	{ N_("_Help"), _player_menu_help },
	{ NULL, NULL }
};
#endif

#ifdef EMBEDDED
static DesktopToolbar _player_toolbar[] =
{
	{ N_("Open"), G_CALLBACK(on_open), GTK_STOCK_OPEN, GDK_CONTROL_MASK,
		GDK_KEY_O, NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ N_("Properties"), G_CALLBACK(on_properties), GTK_STOCK_PROPERTIES,
		GDK_MOD1_MASK, GDK_KEY_Return, NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ N_("Preferences"), G_CALLBACK(on_preferences), GTK_STOCK_PREFERENCES,
		0, 0, NULL },
	{ NULL, NULL, NULL, 0, 0, NULL }
};
#endif

static DesktopToolbar _player_playbar[] =
{
	{ N_("Previous"), G_CALLBACK(on_previous), GTK_STOCK_MEDIA_PREVIOUS, 0,
		0, NULL },
	{ N_("Rewind"), G_CALLBACK(on_rewind), GTK_STOCK_MEDIA_REWIND, 0, 0,
		NULL },
	{ N_("Play"), G_CALLBACK(on_play), GTK_STOCK_MEDIA_PLAY, 0, 0, NULL },
	{ N_("Pause"), G_CALLBACK(on_pause), GTK_STOCK_MEDIA_PAUSE, 0, 0,
		NULL },
	{ N_("Stop"), G_CALLBACK(on_stop), GTK_STOCK_MEDIA_STOP, 0, 0, NULL },
	{ N_("Forward"), G_CALLBACK(on_forward), GTK_STOCK_MEDIA_FORWARD, 0, 0,
		NULL },
	{ N_("Next"), G_CALLBACK(on_next), GTK_STOCK_MEDIA_NEXT, 0, 0, NULL },
	{ NULL, NULL, NULL, 0, 0, NULL }
};

static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};


/* prototypes */
/* accessors */
static char * _player_get_filename(Player * player);
static void _player_set_metadata(Player * player, unsigned int column,
		char const * value);
static void _player_set_progress(Player * player, unsigned int progress);

static gboolean _player_config_get_boolean(Player * player,
		char const * variable, gboolean _default);

/* useful */
static char * _player_config_filename(void);
static int _player_config_load(Player * player);
static int _player_config_save(Player * player);

static int _player_command(Player * player, char const * cmd, size_t cmd_len);
static void _player_filters(GtkWidget * dialog);
static void _player_message(Player * player, char const * message,
		unsigned int duration);
static void _player_reset(Player * player, char const * filename);
static int _player_start(Player * player);

/* callbacks */
static gboolean _command_read(GIOChannel * source, GIOCondition condition,
		gpointer data);
static gboolean _command_timeout(gpointer data);
static gboolean _command_write(GIOChannel * source, GIOCondition condition,
		gpointer data);


/* public */
/* functions */
/* player_new */
static void _new_column_text(GtkWidget * view, char const * title, int id);

Player * player_new(void)
{
	Player * player;
	GtkAccelGroup * group;
	GtkWidget * widget;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * toolbar;
	GtkToolItem * toolitem;
	GdkColor black = { 0, 0, 0, 0 };
	GtkTreeSelection * selection;

	if((player = object_new(sizeof(*player))) == NULL)
		return NULL;
	if((player->config = config_new()) == NULL)
	{
		object_delete(player);
		return NULL;
	}
	_player_config_load(player);
	/* view */
	player->paused = 0;
	player->fullscreen = FALSE;
	/* current file */
	player->current = NULL;
	player->audio_codec = NULL;
	player->video_codec = NULL;
	/* mplayer */
	player->pid = -1;
	memset(&player->fd, 0, sizeof(player->fd));
	player->buf = NULL;
	player->buf_len = 0;
	/* callbacks */
	player->read_id = 0;
	player->write_id = 0;
	player->timeout_id = 0;
	/* widgets */
	group = gtk_accel_group_new();
	player->bold = pango_font_description_new();
	pango_font_description_set_weight(player->bold, PANGO_WEIGHT_BOLD);
	player->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_add_accel_group(GTK_WINDOW(player->window), group);
	g_object_unref(group);
	gtk_window_set_default_size(GTK_WINDOW(player->window), 512, 384);
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_window_set_icon_name(GTK_WINDOW(player->window), PLAYER_ICON_NAME);
#endif
	gtk_window_set_title(GTK_WINDOW(player->window), _("Media player"));
	gtk_widget_realize(player->window);
	g_signal_connect_swapped(player->window, "delete-event", G_CALLBACK(
				on_player_closex), player);
#if GTK_CHECK_VERSION(3, 0, 0)
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
	vbox = gtk_vbox_new(FALSE, 0);
#endif
	gtk_container_add(GTK_CONTAINER(player->window), vbox);
	desktop_accel_create(_player_accel, player, group);
#ifndef EMBEDDED
	/* menubar */
	player->menubar = desktop_menubar_create(_player_menubar, player,
			group);
#else
	/* toolbar */
	player->menubar = desktop_toolbar_create(_player_toolbar, player,
			group);
#endif
	gtk_box_pack_start(GTK_BOX(vbox), player->menubar, FALSE, FALSE, 0);
#if GTK_CHECK_VERSION(2, 18, 0)
	/* infobar */
	player->infobar = gtk_info_bar_new_with_buttons(GTK_STOCK_CLOSE,
			GTK_RESPONSE_CLOSE, NULL);
	gtk_info_bar_set_message_type(GTK_INFO_BAR(player->infobar),
			GTK_MESSAGE_ERROR);
	g_signal_connect(player->infobar, "close", G_CALLBACK(gtk_widget_hide),
			NULL);
	g_signal_connect(player->infobar, "response", G_CALLBACK(
				gtk_widget_hide), NULL);
	widget = gtk_info_bar_get_content_area(GTK_INFO_BAR(player->infobar));
	player->infobar_label = gtk_label_new(NULL);
	gtk_widget_show(player->infobar_label);
	gtk_box_pack_start(GTK_BOX(widget), player->infobar_label, TRUE, TRUE,
			0);
	gtk_widget_set_no_show_all(player->infobar, TRUE);
	gtk_box_pack_start(GTK_BOX(vbox), player->infobar, FALSE, TRUE, 0);
#endif
	/* view */
	player->view_window = gtk_socket_new();
	gtk_widget_modify_bg(player->view_window, GTK_STATE_NORMAL, &black);
	g_signal_connect_swapped(player->view_window, "plug-removed",
			G_CALLBACK(on_player_removed), player);
	gtk_box_pack_start(GTK_BOX(vbox), player->view_window, TRUE, TRUE, 0);
	/* playbar */
	toolbar = desktop_toolbar_create(_player_playbar, player, group);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
#if GTK_CHECK_VERSION(2, 12, 0)
	player->tb_volume = gtk_volume_button_new();
	gtk_scale_button_set_value(GTK_SCALE_BUTTON(player->tb_volume), 0.5);
	g_signal_connect_swapped(player->tb_volume, "value-changed", G_CALLBACK(
				on_volume_changed), player);
	toolitem = gtk_tool_item_new();
	gtk_container_add(GTK_CONTAINER(toolitem), player->tb_volume);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
#endif
	widget = gtk_image_new_from_icon_name("stock_fullscreen",
			GTK_ICON_SIZE_SMALL_TOOLBAR);
	player->tb_fullscreen = gtk_tool_button_new(widget, _("Fullscreen"));
	g_signal_connect_swapped(player->tb_fullscreen, "clicked", G_CALLBACK(
				on_fullscreen), player);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), player->tb_fullscreen, -1);
	gtk_box_pack_end(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	/* progress bar */
	toolbar = gtk_toolbar_new();
	toolitem = gtk_tool_item_new();
	gtk_tool_item_set_expand(toolitem, TRUE);
#if GTK_CHECK_VERSION(3, 0, 0)
	/* FIXME seems to be causing trouble */
	player->progress = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
			0.0, 100.0, 0.1);
#else
	player->progress = gtk_hscale_new_with_range(0.0, 100.0, 0.1);
#endif
	gtk_scale_set_draw_value(GTK_SCALE(player->progress), FALSE);
	g_signal_connect_swapped(player->progress, "value-changed", G_CALLBACK(
				on_progress_changed), player);
	player->progress_ignore = 0;
	gtk_container_add(GTK_CONTAINER(toolitem), player->progress);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	/* length */
	toolitem = gtk_tool_item_new();
	player->progress_length = gtk_label_new("00:00:00");
	gtk_container_add(GTK_CONTAINER(toolitem), player->progress_length);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), toolitem, -1);
	gtk_box_pack_end(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	_player_set_progress(player, 0);
	gtk_widget_show_all(player->window);
	/* playlist */
	/* FIXME make it dockable */
	player->pl_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(player->pl_window), _("Playlist"));
	g_signal_connect_swapped(player->pl_window, "delete-event", G_CALLBACK(
				on_playlist_closex), player);
#if GTK_CHECK_VERSION(3, 0, 0)
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
	vbox = gtk_vbox_new(FALSE, 0);
#endif
	/* view */
	player->pl_store = gtk_list_store_new(PL_NUM_COLS,
			G_TYPE_BOOLEAN,	/* enabled */
			GDK_TYPE_PIXBUF,/* icon */
			G_TYPE_STRING,	/* filename */
			G_TYPE_UINT,	/* track number */
			G_TYPE_STRING,	/* artist */
			G_TYPE_STRING,	/* album */
			G_TYPE_STRING,	/* title */
			G_TYPE_STRING);	/* duration */
	widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	player->pl_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
				player->pl_store));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(player->pl_view));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(player->pl_view), TRUE);
	gtk_tree_view_append_column(GTK_TREE_VIEW(player->pl_view),
			gtk_tree_view_column_new_with_attributes("",
				gtk_cell_renderer_pixbuf_new(), "pixbuf",
				PL_COL_ICON, NULL));
	_new_column_text(player->pl_view, _("Artist"), PL_COL_ARTIST);
	_new_column_text(player->pl_view, _("Album"), PL_COL_ALBUM);
	_new_column_text(player->pl_view, _("Title"), PL_COL_TITLE);
	_new_column_text(player->pl_view, _("Duration"), PL_COL_DURATION);
	g_signal_connect_swapped(player->pl_view, "row-activated", G_CALLBACK(
				on_playlist_activated), player);
	gtk_container_add(GTK_CONTAINER(widget), player->pl_view);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
#if GTK_CHECK_VERSION(3, 0, 0)
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_set_homogeneous(GTK_BOX(hbox), TRUE);
#else
	hbox = gtk_hbox_new(TRUE, 0);
#endif
	widget = gtk_button_new_from_stock(GTK_STOCK_OPEN);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				on_playlist_load), player);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_SAVE);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				on_playlist_save), player);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_ADD);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				on_playlist_add), player);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	widget = gtk_button_new_from_stock(GTK_STOCK_REMOVE);
	g_signal_connect_swapped(widget, "clicked", G_CALLBACK(
				on_playlist_remove), player);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 4);
	gtk_container_add(GTK_CONTAINER(player->pl_window), vbox);
	/* about window */
	player->ab_window = NULL;
	/* preferences window */
	player->pr_window = NULL;
	/* properties window */
	player->me_window = NULL;
#ifdef DEBUG
	gtk_widget_show_all(player->pl_window);
#else
	gtk_widget_show_all(vbox);
#endif
	/* mplayer */
	_player_start(player);
	/* messages */
	desktop_message_register(player->window, PLAYER_CLIENT_MESSAGE,
			on_player_message, player);
	return player;
}

static void _new_column_text(GtkWidget * view, char const * title, int id)
{
	GtkTreeViewColumn * column;
	GtkCellRenderer * renderer;

	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	column = gtk_tree_view_column_new_with_attributes(title, renderer,
			"text", id, NULL);
#if GTK_CHECK_VERSION(2, 4, 0)
	gtk_tree_view_column_set_expand(column, TRUE);
#endif
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, id);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
}


/* player_delete */
void player_delete(Player * player)
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
	pango_font_description_free(player->bold);
	config_delete(player->config);
	object_delete(player);
}


/* accessors */
/* player_get_fullscreen */
gboolean player_get_fullscreen(Player * player)
{
	return player->fullscreen;
}


/* player_set_fullscreen */
void player_set_fullscreen(Player * player, gboolean fullscreen)
{
	if(player->fullscreen == fullscreen)
		return;
	if(fullscreen)
	{
		gtk_widget_hide(player->menubar);
		gtk_window_fullscreen(GTK_WINDOW(player->window));
	}
	else
	{
		gtk_window_unfullscreen(GTK_WINDOW(player->window));
		gtk_widget_show(player->menubar);
	}
	player->fullscreen = fullscreen;
}


/* player_set_progress */
void player_set_progress(Player * player, gdouble progress)
{
	char buf[48];
	int len;

	if(player->progress_ignore != 0)
		return;
	if(progress < 0)
		/* XXX hack */
		progress = gtk_range_get_value(GTK_RANGE(player->progress));
	len = snprintf(buf, sizeof(buf), "%s %.1f %d\n", "pausing_keep seek",
			progress, 1);
	_player_command(player, buf, len);
}


/* player_set_size */
void player_set_size(Player * player, int width, int height)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%d, %d)\n", __func__, width, height);
#endif
	if(width < 0)
		width = player->width;
	if(height < 0)
		height = player->height;
	gtk_widget_set_size_request(player->view_window, width, height);
	player->width = width;
	player->height = height;
}


/* player_set_volume */
void player_set_volume(Player * player, gdouble volume)
{
	char buf[256];
	int len;

#if GTK_CHECK_VERSION(2, 12, 0)
	if(!(volume >= 0.0 && volume <= 1.0))
	{
		volume = gtk_scale_button_get_value(GTK_SCALE_BUTTON(
					player->tb_volume));
	}
#endif
	len = snprintf(buf, sizeof(buf), "volume %u 1\n",
			(unsigned)(volume * 100.0));
	_player_command(player, buf, len);
}


/* useful */
/* player_about */
static gboolean _about_on_closex(gpointer data);

void player_about(Player * player)
{
	if(player->ab_window != NULL)
	{
		gtk_window_present(GTK_WINDOW(player->ab_window));
		return;
	}
	player->ab_window = desktop_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(player->ab_window), GTK_WINDOW(
				player->window));
	desktop_about_dialog_set_authors(player->ab_window, _authors);
	desktop_about_dialog_set_comments(player->ab_window,
			_("Media player for the DeforaOS desktop"));
	desktop_about_dialog_set_copyright(player->ab_window, _copyright);
	desktop_about_dialog_set_logo_icon_name(player->ab_window,
			PLAYER_ICON_NAME);
	desktop_about_dialog_set_license(player->ab_window, _license);
	desktop_about_dialog_set_program_name(player->ab_window, PACKAGE);
	desktop_about_dialog_set_translator_credits(player->ab_window,
			_("translator-credits"));
	desktop_about_dialog_set_version(player->ab_window, VERSION);
	desktop_about_dialog_set_website(player->ab_window,
			"http://www.defora.org/");
	g_signal_connect_swapped(player->ab_window, "delete-event", G_CALLBACK(
				_about_on_closex), player);
	gtk_widget_show(player->ab_window);
}

static gboolean _about_on_closex(gpointer data)
{
	Player * player = data;

	gtk_widget_hide(player->ab_window);
	return TRUE;
}


/* player_error */
static int _error_text(char const * message, int ret);

int player_error(Player * player, char const * message, int ret)
{
#if GTK_CHECK_VERSION(2, 18, 0)
	if(player == NULL)
		return _error_text(message, ret);
	gtk_label_set_text(GTK_LABEL(player->infobar_label), message);
	gtk_widget_show(player->infobar);
#else
	GtkWidget * dialog;

	if(player == NULL)
		return _error_text(message, ret);
	dialog = gtk_message_dialog_new(GTK_WINDOW(player->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
# if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
# endif
			"%s", message);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy),
			NULL);
	gtk_dialog_run(GTK_DIALOG(dialog));
#endif
	return ret;
}

static int _error_text(char const * message, int ret)
{
	fputs(PROGNAME ": ", stderr);
	perror(message);
	return ret;
}


/* player_forward */
void player_forward(Player * player)
{
	char const cmd[] = "seek 10 0\n";

	_player_command(player, cmd, sizeof(cmd) - 1);
	_player_message(player, _("Forward"), 1000);
}


/* player_sigchld */
int player_sigchld(Player * player)
{
	pid_t pid;
	int status;
	char buf[80];

	if(player->pid == -1)
		return 1;
	if((pid = waitpid(player->pid, &status, WNOHANG)) == -1)
		return player_error(player, "waitpid", 1);
	if(pid == 0)
		return 1;
	if(WIFEXITED(status))
	{
		snprintf(buf, sizeof(buf), _("mplayer %d: exited with code %u"),
				pid, WEXITSTATUS(status));
		player_error(player, buf, 1);
	}
	else
		fprintf(stderr, "%s%s%d%s", "player: ", "mplayer ", pid,
				": Unknown state\n");
	player->pid = -1;
	return 0;
}


/* player_mute */
void player_mute(Player * player, PlayerMute mute)
{
	char cmd[8];
	int len;

	len = snprintf(cmd, sizeof(cmd), "%s%s%s\n", "mute",
			(mute != PLAYER_MUTE_TOGGLE) ? " " : "",
			(mute != PLAYER_MUTE_TOGGLE)
			? (mute == PLAYER_MUTE_UNMUTE ? "0" : "1") : "");
	if(len >= (int)sizeof(cmd))
		fputs("player: String too long\n", stderr);
	else
		_player_command(player, cmd, len);
}


/* player_next */
void player_next(Player * player)
{
	GtkTreeModel * model = GTK_TREE_MODEL(player->pl_store);
	GtkTreePath * path;
	GtkTreeIter iter;
	gboolean res;

	if(player->current == NULL)
		return;
	if((path = gtk_tree_row_reference_get_path(player->current)) == NULL)
		return;
	res = gtk_tree_model_get_iter(model, &iter, path);
	gtk_tree_path_free(path);
	if(res != TRUE)
		return;
	if(gtk_tree_model_iter_next(model, &iter) != TRUE)
		return;
	gtk_tree_row_reference_free(player->current);
	path = gtk_tree_model_get_path(model, &iter);
	player->current = gtk_tree_row_reference_new(model, path);
	gtk_tree_path_free(path);
	player_play(player);
}


/* player_open */
int player_open(Player * player, char const * filename)
{
	GtkTreeModel * model = GTK_TREE_MODEL(player->pl_store);
	GtkTreeIter iter;
	GtkTreePath * path;
	char cmd[512];
	size_t len;
	gboolean autoplay;

	autoplay = _player_config_get_boolean(player, "autoplay", FALSE);
	player_playlist_clear(player);
	player_playlist_add(player, filename);
	if(gtk_tree_model_get_iter_first(model, &iter) == TRUE)
	{
		path = gtk_tree_model_get_path(model, &iter);
		player->current = gtk_tree_row_reference_new(model, path);
		gtk_tree_path_free(path);
	}
	_player_reset(player, filename);
	len = snprintf(cmd, sizeof(cmd), "%s%s%s%s%s",
			autoplay ? "" : "pausing ",
			"loadfile \"", filename, "\" 0\n",
			autoplay ? "" : "frame_step\n");
	if(len >= sizeof(cmd))
	{
		fputs("player: String too long\n", stderr);
		return 1;
	}
	if(_player_command(player, cmd, len) != 0)
		return 1;
	/* XXX avoid code duplication */
	if((player->paused = autoplay ? 0 : 1) == 0 && player->timeout_id == 0)
		player->timeout_id = g_timeout_add(500, _command_timeout,
				player);
	return 0;
}


/* player_open_dialog */
int player_open_dialog(Player * player)
{
	int ret;
	GtkWidget * dialog;
	char * filename = NULL;

	dialog = gtk_file_chooser_dialog_new(_("Open file..."),
			GTK_WINDOW(player->window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	_player_filters(dialog);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename == NULL)
		return 1;
	ret = player_open(player, filename);
	g_free(filename);
	return ret;
}


/* player_open_url */
int player_open_url(Player * player, char const * url)
{
	if(url == NULL)
		return player_open_url_dialog(player);
	return player_open(player, url);
}


/* player_open_url_dialog */
int player_open_url_dialog(Player * player)
{
	int ret = 0;
	GtkWidget * dialog;
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * label;
	GtkWidget * entry;
	char const * p;
	char * url = NULL;

	dialog = gtk_dialog_new_with_buttons(_("Open URL..."),
			GTK_WINDOW(player->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog),
			GTK_RESPONSE_ACCEPT);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	vbox = GTK_DIALOG(dialog)->vbox;
#endif
	gtk_box_set_spacing(GTK_BOX(vbox), 4);
#if GTK_CHECK_VERSION(3, 0, 0)
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
	hbox = gtk_hbox_new(FALSE, 4);
#endif
	label = gtk_label_new(_("URL: "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, TRUE, 0);
	entry = gtk_entry_new();
	gtk_entry_set_activates_default(GTK_ENTRY(entry), TRUE);
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show_all(vbox);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		p = gtk_entry_get_text(GTK_ENTRY(entry));
		if((url = strdup(p)) == NULL)
			ret = -player_error(NULL, "strdup", 1);
	}
	gtk_widget_destroy(dialog);
	if(url == NULL)
		return ret;
	ret = player_open(player, url);
	free(url);
	return ret;
}


/* player_pause */
void player_pause(Player * player)
{
	char const cmd[] = "pause\n";

	if(player->paused != 0)
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
	_player_command(player, cmd, sizeof(cmd) - 1);
	player->paused = (player->paused == 1) ? 0 : 1;
	if(player->paused)
		_player_message(player, _("Paused"), 1000);
}


/* player_play */
void player_play(Player * player)
{
	char cmd[512];
	size_t len;
	char * filename;

	if((filename = _player_get_filename(player)) == NULL)
		return;
	/* FIXME escape double quotes in filename? */
	if(player->paused == 1)
		len = snprintf(cmd, sizeof(cmd), "%s", "pause\n");
	else
		len = snprintf(cmd, sizeof(cmd), "%s%s%s", "loadfile \"",
				filename, "\" 0\n");
	if(len >= sizeof(cmd))
	{
		fputs("player: String too long\n", stderr);
		return;
	}
	else
		_player_reset(player, filename);
	free(filename);
	if(_player_command(player, cmd, len) != 0)
		return;
	player->paused = 0;
	if(player->read_id == 0)
		player->read_id = g_io_add_watch(player->channel[0], G_IO_IN,
				_command_read, player);
	if(player->timeout_id == 0)
		player->timeout_id = g_timeout_add(500, _command_timeout,
				player);
}


/* player_playlist_add */
void player_playlist_add(Player * player, char const * filename)
{
	GtkTreeIter iter;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, filename);
#endif
	/* FIXME fetch the actual artists/albums/titles */
#if GTK_CHECK_VERSION(2, 6, 0)
	gtk_list_store_insert_with_values(player->pl_store, &iter, -1,
#else
	gtk_list_store_insert_after(player->pl_store, iter, NULL);
	gtk_list_store_set(player->pl_store, iter,
#endif
			PL_COL_FILENAME, filename,
			PL_COL_ALBUM, _("Unknown album"),
			PL_COL_ARTIST, _("Unknown artist"),
			PL_COL_TITLE, _("Unknown title"),
			PL_COL_DURATION, _("Unknown"), -1);
}


/* player_playlist_add_dialog */
void player_playlist_add_dialog(Player * player)
{
	GtkWidget * dialog;
	GSList * files = NULL;
	GSList * p;
	char const * filename;

	dialog = gtk_file_chooser_dialog_new(_("Add file(s)..."),
			GTK_WINDOW(player->window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT, NULL);
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
	_player_filters(dialog);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		files = gtk_file_chooser_get_uris(GTK_FILE_CHOOSER(dialog));
	gtk_widget_destroy(dialog);
	for(p = files; p != NULL; p = p->next)
	{
		filename = p->data;
		player_playlist_add_url(player, filename);
	}
	g_slist_foreach(files, (GFunc)g_free, NULL);
	g_slist_free(files);
}


/* player_playlist_add_url */
void player_playlist_add_url(Player * player, char const * url)
{
	char const file[] = "file:/";
	gchar * p = NULL;
	GError * error = NULL;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\")\n", __func__, url);
#endif
	if(strncmp(file, url, sizeof(file) - 1) == 0)
	{
		if((p = g_filename_from_uri(url, NULL, &error)) == NULL)
		{
			/* XXX do not call perror() */
			player_error(NULL, error->message, 1);
			g_error_free(error);
			return;
		}
		url = p;
	}
	player_playlist_add(player, url);
	g_free(p);
}


/* player_playlist_clear */
void player_playlist_clear(Player * player)
{
	if(player->current != NULL)
		gtk_tree_row_reference_free(player->current);
	player->current = NULL;
	gtk_list_store_clear(player->pl_store);
}


/* player_playlist_open */
void player_playlist_open(Player * player, char const * filename)
{
	/* FIXME implement */
}


/* player_playlist_open_dialog */
void player_playlist_open_dialog(Player * player)
{
	GtkWidget * dialog;
	GtkFileFilter * filter;
	gchar * filename = NULL;

	dialog = gtk_file_chooser_dialog_new(_("Open playlist..."),
			GTK_WINDOW(player->window),
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_ADD, GTK_RESPONSE_ACCEPT, NULL);
	/* playlists */
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("Playlists"));
	gtk_file_filter_add_mime_type(filter, "audio/x-mpegurl");
	gtk_file_filter_add_mime_type(filter, "audio/x-scpls");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	/* all files */
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("All files"));
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
					dialog));
	gtk_widget_destroy(dialog);
	if(filename != NULL)
		player_playlist_open(player, filename);
	g_free(filename);
}


/* player_playlist_play_selected */
void player_playlist_play_selected(Player * player)
{
	GtkTreeSelection * selection;
	GList * rows;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(player->pl_view));
	if((rows = gtk_tree_selection_get_selected_rows(selection, NULL))
			== NULL)
		return;
	if(rows->data != NULL && rows->next == NULL)
	{
		if(player->current != NULL)
			gtk_tree_row_reference_free(player->current);
		player->current = gtk_tree_row_reference_new(GTK_TREE_MODEL(
					player->pl_store), rows->data);
		player_play(player);
	}
	g_list_foreach(rows, (GFunc)gtk_tree_path_free, NULL);
	g_list_free(rows);
}


/* player_playlist_remove_selection */
void player_playlist_remove_selection(Player * player)
{
	GtkTreeModel * model = GTK_TREE_MODEL(player->pl_store);
	GtkTreeSelection * selection;
	GList * rows;
	GList * l;
	GtkTreePath * path;
	GtkTreeIter iter;
	gboolean res;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(player->pl_view));
	if((rows = gtk_tree_selection_get_selected_rows(selection, NULL))
			== NULL)
		return;
	/* convert the paths to references */
	for(l = rows; l != NULL; l = l->next)
	{
		path = l->data;
		l->data = gtk_tree_row_reference_new(model, path);
		gtk_tree_path_free(path);
	}
	/* remove the references */
	for(l = rows; l != NULL; l = l->next)
	{
		if((path = gtk_tree_row_reference_get_path(l->data)) == NULL)
			continue;
		res = gtk_tree_model_get_iter(model, &iter, path);
		gtk_tree_path_free(path);
		if(res == TRUE)
			gtk_list_store_remove(player->pl_store, &iter);
	}
	/* free the references */
	g_list_foreach(rows, (GFunc)gtk_tree_row_reference_free, NULL);
	g_list_free(rows);
}


/* player_playlist_save_as_dialog */
void player_playlist_save_as_dialog(Player * player)
{
	/* FIXME implement */
}


/* player_previous */
void player_previous(Player * player)
{
	GtkTreeModel * model = GTK_TREE_MODEL(player->pl_store);
	GtkTreePath * path;

	if(player->current == NULL)
		return;
	if((path = gtk_tree_row_reference_get_path(player->current)) == NULL)
		return;
	if(gtk_tree_path_prev(path) != TRUE)
		return;
	path = gtk_tree_path_copy(path);
	gtk_tree_row_reference_free(player->current);
	player->current = gtk_tree_row_reference_new(model, path);
	gtk_tree_path_free(path);
	player_play(player);
}


/* player_rewind */
void player_rewind(Player * player)
{
	char const cmd[] = "seek -10 0\n";

	_player_command(player, cmd, sizeof(cmd) - 1);
	_player_message(player, _("Rewind"), 1000);
}


/* player_stop */
void player_stop(Player * player)
{
	char const cmd[] = "stop\n";

	_player_command(player, cmd, sizeof(cmd) - 1);
	_player_set_progress(player, 0);
	player->paused = 0; /* FIXME also needs a stopped state */
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


/* player_show_playlist */
void player_show_playlist(Player * player, gboolean show)
{
	if(show)
		gtk_window_present(GTK_WINDOW(player->pl_window));
	else
		gtk_widget_hide(player->pl_window);
}


/* player_show_preferences */
static void _preferences_set(Player * player);
static gboolean _preferences_on_closex(gpointer data);
static void _preferences_on_response(GtkWidget * widget, gint response,
		gpointer data);
static void _preferences_on_cancel(gpointer data);
static void _preferences_on_ok(gpointer data);

void player_show_preferences(Player * player, gboolean show)
{
	GtkWidget * vbox;

	if(player->pr_window != NULL)
	{
		if(show)
			gtk_window_present(GTK_WINDOW(player->pr_window));
		else
			gtk_widget_hide(player->pr_window);
		return;
	}
	player->pr_window = gtk_dialog_new_with_buttons(
			_("Media player preferences"),
			GTK_WINDOW(player->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
	g_signal_connect_swapped(player->pr_window, "delete-event", G_CALLBACK(
				_preferences_on_closex), player);
	g_signal_connect(player->pr_window, "response", G_CALLBACK(
				_preferences_on_response), player);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(player->pr_window));
#else
	vbox = GTK_DIALOG(player->pr_window)->vbox;
#endif
	gtk_box_set_spacing(GTK_BOX(vbox), 4);
	/* auto-play */
	player->pr_autoplay = gtk_check_button_new_with_mnemonic(
			_("_Automatically play files when opened"));
	gtk_box_pack_start(GTK_BOX(vbox), player->pr_autoplay, FALSE, TRUE, 0);
	_preferences_set(player);
	gtk_widget_show_all(vbox);
	if(show)
		gtk_widget_show(player->pr_window);
}

static void _preferences_set(Player * player)
{
	gboolean boolean;
	char const * p;

	boolean = FALSE;
	if((p = config_get(player->config, NULL, "autoplay")) != NULL)
		boolean = strtol(p, NULL, 10);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(player->pr_autoplay),
			boolean);
}

static gboolean _preferences_on_closex(gpointer data)
{
	Player * player = data;

	_preferences_on_cancel(player);
	return TRUE;
}

static void _preferences_on_response(GtkWidget * widget, gint response,
		gpointer data)
{
	gtk_widget_hide(widget);
	if(response == GTK_RESPONSE_OK)
		_preferences_on_ok(data);
	else if(response == GTK_RESPONSE_CANCEL)
		_preferences_on_cancel(data);
}

static void _preferences_on_cancel(gpointer data)
{
	Player * player = data;

	gtk_widget_hide(player->pr_window);
	_preferences_set(player);
}

static void _preferences_on_ok(gpointer data)
{
	Player * player = data;
	gboolean boolean;

	gtk_widget_hide(player->pr_window);
	boolean = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
				player->pr_autoplay));
	/* XXX may fail */
	config_set(player->config, NULL, "autoplay", boolean ? "1" : "0");
	_player_config_save(player);
}


/* player_show_properties */
static void _properties_commands(Player * player);
static GtkWidget * _properties_label(Player * player, GtkSizeGroup * group,
		char const * label, GtkWidget ** widget);
static void _properties_reset(Player * player);
static void _properties_window(Player * player);

void player_show_properties(Player * player, gboolean show)
{
	char * filename;
	char buf[256];

	if(show == FALSE)
	{
		if(player->me_window != NULL)
			gtk_widget_hide(player->me_window);
		return;
	}
	else if(player->me_window == NULL)
		_properties_window(player);
	/* set the window title */
	if((filename = _player_get_filename(player)) == NULL)
		return;
	snprintf(buf, sizeof(buf), "%s%s", _("Properties of "), basename(
				filename));
	free(filename);
	gtk_window_set_title(GTK_WINDOW(player->me_window), buf);
	/* reset the properties */
	_properties_reset(player);
	/* obtain the properties */
	_properties_commands(player);
	/* run commands */
	gtk_dialog_run(GTK_DIALOG(player->me_window));
	gtk_widget_hide(player->me_window);
}

static void _properties_commands(Player * player)
{
	char const buf[] = "get_meta_album\nget_meta_artist\nget_meta_comment\n"
		"get_meta_genre\nget_meta_title\nget_meta_track\n"
		"get_meta_year\n";

	_player_command(player, buf, sizeof(buf) - 1);
}

static GtkWidget * _properties_label(Player * player, GtkSizeGroup * group,
		char const * label, GtkWidget ** widget)
{
	GtkWidget * hbox;

#if GTK_CHECK_VERSION(3, 0, 0)
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#else
	hbox = gtk_hbox_new(FALSE, 4);
#endif
	*widget = gtk_label_new(label);
	gtk_widget_modify_font(*widget, player->bold);
	gtk_misc_set_alignment(GTK_MISC(*widget), 0.0, 0.5);
	gtk_size_group_add_widget(group, *widget);
	gtk_box_pack_start(GTK_BOX(hbox), *widget, FALSE, TRUE, 0);
	*widget = gtk_label_new(NULL);
	gtk_label_set_ellipsize(GTK_LABEL(*widget), PANGO_ELLIPSIZE_END);
	gtk_misc_set_alignment(GTK_MISC(*widget), 0.0, 0.5);
	gtk_box_pack_start(GTK_BOX(hbox), *widget, TRUE, TRUE, 0);
	return hbox;
}

static void _properties_reset(Player * player)
{
	gtk_label_set_text(GTK_LABEL(player->me_album), NULL);
	gtk_label_set_text(GTK_LABEL(player->me_artist), NULL);
	gtk_label_set_text(GTK_LABEL(player->me_comment), NULL);
	gtk_label_set_text(GTK_LABEL(player->me_genre), NULL);
	gtk_label_set_text(GTK_LABEL(player->me_title), NULL);
	gtk_label_set_text(GTK_LABEL(player->me_track), NULL);
	gtk_label_set_text(GTK_LABEL(player->me_year), NULL);
}

static void _properties_window(Player * player)
{
	GtkSizeGroup * group;
	GtkWidget * vbox;
	GtkWidget * hbox;

	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	player->me_window = gtk_message_dialog_new(GTK_WINDOW(player->window),
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Properties"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(
				player->me_window),
#endif
			"");
#if GTK_CHECK_VERSION(2, 10, 0)
	gtk_message_dialog_set_image(GTK_MESSAGE_DIALOG(player->me_window),
			gtk_image_new_from_stock(GTK_STOCK_PROPERTIES,
				GTK_ICON_SIZE_DIALOG));
#endif
	gtk_window_set_default_size(GTK_WINDOW(player->me_window), 300, 200);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(player->me_window));
#else
	vbox = GTK_DIALOG(player->me_window)->vbox;
#endif
	gtk_box_set_spacing(GTK_BOX(vbox), 4);
	/* meta-data */
	hbox = _properties_label(player, group, _("Album: "),
			&player->me_album);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = _properties_label(player, group, _("Artist: "),
			&player->me_artist);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = _properties_label(player, group, _("Comment: "),
			&player->me_comment);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = _properties_label(player, group, _("Genre: "),
			&player->me_genre);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = _properties_label(player, group, _("Title: "),
			&player->me_title);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = _properties_label(player, group, _("Track: "),
			&player->me_track);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = _properties_label(player, group, _("Year: "),
			&player->me_year);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	gtk_widget_show_all(vbox);
}


/* player_switch_angle */
void player_switch_angle(Player * player)
{
	const char cmd[] = "switch_angle\n";

	_player_command(player, cmd, sizeof(cmd) - 1);
}


/* player_switch_audio */
void player_switch_audio(Player * player)
{
	const char cmd[] = "switch_audio\n";

	_player_command(player, cmd, sizeof(cmd) - 1);
}


/* player_switch_subtitles */
void player_switch_subtitles(Player * player)
{
	const char cmd[] = "sub_visibility\n";

	_player_command(player, cmd, sizeof(cmd) - 1);
}


/* private */
/* functions */
/* accessors */
/* player_get_filename */
static char * _player_get_filename(Player * player)
{
	char * ret;
	GtkTreePath * path;
	GtkTreeIter iter;

	if(player->current == NULL)
		return NULL;
	if((path = gtk_tree_row_reference_get_path(player->current)) == NULL)
		return NULL;
	if(gtk_tree_model_get_iter(GTK_TREE_MODEL(player->pl_store), &iter,
				path) != TRUE)
		return NULL;
	gtk_tree_model_get(GTK_TREE_MODEL(player->pl_store), &iter,
			PL_COL_FILENAME, &ret, -1);
	return ret;
}


/* player_set_metadata */
static void _player_set_metadata(Player * player, unsigned int column,
		char const * value)
{
	GtkTreePath * path;
	GtkTreeIter iter;

	if(player->current == NULL)
		return;
	if((path = gtk_tree_row_reference_get_path(player->current)) == NULL)
		return;
	if(gtk_tree_model_get_iter(GTK_TREE_MODEL(player->pl_store), &iter,
				path) != TRUE)
		return;
	gtk_list_store_set(player->pl_store, &iter, column, value, -1);
}


/* player_set_progress */
static void _player_set_progress(Player * player, unsigned int progress)
{
	gdouble fraction;

	fraction = (progress <= 100) ? progress : 100.0;
	player->progress_ignore++;
	gtk_range_set_value(GTK_RANGE(player->progress), fraction);
	player->progress_ignore--;
	if(progress == 0)
		gtk_label_set_text(GTK_LABEL(player->progress_length),
				"00:00:00");
}


/* player_config_get_boolean */
static gboolean _player_config_get_boolean(Player * player,
		char const * variable, gboolean _default)
{
	char const * p;

	if((p = config_get(player->config, NULL, variable)) == NULL)
		return _default;
	return strtol(p, NULL, 10);
}


/* useful */
/* player_command */
static gboolean _command_on_timeout(gpointer data);

static int _player_command(Player * player, char const * cmd, size_t cmd_len)
{
	char * p;

	if(player->pid == -1)
	{
		fputs(PROGNAME ": mplayer not running\n", stderr);
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
	Player * player = data;

	if(_player_start(player) != 0)
		return TRUE;
	player->timeout_id = 0;
	return FALSE;
}


/* player_config_filename */
static char * _player_config_filename(void)
{
	char const * homedir;
	size_t len;
	char * filename;

	if((homedir = getenv("HOME")) == NULL)
		homedir = g_get_home_dir();
	len = strlen(homedir) + 1 + sizeof(PLAYER_CONFIG_FILE);
	if((filename = malloc(len)) == NULL)
		return NULL;
	snprintf(filename, len, "%s/%s", homedir, PLAYER_CONFIG_FILE);
	return filename;
}


/* player_config_load */
static int _player_config_load(Player * player)
{
	int ret;
	char * filename;

	if((filename = _player_config_filename()) == NULL)
		return -1;
	ret = config_load(player->config, filename);
	free(filename);
	return ret;
}


/* player_config_save */
static int _player_config_save(Player * player)
{
	int ret;
	char * filename;

	if((filename = _player_config_filename()) == NULL)
		return -1;
	ret = config_save(player->config, filename);
	free(filename);
	return ret;
}


/* player_filters */
static void _player_filters(GtkWidget * dialog)
{
	GtkFileFilter * filter;
	char const * video[] =
	{
		"video/3gpp",
		"video/3gpp2",
		"video/mp2t",
		"video/mp4",
		"video/mpeg",
		"video/ogg",
		"video/quicktime",
		"video/vivo",
		"video/vnd.vn-realvideo",
		"video/webm",
		"video/x-flv",
		"video/x-javafx",
		"video/x-matroska",
		"video/x-mng",
		"video/x-ms-asf",
		"video/x-ms-wmv",
		"video/x-msvideo",
		"video/x-ogm+ogg",
		"video/x-sgi-movie",
		"video/x-theora+ogg",
		NULL
	};
	char const * audio[] =
	{
		"audio/ac3",
		"audio/mp4",
		"audio/mpeg",
		"audio/vorbis",
		"audio/x-mp2",
		"audio/x-mp3",
		"audio/x-ms-wma",
		"audio/x-vorbis",
		"audio/x-wav",
		NULL
	};
	size_t i;

	/* video and audio files */
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("Video and audio files"));
	for(i = 0; video[i] != NULL; i++)
		gtk_file_filter_add_mime_type(filter, video[i]);
	for(i = 0; audio[i] != NULL; i++)
		gtk_file_filter_add_mime_type(filter, audio[i]);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	/* video files */
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("Video files"));
	for(i = 0; video[i] != NULL; i++)
		gtk_file_filter_add_mime_type(filter, video[i]);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	/* audio files */
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("Audio files"));
	for(i = 0; audio[i] != NULL; i++)
		gtk_file_filter_add_mime_type(filter, audio[i]);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	/* all files */
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, _("All files"));
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
}


/* player_message */
static void _player_message(Player * player, char const * message,
		unsigned int duration)
{
	char const cmd[] = "pausing_keep osd_show_text";
	char buf[128];
	int len;

	if((len = snprintf(buf, sizeof(buf), "%s \"%s\" %u\n", cmd, message,
					duration)) >= (int)sizeof(buf))
		fputs("player: String too long\n", stderr);
	else
		_player_command(player, buf, len);
}


/* player_reset */
static void _player_reset(Player * player, char const * filename)
{
	char * p = NULL;
	char buf[256];

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
	_player_set_progress(player, 0);
	if(filename != NULL)
		p = strdup(filename);
	snprintf(buf, sizeof(buf), "%s%s%s", _("Media player"),
			(filename != NULL) ? " - " : "", (filename != NULL)
			? ((p != NULL) ? basename(p) : filename) : "");
	free(p);
	gtk_window_set_title(GTK_WINDOW(player->window), buf);
}


/* player_start */
static int _player_start(Player * player)
{
	int ret;
	char const buf[] = "pausing loadfile " PLAYER_SPLASH " 0\nframe_step\n";
	char wid[32];
	char * argv[] = { BINDIR "/mplayer", "mplayer", "-slave", "-wid", NULL,
		"-quiet", "-idle", "-framedrop", "-softvol",
		"-softvol-max", "200", "-identify", "-noconsolecontrols",
		"-nomouseinput", NULL };
	GError * error = NULL;

	argv[4] = wid;
	_player_reset(player, NULL);
	/* XXX not portable */
	snprintf(wid, sizeof(wid), "%lu", (unsigned long)gtk_socket_get_id(
				GTK_SOCKET(player->view_window)));
	if(pipe(player->fd[0]) != 0 || pipe(player->fd[1]) != 0)
		return -player_error(player, strerror(errno), 1);
	if((player->pid = fork()) == -1)
		return -player_error(player, strerror(errno), 1);
	if(player->pid == 0) /* child */
	{
		close(player->fd[0][0]);
		close(player->fd[1][1]);
		if(dup2(player->fd[1][0], 0) == -1)
			exit(player_error(NULL, "dup2", 2));
		if(dup2(player->fd[0][1], 1) == -1)
			exit(player_error(NULL, "dup2", 2));
		execv(argv[0], &argv[1]);
		exit(player_error(NULL, argv[0], 2));
	}
	close(player->fd[0][1]);
	close(player->fd[1][0]);
	player->channel[0] = g_io_channel_unix_new(player->fd[0][0]);
	if(g_io_channel_set_encoding(player->channel[0], NULL, &error)
			!= G_IO_STATUS_NORMAL)
	{
		player_error(player, error->message, 1);
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
		player_error(player, error->message, 1);
		g_error_free(error);
	}
	g_io_channel_set_buffered(player->channel[1], FALSE);
	ret = _player_command(player, buf, sizeof(buf) - 1);
	player->paused = 1;
	return ret;
}


/* callbacks */
/* command_read */
static void _read_parse(Player * player, char const * buf);
static void _read_parse_rtrim(char * buf);

static gboolean _command_read(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	Player * player = data;
	static char buf[512];
	static size_t buf_len = 0;
	gsize read;
	size_t i;
	size_t j;
	GIOStatus status;
	GError * error = NULL;

	if(condition != G_IO_IN)
	{
		player_error(player, "", 0); /* FIXME */
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
		player_error(player, error->message, 1);
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

static void _read_parse(Player * player, char const * buf)
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
		_read_parse_rtrim(str);
		gtk_label_set_text(GTK_LABEL(player->me_album), str);
	}
	else if(sscanf(buf, "ANS_META_ARTIST='%255[^'\n]\n", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		_read_parse_rtrim(str);
		gtk_label_set_text(GTK_LABEL(player->me_artist), str);
	}
	else if(sscanf(buf, "ANS_META_COMMENT='%255[^'\n]\n", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		_read_parse_rtrim(str);
		gtk_label_set_text(GTK_LABEL(player->me_comment), str);
	}
	else if(sscanf(buf, "ANS_META_GENRE='%255[^'\n]\n", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		_read_parse_rtrim(str);
		gtk_label_set_text(GTK_LABEL(player->me_genre), str);
	}
	else if(sscanf(buf, "ANS_META_TITLE='%255[^'\n]\n", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		_read_parse_rtrim(str);
		gtk_label_set_text(GTK_LABEL(player->me_title), str);
	}
	else if(sscanf(buf, "ANS_META_TRACK='%255[^'\n]\n", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		_read_parse_rtrim(str);
		gtk_label_set_text(GTK_LABEL(player->me_track), str);
	}
	else if(sscanf(buf, "ANS_META_YEAR='%255[^'\n]\n", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		_read_parse_rtrim(str);
		gtk_label_set_text(GTK_LABEL(player->me_year), str);
	}
	else if(sscanf(buf, "ANS_PERCENT_POSITION=%u\n", &u1) == 1)
	{
		_player_set_progress(player, u1);
		if(u1 == 100)
			player_next(player);
	}
	else if(sscanf(buf, "ANS_TIME_POSITION=%lf\n", &db) == 1)
	{
		t = db;
		gmtime_r(&t, &tm);
		strftime(str, sizeof(str), "%H:%M:%S", &tm);
		gtk_label_set_text(GTK_LABEL(player->progress_length), str);
	}
	else if(sscanf(buf, "ANS_VIDEO_RESOLUTION='%u x %u'\n", &u1, &u2) == 2)
		player_set_size(player, u1, u2);
	else if(sscanf(buf, "ID_AUDIO_BITRATE=%u\n", &u1) == 1)
		player->audio_bitrate = u1;
	else if(sscanf(buf, "ID_AUDIO_CODEC=%255[^\n]", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		_read_parse_rtrim(str);
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
		_read_parse_rtrim(str);
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
		_read_parse_rtrim(str);
		if(player->album >= 0 && (unsigned)player->album == u1)
			_player_set_metadata(player, PL_COL_ALBUM, str);
		else if(player->artist >= 0 && (unsigned)player->artist == u1)
			_player_set_metadata(player, PL_COL_ARTIST, str);
		else if(player->title >= 0 && (unsigned)player->title == u1)
			_player_set_metadata(player, PL_COL_TITLE, str);
		/* FIXME also update the duration */
	}
	else if(sscanf(buf, "ID_LENGTH=%lf\n", &db) == 1)
		player->length = db;
	else if(sscanf(buf, "ID_SEEKABLE=%u\n", &u1) == 1)
		gtk_widget_set_sensitive(player->progress, u1 ? TRUE : FALSE);
	else if(sscanf(buf, "ID_VIDEO_ASPECT=%lf\n", &db) == 1)
		player->video_aspect = db;
	else if(sscanf(buf, "ID_VIDEO_BITRATE=%u\n", &u1) == 1)
		player->video_bitrate = u1;
	else if(sscanf(buf, "ID_VIDEO_CODEC=%255[^\n]", str) == 1)
	{
		str[sizeof(str) - 1] = '\0';
		_read_parse_rtrim(str);
		if(player->video_codec != NULL)
			free(player->video_codec);
		player->video_codec = strdup(str);
	}
	else if(sscanf(buf, "ID_VIDEO_FPS=%lf\n", &db) == 1)
		player->video_fps = db;
	else if(sscanf(buf, "ID_VIDEO_HEIGHT=%u\n", &u1) == 1)
		player_set_size(player, -1, u1);
	else if(sscanf(buf, "ID_VIDEO_RATE=%u\n", &u1) == 1)
		player->video_rate = u1;
	else if(sscanf(buf, "ID_VIDEO_WIDTH=%u\n", &u1) == 1)
		player_set_size(player, u1, -1);
#ifdef DEBUG
	else
		fprintf(stderr, "DEBUG: unknown output \"%s\"\n", buf);
#endif
}

static void _read_parse_rtrim(char * buf)
{
	size_t i;

	for(i = 0; buf[i] != '\0'; i++);
	for(; i > 0 && buf[i - 1] == ' '; i--)
		buf[i - 1] = '\0';
}


/* command_timeout */
static gboolean _command_timeout(gpointer data)
{
	Player * player = data;
	static const char cmd[] = "pausing_keep get_time_pos\n"
		"pausing_keep get_percent_pos\n";

	_player_command(player, cmd, sizeof(cmd) - 1);
	return TRUE;
}


/* command_write */
static gboolean _command_write(GIOChannel * source, GIOCondition condition,
		gpointer data)
{
	Player * player = data;
	gsize written;
	char * p;
	GError * error = NULL;

	if(condition != G_IO_OUT)
	{
		player_error(player, "", 0); /* FIXME */
		gtk_main_quit();
		player->write_id = 0;
		return FALSE; /* FIXME report error */
	}
	if(g_io_channel_write_chars(source, player->buf, player->buf_len,
				&written, &error) != G_IO_STATUS_NORMAL)
	{
		player_error(player, error->message, 1);
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
