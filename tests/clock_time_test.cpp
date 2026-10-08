/*
 * Copyright 2026 Calendar authors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include <assert.h>
#include <stddef.h>

#include "ClockTime.h"


int
main()
{
	int hour = -1;
	int minute = -1;

	assert(ParseClockTime("00:00", hour, minute));
	assert(hour == 0 && minute == 0);
	assert(ParseClockTime("9:05", hour, minute));
	assert(hour == 9 && minute == 5);
	assert(ParseClockTime("23:59", hour, minute));
	assert(hour == 23 && minute == 59);

	assert(!ParseClockTime(NULL, hour, minute));
	assert(!ParseClockTime("", hour, minute));
	assert(!ParseClockTime("9:5", hour, minute));
	assert(!ParseClockTime("09.05", hour, minute));
	assert(!ParseClockTime("24:00", hour, minute));
	assert(!ParseClockTime("12:60", hour, minute));
	assert(!ParseClockTime("12:00 PM", hour, minute));

	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
