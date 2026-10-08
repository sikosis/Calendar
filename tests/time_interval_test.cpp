/*
 * Copyright 2026 Calendar authors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include <assert.h>

#include "TimeInterval.h"


int
main()
{
	const time_t todayStart = 1000;
	const time_t todayEnd = 1999;

	assert(TimeIntervalsOverlap(1100, 1200, todayStart, todayEnd));
	assert(TimeIntervalsOverlap(900, 1100, todayStart, todayEnd));
	assert(TimeIntervalsOverlap(1900, 2100, todayStart, todayEnd));
	assert(TimeIntervalsOverlap(900, 2100, todayStart, todayEnd));
	assert(TimeIntervalsOverlap(todayEnd, todayEnd, todayStart, todayEnd));

	assert(!TimeIntervalsOverlap(0, 999, todayStart, todayEnd));
	assert(!TimeIntervalsOverlap(2000, 2100, todayStart, todayEnd));

	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
