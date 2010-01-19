/*
 * player-operations.c - Basic Audio Player Operations
 *
 * Copyright (C) 2009   Andrew Clayton <andrew@digital-domain.net>
 * Released under the GNU Lesser General Public License (LGPL) version 3. 
 * See COPYING
 */

#include <string.h>
#include <gst/gst.h>

#include "player-operations.h"


/* 
 * Toggle the state of the player to playing/paused
 */
extern void mozart_play_pause()
{
	GstState state;
	gst_element_get_state(mozart_player, &state, NULL, 100000);

	if (state == GST_STATE_PLAYING)
		gst_element_set_state(mozart_player, GST_STATE_PAUSED);
	else 
		gst_element_set_state(mozart_player, GST_STATE_PLAYING);
	
}

/*
 * Play next track of the playlist
 */
extern void mozart_next_track()
{
	if (track_index + 1 > nr_tracks)
		track_index = 0;

	gst_element_set_state(mozart_player, GST_STATE_READY);
	g_signal_emit_by_name(mozart_player, "about-to-finish");
}

/*
 * Move the track index to point to the previous track and signal
 * the player that the current stream is about to finish.
 */
extern void mozart_prev_track()
{
	if (track_index - 2 < 0)
		track_index = nr_tracks - 1;
	else
		track_index -= 2;

	gst_element_set_state(mozart_player, GST_STATE_READY);
	g_signal_emit_by_name(mozart_player, "about-to-finish");
}

/*
 * Go back to the begining of the current stream
 */
extern void mozart_replay_track()
{
	gst_element_seek_simple(mozart_player, GST_FORMAT_TIME,
			GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, 0);
}

/*
 * Control forward/backward seeking in the current track.
 */
extern void mozart_player_seek(char *seek)
{
	GstFormat fmt = GST_FORMAT_TIME;
	gint64 pos;

	if (!gst_element_query_position(mozart_player, &fmt, &pos))
		return;

	if (strcmp(seek, "sseek-fwd") == 0) {
		gst_element_seek_simple(mozart_player, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
							pos + 10 * GST_SECOND);
	} else if (strcmp(seek, "lseek-fwd") == 0) {
		gst_element_seek_simple(mozart_player, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
							pos + 60 * GST_SECOND);
	} else if (strcmp(seek, "sseek-bwd") == 0) {
		if (pos - 10 * GST_SECOND < 0)
			mozart_prev_track();
		else
			gst_element_seek_simple(mozart_player, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
							pos - 10 * GST_SECOND);
	} else if (strcmp(seek, "lseek-bwd") == 0) {
		if (pos - 60 * GST_SECOND < 0)
			mozart_prev_track();
		else
			gst_element_seek_simple(mozart_player, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
							pos - 60 * GST_SECOND);
	}
}

/*
 * Shuffle a list of tracks using the Fisher-Yates Algorithm.
 * Keep a copy of the unshuffled list.
 */
extern void mozart_fisher_yates_shuffle()
{
	int n, i;
	guint32 random;
	gpointer tmp;

	n  = nr_tracks;
	while (n > 1) {
		random = g_random_int() % n;
		i = n - 1;
		tmp = g_ptr_array_index(tracks, i);
		g_ptr_array_index(tracks, i) =
					g_ptr_array_index(tracks, random);
		g_ptr_array_index(tracks, random) = tmp;
		n--;
	}

	shuffled = 1;
}

/* 
 * Restore the playlist to its unshuffled state.
 */
extern void mozart_unshuffle()
{
	int i;
	gchar *track;

	tracks = g_ptr_array_new();

	for (i = 0; i < nr_tracks; i++) {
		track = g_strdup(g_ptr_array_index(unshuffled_tracks, i));
		g_ptr_array_add(tracks, track);
	}

	shuffled = 0;
}

