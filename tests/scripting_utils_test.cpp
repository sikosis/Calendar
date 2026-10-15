#include "ScriptingUtils.h"

#include <assert.h>
#include <string.h>


int
main()
{
	int year = 0;
	int month = 0;
	int day = 0;
	assert(ParseIsoDate("2026-10-15", year, month, day));
	assert(year == 2026 && month == 10 && day == 15);
	assert(!ParseIsoDate("15/10/2026", year, month, day));
	assert(!ParseIsoDate("2026-1-15", year, month, day));
	assert(!ParseIsoDate("calendar", year, month, day));

	int view = -1;
	assert(ParseCalendarView("day", view) && view == 0);
	assert(ParseCalendarView("WEEK", view) && view == 1);
	assert(ParseCalendarView("Month", view) && view == 2);
	assert(!ParseCalendarView("year", view));
	assert(strcmp(CalendarViewName(0), "day") == 0);
	assert(strcmp(CalendarViewName(1), "week") == 0);
	assert(strcmp(CalendarViewName(2), "month") == 0);
	assert(CalendarViewName(3) == NULL);
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
