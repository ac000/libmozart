/*
 * libmozart.c - Audio player 
 *
 * Copyright (C) 2009	Andrew Clayton <andrew@digital-domain.net>
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

#include <gst/gst.h>
#include <glib.h>

#include "libmozart.h"
#include "player-operations.h"
#include "nsleep.h"

GstElement *mozart_player;
GstBus *mozart_bus;
GstMessage *mozart_message;
GPtrArray *tracks, *unshuffled_tracks;
int track_length;
int track_index;
int nr_tracks;
int shuffled = 0;	/* Playlist shuffle state, 0 no, 1 yes */

struct _mozart_tag_info {
	char artist[61];
	char album[61];
	char title[61];
} mozart_tag_info;

/*
 * Add a URI to the playlist.
 */
void mozart_add_to_playlist(char *uri)
{
	char *turi;

	turi = g_strdup(uri);
	mozart_quiesce();
	g_ptr_array_add(tracks, (gpointer) turi);
	nr_tracks++;
	mozart_rock_and_roll();
}

/*
 * Query the pipeline for its current position in nanoseconds.
 */
static gboolean cb_get_position()
{
	GstFormat fmt = GST_FORMAT_TIME;
	gint64 pos;

	if (gst_element_query_position(mozart_player, &fmt, &pos)) {
		printf("Stream position: \n");
	}

	/* Call this function again */
	return TRUE;
}

static void cb_eos(GMainLoop *loop)
{
	g_main_loop_quit(loop);
}

static gboolean cb_tag(GstBus *mozart_bus, GstMessage *mozart_message)
{
	GstTagList *tags;
	GValue *tag;

	tags = gst_tag_list_new();

	gst_message_parse_tag(mozart_message, &tags);
	tag = (GValue *) gst_tag_list_get_value_index(tags, GST_TAG_ARTIST, 0);
	if (tag)        	
		strncpy(mozart_tag_info.artist, (char *) tag, 60);

	tag = (GValue *) gst_tag_list_get_value_index(tags, GST_TAG_ALBUM, 0);
	if (tag)
		strncpy(mozart_tag_info.album, (char *) tag, 60);

	tag = (GValue *) gst_tag_list_get_value_index(tags, GST_TAG_TITLE, 0);
        if (tag)
		strncpy(mozart_tag_info.title, (char *) tag, 60);
	
	gst_tag_list_free(tags);

	return TRUE;
}

/*
 * This gets the ball rolling and is called once at startup and then 
 * whenever the end of the current track is nearing.
 */
void mozart_rock_and_roll()
{
	if (track_index == nr_tracks)
		track_index = 0;
	
	g_object_set(G_OBJECT(mozart_player), "uri", 
				g_ptr_array_index(tracks, track_index), NULL);
	gst_element_set_state(mozart_player, GST_STATE_PLAYING);

	track_index++;
}

/*
 * Reset playlist and player, ready for playing a new playlist.
 */
void mozart_quiesce()
{
	tracks = g_ptr_array_new();
	nr_tracks = 0;
	track_index = 0;

	gst_element_set_state(mozart_player, GST_STATE_NULL);
}

/*
 * Make a copy of the playlist
 */
void mozart_copy_playlist()
{
	int i;
	gchar *track;

	unshuffled_tracks = g_ptr_array_new();

	for (i = 0; i < nr_tracks; i++) {
		track = g_strdup(g_ptr_array_index(tracks, i));
		g_ptr_array_add(unshuffled_tracks, track);
	}
}

/*
 * Returns the  GstState of the player
 */
GstState mozart_get_player_state()
{
	GstState state;

	gst_element_get_state(mozart_player, &state, NULL, 100000);

	return state;
}

void mozart_init(int argc, char *argv[])
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

	/* 
 	 * Add a timer callback to get track position information.
	 */
	g_timeout_add_seconds_full(G_PRIORITY_DEFAULT, 1,
						(GSourceFunc) cb_get_position,
							mozart_player, NULL);

	/* Start up in a quiescent state, ready for receiving instructions */
	mozart_quiesce();
	printf("DEBUG: quiesced\n");
}

void mozart_destroy()
{
	gst_element_set_state(mozart_player, GST_STATE_NULL);
	gst_object_unref(mozart_player);
	gst_object_unref(mozart_bus);
}

