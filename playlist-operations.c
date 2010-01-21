/*
 * playlist-operations.c - libmozart playlist operations 
 *
 * Copyright (C) 2010	Andrew Clayton <andrew@digital-domain.net>
 * Released under the GNU Lesser General Public License (LGPL) version 3. 
 * See COPYING
 */

#include <glib.h>

#include "playlist-operations.h"


/*
 * Add a URI to the playlist.
 */
extern void mozart_add_uri_to_playlist(char *uri)
{
	char *turi;

	turi = g_strdup(uri);
	g_ptr_array_add(tracks, (gpointer)turi);
	nr_tracks++;
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
 * Return the current position in the playlist
 */
extern int mozart_get_playlist_position()
{
	return track_index;
}

/*
 * Return the number of entries in the playlist
 */
extern int mozart_get_playlist_size()
{
	return nr_tracks;
}


