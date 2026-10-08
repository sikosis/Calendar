/*
 * Copyright 2026 Calendar authors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "ReminderTime.h"

#include <errno.h>
#include <limits.h>
#include <limits>
#include <stdlib.h>


bool
CalculateReminderTime(const char* text, int unitIndex, time_t start,
	time_t& reminderTime)
{
	if (text == NULL || text[0] == '\0' || unitIndex < 0 || unitIndex > 2)
		return false;

	for (const char* character = text; character[0] != '\0'; character++) {
		if (character[0] < '0' || character[0] > '9')
			return false;
	}

	errno = 0;
	char* end;
	unsigned long long amount = strtoull(text, &end, 10);
	if (errno == ERANGE || end == text || end[0] != '\0')
		return false;

	unsigned long long multiplier = 1;
	if (unitIndex == 0)
		multiplier = 3600;
	else if (unitIndex == 1)
		multiplier = 60;

	if (amount > ULLONG_MAX / multiplier)
		return false;

	unsigned long long delay = amount * multiplier;
	unsigned long long maximumDelay;
	if (std::numeric_limits<time_t>::is_signed) {
		const time_t minimum = std::numeric_limits<time_t>::min();
		if (start >= 0) {
			maximumDelay = (unsigned long long)start
				+ (unsigned long long)(-(minimum + 1)) + 1;
		} else
			maximumDelay = (unsigned long long)(start - minimum);
	} else
		maximumDelay = (unsigned long long)start;

	if (delay > maximumDelay)
		return false;

	if (!std::numeric_limits<time_t>::is_signed || start < 0
		|| delay <= (unsigned long long)start) {
		reminderTime = start - (time_t)delay;
	} else {
		unsigned long long magnitude = delay - (unsigned long long)start;
		reminderTime = -(time_t)(magnitude - 1) - 1;
	}

	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//
