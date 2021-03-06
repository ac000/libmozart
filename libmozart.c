/*
 * libmozart.c - Audio player framework
 *
 * Copyright (C) 2009-2011	OpenTech Labs
 * Copyright (C) 2009-2011	Andrew Clayton <andrew@opentechlabs.co.uk>
 * Released under the GNU Lesser General Public License (LGPL) version 3. 
 * See COPYING
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <math.h>
#include <time.h>

#include <gst/gst.h>
#include <glib.h>

#include "debug.h"
#include "utils.h"
#include "libmozart.h"
#include "player-operations.h"
#include "playlist-operations.h"

GstElement *mozart_player;
GstBus *mozart_bus;
GstMessage *mozart_message;
GList *mozart_playlists;
char *mozart_active_playlist = NULL;
int mozart_updated_tags = 0;
int mozart_active_playlist_index;
gboolean mozart_repeat_single = FALSE;
gboolean mozart_repeat_all = TRUE;
int mozart_debug_level = 0;
char mozart_tag_artist[TAG_LENGTH + 1];
char mozart_tag_album[TAG_LENGTH + 1];
char mozart_tag_title[TAG_LENGTH + 1];


static void mozart_cb_eos(GstBus *mozart_bus, gpointer user_data,
						GstElement *mozart_player)
{
	gst_element_set_state(mozart_player, GST_STATE_PAUSED);
}

static gboolean mozart_cb_tag(GstBus *mozart_bus, GstMessage *mozart_message)
{
	GstTagList *tags;
	GValue *tag;

	tags = gst_tag_list_new();

	gst_message_parse_tag(mozart_message, &tags);
	tag = (GValue *)gst_tag_list_get_value_index(tags, GST_TAG_ARTIST, 0);
	if (tag) {
		strncpy(mozart_tag_artist, (char *)g_value_get_string(tag),
								TAG_LENGTH);
		mozart_tag_artist[strlen(mozart_tag_artist)] = '\0';
	}
	tag = (GValue *)gst_tag_list_get_value_index(tags, GST_TAG_ALBUM, 0);
	if (tag) {
		strncpy(mozart_tag_album, (char *)g_value_get_string(tag),
								TAG_LENGTH);
		mozart_tag_album[strlen(mozart_tag_album)] = '\0';
	}
	tag = (GValue *)gst_tag_list_get_value_index(tags, GST_TAG_TITLE, 0);
	if (tag) {
		strncpy(mozart_tag_title, (char *)g_value_get_string(tag),
								TAG_LENGTH);
		mozart_tag_title[strlen(mozart_tag_title)] = '\0';
	}
	gst_tag_list_free(tags);

	mozart_updated_tags = 1;

	return TRUE;
}

/*
 * This gets the ball rolling and is called once at startup and then 
 * whenever the end of the current track is nearing.
 */
void mozart_rock_and_roll(void)
{
	struct mozart_list_info_data *list_info;

	mozart_active_playlist_index++;

	list_info = g_list_nth_data(mozart_playlists,
				mozart_find_list(mozart_active_playlist));

	if (mozart_repeat_single) {
		if (mozart_active_playlist_index)
			mozart_active_playlist_index--;
	}

	if (mozart_active_playlist_index == list_info->nr_tracks) {
		mozart_active_playlist_index--;
		if (!mozart_repeat_all)
			return;

		mozart_active_playlist_index = 0;
	}

	g_object_set(G_OBJECT(mozart_player), "uri",
				g_ptr_array_index(list_info->tracks,
					mozart_active_playlist_index), NULL);
	gst_element_set_state(mozart_player, GST_STATE_PLAYING);
}

/*
 * Reset playlist and player, ready for playing a new playlist.
 */
static void mozart_quiesce(void)
{
	mozart_playlists = NULL;
	mozart_active_playlist_index = -1;
	mozart_init_playlist("default");
	mozart_active_playlist = "default";
	gst_element_set_state(mozart_player, GST_STATE_NULL);
}

/**
 * mozart_get_player_state - Returns the GstState of the player
 *
 * Returns one of; GST_STATE_VOID_PENDING, GST_STATE_NULL, GST_STATE_READY,
 * 					GST_STATE_PAUSED, GST_STATE_PLAYING
 */
