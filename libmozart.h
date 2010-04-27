/*
 * libmozart.h - mozart API header 
 *
 * Copyright (C) 2009-2010	Andrew Clayton <andrew@digital-domain.net>
 * Released under the GNU Lesser General Public License (LGPL) version 3. 
 * See COPYING
 */

#ifndef _LIBMOZART_H_
#define _LIBMOZART_H_

#include "player-operations.h"
#include "playlist-operations.h"

#define TAG_LENGTH	80

void cb_eos(GMainLoop *loop);
void nsleep(gint64 period);
gboolean mozart_cb_tag(GstBus *mozart_bus, GstMessage *mozart_message);
extern void mozart_rock_and_roll();
extern void mozart_quiesce();
void mozart_copy_playlist();
extern GstState mozart_get_player_state();
extern gint64 mozart_get_stream_position_ns();
extern int mozart_get_stream_position_sec();
extern int mozart_get_stream_position_hms(int *hours, int *minutes, 
								int *seconds);
extern float mozart_get_stream_progress();
extern gint64 mozart_get_stream_duration_ns();
extern int mozart_get_stream_duration_sec();
extern int mozart_get_stream_duration_hms(int *hours, int *minutes,
								int *seconds);
extern int mozart_convert_seconds_to_hms(int secs, int *hours, int *minutes,
                                                                int *seconds);
extern char *mozart_get_tag_artist();
extern char *mozart_get_tag_album();
extern char *mozart_get_tag_title();
extern int mozart_get_playlist_position();
extern int mozart_get_playlist_size();
extern int mozart_tags_updated();
extern void mozart_set_got_tags();
extern void mozart_init(int argc, char *argv[]);
void free_playlists(struct list_info_data *list_info);
extern void mozart_destroy();

#endif /* _LIBMOZART_H_ */
