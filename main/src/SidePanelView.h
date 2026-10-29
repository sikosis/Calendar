/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2022, Harshit Sharma, harshits908@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef SIDE_PANEL_VIEW_H
#define SIDE_PANEL_VIEW_H

#include <DateTime.h>
#include <InterfaceDefs.h>
#include <View.h>

#include "CalendarView.h"
#include "DateHeaderButton.h"

class BButton;
class BStatusBar;
class BStringView;
class BTextControl;

enum {
	kSelectionMessage,
	kMonthDownMessage,
	kMonthUpMessage,
	kShowWeekNumberMessage,
	kSetStartOfWeekMessage,
	kSetCalendarToCurrentDate,
	kSelectedDateChanged,
	kFilterCleared,
	kFilterChanged,
};


class SidePanelView : public BView
{
public:
					SidePanelView();

	void			MessageReceived(BMessage* message);
	BDate			GetSelectedDate() const;
	const char*		GetFilterQuery() const;
	void			SetStartOfWeek(int32);
	void			ShowWeekHeader(bool);
	void			ShowYearProgress(bool state);
	void			SetYearProgressColour(rgb_color colour);
	void			ShowLifeProgress(bool state);
	void			SetLifeProgressDetails(int32 birthYear, int32 lifeExpectancy);
	void			SetLifeProgressColour(rgb_color colour);
	void			ClearFilterText();

private:
	void			_UpdateDate(const BDate&);
	void			_UpdateDateLabel();
	void			_UpdateYearProgress();
	void			_UpdateLifeProgress();

	BStringView*	fYearLabel;
	BStringView*	fMonthLabel;
	BStatusBar*	fYearProgressBar;
	BStatusBar*	fLifeProgressBar;
	CalendarView*	fCalendarView;
	DateHeaderButton* fDateHeaderButton;
	BButton*		fMonthUpButton;
	BButton*		fMonthDownButton;
	BButton*		fFilterClearButton;
	BTextControl*	fTextFilter;
	int32			fLifeBirthYear;
	int32			fLifeExpectancy;
};

#endif
