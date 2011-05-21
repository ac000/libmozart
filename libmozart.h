/*
 * libmozart.h - mozart API header 
 *
 * Copyright (C) 2009-2011	OpenTech Labs
 * Copyright (C) 2009-2011	Andrew Clayton <andrew@opentechlabs.co.uk>
 * Released under the GNU Lesser General Public License (LGPL) version 3. 
 * See COPYING
 */

#ifndef _LIBMOZART_H_
#define _LIBMOZART_H_

#include "player-operations.h"
#include "playlist-operations.h"

#define LIBMOZART_VERSION	"0.0.0"

#define TAG_LENGTH		80

void mozart_cb_eos(GstBus *mozart_bus, gpointer user_data,
						GstElement *mozart_player);
gboolean mozart_cb_tag(GstBus *mozart_bus, GstMessage *mozart_message);
extern void mozart_rock_and_roll();
void mozart_quiesce();
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
extern void mozart_dump_state();
void mozart_free_playlist(struct mozart_list_info_data *list_info);
extern void mozart_destroy();

extern GstBus *mozart_bus;

#endif /* _LIBMOZART_H_ */
