/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef NOTE_MODEL_H
#define NOTE_MODEL_H


#include <string>
#include <vector>


struct CalendarNote {
	long long id;
	std::string title;
	std::string text;
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};


class NoteModel
{
public:
	bool			AddNote(long long id, const char* title, const char* text,
					unsigned char red, unsigned char green, unsigned char blue);
	bool			UpdateNote(long long id, const char* title, const char* text,
					unsigned char red, unsigned char green, unsigned char blue);
	bool			RemoveNote(long long id);
	CalendarNote*	FindNote(long long id);
	const CalendarNote* FindNote(long long id) const;
	const std::vector<CalendarNote>& Notes() const;
	void			Clear();

	static bool		IsValid(const char* title, const char* text);

private:
	std::vector<CalendarNote> fNotes;
};


#endif
