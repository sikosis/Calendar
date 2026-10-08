/*
 * Copyright 2026 Calendar authors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "ClockTime.h"

#include <string.h>


bool
ParseClockTime(const char* text, int& hour, int& minute)
{
	if (text == NULL)
		return false;

	size_t length = strlen(text);
	if (length != 4 && length != 5)
		return false;

	size_t colon = length - 3;
	if (text[colon] != ':' || text[length - 2] < '0'
		|| text[length - 2] > '9' || text[length - 1] < '0'
		|| text[length - 1] > '9') {
		return false;
	}

	if (text[0] < '0' || text[0] > '9')
		return false;

	hour = text[0] - '0';
	if (colon == 2) {
		if (text[1] < '0' || text[1] > '9')
			return false;
		hour = hour * 10 + text[1] - '0';
	}
	minute = (text[length - 2] - '0') * 10 + text[length - 1] - '0';

	return hour <= 23 && minute <= 59;
}
//---------------------------------------------------------------------------------------------------------------------------------//
