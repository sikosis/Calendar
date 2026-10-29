/*
 * Copyright 2026 Calendar authors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include <assert.h>

#include "YearProgress.h"


int
main()
{
	assert(CalculateYearProgress(0, 365) == 0.0f);
	assert(CalculateYearProgress(1, 0) == 0.0f);
	assert((int)CalculateYearProgress(1, 365) == 0);
	assert((int)CalculateYearProgress(182, 365) == 49);
	assert(CalculateYearProgress(365, 365) == 100.0f);
	assert(CalculateYearProgress(366, 366) == 100.0f);
	assert(CalculateYearProgress(400, 365) == 100.0f);

	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
