/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "CalendarDeskbarView.h"

#include <time.h>

#include <Application.h>
#include <Archivable.h>
#include <Catalog.h>
#include <Deskbar.h>
#include <Font.h>
#include <MenuItem.h>
#include <Message.h>
#include <Messenger.h>
#include <PopUpMenu.h>
#include <Roster.h>
#include <String.h>
#include <Window.h>

#include "App.h"
#include "TaskLauncher.h"
#include "TaskModel.h"
#include "TaskSchedule.h"
#include "TaskStore.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "CalendarDeskbarView"


const char* const kCalendarDeskbarItemName = "CalendarDeskbar";


CalendarDeskbarView::CalendarDeskbarView(BRect frame)
	:
	BView(frame, kCalendarDeskbarItemName, B_FOLLOW_LEFT | B_FOLLOW_TOP,
		B_WILL_DRAW | B_PULSE_NEEDED),
	fDate(BDate::CurrentDate(B_LOCAL_TIME)),
	fLastTaskMinute(-1),
	fRanStartupTasks(false)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetToolTip(B_TRANSLATE("Open Calendar"));
}
//---------------------------------------------------------------------------------------------------------------------------------//


CalendarDeskbarView::CalendarDeskbarView(BMessage* archive)
	:
	BView(archive),
	fDate(BDate::CurrentDate(B_LOCAL_TIME)),
	fLastTaskMinute(-1),
	fRanStartupTasks(false)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	SetToolTip(B_TRANSLATE("Open Calendar"));
}
//---------------------------------------------------------------------------------------------------------------------------------//


