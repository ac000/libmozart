/*
 * player-operations.c - Basic Audio Player Operations
 *
 * Copyright (C) 2009-2010	Andrew Clayton <andrew@digital-domain.net>
 * Released under the GNU Lesser General Public License (LGPL) version 3. 
 * See COPYING
 */

#include <string.h>
#include <gst/gst.h>

#include "debug.h"
#include "libmozart.h"
#include "player-operations.h"
#include "playlist-operations.h"

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
	if (mozart_active_playlist_index + 1 == mozart_get_playlist_size())
		mozart_active_playlist_index = -1;

	if (mozart_repeat_single)
		mozart_toggle_repeat_single();

	gst_element_set_state(mozart_player, GST_STATE_NULL);
	g_signal_emit_by_name(mozart_player, "about-to-finish");
}

/*
 * Move the track index to point to the previous track and signal
 * the player that the current stream is about to finish.
 */
extern void mozart_prev_track()
{
	/*
	 * We decrement the index by 2 here because
	 * mozart_active_playlist_index is ++'d first thing in
	 * mozart_rock_and_roll.
	 */
	if (mozart_active_playlist_index == 0)
		mozart_active_playlist_index = mozart_get_playlist_size() - 2;
	else
		mozart_active_playlist_index -= 2;

	if (mozart_repeat_single)
		mozart_toggle_repeat_single();

	gst_element_set_state(mozart_player, GST_STATE_NULL);
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
 * Control forward/backward seeking
 */
extern void mozart_player_seek(char *seek)
{
	GstFormat fmt = GST_FORMAT_TIME;
	gint64 pos;		/* position in current stream in ns */
	gint64 duration;	/* duration of current stream in ns */
	gint64 excess;		/* excess ns after seeking beyond stream */

	if (!gst_element_query_position(mozart_player, &fmt, &pos))
		return;

	if (strcmp(seek, "sseek-fwd") == 0) {
		duration = mozart_get_stream_duration_ns();
		if ((pos + 10 * GST_SECOND) > duration) {
			excess = pos + 10 * GST_SECOND - duration;
			mozart_next_track();
			mozart_nsleep(50000000);
			gst_element_seek_simple(mozart_player, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
									excess);
		} else {
			gst_element_seek_simple(mozart_player, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
							pos + 10 * GST_SECOND);
		}
	} else if (strcmp(seek, "lseek-fwd") == 0) {
		duration = mozart_get_stream_duration_ns();
		if ((pos + 60 * GST_SECOND) > duration) {
			excess = pos + 60 * GST_SECOND - duration;
			mozart_next_track();
			mozart_nsleep(50000000);
			gst_element_seek_simple(mozart_player, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
									excess);
		} else {
			gst_element_seek_simple(mozart_player, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
							pos + 60 * GST_SECOND);
		}
	} else if (strcmp(seek, "sseek-bwd") == 0) {
		if (pos - 10 * GST_SECOND < 0) {
			mozart_prev_track();
			mozart_nsleep(50000000);
			excess = pos - 10 * GST_SECOND;
			duration = mozart_get_stream_duration_ns();
			gst_element_seek_simple(mozart_player, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
							duration + excess);
		} else {
			gst_element_seek_simple(mozart_player, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
							pos - 10 * GST_SECOND);
		}
	} else if (strcmp(seek, "lseek-bwd") == 0) {
		if (pos - 60 * GST_SECOND < 0) {
			mozart_prev_track();
			mozart_nsleep(50000000);
			excess = pos - 60 * GST_SECOND;
			duration = mozart_get_stream_duration_ns();
			gst_element_seek_simple(mozart_player, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
							duration + excess);
		} else {
			gst_element_seek_simple(mozart_player, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
							pos - 60 * GST_SECOND);
		}
	}
}

/*
 * Shuffle a list of tracks using the Fisher-Yates Algorithm
 * Keep a copy of the unshuffled list.
 */
void mozart_fisher_yates_shuffle(char *playlist)
{
	struct mozart_list_info_data *list_info;
	int n, i;
	guint32 random;
	gpointer tmp;

	list_info = g_list_nth_data(mozart_playlists,
						mozart_find_list(playlist));

	n = mozart_get_playlist_size();
	while (n > 1) {
		random = g_random_int() % n;
		i = n - 1;
		tmp = g_ptr_array_index(list_info->tracks, i);
		g_ptr_array_index(list_info->tracks, i) =
					g_ptr_array_index(list_info->tracks,
									random);
		g_ptr_array_index(list_info->tracks, random) = tmp;
		n--;
	}

	if (mozart_debug_level == 7) {
		n = mozart_get_playlist_size();
		for (i = 0; i < n; i++)
			d_printf(7, "libmozart %s: %s\n", __FUNCTION__,
				(char *)g_ptr_array_index(list_info->tracks,
									i));
	}
}

/*
 * Shuffle the playlist, Currently just a wrapper around
 * mozart_fisher_yates_shuffle() but could in future use
 * different shuffling algorithms.
 */
extern void mozart_shuffle(char *playlist)
{
	char *current_uri, *uname;

	if (!playlist)
		playlist = mozart_active_playlist;

	current_uri = mozart_get_current_uri();

	if (!mozart_playlist_shuffled(playlist)) {
		uname = malloc(strlen(playlist) + 4);
		sprintf(uname, "%s/ul", playlist);
		mozart_init_playlist(uname);
		mozart_copy_playlist(uname);
		free(uname);
	}

	mozart_fisher_yates_shuffle(playlist);
	mozart_active_playlist_index = mozart_find_uri_index(current_uri);
}

/* 
 * Restore the playlist to its unshuffled state.
 */
extern void mozart_unshuffle(char *playlist)
{
	char *current_uri, *uname;
	struct mozart_list_info_data *list_info, *u_list_info;
	int i, s;
	gchar *track;

	if (!playlist)
		playlist = mozart_active_playlist;

	current_uri = mozart_get_current_uri();

	uname = malloc(strlen(playlist) + 4);
	sprintf(uname, "%s/ul", playlist);
	u_list_info = g_list_nth_data(mozart_playlists,
						mozart_find_list(uname));
	if (!u_list_info)
		goto out;

	list_info = g_list_nth_data(mozart_playlists,
						mozart_find_list(playlist));
	list_info->tracks = g_ptr_array_new();

	s = mozart_get_playlist_size();
	for (i = 0; i < s; i++) {
		track = g_strdup(g_ptr_array_index(u_list_info->tracks, i));
		g_ptr_array_add(list_info->tracks, track);
		d_printf(7, "libmozart %s: %s\n", __FUNCTION__,
			(char *)g_ptr_array_index(list_info->tracks, i));
	}

	mozart_active_playlist_index = mozart_find_uri_index(current_uri);
	mozart_remove_playlist(uname);

out:
	free(uname);
}

/*
 * Set/unset the repeat single track flag
 */
extern void mozart_toggle_repeat_single()
{
	if (!mozart_repeat_single)
		mozart_repeat_single = TRUE;
	else
		mozart_repeat_single = FALSE;
}

/*
 * Set/unset the repeat all track flag
 */
extern void mozart_toggle_repeat_all()
{
        if (!mozart_repeat_all)
                mozart_repeat_all = TRUE;
        else
                mozart_repeat_all = FALSE;
}

/*
 * Retrieve the status of repeat single
 * Return TRUE for repeat single is set
 * Return FALSE for repwat single is unset
 */
extern gboolean mozart_get_repeat_single()
{
	return mozart_repeat_single;
}

/*
 * Retrieve the status of repeat all
 * Return TRUE for repeat all is set
 * Return FALSE for repwat all is unset
 */
extern gboolean mozart_get_repeat_all()
{
	return mozart_repeat_all;
}
