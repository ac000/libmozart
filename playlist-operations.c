/*
 * playlist-operations.c - libmozart playlist operations 
 *
 * Copyright (C) 2010	Andrew Clayton <andrew@digital-domain.net>
 * Released under the GNU Lesser General Public License (LGPL) version 3. 
 * See COPYING
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <gst/gst.h>

#include "playlist-operations.h"
#include "player-operations.h"


/*
 * Function to initialise a new playlist.
 * Return 1 if there is already a playlist of the given name.
 * Return 0 is successful.
 */
extern int mozart_init_playlist(char *playlist)
{
	struct list_info_data *list_info;

	if (find_list(playlist) > -1)
		return 1;

	list_info = malloc(sizeof(struct list_info_data));
	list_info->tracks = g_ptr_array_new();
	list_info->nr_tracks = 0;
	list_info->name = malloc(strlen(playlist) + 1);
	strcpy(list_info->name, playlist);

	mozart_playlists = g_list_append(mozart_playlists, list_info);

	return 0;
}

/*
 * Sets the currently active playlist
 */
extern int mozart_switch_playlist(char *playlist)
{
	if (find_list(playlist) < 0)
		return 1;

	active_playlist = malloc(strlen(playlist) + 1);
	strcpy(active_playlist, playlist);
	active_playlist_index = 0;

	gst_element_set_state(mozart_player, GST_STATE_READY);
	g_signal_emit_by_name(mozart_player, "about-to-finish");

	return 0;
}

/*
 * See if there is a playlist already of a given name.
 * Return an integer >= 0 if there is a playlist, this integer
 * specifies the position on the list of the playlist.
 * Return -1 if no list of the gien name is found.
 */
int find_list(char *playlist)
{
	struct list_info_data *list_info;
	int list_len, i;

	list_len = g_list_length(mozart_playlists);
	for (i = 0; i < list_len; i++) {
		list_info = g_list_nth_data(mozart_playlists, i);
		if (strcmp(list_info->name, playlist) == 0)
			return i;
	}

	return -1;
}

/*
 * Add a URI to the playlist.
 */
extern void mozart_add_uri_to_playlist(char *uri, char *playlist)
{
	struct list_info_data *list_info;

	if (!playlist)
		playlist = "default";

	list_info = g_list_nth_data(mozart_playlists, find_list(playlist));

	g_ptr_array_add(list_info->tracks, (gpointer)g_strdup(uri));
	list_info->nr_tracks++;
}

/*
 * Adds a playlist in m3u format to the playlist
 */
extern void mozart_add_m3u_to_playlist(char *m3u, char *playlist)
{
	char buf[PATH_MAX + 1], path[PATH_MAX + 1], playlist_d[PATH_MAX + 1];
	static FILE *fp;

	if (!playlist)
		playlist = "default";

	fp = fopen(m3u, "r");

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

/*
 * Make a copy of the playlist
 */
void mozart_copy_playlist()
{
	struct list_info_data *list_info;
	int i;
	gchar *track;

	list_info = g_list_nth_data(mozart_playlists,
						find_list(active_playlist));
	unshuffled_playlist = g_ptr_array_new();

	for (i = 0; i < list_info->nr_tracks; i++) {
		track = g_strdup(g_ptr_array_index(list_info->tracks, i));
		g_ptr_array_add(unshuffled_playlist, track);
	}
}

/*
 * Return the current position in the playlist
 */
extern int mozart_get_playlist_position()
{
	return active_playlist_index;
}

/*
 * Return the number of entries in the playlist
 */
extern int mozart_get_playlist_size()
{
	struct list_info_data *list_info;

	list_info = g_list_nth_data(mozart_playlists,
						find_list(active_playlist));

	return list_info->nr_tracks;
}

/*
 * Returns the shuffled state of the playlist
 * 0 Unshuffled
 * 1 Shuffled
 */
extern int mozart_playlist_shuffled()
{
	if (!playlist_shuffled)
		return 0;
	else
		return 1;
}

/*
 * Remove a given playlist.
 * Return 0 if specified playlist is not found
 * Return 1 on success.
 */
extern int mozart_remove_playlist(char *playlist)
{
	struct list_info_data *list_info;

	if (find_list(playlist) < 0)
                return 0;

	list_info = g_list_nth_data(mozart_playlists,
						find_list(active_playlist));

	g_ptr_array_foreach(list_info->tracks, (GFunc)g_free,
				g_ptr_array_index(list_info->tracks, 0));
	g_ptr_array_free(list_info->tracks, TRUE);
	free(list_info->name);
	free(list_info);

	return 1;
}
