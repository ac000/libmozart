/*
 * debug.h - A simple debug macro that wraps printf
 *
 * Copyright (C) 2010	Andrew Clayton <andrew@digital-domain.net>
 * Released under the GNU Lesser General Public License (LGPL) version 3.
 * See COPYING
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>
#include <stdlib.h>

# define d_printf(level, fmt, ...) \
{ \
	int debug_level; \
	char *debug; \
	if ((debug = getenv("LIBMOZART_DEBUG"))) { \
		debug_level = atoi(debug); \
		if (level <= debug_level) \
			printf(fmt, ##__VA_ARGS__); \
	} \
} \

#endif /* _DEBUG_H_ */
