/*
 * utils.c - Misc "utility" functions
 *
 * Copyright (C) 2011	OpenTech Labs
 * Copyright (C) 2011	Andrew Clayton <andrew@opentechlabs.co.uk>
 * Released under the GNU Lesser General Public License (LGPL) version 3.
 * See COPYING
 */

#include <time.h>

#include <glib.h>

#include "utils.h"

/**
 * mozart_nsleep - A wrapper around nanosleep()
 * @period: The number of nanoseconds to sleep for
 */
void mozart_nsleep(gint64 period)
{
	struct timespec req;
	struct timespec rem;
	int ret;

	req.tv_sec = 0;
	req.tv_nsec = period;

sleep:
	ret = nanosleep(&req, &rem);
	if (ret) {
		req.tv_sec = rem.tv_sec;
		req.tv_nsec = rem.tv_nsec;
		goto sleep;
	}
}
