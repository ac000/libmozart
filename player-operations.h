/*
 * player-operations.h - Basic Audio Player Operations API header 
 * 
 * Copyright (C) 2009-2010	Andrew Clayton <andrew@digital-domain.net>
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
void mozart_fisher_yates_shuffle();
extern void mozart_shuffle();
extern void mozart_unshuffle();

extern GstElement *mozart_player;
extern GPtrArray *unshuffled_playlist;
extern int active_playlist_index;
extern int playlist_shuffled;

#endif /* _PLAYEROPERATIONS_H_ */
