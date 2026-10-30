/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "NoteStore.h"

#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Path.h>
#include <String.h>

#include "NoteModel.h"


status_t
FindCalendarNotesPath(BPath& path)
{
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (status != B_OK)
		return status;

	status = path.Append("Calendar");
	if (status != B_OK)
		return status;

	BDirectory directory(path.Path());
	if (directory.InitCheck() == B_ENTRY_NOT_FOUND) {
		BPath settingsPath(path);
		settingsPath.GetParent(&path);
		BDirectory settingsDirectory(path.Path());
		status = settingsDirectory.CreateDirectory("Calendar", &directory);
		if (status != B_OK && status != B_FILE_EXISTS)
			return status;
		path = settingsPath;
		path.Append("Calendar");
	}

	return path.Append("notes");
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
LoadCalendarNotes(NoteModel& model)
{
	BPath path;
	status_t status = FindCalendarNotesPath(path);
	if (status != B_OK)
		return status;

	BFile file(path.Path(), B_READ_ONLY);
	status = file.InitCheck();
	if (status == B_ENTRY_NOT_FOUND) {
		model.Clear();
		return B_OK;
	}
	if (status != B_OK)
		return status;

	BMessage archive;
	status = archive.Unflatten(&file);
	if (status != B_OK)
		return status;

	NoteModel loaded;
	BMessage noteArchive;
	for (int32 index = 0;
		archive.FindMessage("note", index, &noteArchive) == B_OK; index++) {
		int64 id = noteArchive.GetInt64("id", 0);
		const char* title = noteArchive.GetString("title", "");
		const char* text = noteArchive.GetString("text", "");
		int32 red = noteArchive.GetInt32("red", 255);
		int32 green = noteArchive.GetInt32("green", 244);
		int32 blue = noteArchive.GetInt32("blue", 128);
		if (!loaded.AddNote(id, title, text, (uint8)red, (uint8)green,
				(uint8)blue)) {
			return B_BAD_DATA;
		}
	}

	model = loaded;
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
SaveCalendarNotes(const NoteModel& model)
{
	BPath path;
	status_t status = FindCalendarNotesPath(path);
	if (status != B_OK)
		return status;

	BMessage archive('note');
	const std::vector<CalendarNote>& notes = model.Notes();
	for (size_t index = 0; index < notes.size(); index++) {
		BMessage noteArchive('ntit');
		noteArchive.AddInt64("id", notes[index].id);
		noteArchive.AddString("title", notes[index].title.c_str());
		noteArchive.AddString("text", notes[index].text.c_str());
		noteArchive.AddInt32("red", notes[index].red);
		noteArchive.AddInt32("green", notes[index].green);
		noteArchive.AddInt32("blue", notes[index].blue);
		status = archive.AddMessage("note", &noteArchive);
		if (status != B_OK)
			return status;
	}

	BPath parentPath;
	path.GetParent(&parentPath);
	BPath temporaryPath(parentPath);
	BString temporaryLeaf(path.Leaf());
	temporaryLeaf << ".new";
	status = temporaryPath.Append(temporaryLeaf.String());
	if (status != B_OK)
		return status;

	BFile file(temporaryPath.Path(),
		B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	status = file.InitCheck();
	if (status != B_OK)
		return status;
	status = archive.Flatten(&file);
	if (status != B_OK)
		return status;
	status = file.Sync();
	if (status != B_OK)
		return status;

	BEntry temporary(temporaryPath.Path());
	BDirectory parent(parentPath.Path());
	return temporary.MoveTo(&parent, path.Leaf(), true);
}
//---------------------------------------------------------------------------------------------------------------------------------//
