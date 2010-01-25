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

#include "libmozart.h"
#include "player-operations.h"
#include "playlist-operations.h"

GstElement *mozart_player;
GstBus *mozart_bus;
GstMessage *mozart_message;
GPtrArray *playlist, *unshuffled_tracks;
int tags_updated = 0;
int playlist_index;
int playlist_size;
int playlist_shuffled = 0;	/* Playlist shuffle state, 0 no, 1 yes */

char *mozart_tag_artist;
char *mozart_tag_album;
char *mozart_tag_title;

void cb_eos(GMainLoop *loop)
{
	g_main_loop_quit(loop);
}

gboolean cb_tag(GstBus *mozart_bus, GstMessage *mozart_message)
{
	GstTagList *tags;
	GValue *tag;

	tags = gst_tag_list_new();

	gst_message_parse_tag(mozart_message, &tags);
	tag = (GValue *)gst_tag_list_get_value_index(tags, GST_TAG_ARTIST, 0);
	if (tag) {
		mozart_tag_artist = (char *)malloc(sizeof(char) * 
					strlen(g_value_get_string(tag)));
		strcpy(mozart_tag_artist, g_value_get_string(tag));
	}
	
	tag = (GValue *)gst_tag_list_get_value_index(tags, GST_TAG_ALBUM, 0);
	if (tag) {
		mozart_tag_album = (char *)malloc(sizeof(char) *
					strlen(g_value_get_string(tag)));
		strcpy(mozart_tag_album, g_value_get_string(tag));
	}

	tag = (GValue *)gst_tag_list_get_value_index(tags, GST_TAG_TITLE, 0);
        if (tag) {
		mozart_tag_title = (char *)malloc(sizeof(char) *
					strlen(g_value_get_string(tag)));
		strcpy(mozart_tag_title, g_value_get_string(tag));
	}

	gst_tag_list_free(tags);

	tags_updated = 1;

	return TRUE;
}

/*
 * This gets the ball rolling and is called once at startup and then 
 * whenever the end of the current track is nearing.
 */
extern void mozart_rock_and_roll()
{
	if (playlist_index == playlist_size)
		playlist_index = 0;
	
	g_object_set(G_OBJECT(mozart_player), "uri", 
				g_ptr_array_index(playlist, playlist_index),
									NULL);
	gst_element_set_state(mozart_player, GST_STATE_PLAYING);

	playlist_index++;
}

/*
 * Reset playlist and player, ready for playing a new playlist.
 */
void mozart_quiesce()
{
	playlist = g_ptr_array_new();
	playlist_size = 0;
	playlist_index = 0;

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
extern long int mozart_get_stream_position_ns()
{
	GstFormat fmt = GST_FORMAT_TIME;
	long int pos;

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
	long int ns;

	ns = mozart_get_stream_position_ns();

	if (ns < 0)
		return -1;

	return floor((double)ns / GST_SECOND);
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
extern double mozart_get_stream_progress()
{
	return ((double)mozart_get_stream_position_sec() /
		(double)mozart_get_stream_duration_sec()) * 100;
}

/*
 * Get the duration of the stream in nanoseconds
 */
extern long int mozart_get_stream_duration_ns()
{
	GstFormat fmt = GST_FORMAT_TIME;
	long int duration;

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
	long int ns;

	ns = mozart_get_stream_duration_ns();

	if (ns < 0)
		return -1;

	return floor((double)ns / GST_SECOND);
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
	if (secs < 0)
		return -1;

	if (secs < 60) {
		*hours = 0;
		*minutes = 0;
		*seconds = secs;
	} else if (secs > 59 && secs < 3600) {
		*hours = 0;
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
	return tags_updated;
}

/*
 * Resets tags_updated back to 0 to indicate the tag
 * information hasn't changed
 * tags_updated is set to 1 by cb_tag()
 *
 * This function is used by applications to indicate that
 * they have recieved the updated tags
 */
extern void mozart_set_got_tags()
{
	tags_updated = 0;
}

/*
 * Initialize GStreamer stuff
 */
extern void mozart_init(int argc, char *argv[])
{
	static GMainLoop *loop;

	gst_init(&argc, &argv);

	g_print("Using (%s)\n", gst_version_string());

	/* Set PulseAudio stream tag */
	g_setenv("PULSE_PROP_media.role", "music", TRUE);
	
	mozart_player = gst_element_factory_make("playbin2", "mozart_player");
	printf("DEBUG: player created.\n");

	mozart_bus = gst_pipeline_get_bus(GST_PIPELINE(mozart_player));
	gst_bus_add_signal_watch(mozart_bus);
	g_signal_connect(mozart_bus, "message::eos", G_CALLBACK(cb_eos), loop);
	g_signal_connect(mozart_bus, "message::tag", G_CALLBACK(cb_tag), NULL);

	/* 
 	 * Catch when the current track is about to finish and 
 	 * prepare the next one.
 	 */
	g_signal_connect(mozart_player, "about-to-finish",
					G_CALLBACK(mozart_rock_and_roll), 
								mozart_player);

	/* Start up in a quiescent state, ready for receiving instructions */
	mozart_quiesce();
	printf("DEBUG: quiesced\n");
}

/*
 * Clean up GStreamer stuff
 */
extern void mozart_destroy()
{
	gst_element_set_state(mozart_player, GST_STATE_NULL);
	gst_object_unref(mozart_player);
	gst_object_unref(mozart_bus);
}

