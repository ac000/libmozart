/*
 * playlist-operations.h - limozart playlist operations header 
 * 
 * Copyright (C) 201o	Andrew Clayton <andrew@digital-domain.net>
 * Released under the GNU Lesser General Public License (LGPL) version 3. 
 * See COPYING
 */

#ifndef _PLAYLISTOPERATIONS_H_
#define _PLAYLISTOPERATIONS_H_

extern void mozart_add_uri_to_playlist(char *uri);
extern void mozart_add_m3u_to_playlist(char *m3u);
void mozart_copy_playlist();
extern int mozart_get_playlist_position();
extern int mozart_get_playlist_size();

extern GPtrArray *tracks, *unshuffled_tracks;
extern int track_index;
extern int nr_tracks;

#endif /* _PLAYLISTOPERATIONS_H_ */
