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

static void mozart_add_to_playlist(char *uri);
static gboolean cb_get_position();
static void cb_eos(GMainLoop *loop);
static void mozart_rock_and_roll();
static void mozart_quiesce();
static void mozart_copy_playlist();
static void mozart_init();

#endif /* _LIBMOZART_H_ */
