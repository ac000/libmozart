/*
 * playlist-operations.c - libmozart playlist operations 
 *
 * Copyright (C) 2010	Andrew Clayton <andrew@digital-domain.net>
 * Released under the GNU Lesser General Public License (LGPL) version 3. 
 * See COPYING
 */

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <glib.h>

#include "playlist-operations.h"


/*
 * Add a URI to the playlist.
 */
extern void mozart_add_uri_to_playlist(char *uri)
{
	char *turi;

	turi = g_strdup(uri);
	g_ptr_array_add(playlist, (gpointer)turi);
	playlist_size++;
}

/*
 * Adds a playlist in m3u format to the playlist
 */
extern void mozart_add_m3u_to_playlist(char *m3u)
{
	char buf[PATH_MAX + 1], path[PATH_MAX + 1], playlist_d[PATH_MAX + 1];
	static FILE *fp;

	fp = fopen(m3u, "r");

	/*
	 * dirname() modifies the string passed to it, so make
	 * a copy of it first.
	 */
	strncpy(playlist_d, dirname(m3u), PATH_MAX);

	while (fgets(buf, PATH_MAX - strlen(path), fp)) {
		strcpy(path, "file://");
		strncat(path, playlist_d, PATH_MAX - strlen(path) - 2);
		strcat(path, "/");
		strncat(path, g_strchomp(buf), PATH_MAX - strlen(path));
		mozart_add_uri_to_playlist(path);
	}

	fclose(fp);
}

/*
 * Make a copy of the playlist
 */
void mozart_copy_playlist()
{
	int i;
	gchar *track;

	unshuffled_playlist = g_ptr_array_new();

	for (i = 0; i < playlist_size; i++) {
		track = g_strdup(g_ptr_array_index(playlist, i));
		g_ptr_array_add(unshuffled_playlist, track);
	}
}

/*
 * Return the current position in the playlist
 */
extern int mozart_get_playlist_position()
{
	return playlist_index;
}

/*
 * Return the number of entries in the playlist
 */
extern int mozart_get_playlist_size()
{
	return playlist_size;
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
