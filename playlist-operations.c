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
 * Switch to a new playlist
 * Return 1 on failure
 * Return 0 on success
 */
extern int mozart_switch_playlist(char *playlist)
{
	struct list_info_data *list_info;
	int i;
	struct timespec req = { .tv_sec = 0, .tv_nsec = 50000000 };
	struct timespec rem;
	int ret;

	if ((i = find_list(playlist)) < 0)
		return 1;

	list_info = g_list_nth_data(mozart_playlists, i);
	if (list_info->nr_tracks == 0)
		return 1;

	/*
	 * Avoid a lockup here, waiting on a futex, when switching
	 * playlists too quickly.
	 *
	 * Hiding some other bug?
	 */
sleep:
	ret = nanosleep(&req, &rem);
	if (ret) {
		req.tv_sec = rem.tv_sec;
		req.tv_nsec = rem.tv_nsec;
		goto sleep;
	}

	active_playlist = malloc(strlen(playlist) + 1);
	strcpy(active_playlist, playlist);
	active_playlist_index = 0;

	gst_element_set_state(mozart_player, GST_STATE_NULL);
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
 * Returns the active playlist index of the passed URI.
 * Returns -1 if the URI is not found.
 */
int find_uri_index(char *uri)
{
        struct list_info_data *list_info;
        int list_len, i;

        list_info = g_list_nth_data(mozart_playlists,
                                                find_list(active_playlist));

        list_len = mozart_get_playlist_size();
        for (i = 0; i < list_len; i++) {
                if (strcmp(g_ptr_array_index(list_info->tracks, i), uri) == 0)
                        return i + 1;
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
	if (!list_info)
		return;

	g_ptr_array_add(list_info->tracks, (gpointer)g_strdup(uri));
	list_info->nr_tracks++;

	d_printf(7, "libmozart %s: Adding %s to %s\n",
						__FUNCTION__, uri, playlist);
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
 * Returns the URI of the currently playing track.
 */
extern char *mozart_get_current_uri()
{
	struct list_info_data *list_info;

	list_info = g_list_nth_data(mozart_playlists,
						find_list(active_playlist));

	return (char *)g_ptr_array_index(list_info->tracks,
						active_playlist_index - 1);
}

/*
 * Return the name of the active playlist
 */
extern char *mozart_get_active_playlist_name()
{
	return active_playlist;
}

/*
 * Returns the number of playlists
 */
extern int mozart_get_number_of_playlists()
{
	return g_list_length(mozart_playlists);
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
	GList *node;
	int pos;

	/* Don't try to remove the active playlist */
	if (strcmp(playlist, active_playlist) == 0)
		return 0;

	pos = find_list(playlist);
	if (pos < 0)
		return 0;

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

	return 1;
}
