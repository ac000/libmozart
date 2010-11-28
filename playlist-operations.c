/*
 * playlist-operations.c - libmozart playlist operations 
 *
 * Copyright (C) 2010	Andrew Clayton <andrew@digital-domain.net>
 * Released under the GNU Lesser General Public License (LGPL) version 3. 
 * See COPYING
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <libgen.h>
#include <gst/gst.h>

#include "debug.h"
#include "libmozart.h"
#include "playlist-operations.h"
#include "player-operations.h"


/**
 * mozart_init_playlist - Initialise a new playlist
 * @playlist: The name of new playlist
 *.
 * Returns 0 on success or -1 on failure
 */
extern int mozart_init_playlist(char *playlist)
{
	struct mozart_list_info_data *list_info;

	if (mozart_find_list(playlist) > -1)
		return -1;

	list_info = malloc(sizeof(struct mozart_list_info_data));
	list_info->tracks = g_ptr_array_new();
	list_info->nr_tracks = 0;
	list_info->name = malloc(strlen(playlist) + 1);
	strcpy(list_info->name, playlist);

	mozart_playlists = g_list_append(mozart_playlists, list_info);

	d_printf(7, "libmozart %s: Created %s playlist\n",
							__FUNCTION__, playlist);
	return 0;
}

/**
 * mozart_switch_playlist - Switch to a given playlist
 * @playlist: The name of the playlist to switch to
 *
 * Returns 0 on success or -1 on failure
 */
extern int mozart_switch_playlist(char *playlist)
{
	struct mozart_list_info_data *list_info;
	int i;

	if ((i = mozart_find_list(playlist)) < 0)
		return -1;

	list_info = g_list_nth_data(mozart_playlists, i);
	if (list_info->nr_tracks == 0)
		return -1;

	/*
	 * Avoid a lockup here, waiting on a futex, when switching
	 * playlists too quickly.
	 *
	 * Seems to be stuck at setting the player state to NULL
	 *
	 * Hiding some other bug?
	 */
	mozart_nsleep(50000000);

	mozart_active_playlist = malloc(strlen(playlist) + 1);
	strcpy(mozart_active_playlist, playlist);
	mozart_active_playlist_index = -1;

	gst_element_set_state(mozart_player, GST_STATE_NULL);
	g_signal_emit_by_name(mozart_player, "about-to-finish");

	return 0;
}

/**
 * mozart_find_list - Find a playlist
 * @playlist: The name of the playlist to find
 *
 * Returns >= 0 on success, this integer designates the position
 * in the list of playlists of this playlist
 *
 * Return -1 if no list of the given name is found.
 */
int mozart_find_list(char *playlist)
{
	struct mozart_list_info_data *list_info;
	int list_len;
	int i;

	list_len = g_list_length(mozart_playlists);
	for (i = 0; i < list_len; i++) {
		list_info = g_list_nth_data(mozart_playlists, i);
		if (strcmp(list_info->name, playlist) == 0)
			return i;
	}

	return -1;
}

/**
 * mozart_find_uri_index - Find the active playlist index of the passed URI
 * @uri: The URI to find in the active playlist
 *
 * Returns 0..nr_tracks - 1 on success or -1 on failure
 */
int mozart_find_uri_index(char *uri)
{
	struct mozart_list_info_data *list_info;
	int list_len;
	int i;

	list_info = g_list_nth_data(mozart_playlists,
				mozart_find_list(mozart_active_playlist));

	list_len = mozart_get_playlist_size();
	for (i = 0; i < list_len; i++) {
		if (strcmp(g_ptr_array_index(list_info->tracks, i), uri) == 0)
			return i;
	}

	return -1;
}

/**
 * mozart_play_index_at_pos - Play a track at a certain position in the stream
 * @index: The position of the track in the array to play
 * @pos: The position in ns to start at
 */
