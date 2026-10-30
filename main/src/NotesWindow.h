/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef NOTES_WINDOW_H
#define NOTES_WINDOW_H


#include <Messenger.h>
#include <Window.h>

#include "NoteModel.h"


class BButton;
class BFilePanel;
class BListView;
class BStringView;
class NoteEditWindow;


const uint32 kNotesWindowQuitting = 'knwq';
const uint32 kNewNoteRequested = 'knnr';


class NotesWindow : public BWindow
{
public:
					NotesWindow(BMessenger owner);
					~NotesWindow();

	virtual void	MessageReceived(BMessage* message);
	virtual bool	QuitRequested();

private:
	void			_LoadNotes();
	void			_ReloadList();
	void			_OpenEditor(int32 noteIndex);
	void			_DeleteSelectedNote();
	void			_DisplaySelectedNote();
	void			_ArchiveSelectedNote();
	void			_SaveArchive(BMessage* message);
	void			_PrintSelectedNote();
	status_t		_SaveModel(const NoteModel& model);
	void			_SetSelectionEnabled(bool enabled);
	const CalendarNote* _SelectedNote() const;
	void			_ShowError(const char* text, status_t status);

	static const uint32 kNewPressed = 1000;
	static const uint32 kEditPressed = 1001;
	static const uint32 kDeletePressed = 1002;
	static const uint32 kNoteSelected = 1003;
	static const uint32 kDisplayPressed = 1004;
	static const uint32 kArchivePressed = 1005;
	static const uint32 kArchiveSaveRequested = 1006;
	static const uint32 kPrintPressed = 1007;

	BMessenger		fOwner;
	BListView*		fNoteList;
	BButton*		fNewButton;
	BButton*		fEditButton;
	BButton*		fDeleteButton;
	BButton*		fDisplayButton;
	BButton*		fArchiveButton;
	BButton*		fPrintButton;
	BStringView*	fStatusView;
	NoteEditWindow* fEditWindow;
	BFilePanel*	fArchivePanel;
	long long		fArchiveNoteId;
	NoteModel		fModel;
	bool			fLoaded;
};


#endif
