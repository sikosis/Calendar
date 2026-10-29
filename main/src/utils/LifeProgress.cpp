/*
 * Copyright 2026 Calendar authors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "LifeProgress.h"


float
CalculateLifeProgress(int currentYear, int dayOfYear, int daysInYear,
	int birthYear, int lifeExpectancy)
{
	if (birthYear <= 0 || lifeExpectancy <= 0 || daysInYear <= 0
		|| dayOfYear <= 0) {
		return 0.0f;
	}

	float age = currentYear - birthYear
		+ (dayOfYear - 1) / (float)daysInYear;
	if (age <= 0.0f)
		return 0.0f;
	if (age >= lifeExpectancy)
		return 100.0f;

	return 100.0f * age / lifeExpectancy;
}
//---------------------------------------------------------------------------------------------------------------------------------//
