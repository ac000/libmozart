/*
 * playlist-operations.h - limozart playlist operations header 
 * 
 * Copyright (C) 2010-2011	OpenTech Labs
 * Copyright (C) 2010-2011	Andrew Clayton <andrew@opentechlabs.co.uk>
 * Released under the GNU Lesser General Public License (LGPL) version 3. 
 * See COPYING
 */

#ifndef _PLAYLISTOPERATIONS_H_
#define _PLAYLISTOPERATIONS_H_

int mozart_init_playlist(char *playlist);
int mozart_switch_playlist(char *playlist);
int __attribute__((visibility("hidden"))) mozart_find_list(char *playlist);
int __attribute__((visibility("hidden"))) mozart_find_uri_index(char *uri);
void mozart_play_index_at_pos(int index, gint64 pos);
void mozart_play_uri_at_pos(char *uri, gint64 pos);
void mozart_add_uri_to_playlist(char *uri, char *playlist);
void mozart_add_m3u_to_playlist(char *m3u, char *playlist);
void __attribute__((visibility("hidden"))) mozart_copy_playlist();
int mozart_get_playlist_position();
int mozart_get_playlist_size();
char *mozart_get_current_uri();
int mozart_get_number_of_playlists();
char *mozart_get_active_playlist_name();
int mozart_playlist_shuffled(char *playlist);
int mozart_remove_playlist(char *playlist);

extern GList *mozart_playlists;	/* A list of struct mozart_list_info_data's */
extern char *mozart_active_playlist;
extern int mozart_active_playlist_index;

/**
 * struct mozart_list_info_data - The playlist structure
 * @tracks - An array of tracks
 * @nr_tracks - Number of tracks in the playlist
 * @name - The name of the playlist
 */
struct mozart_list_info_data {
	GPtrArray *tracks;
	int nr_tracks;
	char *name;
};

#endif /* _PLAYLISTOPERATIONS_H_ */
