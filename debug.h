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

extern int mozart_debug_level;

#define d_printf(level, fmt, ...) \
	do { \
		if (level <= mozart_debug_level) \
			printf(fmt, ##__VA_ARGS__); \
	} while (0)

#endif /* _DEBUG_H_ */
