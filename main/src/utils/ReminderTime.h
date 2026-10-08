/*
 * Copyright 2026 Calendar authors
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef REMINDER_TIME_H
#define REMINDER_TIME_H

#include <time.h>


bool CalculateReminderTime(const char* text, int unitIndex, time_t start,
	time_t& reminderTime);


#endif