BArchivable*
CalendarDeskbarView::Instantiate(BMessage* archive)
{
	if (!validate_instantiation(archive, "CalendarDeskbarView"))
		return NULL;

	return new CalendarDeskbarView(archive);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
CalendarDeskbarView::Archive(BMessage* archive, bool deep) const
{
	status_t status = BView::Archive(archive, deep);
	if (status != B_OK)
		return status;

	status = archive->AddString("add_on", kSignature);
	if (status != B_OK)
		return status;

	return archive->AddString("class", "CalendarDeskbarView");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
CalendarDeskbarView::AttachedToWindow()
{
	BView::AttachedToWindow();
	if (Parent() != NULL) {
		if ((Parent()->Flags() & B_DRAW_ON_CHILDREN) != 0)
			SetViewColor(B_TRANSPARENT_COLOR);
		else
			AdoptParentColors();
	} else
		SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	if (!fRanStartupTasks) {
		_RunDueTasks(true);
		fRanStartupTasks = true;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
CalendarDeskbarView::Draw(BRect updateRect)
{
	if (ViewColor() != B_TRANSPARENT_COLOR) {
		SetHighColor(ViewColor());
		FillRect(updateRect);
	}
	_DrawCalendarPage();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
CalendarDeskbarView::_DrawCalendarPage()
{
	BRect page = Bounds();
	float inset = page.Width() * 0.12f;
	if (inset < 2.0f)
		inset = 2.0f;
	page.InsetBy(inset, inset);

	SetHighColor(248, 248, 248);
	FillRoundRect(page, 2.0f, 2.0f);

	SetHighColor(90, 90, 90);
	StrokeRoundRect(page, 2.0f, 2.0f);

	BRect header(page);
	header.bottom = page.top + page.Height() * 0.28f;
	SetHighColor(214, 72, 72);
	FillRect(header);

	BString day;
	day << fDate.Day();
	BFont font(be_bold_font);
	font.SetSize(page.Height() * 0.48f);
	SetFont(&font);
	SetHighColor(35, 35, 35);

	font_height height;
	font.GetHeight(&height);
	float textWidth = font.StringWidth(day.String());
	float textX = page.left + (page.Width() - textWidth) / 2.0f;
	float textY = header.bottom
		+ (page.bottom - header.bottom + height.ascent - height.descent) / 2.0f;
	DrawString(day.String(), BPoint(textX, textY));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
CalendarDeskbarView::MouseDown(BPoint where)
{
	int32 buttons = B_PRIMARY_MOUSE_BUTTON;
	if (Window() != NULL && Window()->CurrentMessage() != NULL)
		Window()->CurrentMessage()->FindInt32("buttons", &buttons);

	if ((buttons & B_SECONDARY_MOUSE_BUTTON) != 0)
		_ShowContextMenu(where);
	else if ((buttons & B_PRIMARY_MOUSE_BUTTON) != 0)
		_OpenCalendar();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
CalendarDeskbarView::_LaunchTask(const char* command)
{
	LaunchCalendarTask(command);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
CalendarDeskbarView::_OpenCalendar()
{
	team_id team = be_roster->TeamFor(kSignature);
	if (team < B_OK) {
		status_t status = be_roster->Launch(kSignature,
			static_cast<BMessage*>(NULL), &team);
		if (status != B_OK && status != B_ALREADY_RUNNING)
			return;
		if (status == B_ALREADY_RUNNING)
			team = be_roster->TeamFor(kSignature);
	}

	if (team >= B_OK)
		be_roster->ActivateApp(team);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
CalendarDeskbarView::_QuitCalendar()
{
	team_id team = be_roster->TeamFor(kSignature);
	if (team >= B_OK) {
		status_t status;
		BMessenger calendar(kSignature, team, &status);
		if (status == B_OK)
			calendar.SendMessage(B_QUIT_REQUESTED);
	}

	BDeskbar deskbar;
	deskbar.RemoveItem(kCalendarDeskbarItemName);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
CalendarDeskbarView::_RunDueTasks(bool startup)
{
	TaskModel model;
	if (LoadCalendarTasks(model) != B_OK)
		return;

	time_t now = time(NULL);
	struct tm localTime;
	if (localtime_r(&now, &localTime) == NULL)
		return;

	const std::vector<CalendarTask>& tasks = model.Tasks();
	for (size_t index = 0; index < tasks.size(); index++) {
		bool shouldRun = startup
			? TaskSchedule::IsStartup(tasks[index].schedule)
			: TaskSchedule::Matches(tasks[index].schedule,
				localTime.tm_min, localTime.tm_hour, localTime.tm_mday,
				localTime.tm_mon + 1, localTime.tm_wday);
		if (shouldRun)
			_LaunchTask(tasks[index].command.c_str());
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
CalendarDeskbarView::_ShowContextMenu(BPoint where)
{
	BPopUpMenu* menu = new BPopUpMenu("Calendar deskbar menu", false, false);
	BMenuItem* openItem = new BMenuItem(B_TRANSLATE("Open Calendar"), NULL);
	BMenuItem* quitItem = new BMenuItem(B_TRANSLATE("Quit Calendar"), NULL);

	menu->AddItem(openItem);
	menu->AddItem(quitItem);

	ConvertToScreen(&where);
	BMenuItem* selected = menu->Go(where, false, true);
	bool openCalendar = selected == openItem;
	bool quitCalendar = selected == quitItem;
	delete menu;

	if (openCalendar)
		_OpenCalendar();
	else if (quitCalendar)
		_QuitCalendar();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
CalendarDeskbarView::Pulse()
{
	time_t now = time(NULL);
	int64 currentMinute = (int64)now / 60;
	if (currentMinute != fLastTaskMinute) {
		fLastTaskMinute = currentMinute;
		_RunDueTasks(false);
	}

	BDate today = BDate::CurrentDate(B_LOCAL_TIME);
	if (today == fDate)
		return;

	fDate = today;
	Invalidate();
}
//---------------------------------------------------------------------------------------------------------------------------------//


extern "C" _EXPORT BView*
instantiate_deskbar_item(float maxWidth, float maxHeight)
{
	float side = maxHeight;
	if (maxWidth < side)
		side = maxWidth;

	return new CalendarDeskbarView(BRect(0, 0, side - 1, side - 1));
}
//---------------------------------------------------------------------------------------------------------------------------------//
