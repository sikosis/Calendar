/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "TaskSchedule.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>

#include <vector>


static bool
SplitSchedule(const std::string& schedule, std::vector<std::string>& fields)
{
	fields.clear();
	size_t position = 0;
	while (position < schedule.length()) {
		while (position < schedule.length()
			&& (schedule[position] == ' ' || schedule[position] == '\t')) {
			position++;
		}
		if (position >= schedule.length())
			break;

		size_t start = position;
		while (position < schedule.length() && schedule[position] != ' '
			&& schedule[position] != '\t') {
			position++;
		}
		fields.push_back(schedule.substr(start, position - start));
	}
	return fields.size() == 5;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
TaskSchedule::IsValid(const std::string& schedule)
{
	if (schedule == "@reboot" || schedule == "@hourly"
		|| schedule == "@daily" || schedule == "@midnight"
		|| schedule == "@weekly" || schedule == "@monthly"
		|| schedule == "@yearly" || schedule == "@annually") {
		return true;
	}

	std::vector<std::string> fields;
	return SplitSchedule(schedule, fields)
		&& _FieldIsValid(fields[0], 0, 59)
		&& _FieldIsValid(fields[1], 0, 23)
		&& _FieldIsValid(fields[2], 1, 31)
		&& _FieldIsValid(fields[3], 1, 12)
		&& _FieldIsValid(fields[4], 0, 7);
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
TaskSchedule::IsStartup(const std::string& schedule)
{
	return schedule == "@reboot";
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
TaskSchedule::Matches(const std::string& schedule, int minute, int hour,
	int day, int month, int dayOfWeek)
{
	if (schedule == "@hourly")
		return minute == 0;
	if (schedule == "@daily" || schedule == "@midnight")
		return minute == 0 && hour == 0;
	if (schedule == "@weekly")
		return minute == 0 && hour == 0 && dayOfWeek == 0;
	if (schedule == "@monthly")
		return minute == 0 && hour == 0 && day == 1;
	if (schedule == "@yearly" || schedule == "@annually")
		return minute == 0 && hour == 0 && day == 1 && month == 1;
	if (schedule == "@reboot")
		return false;

	std::vector<std::string> fields;
	if (!SplitSchedule(schedule, fields))
		return false;

	bool dayMatches = _FieldMatches(fields[2], day, 1, 31);
	bool weekDayMatches = _FieldMatches(fields[4], dayOfWeek, 0, 7);
	if (dayOfWeek == 0)
		weekDayMatches = weekDayMatches || _FieldMatches(fields[4], 7, 0, 7);

	bool calendarDayMatches;
	if (fields[2] == "*" || fields[4] == "*")
		calendarDayMatches = dayMatches && weekDayMatches;
	else
		calendarDayMatches = dayMatches || weekDayMatches;

	return _FieldMatches(fields[0], minute, 0, 59)
		&& _FieldMatches(fields[1], hour, 0, 23)
		&& calendarDayMatches
		&& _FieldMatches(fields[3], month, 1, 12);
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
TaskSchedule::_FieldIsValid(const std::string& field, int minimum, int maximum)
{
	for (int value = minimum; value <= maximum; value++) {
		if (_FieldMatches(field, value, minimum, maximum))
			return true;
	}
	return false;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
TaskSchedule::_FieldMatches(const std::string& field, int value, int minimum,
	int maximum)
{
	if (field.empty())
		return false;
	if (field[field.length() - 1] == ',')
		return false;

	bool matched = false;
	size_t partStart = 0;
	while (partStart < field.length()) {
		size_t partEnd = field.find(',', partStart);
		if (partEnd == std::string::npos)
			partEnd = field.length();
		std::string part = field.substr(partStart, partEnd - partStart);
		if (part.empty())
			return false;

		int step = 1;
		size_t slash = part.find('/');
		if (slash != std::string::npos) {
			if (part.find('/', slash + 1) != std::string::npos
				|| !_ParseNumber(part.substr(slash + 1), step) || step <= 0) {
				return false;
			}
			part.erase(slash);
		}

		int start = minimum;
		int end = maximum;
		if (part != "*") {
			size_t dash = part.find('-');
			if (dash == std::string::npos) {
				if (!_ParseNumber(part, start))
					return false;
				end = start;
			} else {
				if (part.find('-', dash + 1) != std::string::npos
					|| !_ParseNumber(part.substr(0, dash), start)
					|| !_ParseNumber(part.substr(dash + 1), end)) {
					return false;
				}
			}
		}

		if (start < minimum || end > maximum || start > end)
			return false;
		if (value >= start && value <= end && (value - start) % step == 0)
			matched = true;

		partStart = partEnd + 1;
	}
	return matched;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
TaskSchedule::_ParseNumber(const std::string& value, int& number)
{
	if (value.empty())
		return false;

	errno = 0;
	char* end;
	long parsed = strtol(value.c_str(), &end, 10);
	if (*end != '\0' || errno == ERANGE || parsed < INT_MIN || parsed > INT_MAX)
		return false;
	number = (int)parsed;
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//
