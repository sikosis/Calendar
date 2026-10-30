/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "NoteWindow.h"

#include <Catalog.h>

#include "NoteView.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NoteWindow"


NoteWindow::NoteWindow(const CalendarNote& note)
	:
	BWindow(BRect(100, 100, 339, 279),
		note.title.empty() ? B_TRANSLATE("Note") : note.title.c_str(),
		B_TITLED_WINDOW, B_NOT_ZOOMABLE)
{
	NoteView* view = new NoteView(Bounds(), note);
	AddChild(view);
	CenterOnScreen();
}
//---------------------------------------------------------------------------------------------------------------------------------//
