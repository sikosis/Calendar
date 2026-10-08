/*
 * Copyright 2026 Calendar authors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "TimeInterval.h"


bool
TimeIntervalsOverlap(time_t firstStart, time_t firstEnd, time_t secondStart,
	time_t secondEnd)
{
	return firstEnd >= secondStart && firstStart <= secondEnd;
}
//---------------------------------------------------------------------------------------------------------------------------------//