GstState mozart_get_player_state(void)
{
	GstState state;

	gst_element_get_state(mozart_player, &state, NULL, 100000);

	return state;
}

/**
 * mozart_convert_seconds_to_hms - Split a number of seconds up into hours,
 * 							minutes and seconds
 * @secs: Number of seconds to split up
 * @hours: Variable to hold the number of hours
 * @minutes: Variable to hold the number of minutes
 * @seconds: Variable to hold the number of seconds
 *
 * Returns 0 on success or -1 on failure
 */
int mozart_convert_seconds_to_hms(int secs, int *hours, int *minutes,
								int *seconds)
{
	if (secs < 0)
		return -1;

	*seconds = secs % 60;
	secs /= 60;
	*minutes = secs % 60;
	*hours = secs / 60;

	return 0;
}

/**
 * mozart_get_stream_position_ns - Get the position of the stream in nanoseconds
 *
 * Returns >= 0 on success or -1 on failure
 */
gint64 mozart_get_stream_position_ns(void)
{
	GstFormat fmt = GST_FORMAT_TIME;
	gint64 pos;

	if (gst_element_query_position(mozart_player, &fmt, &pos))
		return pos;
	else
		return -1;
}

/**
 * mozart_get_stream_position_sec - Get the position of the stream in seconds
 *
 * Returns >= 0 on success or -1 on failure
 */
int mozart_get_stream_position_sec(void)
{
	gint64 ns;

	ns = mozart_get_stream_position_ns();

	if (ns < 0)
		return -1;

	return ns / GST_SECOND;
}

/**
 * mozart_get_stream_position_hms - Get the position of the stream split up
 * 						into hours, minutes and seconds
 * @hours: Variable to hold the hours
 * @minutes: Variable to hold the minutes
 * @seconds: Variable to hold the seconds
 *
 * Returns 0 on success or -1 on failure
 */
int mozart_get_stream_position_hms(int *hours, int *minutes, int *seconds)
{
	return mozart_convert_seconds_to_hms(
					mozart_get_stream_position_sec(),
						hours, minutes, seconds);
}

/**
 * mozart_get_stream_progress - Returns the position of the stream as a
 * 								percentage
 *
 * Returns a value between 0.0 and 100.0
 */
float __attribute__((deprecated)) mozart_get_stream_progress(void)
{
	return ((float)mozart_get_stream_position_sec() /
		(float)mozart_get_stream_duration_sec()) * 100;
}

/**
 * mozart_get_stream_duration_ns - Get the duration of the stream in nanoseconds
 *
 * Returns >= 0 on success or -1 on failure
 */
gint64 mozart_get_stream_duration_ns(void)
{
	GstFormat fmt = GST_FORMAT_TIME;
	gint64 duration;

	if (!(gst_element_query_duration(mozart_player, &fmt, &duration)))
		return -1;
	else
		return duration;
}

/**
 * mozart_get_stream_duration_sec - Get the duration of the stream in seconds
 *
 * Returns >= 0 on success or -1 on failure
 */
int mozart_get_stream_duration_sec(void)
{
	gint64 ns;

	ns = mozart_get_stream_duration_ns();
	if (ns < 0)
		return -1;

	return ns / GST_SECOND;
}

/**
 * mozart_get_stream_duration_hms - Get the duration of the stream split up
 * 						into hours, minutes and seconds
 * @hours: Variable to hold the number of hours
 * @minutes: variable to hold the number of minutes
 * @seconds: Variable to hold the number of seconds
 *
 * Returns 0 on success or -1 on failure
 */
int __attribute__((deprecated)) mozart_get_stream_duration_hms(int *hours,
						int *minutes, int *seconds)
{
	return mozart_convert_seconds_to_hms(
					mozart_get_stream_duration_sec(),
						hours, minutes, seconds);
}

/*
 * Return the value of the artist tag
 */
char *mozart_get_tag_artist(void)
{
	return mozart_tag_artist;
}

/*
 * Return the value of the album tag
 */
char *mozart_get_tag_album(void)
{
	return mozart_tag_album;
}

/*
 * Return the value of the title tag
 */
