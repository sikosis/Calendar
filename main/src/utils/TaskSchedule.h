/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef TASK_SCHEDULE_H
#define TASK_SCHEDULE_H


#include <string>


class TaskSchedule
{
public:
	static bool			IsValid(const std::string& schedule);
	static bool			IsStartup(const std::string& schedule);
	static bool			Matches(const std::string& schedule, int minute,
							int hour, int day, int month, int dayOfWeek);

private:
	static bool			_FieldIsValid(const std::string& field, int minimum,
							int maximum);
	static bool			_FieldMatches(const std::string& field, int value,
							int minimum, int maximum);
	static bool			_ParseNumber(const std::string& value, int& number);
};


#endif
