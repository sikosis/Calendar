/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "NoteView.h"

#include <Archivable.h>
#include <Dragger.h>
#include <Font.h>
#include <Message.h>
#include <TextView.h>

#include "App.h"


static const float kDraggerSize = 7.0f;
static const float kNoteInset = 10.0f;
static const float kTitleHeight = 24.0f;


static rgb_color
TextColourFor(rgb_color background)
{
	int32 brightness = background.red * 299 + background.green * 587
		+ background.blue * 114;
	if (brightness < 128000)
		return (rgb_color){255, 255, 255, 255};
	return (rgb_color){30, 30, 30, 255};
}
//---------------------------------------------------------------------------------------------------------------------------------//


NoteView::NoteView(BRect frame, const CalendarNote& note)
	:
	BView(frame, "CalendarNote", B_FOLLOW_NONE, B_WILL_DRAW),
	fTitle(note.title.c_str()),
	fText(note.text.c_str()),
	fColour((rgb_color){note.red, note.green, note.blue, 255}),
	fTextView(NULL)
{
	_AddChildren();
}
//---------------------------------------------------------------------------------------------------------------------------------//


NoteView::NoteView(BMessage* archive)
	:
	BView(archive),
	fTitle(archive->GetString("note_title", "")),
	fText(archive->GetString("note_text", "")),
	fTextView(NULL)
{
	fColour.red = (uint8)archive->GetInt32("note_red", 255);
	fColour.green = (uint8)archive->GetInt32("note_green", 244);
	fColour.blue = (uint8)archive->GetInt32("note_blue", 128);
	fColour.alpha = 255;
	fTextView = dynamic_cast<BTextView*>(FindView("NoteText"));
	if (fTextView == NULL)
		_AddChildren();
}
//---------------------------------------------------------------------------------------------------------------------------------//


BArchivable*
NoteView::Instantiate(BMessage* archive)
{
	if (!validate_instantiation(archive, "NoteView"))
		return NULL;
	return new NoteView(archive);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
NoteView::Archive(BMessage* archive, bool deep) const
{
	status_t status = BView::Archive(archive, deep);
	if (status == B_OK)
		status = archive->AddString("add_on", kSignature);
	if (status == B_OK)
		status = archive->AddString("class", "NoteView");
	if (status == B_OK)
		status = archive->AddString("note_title", fTitle);
	if (status == B_OK)
		status = archive->AddString("note_text", fText);
	if (status == B_OK)
		status = archive->AddInt32("note_red", fColour.red);
	if (status == B_OK)
		status = archive->AddInt32("note_green", fColour.green);
	if (status == B_OK)
		status = archive->AddInt32("note_blue", fColour.blue);
	return status;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NoteView::AttachedToWindow()
{
	BView::AttachedToWindow();
	_ApplyColours();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NoteView::Draw(BRect updateRect)
{
	SetHighColor(fColour);
	FillRect(updateRect);

	BFont titleFont(be_bold_font);
	SetFont(&titleFont);
	SetHighColor(TextColourFor(fColour));
	BString title(fTitle);
	if (title.IsEmpty())
		title = "Note";
	titleFont.TruncateString(&title, B_TRUNCATE_END,
		Bounds().Width() - kNoteInset * 2.0f);
	DrawString(title.String(), BPoint(kNoteInset, kNoteInset + 12.0f));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NoteView::_AddChildren()
{
	BRect textFrame(Bounds());
	textFrame.left += kNoteInset;
	textFrame.right -= kNoteInset;
	textFrame.top += kTitleHeight;
	textFrame.bottom -= kNoteInset;
	fTextView = new BTextView(textFrame, "NoteText", textFrame.OffsetToCopy(B_ORIGIN),
		B_FOLLOW_ALL, B_WILL_DRAW);
	fTextView->SetText(fText.String());
	fTextView->MakeEditable(false);
	fTextView->MakeSelectable(true);
	fTextView->SetWordWrap(true);
	AddChild(fTextView);

	BRect draggerFrame(Bounds());
	draggerFrame.left = draggerFrame.right - kDraggerSize;
	draggerFrame.top = draggerFrame.bottom - kDraggerSize;
	BDragger* dragger = new BDragger(draggerFrame, this,
		B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	AddChild(dragger);
	_ApplyColours();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NoteView::_ApplyColours()
{
	SetViewColor(fColour);
	SetLowColor(fColour);
	if (fTextView == NULL)
		return;

	rgb_color textColour = TextColourFor(fColour);
	fTextView->SetViewColor(fColour);
	fTextView->SetLowColor(fColour);
	fTextView->SetFontAndColor(be_plain_font, B_FONT_ALL, &textColour);
}
//---------------------------------------------------------------------------------------------------------------------------------//
