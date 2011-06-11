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

void mozart_rock_and_roll(void);
GstState mozart_get_player_state(void);
int mozart_convert_seconds_to_hms(int secs, int *hours, int *minutes,
								int *seconds);
gint64 mozart_get_stream_position_ns(void);
int mozart_get_stream_position_sec(void);
int mozart_get_stream_position_hms(int *hours, int *minutes, int *seconds);
float __attribute__((deprecated)) mozart_get_stream_progress();
gint64 mozart_get_stream_duration_ns(void);
int mozart_get_stream_duration_sec(void);
int __attribute__((deprecated)) mozart_get_stream_duration_hms(int *hours,
						int *minutes, int *seconds);
char *mozart_get_tag_artist(void);
char *mozart_get_tag_album(void);
char *mozart_get_tag_title(void);
int mozart_get_playlist_position(void);
int mozart_get_playlist_size(void);
int mozart_tags_updated(void);
void mozart_set_got_tags(void);
void mozart_init(int argc, char *argv[]);
void mozart_dump_state(void);
void mozart_destroy(void);

extern GstBus *mozart_bus;

#endif /* _LIBMOZART_H_ */
