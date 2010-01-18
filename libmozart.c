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

GstElement *player;
GstBus *bus;
GPtrArray *tracks, *unshuffled_tracks;
int track_length;
int track_index;
int nr_tracks;
int shuffled = 0;	/* Playlist shuffle state, 0 no, 1 yes */

 
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

	if (gst_element_query_position(player, &fmt, &pos)) {
	}

	/* Call this function again */
	return TRUE;
}

static void cb_eos(GMainLoop *loop)
{
	g_main_loop_quit(loop);
}

/*
 * This gets the ball rolling and is called once at startup and then 
 * whenever the end of the current track is nearing.
 */
void mozart_rock_and_roll()
{
	if (track_index == nr_tracks)
		track_index = 0;
	
	g_object_set(G_OBJECT(player), "uri", 
				g_ptr_array_index(tracks, track_index), NULL);
	gst_element_set_state(player, GST_STATE_PLAYING);

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

	gst_element_set_state(player, GST_STATE_NULL);
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

static void mozart_init()
{
	GMainLoop *loop;

	gst_init(NULL, NULL);
	loop = g_main_loop_new(NULL, TRUE);

	g_print("Using (%s)\n", gst_version_string());

	/* Set PulseAudio stream tag */
	g_setenv("PULSE_PROP_media.role", "music", TRUE);
	
	player = gst_element_factory_make("playbin2", "player");
	printf("DEBUG: player created.\n");

        bus = gst_pipeline_get_bus(GST_PIPELINE(player));
        gst_bus_add_signal_watch(bus);
        g_signal_connect(bus, "message::eos", G_CALLBACK(cb_eos), loop);

	/* 
 	 * Catch when the current track is about to finish and 
 	 * prepare the next one.
 	 */
	g_signal_connect(player, "about-to-finish",
				G_CALLBACK(mozart_rock_and_roll), player);

	/* 
 	 * Add a timer callback to get track position information.
	 */
	g_timeout_add_seconds_full(G_PRIORITY_DEFAULT, 1,
						(GSourceFunc) cb_get_position,
								player, NULL);

	/* Start up in a quiescent state, ready for receiving instructions */
	mozart_quiesce();
	printf("DEBUG: quiesced\n");
		
	g_main_loop_run(loop);

	/* Cleanup */
	g_main_loop_unref(loop);
	gst_element_set_state(player, GST_STATE_NULL);
	gst_object_unref(player);
	gst_object_unref(bus);
}

