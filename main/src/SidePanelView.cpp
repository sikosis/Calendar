/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2022, Harshit Sharma, harshits908@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "SidePanelView.h"

#include <Button.h>
#include <Catalog.h>
#include <DateFormat.h>
#include <Font.h>
#include <LayoutBuilder.h>
#include <LocaleRoster.h>
#include <Message.h>
#include <StatusBar.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>

#include "CalendarView.h"
#include "EventTabView.h"
#include "LifeProgress.h"
#include "MainView.h"
#include "MainWindow.h"
#include "PreferenceWindow.h"
#include "YearProgress.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SidePanelView"

enum StartOfWeek {
	kLocaleStartOfWeek,
	kWeekDayMonday,
	kWeekDayTuesday,
	kWeekDayWednesday,
	kWeekDayThursday,
	kWeekDayFriday,
	kWeekDaySaturday,
	kWeekDaySunday,
};


SidePanelView::SidePanelView()
	:
	BView("SidePanelView", B_WILL_DRAW)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	fCalendarView = new CalendarView("calendar");
	fCalendarView->SetWeekNumberHeaderVisible(false);
	// fCalendarView->SetInvocationMessage(new BMessage(kInvokationMessage));
	fCalendarView->SetSelectionMessage(new BMessage(kSelectionMessage));

	fDateHeaderButton = new DateHeaderButton();

	fYearLabel = new BStringView("year", "");
	fMonthLabel = new BStringView("month", "");

	fMonthUpButton
		= new BButton("MonthuUp", "►", new BMessage(kMonthUpMessage));
	fMonthDownButton
		= new BButton("MonthDown", "◄", new BMessage(kMonthDownMessage));

	fMonthUpButton->SetFlat(true);
	fMonthDownButton->SetFlat(true);
	fYearProgressBar = new BStatusBar("YearProgress");
	fYearProgressBar->SetMaxValue(100.0f);
	fYearProgressBar->SetBarHeight(fYearProgressBar->BarHeight() / 2.0f);
	fYearProgressBar->SetToolTip(B_TRANSLATE("Year progress"));
	fLifeProgressBar = new BStatusBar("LifeProgress");
	fLifeProgressBar->SetMaxValue(100.0f);
	fLifeProgressBar->SetBarHeight(fLifeProgressBar->BarHeight() / 2.0f);
	fLifeProgressBar->SetToolTip(B_TRANSLATE("Life progress"));
	fLifeBirthYear = 0;
	fLifeExpectancy = 90;
	fYearProgressVisible = true;
	fLifeProgressVisible = true;

	BFont font;
	fMonthLabel->GetFont(&font);
	font.SetSize(font.Size() * 1.1);
	fMonthLabel->SetFont(&font, B_FONT_ALL);

	float width, height;
	fMonthLabel->GetPreferredSize(&width, &height);
	fMonthLabel->SetExplicitMinSize(
		BSize(font.StringWidth(B_TRANSLATE_COMMENT(
				"September XXXX", "Choose the longest month name")),
			height));

	fMonthUpButton->SetExplicitMinSize(BSize(height + 5, height + 5));
	fMonthDownButton->SetExplicitMinSize(BSize(height + 5, height + 5));

	fTextFilter = new BTextControl("FilterString", B_TRANSLATE("Filter:"),
		NULL, new BMessage(kFilterChanged));
	fTextFilter->SetModificationMessage(new BMessage(kFilterChanged));

	fFilterClearButton =
		new BButton("ClearFilter", B_TRANSLATE("Clear"), new BMessage(kFilterCleared));

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(fDateHeaderButton)
		.AddStrut(30)
		.AddGroup(B_HORIZONTAL)
			.Add(fMonthLabel)
			.AddGlue()
			.Add(fMonthDownButton)
			.Add(fMonthUpButton)
			.End()
		.AddStrut(5)
		.Add(fCalendarView)
		.AddStrut(5)
		.AddGroup(B_VERTICAL, 5)
			.Add(fYearProgressBar)
			.Add(fLifeProgressBar)
			.End()
		.AddStrut(80)
		.AddGlue(10)
		.Add(fTextFilter)
		.AddStrut(10)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(fFilterClearButton)
			.End()
		.End();

	_UpdateDate(BDate::CurrentDate(B_LOCAL_TIME));
	_UpdateYearProgress();
	_UpdateLifeProgress();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SidePanelView::MessageReceived(BMessage* message)
{
	int32 change;
	switch (message->what) {
		case B_OBSERVER_NOTICE_CHANGE:
		{
			message->FindInt32(B_OBSERVE_WHAT_CHANGE, &change);
			switch (change) {
				case kSystemDateChangeMessage:
					fDateHeaderButton->MessageReceived(message);
					_UpdateYearProgress();
					_UpdateLifeProgress();
					break;

				default:
					BView::MessageReceived(message);
					break;
			}
			break;
		}
		case kSelectionMessage:
		{
			int32 day, month, year;
			message->FindInt32("day", &day);
			message->FindInt32("month", &month);
			message->FindInt32("year", &year);

			_UpdateDate(BDate(year, month, day));
			break;
		}
		case kMonthDownMessage:
		case kMonthUpMessage:
		{
			BDate date = fCalendarView->Date();
			date.AddMonths(kMonthDownMessage == message->what ? -1 : 1);
			_UpdateDate(date);
			break;
		}
		case B_LOCALE_CHANGED:
		{
			BLocaleRoster::Default()->Refresh();
			fDateHeaderButton->MessageReceived(message);
			_UpdateDateLabel();
			break;
		}
		case kListModeChanged:
		{
			bool hidden = false;
			if (message->FindBool("hidden", &hidden) == B_OK)
				fCalendarView->SetMarkHidden(hidden);
			break;
		}
		case kSetCalendarToCurrentDate:
			_UpdateDate(BDate::CurrentDate(B_LOCAL_TIME));
			break;
		default:
			BView::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SidePanelView::SetStartOfWeek(int32 index)
{
	StartOfWeek startOfWeekDay = static_cast<StartOfWeek>(index);
		// Preference menu index to start of week map
	BWeekday firstDay;

	if (startOfWeekDay == kLocaleStartOfWeek) {
		BDateFormat().GetStartOfWeek(&firstDay);
		fCalendarView->SetStartOfWeek(firstDay);
	} else {
		firstDay = static_cast<BWeekday>(index);
		fCalendarView->SetStartOfWeek(firstDay);
	}
}


void
SidePanelView::ShowWeekHeader(bool state)
{
	fCalendarView->SetWeekNumberHeaderVisible(state);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SidePanelView::ShowYearProgress(bool state)
{
	if (state == fYearProgressVisible)
		return;

	fYearProgressVisible = state;
	if (state)
		fYearProgressBar->Show();
	else
		fYearProgressBar->Hide();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SidePanelView::SetYearProgressColour(rgb_color colour)
{
	colour.alpha = 255;
	fYearProgressBar->SetBarColor(colour);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SidePanelView::ShowLifeProgress(bool state)
{
	if (state == fLifeProgressVisible)
		return;

	fLifeProgressVisible = state;
	if (state)
		fLifeProgressBar->Show();
	else
		fLifeProgressBar->Hide();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SidePanelView::SetLifeProgressDetails(int32 birthYear, int32 lifeExpectancy)
{
	fLifeBirthYear = birthYear;
	fLifeExpectancy = lifeExpectancy;
	_UpdateLifeProgress();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SidePanelView::SetLifeProgressColour(rgb_color colour)
{
	colour.alpha = 255;
	fLifeProgressBar->SetBarColor(colour);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SidePanelView::_UpdateDate(const BDate& date)
{
	if (!date.IsValid())
		return;

	fCalendarView->SetDate(date);
	fDateHeaderButton->SetDate(date);
	_UpdateDateLabel();

	Window()->PostMessage(kSelectedDateChanged);
}
//---------------------------------------------------------------------------------------------------------------------------------//


BDate
SidePanelView::GetSelectedDate() const
{
	return fCalendarView->Date();
}

const char*
SidePanelView::GetFilterQuery() const
{
		return fTextFilter->Text();
}

void
SidePanelView::ClearFilterText()
{
	fTextFilter->SetText("");
}


void
SidePanelView::_UpdateDateLabel()
{
	BDate date = fCalendarView->Date();

	BString yearString;
	BString monthYearString;
	yearString << date.Year();

	BDateFormat().GetMonthName(date.Month(), monthYearString);
	monthYearString += " ";
	monthYearString += yearString.String();

	fMonthLabel->SetText(monthYearString);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SidePanelView::_UpdateYearProgress()
{
	BDate today = BDate::CurrentDate(B_LOCAL_TIME);
	if (!today.IsValid())
		return;

	float percentage
		= CalculateYearProgress(today.DayOfYear(), today.DaysInYear());
	int32 percentageValue = (int32)percentage;

	BString yearText;
	yearText << today.Year();
	BString percentageText;
	percentageText << percentageValue << "%";
	fYearProgressBar->SetTo(
		percentage, yearText.String(), percentageText.String());
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
SidePanelView::_UpdateLifeProgress()
{
	BDate today = BDate::CurrentDate(B_LOCAL_TIME);
	if (!today.IsValid())
		return;

	float percentage = CalculateLifeProgress(today.Year(), today.DayOfYear(),
		today.DaysInYear(), fLifeBirthYear, fLifeExpectancy);
	int32 percentageValue = (int32)percentage;
	BString percentageText;
	percentageText << percentageValue << "%";
	fLifeProgressBar->SetTo(
		percentage, B_TRANSLATE("LIFE"), percentageText.String());
}
//---------------------------------------------------------------------------------------------------------------------------------//
