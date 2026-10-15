/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "ScriptingUtils.h"

#include <ctype.h>
#include <string.h>
#include <strings.h>


bool
ParseIsoDate(const char* value, int& year, int& month, int& day)
{
	if (value == NULL || strlen(value) != 10 || value[4] != '-'
		|| value[7] != '-') {
		return false;
	}

	for (int index = 0; index < 10; index++) {
		if (index != 4 && index != 7
			&& !isdigit(static_cast<unsigned char>(value[index]))) {
			return false;
		}
	}

	year = (value[0] - '0') * 1000 + (value[1] - '0') * 100
		+ (value[2] - '0') * 10 + value[3] - '0';
	month = (value[5] - '0') * 10 + value[6] - '0';
	day = (value[8] - '0') * 10 + value[9] - '0';
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
ParseCalendarView(const char* value, int& view)
{
	if (value == NULL)
		return false;

	if (strcasecmp(value, "day") == 0)
		view = 0;
	else if (strcasecmp(value, "week") == 0)
		view = 1;
	else if (strcasecmp(value, "month") == 0)
		view = 2;
	else
		return false;

	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


const char*
CalendarViewName(int view)
{
	switch (view) {
		case 0:
			return "day";
		case 1:
			return "week";
		case 2:
			return "month";
		default:
			return NULL;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//
