/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2021 Jaidyn Levesque, jadedctrl@teknik.io
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _DATEHEADERBUTTON_H_
#define _DATEHEADERBUTTON_H_

#include <Control.h>
#include <DateTime.h>


class DateHeaderButton : public BControl
{
public:
					DateHeaderButton();
	virtual void	Draw(BRect updateRect);
	virtual void	MessageReceived(BMessage* message);
	virtual void	MouseDown(BPoint where);
	virtual void	MouseUp(BPoint where);
	virtual void	MouseMoved(BPoint where, uint32 code, const BMessage* drag);

	void			SetDate(const BDate& date);

private:
	void			_UpdateDateHeader();

	BString			fDayText;
	BString			fDayOfWeekText;
	BString			fMonthYearText;
	BDate			fDate;

	bool			fMouseInView;
	bool			fMouseDown;
};

#endif
