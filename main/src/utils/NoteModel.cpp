/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "NoteModel.h"


static bool
HasVisibleText(const char* text)
{
	if (text == NULL)
		return false;

	for (; *text != '\0'; text++) {
		if (*text != ' ' && *text != '\t' && *text != '\r' && *text != '\n')
			return true;
	}
	return false;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
NoteModel::AddNote(long long id, const char* title, const char* text,
	unsigned char red, unsigned char green, unsigned char blue)
{
	if (id <= 0 || FindNote(id) != NULL || !IsValid(title, text))
		return false;

	CalendarNote note;
	note.id = id;
	note.title = title != NULL ? title : "";
	note.text = text != NULL ? text : "";
	note.red = red;
	note.green = green;
	note.blue = blue;
	fNotes.push_back(note);
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
NoteModel::UpdateNote(long long id, const char* title, const char* text,
	unsigned char red, unsigned char green, unsigned char blue)
{
	CalendarNote* note = FindNote(id);
	if (note == NULL || !IsValid(title, text))
		return false;

	note->title = title != NULL ? title : "";
	note->text = text != NULL ? text : "";
	note->red = red;
	note->green = green;
	note->blue = blue;
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
NoteModel::RemoveNote(long long id)
{
	for (std::vector<CalendarNote>::iterator iterator = fNotes.begin();
		iterator != fNotes.end(); iterator++) {
		if (iterator->id == id) {
			fNotes.erase(iterator);
			return true;
		}
	}
	return false;
}
//---------------------------------------------------------------------------------------------------------------------------------//


CalendarNote*
NoteModel::FindNote(long long id)
{
	for (size_t index = 0; index < fNotes.size(); index++) {
		if (fNotes[index].id == id)
			return &fNotes[index];
	}
	return NULL;
}
//---------------------------------------------------------------------------------------------------------------------------------//


const CalendarNote*
NoteModel::FindNote(long long id) const
{
	for (size_t index = 0; index < fNotes.size(); index++) {
		if (fNotes[index].id == id)
			return &fNotes[index];
	}
	return NULL;
}
//---------------------------------------------------------------------------------------------------------------------------------//


const std::vector<CalendarNote>&
NoteModel::Notes() const
{
	return fNotes;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NoteModel::Clear()
{
	fNotes.clear();
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
NoteModel::IsValid(const char* title, const char* text)
{
	return HasVisibleText(title) || HasVisibleText(text);
}
//---------------------------------------------------------------------------------------------------------------------------------//
