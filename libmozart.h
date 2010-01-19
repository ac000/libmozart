/*
 * libmozart.h - mozart API header 
 *
 * Copyright (C) 2009   Andrew Clayton <andrew@digital-domain.net>
 * Released under the GNU Lesser General Public License (LGPL) version 3. 
 * See COPYING
 */

#ifndef _LIBMOZART_H_
#define _LIBMOZART_H_

#define likely(x)       __builtin_expect (!!(x), 1)
#define unlikely(x)     __builtin_expect (!!(x), 0)

extern void mozart_add_to_playlist(char *uri);
static void cb_eos(GMainLoop *loop);
static gboolean cb_tag(GstBus *mozart_bus, GstMessage *mozart_message);
void mozart_rock_and_roll();
void mozart_quiesce();
void mozart_copy_playlist();
extern GstState mozart_get_player_state();
extern long int mozart_get_stream_position_ns();
extern int mozart_get_stream_position_sec();
extern int mozart_get_stream_position_hms(int *hours, int *minutes, 
								int *seconds);
extern void mozart_init(int argc, char *argv[]);
extern void mozart_destroy();

#endif /* _LIBMOZART_H_ */
