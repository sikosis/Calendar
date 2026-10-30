/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef NOTE_STORE_H
#define NOTE_STORE_H


#include <SupportDefs.h>


class BPath;
class NoteModel;


status_t FindCalendarNotesPath(BPath& path);
status_t LoadCalendarNotes(NoteModel& model);
status_t SaveCalendarNotes(const NoteModel& model);


#endif
