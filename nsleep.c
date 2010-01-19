/*
 * nsleep.c - A nanosceond wrapper 
 *
 * Copyright (C) 2009-2010	Andrew Clayton <andrew@digital-domain.net>
 * Released under the GNU Lesser General Public License (LGPL) version 3. 
 * See COPYING
 */

#include <time.h>

#include "nsleep.h"

extern int nsleep(unsigned long long nsecs)
{
	struct timespec req;
	
	if (nsecs < A_SEC) {
		req.tv_sec = 0;
		req.tv_nsec = nsecs;
	} else if (nsecs >= A_SEC) {
		req.tv_sec = nsecs / A_SEC;
		req.tv_nsec = nsecs % A_SEC;
	}

	nanosleep(&req, &req);

	return 0;
}
