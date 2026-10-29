/*
 * Copyright 2026 Calendar authors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "YearProgress.h"


float
CalculateYearProgress(int dayOfYear, int daysInYear)
{
	if (daysInYear <= 0 || dayOfYear <= 0)
		return 0.0f;
	if (dayOfYear >= daysInYear)
		return 100.0f;

	return 100.0f * dayOfYear / daysInYear;
}
//---------------------------------------------------------------------------------------------------------------------------------//
