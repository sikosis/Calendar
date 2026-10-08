/*
 * Copyright 2026 Calendar authors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#ifndef TIME_INTERVAL_H
#define TIME_INTERVAL_H

#include <time.h>


bool TimeIntervalsOverlap(time_t firstStart, time_t firstEnd,
	time_t secondStart, time_t secondEnd);


#endif // TIME_INTERVAL_H
