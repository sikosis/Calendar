/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef SCRIPTING_UTILS_H
#define SCRIPTING_UTILS_H


bool ParseIsoDate(const char* value, int& year, int& month, int& day);
bool ParseCalendarView(const char* value, int& view);
const char* CalendarViewName(int view);


#endif
