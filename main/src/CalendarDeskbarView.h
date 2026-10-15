/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef CALENDAR_DESKBAR_VIEW_H
#define CALENDAR_DESKBAR_VIEW_H


#include <DateTime.h>
#include <View.h>


extern const char* const kCalendarDeskbarItemName;


class CalendarDeskbarView : public BView
{
public:
					CalendarDeskbarView(BRect frame);
					CalendarDeskbarView(BMessage* archive);

	static BArchivable* Instantiate(BMessage* archive);
	virtual status_t	Archive(BMessage* archive, bool deep = true) const;

	virtual void		AttachedToWindow();
	virtual void		Draw(BRect updateRect);
	virtual void		MouseDown(BPoint where);
	virtual void		Pulse();

private:
	void			_DrawCalendarPage();
	void			_OpenCalendar();
	void			_QuitCalendar();
	void			_ShowContextMenu(BPoint where);

	BDate			fDate;
};


#endif
