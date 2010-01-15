/*
 * player-operations.h - Basic Audio Player Operations API header 
 * 
 * Copyright (C) 2009   Andrew Clayton <andrew@digital-domain.net>
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
extern void mozart_fisher_yates_shuffle();
extern void mozart_unshuffle();

extern GstElement *player;
extern GPtrArray *tracks, *unshuffled_tracks;
extern int track_index;
extern int nr_tracks;
extern int shuffled;

#endif /* _PLAYEROPERATIONS_H_ */
