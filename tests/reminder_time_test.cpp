/*
 * Copyright 2026 Calendar authors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include <assert.h>
#include <limits>

#include "ReminderTime.h"


int
main()
{
	time_t reminderTime = 0;

	assert(CalculateReminderTime("2", 0, 10000, reminderTime));
	assert(reminderTime == 2800);
	assert(CalculateReminderTime("15", 1, 1000, reminderTime));
	assert(reminderTime == 100);
	assert(CalculateReminderTime("30", 2, 1000, reminderTime));
	assert(reminderTime == 970);
	assert(CalculateReminderTime("0", 2, 1000, reminderTime));
	assert(reminderTime == 1000);

	assert(!CalculateReminderTime("", 0, 1000, reminderTime));
	assert(!CalculateReminderTime("-1", 0, 1000, reminderTime));
	assert(!CalculateReminderTime("1 hour", 0, 1000, reminderTime));
	assert(!CalculateReminderTime("1", 3, 1000, reminderTime));
	assert(!CalculateReminderTime("18446744073709551616", 2, 1000,
		reminderTime));
	assert(!CalculateReminderTime("18446744073709551615", 0, 1000,
		reminderTime));
	assert(!CalculateReminderTime("1", 2,
		std::numeric_limits<time_t>::min(), reminderTime));

	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
