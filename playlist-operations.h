/*
 * playlist-operations.h - limozart playlist operations header 
 * 
 * Copyright (C) 201o	Andrew Clayton <andrew@digital-domain.net>
 * Released under the GNU Lesser General Public License (LGPL) version 3. 
 * See COPYING
 */

#ifndef _PLAYLISTOPERATIONS_H_
#define _PLAYLISTOPERATIONS_H_

extern int mozart_init_playlist(char *playlist);
extern int mozart_switch_playlist(char *playlist);
int find_list(char *playlist);
extern void mozart_add_uri_to_playlist(char *uri, char *playlist);
extern void mozart_add_m3u_to_playlist(char *m3u, char *playlist);
void mozart_copy_playlist();
extern int mozart_get_playlist_position();
extern int mozart_get_playlist_size();
extern char *mozart_current_uri();
extern int mozart_get_number_of_playlists();
extern char *mozart_get_active_playlist_name();
extern int mozart_playlist_shuffled();
extern int mozart_remove_playlist(char *playlist);

extern GPtrArray *unshuffled_playlist;
extern GList *mozart_playlists;
extern char *active_playlist;
extern int active_playlist_index;
extern int playlist_shuffled;

struct list_info_data {
	GPtrArray *tracks;
	int nr_tracks;
	char *name;
};

#endif /* _PLAYLISTOPERATIONS_H_ */
