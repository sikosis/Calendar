/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef NOTE_WINDOW_H
#define NOTE_WINDOW_H


#include <Window.h>

#include "NoteModel.h"


class NoteWindow : public BWindow
{
public:
					NoteWindow(const CalendarNote& note);
};


#endif
