/*
 * player-operations.h - Basic Audio Player Operations API header 
 * 
 * Copyright (C) 2009-2011	OpenTech Labs
 * Copyright (C) 2009-2011	Andrew Clayton <andrew@opentechlabs.co.uk>
 * Released under the GNU Lesser General Public License (LGPL) version 3. 
 * See COPYING
 */

#ifndef _PLAYEROPERATIONS_H_
#define _PLAYEROPERATIONS_H_

extern void mozart_play_pause();
extern void mozart_next_track();
extern void mozart_prev_track();
extern void mozart_replay_track();
extern void mozart_player_seek(char *seek);
extern void __attribute__((visibility("hidden"))) mozart_fisher_yates_shuffle(
							char *playlist);
extern void mozart_shuffle(char *playlist);
extern void mozart_unshuffle(char *playlist);
extern void mozart_toggle_repeat_single();
extern void mozart_toggle_repeat_all();
extern gboolean mozart_get_repeat_single();
extern gboolean mozart_get_repeat_all();

extern GstElement *mozart_player;
extern int mozart_active_playlist_index;
extern gboolean mozart_repeat_single;
extern gboolean mozart_repeat_all;

#endif /* _PLAYEROPERATIONS_H_ */
