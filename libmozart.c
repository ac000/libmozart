/*
 * libmozart.c - Audio player 
 *
 * Copyright (C) 2009-2010	Andrew Clayton <andrew@digital-domain.net>
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


void mozart_cb_eos(GstBus *mozart_bus, gpointer user_data,
						GstElement *mozart_player)
{
	gst_element_set_state(mozart_player, GST_STATE_PAUSED);
}

/*
 * A wrapper around nanosleep()
 * Takes a number of nanoseconds to sleep for.
 */
void mozart_nsleep(gint64 period)
{
	struct timespec req;
	struct timespec rem;
	int ret;

	req.tv_sec = 0;
	req.tv_nsec = period;

sleep:
	ret = nanosleep(&req, &rem);
	if (ret) {
		req.tv_sec = rem.tv_sec;
		req.tv_nsec = rem.tv_nsec;
		goto sleep;
	}
}

gboolean mozart_cb_tag(GstBus *mozart_bus, GstMessage *mozart_message)
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
extern void mozart_rock_and_roll()
{
	struct mozart_list_info_data *list_info;

	mozart_active_playlist_index++;

	if (mozart_active_playlist == NULL)
		list_info = g_list_nth_data(mozart_playlists, 0);
	else
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
extern void mozart_quiesce()
{
	mozart_playlists = NULL;
	mozart_active_playlist_index = -1;
	mozart_init_playlist("default");
	mozart_active_playlist = "default";
	gst_element_set_state(mozart_player, GST_STATE_NULL);
}

/*
 * Returns the GstState of the player
 */
extern GstState mozart_get_player_state()
{
	GstState state;

	gst_element_get_state(mozart_player, &state, NULL, 100000);

	return state;
}

/*
 * Get the position of the stream in nanoseconds
 */
extern gint64 mozart_get_stream_position_ns()
{
	GstFormat fmt = GST_FORMAT_TIME;
	gint64 pos;

	if (gst_element_query_position(mozart_player, &fmt, &pos))
		return pos;
	else
		return -1;
}

/*
 * Get the position of the stream in seconds
 */
extern int mozart_get_stream_position_sec()
{
	gint64 ns;

	ns = mozart_get_stream_position_ns();

	if (ns < 0)
		return -1;

	return ns / GST_SECOND;
}

/*
 * Get the position of the stream split up into hours, minutes and seconds
 */
extern int mozart_get_stream_position_hms(int *hours, int *minutes, 
								int *seconds)
{
	int secs, ret;

	secs = mozart_get_stream_position_sec();
	ret = mozart_convert_seconds_to_hms(secs, hours, minutes, seconds);

	return ret;
}

/*
 * Returns the position of the stream as a percentage
 */
extern float mozart_get_stream_progress()
{
	return ((float)mozart_get_stream_position_sec() /
		(float)mozart_get_stream_duration_sec()) * 100;
}

/*
 * Get the duration of the stream in nanoseconds
 */
extern gint64 mozart_get_stream_duration_ns()
{
	GstFormat fmt = GST_FORMAT_TIME;
	gint64 duration;

	if (!(gst_element_query_duration(mozart_player, &fmt, &duration)))
		return -1;
	else
		return duration;
}

/*
 * Get the duration of the stream in seconds
 */
extern int mozart_get_stream_duration_sec()
{
	gint64 ns;

	ns = mozart_get_stream_duration_ns();

	if (ns < 0)
		return -1;

	return ns / GST_SECOND;
}

/*
 * Get the duration of the stream split up into hours, minutes and seconds
 */
extern int mozart_get_stream_duration_hms(int *hours, int *minutes,
								int *seconds)
{
	int secs, ret;

	secs = mozart_get_stream_duration_sec();
	ret = mozart_convert_seconds_to_hms(secs, hours, minutes, seconds);

	return ret;
}

/*
 * Split a number of seconds up into hours, minutes and seconds
 */
extern int mozart_convert_seconds_to_hms(int secs, int *hours, int *minutes, 
								int *seconds)
{
	*seconds = 0;
	*minutes = 0;
	*hours = 0;

	if (secs < 0)
		return -1;

	if (secs < 60) {
		*seconds = secs;
	} else if (secs > 59 && secs < 3600) {
		*minutes = secs / 60;
		*seconds = secs - (*minutes * 60);
	} else if (secs > 3599) {
		*hours = secs / 3600;
		*seconds = secs - (*hours * 3600);
		*minutes = secs / 60;
		*seconds = secs - (*minutes * 60);
	}

	return 0;
}

/*
 * Return the value of the artist tag
 */
extern char *mozart_get_tag_artist()
{
	return mozart_tag_artist;
}

/*
 * Return the value of the album tag
 */
extern char *mozart_get_tag_album()
{
	return mozart_tag_album;
}

/*
 * Return the value of the title tag
 */
extern char *mozart_get_tag_title()
{
	return mozart_tag_title;
}

/*
 * Returns the value of tags_updated to indicate if
 * the tag information has been updated
 */
extern int mozart_tags_updated()
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
extern void mozart_set_got_tags()
{
	mozart_updated_tags = 0;
}

/*
 * Initialize GStreamer stuff
 */
extern void mozart_init(int argc, char *argv[])
{
	char *mozart_debug;

	gst_init(&argc, &argv);

	if ((mozart_debug = getenv("LIBMOZART_DEBUG")))
		mozart_debug_level = atoi(mozart_debug);

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

/*
 * Free the playlists
 */

void mozart_free_playlists(struct mozart_list_info_data *list_info)
{
	g_ptr_array_foreach(list_info->tracks, (GFunc)g_free,
				g_ptr_array_index(list_info->tracks, 0));

	g_ptr_array_free(list_info->tracks, TRUE);
	free(list_info->name);
	free(list_info);
}

/*
 * Clean up GStreamer stuff
 */
extern void mozart_destroy()
{
	struct mozart_list_info_data *list_info;

	gst_element_set_state(mozart_player, GST_STATE_NULL);
	gst_object_unref(mozart_player);
	gst_object_unref(mozart_bus);

	g_list_foreach(mozart_playlists, (GFunc)mozart_free_playlists,
								&list_info);
	g_list_free(mozart_playlists);
}