extern void mozart_play_index_at_pos(int index, gint64 pos)
{
	struct mozart_list_info_data *list_info;

	if (mozart_active_playlist == NULL)
		list_info = g_list_nth_data(mozart_playlists, 0);
	else
		list_info = g_list_nth_data(mozart_playlists,
				mozart_find_list(mozart_active_playlist));

	gst_element_set_state(mozart_player, GST_STATE_NULL);
	g_object_set(G_OBJECT(mozart_player), "uri",
				g_ptr_array_index(list_info->tracks,
								index), NULL);
	/*
	 * Pause the stream and sleep here or a call to
	 * gst_element_query_duration() can fail.
	 */
	gst_element_set_state(mozart_player, GST_STATE_PAUSED);
	mozart_nsleep(50000000);
	gst_element_set_state(mozart_player, GST_STATE_PLAYING);
	/* Sleep needed here to prevent the seek from failing */
	mozart_nsleep(50000000);
	gst_element_seek_simple(mozart_player, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
									pos);
	mozart_active_playlist_index = index;
}

extern void mozart_play_uri_at_pos(char *uri, gint64 pos)
{
	struct mozart_list_info_data *list_info;
	int index;

	if (mozart_active_playlist == NULL)
		list_info = g_list_nth_data(mozart_playlists, 0);
	else
		list_info = g_list_nth_data(mozart_playlists,
				mozart_find_list(mozart_active_playlist));

	index = mozart_find_uri_index(uri);

	gst_element_set_state(mozart_player, GST_STATE_NULL);
	g_object_set(G_OBJECT(mozart_player), "uri",
					g_ptr_array_index(list_info->tracks,
								index), NULL);
	/*
	 * Pause the stream and sleep here or a call to
	 * gst_element_query_duration() can fail.
	 */
	gst_element_set_state(mozart_player, GST_STATE_PAUSED);
	mozart_nsleep(50000000);
	gst_element_set_state(mozart_player, GST_STATE_PLAYING);
	/* Sleep needed here to prevent the seek from failing */
	mozart_nsleep(50000000);
	gst_element_seek_simple(mozart_player, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
									pos);
	mozart_active_playlist_index = index;
}

/**
 * mozart_add_uri_to_playlist - Add a URI to a playlist
 * @uri: The URI to add
 * @playlist: The name of the playlist to add it to
 */
extern void mozart_add_uri_to_playlist(char *uri, char *playlist)
{
	struct mozart_list_info_data *list_info;

	if (!playlist)
		playlist = "default";

	list_info = g_list_nth_data(mozart_playlists,
						mozart_find_list(playlist));
	if (!list_info)
		return;

	g_ptr_array_add(list_info->tracks, (gpointer)g_strdup(uri));
	list_info->nr_tracks++;

	d_printf(7, "libmozart %s: Adding %s to %s\n",
						__FUNCTION__, uri, playlist);
}

/**
 * mozart_add_m3u_to_playlist - Add m3u playlist to a playlist
 * @m3u: The m3u to add
 * @playlist: The name of the playlist to add it to
 */
extern void mozart_add_m3u_to_playlist(char *m3u, char *playlist)
{
	char buf[PATH_MAX + 1] = "\0";
	char path[PATH_MAX + 1] = "\0";
	char playlist_d[PATH_MAX + 1] = "\0";
	static FILE *fp;

	if (!playlist)
		playlist = "default";

	if (!(fp = fopen(m3u, "r"))) {
		fprintf(stderr, "libmozart %s: Can't open file %s\n",
							__FUNCTION__, m3u);
		return;
	}

	d_printf(7, "libmozart %s: Adding %s to %s\n",
						__FUNCTION__, m3u, playlist);
	/*
	 * dirname() modifies the string passed to it, so make
	 * a copy of it first.
	 */
	strncpy(playlist_d, dirname(m3u), PATH_MAX);

	while (fgets(buf, PATH_MAX - strlen(path), fp)) {
		/*
		 * Catch any blank lines at the end of the file
		 */
		if (strcmp(buf, "\n") == 0)
			continue;

		strcpy(path, "file://");
		strncat(path, playlist_d, PATH_MAX - strlen(path) - 2);
		strcat(path, "/");
		strncat(path, g_strchomp(buf), PATH_MAX - strlen(path));
		mozart_add_uri_to_playlist(path, playlist);
	}
	fclose(fp);
}

