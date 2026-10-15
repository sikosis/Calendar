#include "TaskSchedule.h"

#include <assert.h>


int
main()
{
	assert(TaskSchedule::IsValid("*/15 9-17 * * 1-5"));
	assert(TaskSchedule::IsValid("0,30 * * * *"));
	assert(TaskSchedule::IsValid("@daily"));
	assert(TaskSchedule::IsValid("@reboot"));
	assert(!TaskSchedule::IsValid("60 * * * *"));
	assert(!TaskSchedule::IsValid("* 24 * * *"));
	assert(!TaskSchedule::IsValid("* * 0 * *"));
	assert(!TaskSchedule::IsValid("* * * * monday"));
	assert(!TaskSchedule::IsValid("1, * * * *"));
	assert(!TaskSchedule::IsValid("*/0 * * * *"));
	assert(!TaskSchedule::IsValid("@sometimes"));

	assert(TaskSchedule::Matches("*/15 9-17 * * 1-5", 30, 10, 15, 10, 4));
	assert(!TaskSchedule::Matches("*/15 9-17 * * 1-5", 31, 10, 15, 10, 4));
	assert(!TaskSchedule::Matches("*/15 9-17 * * 1-5", 30, 18, 15, 10, 4));
	assert(!TaskSchedule::Matches("*/15 9-17 * * 1-5", 30, 10, 15, 10, 0));
	assert(TaskSchedule::Matches("0 0 15 * 1", 0, 0, 15, 10, 4));
	assert(TaskSchedule::Matches("0 0 15 * 1", 0, 0, 16, 10, 1));
	assert(TaskSchedule::Matches("0 0 * * 0", 0, 0, 18, 10, 0));
	assert(TaskSchedule::Matches("0 0 * * 7", 0, 0, 18, 10, 0));
	assert(TaskSchedule::Matches("@daily", 0, 0, 15, 10, 4));
	assert(!TaskSchedule::Matches("@daily", 1, 0, 15, 10, 4));
	assert(TaskSchedule::IsStartup("@reboot"));
	assert(!TaskSchedule::Matches("@reboot", 0, 0, 15, 10, 4));
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