char *mozart_get_tag_title(void)
{
	return mozart_tag_title;
}

/*
 * Returns the value of tags_updated to indicate if
 * the tag information has been updated
 */
int mozart_tags_updated(void)
{
	return mozart_updated_tags;
}

/*
 * Resets tags_updated back to 0 to indicate the tag
 * information hasn't changed
 * tags_updated is set to 1 by mozart_cb_tag()
 *
 * This function is used by applications to indicate that
 * they have recieved the updated tags
 */
void mozart_set_got_tags(void)
{
	mozart_updated_tags = 0;
}

/*
 * Initialize GStreamer stuff
 */
void mozart_init(int argc, char *argv[])
{
	char *mozart_debug;

	gst_init(&argc, &argv);

	if ((mozart_debug = getenv("LIBMOZART_DEBUG")))
		mozart_debug_level = atoi(mozart_debug);

	d_printf(1, "libmozart version %s\n", LIBMOZART_VERSION);
	d_printf(1, "Using %s\n", gst_version_string());

	/* Set PulseAudio stream tag */
	g_setenv("PULSE_PROP_media.role", "music", TRUE);
	
	mozart_player = gst_element_factory_make("playbin2", "mozart_player");

	mozart_bus = gst_pipeline_get_bus(GST_PIPELINE(mozart_player));
	gst_bus_add_signal_watch(mozart_bus);
	g_signal_connect(mozart_bus, "message::eos", G_CALLBACK(mozart_cb_eos),
								mozart_player);
	g_signal_connect(mozart_bus, "message::tag", G_CALLBACK(mozart_cb_tag),
									NULL);

	/* 
 	 * Catch when the current track is about to finish and 
 	 * prepare the next one.
 	 */
	g_signal_connect(mozart_player, "about-to-finish",
					G_CALLBACK(mozart_rock_and_roll), 
								mozart_player);

	/* Start up in a quiescent state, ready for receiving instructions */
	mozart_quiesce();
}

void mozart_dump_state(void)
{
	int l;
	int t;
	int n = mozart_get_number_of_playlists();
	char shuffled[3] = "\0";
	struct mozart_list_info_data *list_info;

	if (mozart_playlist_shuffled(NULL))
		strcpy(shuffled, "S ");
	else
		strcpy(shuffled, "");

	fprintf(stderr, "Number of playlists : %d\n", n);
	fprintf(stderr, "Active playlist     : %s%s\n",
					shuffled,
					mozart_get_active_playlist_name());
	fprintf(stderr, "Current track       : %d / %s\n",
					mozart_get_playlist_position(),
					mozart_get_tag_title());
	fprintf(stderr, "Playlists           :\n");
	for (l = 0; l < n; l++) {
		list_info = g_list_nth_data(mozart_playlists, l);
		fprintf(stderr, "\t%d %s\n", list_info->nr_tracks,
						(char *)list_info->name);
		if (list_info->nr_tracks > 0) {
			for (t = 0; t < list_info->nr_tracks; t++) {
				fprintf(stderr, "\t\t%s\n",
						(char *)g_ptr_array_index(
						list_info->tracks, t));
			}
		}
		fprintf(stderr, "\n");
	}
}

/**
 * mozart_free_playlist - Free a playlist
 * @list_info: playlist to free
 */
static void mozart_free_playlist(struct mozart_list_info_data *list_info)
{
	/*
	 * Avoid a sigsev by only freeing tracks if there _are_ some.
	 * e.g the _default_ playlist is empty.
	 */
	if (list_info->nr_tracks > 0)
		g_ptr_array_foreach(list_info->tracks, (GFunc)g_free,
				g_ptr_array_index(list_info->tracks, 0));

	g_ptr_array_free(list_info->tracks, TRUE);
	free(list_info->name);
	free(list_info);
}

/*
 * Clean up GStreamer stuff
 */
void mozart_destroy(void)
{
	struct mozart_list_info_data *list_info;

	gst_element_set_state(mozart_player, GST_STATE_NULL);
	gst_object_unref(mozart_player);
	gst_object_unref(mozart_bus);

	g_list_foreach(mozart_playlists, (GFunc)mozart_free_playlist,
								&list_info);
	g_list_free(mozart_playlists);
}
