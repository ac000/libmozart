/*
 * nsleep.h - nsleep API header 
 *
 * Copyright (C) 2009   Andrew Clayton <andrew@digital-domain.net>
 * Released under the GNU Lesser General Public License (LGPL) version 3. 
 * See COPYING
 */

#ifndef _NSLEEP_H_
#define _NSLEEP_H_

/*
 * Define a microsecond, milliseond and a second in terms of nanoseconds.
 */
#define U_SEC   1000
#define M_SEC   1000000
#define A_SEC   1000000000

extern int nsleep(unsigned long long nsecs);

#endif /* _NSLEEP_H_ */
