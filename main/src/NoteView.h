/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef NOTE_VIEW_H
#define NOTE_VIEW_H


#include <String.h>
#include <View.h>

#include "NoteModel.h"


class BTextView;


class NoteView : public BView
{
public:
					NoteView(BRect frame, const CalendarNote& note);
					NoteView(BMessage* archive);

	static BArchivable* Instantiate(BMessage* archive);
	virtual status_t Archive(BMessage* archive, bool deep = true) const;

	virtual void	AttachedToWindow();
	virtual void	Draw(BRect updateRect);

private:
	void			_AddChildren();
	void			_ApplyColours();

	BString			fTitle;
	BString			fText;
	rgb_color		fColour;
	BTextView*		fTextView;
};


#endif
