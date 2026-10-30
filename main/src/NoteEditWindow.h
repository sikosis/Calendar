/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef NOTE_EDIT_WINDOW_H
#define NOTE_EDIT_WINDOW_H


#include <Messenger.h>
#include <Window.h>

#include "NoteModel.h"


class BButton;
class BColorControl;
class BMenuItem;
class BPopUpMenu;
class BTextControl;
class BTextView;


const uint32 kNoteEditSaved = 'knes';
const uint32 kNoteEditClosed = 'knec';


class NoteEditWindow : public BWindow
{
public:
					NoteEditWindow(BMessenger target, const CalendarNote* note);

	virtual void	MessageReceived(BMessage* message);
	virtual bool	QuitRequested();

private:
	void			_Save();

	static const uint32 kSavePressed = 1000;
	static const uint32 kCancelPressed = 1001;
	static const uint32 kColourPresetSelected = 1002;
	static const uint32 kCustomColourSelected = 1003;

	BMessenger		fTarget;
	long long		fNoteId;
	BTextControl*	fTitleControl;
	BTextView*		fTextView;
	BColorControl*	fColourControl;
	BPopUpMenu*		fColourPresetMenu;
	BMenuItem*		fCustomColourItem;
	BButton*		fSaveButton;
};


#endif
