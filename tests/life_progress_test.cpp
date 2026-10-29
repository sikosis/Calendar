/*
 * Copyright 2026 Calendar authors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include <assert.h>

#include "LifeProgress.h"


int
main()
{
	assert(CalculateLifeProgress(2026, 1, 365, 0, 90) == 0.0f);
	assert(CalculateLifeProgress(2026, 1, 365, 1970, 0) == 0.0f);
	assert(CalculateLifeProgress(1969, 365, 365, 1970, 90) == 0.0f);
	assert((int)CalculateLifeProgress(2026, 1, 365, 1970, 90) == 62);
	assert((int)CalculateLifeProgress(2026, 183, 365, 1970, 90) == 62);
	assert(CalculateLifeProgress(2060, 1, 366, 1970, 90) == 100.0f);
	assert(CalculateLifeProgress(2070, 1, 365, 1970, 90) == 100.0f);

	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