/**
 * mozart_copy_playlist - Make a copy of the active playlist
 * @playlist: The name of the new playlist
 */
void mozart_copy_playlist(char *playlist)
{
	struct mozart_list_info_data *list_info;
	int i;
	gchar *track;

	list_info = g_list_nth_data(mozart_playlists,
				mozart_find_list(mozart_active_playlist));

	for (i = 0; i < list_info->nr_tracks; i++) {
		track = g_strdup(g_ptr_array_index(list_info->tracks, i));
		mozart_add_uri_to_playlist(track, playlist);
	}
}

/**
 * mozart_get_playlist_position - Get the track number in the active playlist
 *
 * Returns 1..nr_tracks
 */
extern int mozart_get_playlist_position()
{
	return mozart_active_playlist_index + 1;
}

/**
 * mozart_get_playlist_size - Get the number of entries in the active playlist
 *
 * Returns the number of entries in the playlist
 */
extern int mozart_get_playlist_size()
{
	struct mozart_list_info_data *list_info;

	list_info = g_list_nth_data(mozart_playlists,
				mozart_find_list(mozart_active_playlist));

	return list_info->nr_tracks;
}

/**
 * mozart_get_current_uri - Get the URI of the current track
 *
 * Returns the URI of the currently playing track.
 */
extern char *mozart_get_current_uri()
{
	struct mozart_list_info_data *list_info;

	list_info = g_list_nth_data(mozart_playlists,
				mozart_find_list(mozart_active_playlist));

	return g_ptr_array_index(list_info->tracks,
						mozart_active_playlist_index);
}

/**
 * mozart_get_active_playlist_name - Get the name of the active playlist
 *
 * Returns the name of the active playlist
 */
extern char *mozart_get_active_playlist_name()
{
	return mozart_active_playlist;
}

/**
 * mozart_get_number_of_playlists - Get the number of playlists
 *
 * Returns the number of playlists
 */
extern int mozart_get_number_of_playlists()
{
	return g_list_length(mozart_playlists);
}

/**
 * mozart_playlist_shuffled - Get the shuffled status of a playlist
 * @playlist: The name of the playlist to check
 *
 * Returns 0 for unshuffled or 1 for shuffled
 */
extern int mozart_playlist_shuffled(char *playlist)
{
	struct mozart_list_info_data *list_info;
	char *uname;

	if (!playlist)
		playlist = mozart_active_playlist;

	uname = malloc(strlen(playlist) + 4);
	sprintf(uname, "%s/ul", playlist);
	list_info = g_list_nth_data(mozart_playlists, mozart_find_list(uname));
	free(uname);

	if (!list_info)
		return 0;
	else
		return 1;
}

/** mozart_remove_playlist - Remove a specified playlist
 * @playlist: The playlist to remove
 *
 * Returns 0 on success or -1 on failure.
 */
extern int mozart_remove_playlist(char *playlist)
{
	struct mozart_list_info_data *list_info;
	GList *node;
	int pos;

	/* Don't try to remove the active playlist */
	if (strcmp(playlist, mozart_active_playlist) == 0)
		return -1;

	pos = mozart_find_list(playlist);
	if (pos < 0)
		return -1;

	list_info = g_list_nth_data(mozart_playlists, pos);

	/* Don't try to free non-existent array entries */
	if (list_info->nr_tracks > 0)
		g_ptr_array_foreach(list_info->tracks, (GFunc)g_free,
				g_ptr_array_index(list_info->tracks, 0));

	g_ptr_array_free(list_info->tracks, TRUE);
	free(list_info->name);
	free(list_info);
	node = g_list_nth(mozart_playlists, pos);
	mozart_playlists = g_list_delete_link(mozart_playlists, node);

	return 0;
}
