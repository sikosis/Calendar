/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "NotesWindow.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <Alert.h>
#include <Button.h>
#include <Catalog.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FilePanel.h>
#include <FindDirectory.h>
#include <Font.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <Message.h>
#include <OS.h>
#include <Path.h>
#include <PrintJob.h>
#include <ScrollView.h>
#include <String.h>
#include <StringItem.h>
#include <StringView.h>
#include <TextView.h>

#include "NoteEditWindow.h"
#include "NoteStore.h"
#include "NoteWindow.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NotesWindow"


static status_t
WriteNoteTextFile(const CalendarNote& note, const char* path)
{
	BFile file(path, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	status_t status = file.InitCheck();
	if (status != B_OK)
		return status;

	BString contents;
	if (!note.title.empty())
		contents << note.title.c_str() << "\n\n";
	contents << note.text.c_str() << "\n";
	ssize_t written = file.Write(contents.String(), contents.Length());
	if (written < 0)
		return (status_t)written;
	return written == contents.Length() ? file.Sync() : B_IO_ERROR;
}
//---------------------------------------------------------------------------------------------------------------------------------//


static status_t
CreateNoteZip(const CalendarNote& note, const char* outputPath)
{
	char directoryTemplate[] = "/tmp/hcalendar-note-XXXXXX";
	char* temporaryDirectory = mkdtemp(directoryTemplate);
	if (temporaryDirectory == NULL)
		return errno;

	BPath textPath(temporaryDirectory);
	status_t status = textPath.Append("Note.txt");
	if (status == B_OK)
		status = WriteNoteTextFile(note, textPath.Path());
	if (status != B_OK) {
		rmdir(temporaryDirectory);
		return status;
	}

	BString temporaryOutput(outputPath);
	temporaryOutput << ".new";
	unlink(temporaryOutput.String());

	BPath zipCommand;
	status = find_directory(B_SYSTEM_BIN_DIRECTORY, &zipCommand);
	if (status == B_OK)
		status = zipCommand.Append("zip");
	if (status == B_OK) {
		pid_t child = fork();
		if (child == 0) {
			execl(zipCommand.Path(), zipCommand.Path(), "-j", "-q",
				temporaryOutput.String(), textPath.Path(), (char*)NULL);
			_exit(127);
		}
		if (child < 0)
			status = errno;
		else {
			int childStatus = 0;
			if (waitpid(child, &childStatus, 0) < 0)
				status = errno;
			else if (!WIFEXITED(childStatus) || WEXITSTATUS(childStatus) != 0)
				status = B_ERROR;
		}
	}

	if (status == B_OK && rename(temporaryOutput.String(), outputPath) != 0)
		status = errno;
	if (status != B_OK)
		unlink(temporaryOutput.String());
	unlink(textPath.Path());
	rmdir(temporaryDirectory);
	return status;
}
//---------------------------------------------------------------------------------------------------------------------------------//


NotesWindow::NotesWindow(BMessenger owner)
	:
	BWindow(BRect(), B_TRANSLATE("Notes"), B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS),
	fOwner(owner),
	fEditWindow(NULL),
	fArchivePanel(NULL),
	fArchiveNoteId(0),
	fLoaded(false)
{
	fNoteList = new BListView("NoteList", B_SINGLE_SELECTION_LIST);
	fNoteList->SetSelectionMessage(new BMessage(kNoteSelected));
	fNoteList->SetInvocationMessage(new BMessage(kEditPressed));
	BScrollView* scroll = new BScrollView(
		"NoteScroll", fNoteList, B_WILL_DRAW, true, true);
	scroll->SetExplicitMinSize(BSize(560, 280));

	fNewButton = new BButton("New", B_TRANSLATE("New" B_UTF8_ELLIPSIS),
		new BMessage(kNewPressed));
	fEditButton = new BButton("Edit", B_TRANSLATE("Edit" B_UTF8_ELLIPSIS),
		new BMessage(kEditPressed));
	fDeleteButton = new BButton(
		"Delete", B_TRANSLATE("Delete"), new BMessage(kDeletePressed));
	fDisplayButton = new BButton(
		"Display", B_TRANSLATE("Display"), new BMessage(kDisplayPressed));
	fArchiveButton = new BButton("Archive", B_TRANSLATE("Archive (ZIP)"),
		new BMessage(kArchivePressed));
	fPrintButton = new BButton(
		"Print", B_TRANSLATE("Print" B_UTF8_ELLIPSIS),
		new BMessage(kPrintPressed));
	fStatusView = new BStringView("NoteStatus",
		B_TRANSLATE("Display a note, then drag its replicant handle to the Desktop."));
	_SetSelectionEnabled(false);
	fNewButton->SetEnabled(false);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(scroll)
		.Add(fStatusView)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fDeleteButton)
			.Add(fArchiveButton)
			.Add(fPrintButton)
			.AddGlue()
			.Add(fNewButton)
			.Add(fEditButton)
			.Add(fDisplayButton)
			.End()
		.End();

	CenterOnScreen();
	_LoadNotes();
}
//---------------------------------------------------------------------------------------------------------------------------------//


NotesWindow::~NotesWindow()
{
	delete fArchivePanel;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NotesWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kNewPressed:
		case kNewNoteRequested:
			_OpenEditor(-1);
			break;
		case kEditPressed:
			_OpenEditor(fNoteList->CurrentSelection());
			break;
		case kDeletePressed:
			_DeleteSelectedNote();
			break;
		case kDisplayPressed:
			_DisplaySelectedNote();
			break;
		case kArchivePressed:
			_ArchiveSelectedNote();
			break;
		case kArchiveSaveRequested:
			_SaveArchive(message);
			break;
		case kPrintPressed:
			_PrintSelectedNote();
			break;
		case kNoteSelected:
			_SetSelectionEnabled(fNoteList->CurrentSelection() >= 0);
			break;
		case kNoteEditSaved:
		{
			long long noteId = message->GetInt64("noteId", 0);
			const char* title = message->GetString("title", "");
			const char* text = message->GetString("text", "");
			uint8 red = (uint8)message->GetInt32("red", 255);
			uint8 green = (uint8)message->GetInt32("green", 244);
			uint8 blue = (uint8)message->GetInt32("blue", 128);
			NoteModel updated = fModel;
			if (noteId <= 0) {
				noteId = (long long)real_time_clock_usecs();
				while (updated.FindNote(noteId) != NULL)
					noteId++;
			}
			bool changed = fModel.FindNote(noteId) == NULL
				? updated.AddNote(noteId, title, text, red, green, blue)
				: updated.UpdateNote(noteId, title, text, red, green, blue);
			status_t status = changed ? _SaveModel(updated) : B_BAD_VALUE;
			BMessage reply;
			reply.AddInt32("status", status);
			message->SendReply(&reply);
			break;
		}
		case kNoteEditClosed:
			fEditWindow = NULL;
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
NotesWindow::QuitRequested()
{
	if (fEditWindow != NULL) {
		fEditWindow->Activate();
		return false;
	}
	fOwner.SendMessage(kNotesWindowQuitting);
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NotesWindow::_LoadNotes()
{
	status_t status = LoadCalendarNotes(fModel);
	if (status != B_OK) {
		_ShowError(B_TRANSLATE("Calendar could not read its notes file."), status);
		return;
	}
	fLoaded = true;
	fNewButton->SetEnabled(true);
	_ReloadList();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NotesWindow::_ReloadList()
{
	BListItem* item;
	while ((item = fNoteList->RemoveItem((int32)0)) != NULL)
		delete item;

	const std::vector<CalendarNote>& notes = fModel.Notes();
	for (size_t index = 0; index < notes.size(); index++) {
		BString label(notes[index].title.c_str());
		if (label.IsEmpty())
			label = B_TRANSLATE("Untitled note");
		BString preview(notes[index].text.c_str());
		preview.ReplaceAll("\n", " ");
		preview.Truncate(80);
		if (!preview.IsEmpty())
			label << "    " << preview;
		fNoteList->AddItem(new BStringItem(label.String()));
	}
	_SetSelectionEnabled(false);
	BString status(B_TRANSLATE("Saved notes: %count%. Display a note to drag its "
		"replicant handle to the Desktop."));
	BString count;
	count << (int32)notes.size();
	status.ReplaceFirst("%count%", count.String());
	fStatusView->SetText(status.String());
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NotesWindow::_OpenEditor(int32 noteIndex)
{
	if (!fLoaded)
		return;
	if (fEditWindow != NULL) {
		fEditWindow->Activate();
		return;
	}

	const CalendarNote* note = NULL;
	if (noteIndex >= 0 && (size_t)noteIndex < fModel.Notes().size())
		note = &fModel.Notes()[noteIndex];
	fEditWindow = new NoteEditWindow(BMessenger(this), note);
	fEditWindow->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NotesWindow::_DeleteSelectedNote()
{
	const CalendarNote* note = _SelectedNote();
	if (note == NULL)
		return;

	BAlert* alert = new BAlert(B_TRANSLATE("Delete note"),
		B_TRANSLATE("Delete the selected saved note? Desktop replicant copies "
			"are not affected."),
		B_TRANSLATE("Cancel"), B_TRANSLATE("Delete"), NULL,
		B_WIDTH_AS_USUAL, B_WARNING_ALERT);
	alert->SetShortcut(0, B_ESCAPE);
	if (alert->Go() != 1)
		return;

	NoteModel updated = fModel;
	if (updated.RemoveNote(note->id))
		_SaveModel(updated);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NotesWindow::_DisplaySelectedNote()
{
	const CalendarNote* note = _SelectedNote();
	if (note == NULL)
		return;
	(new NoteWindow(*note))->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NotesWindow::_ArchiveSelectedNote()
{
	const CalendarNote* note = _SelectedNote();
	if (note == NULL)
		return;
	fArchiveNoteId = note->id;
	if (fArchivePanel == NULL) {
		BMessenger target(this);
		BMessage archiveMessage(kArchiveSaveRequested);
		fArchivePanel = new BFilePanel(B_SAVE_PANEL, &target, NULL, 0, false,
			&archiveMessage);
	}
	BString name(note->title.c_str());
	if (name.IsEmpty())
		name = "Note";
	name.ReplaceAll("/", "-");
	name << ".zip";
	fArchivePanel->SetSaveText(name.String());
	fArchivePanel->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NotesWindow::_SaveArchive(BMessage* message)
{
	const CalendarNote* note = fModel.FindNote(fArchiveNoteId);
	entry_ref directoryRef;
	const char* name = NULL;
	if (note == NULL || message->FindRef("directory", &directoryRef) != B_OK
		|| message->FindString("name", &name) != B_OK) {
		return;
	}

	BDirectory directory(&directoryRef);
	BEntry directoryEntry;
	status_t status = directory.GetEntry(&directoryEntry);
	BPath path;
	if (status == B_OK)
		status = path.SetTo(&directoryEntry);
	if (status == B_OK)
		status = path.Append(name);
	if (status != B_OK) {
		_ShowError(B_TRANSLATE("Calendar could not create the note archive."),
			status);
		return;
	}
	BString outputPath(path.Path());
	if (!outputPath.EndsWith(".zip"))
		outputPath << ".zip";
	status = CreateNoteZip(*note, outputPath.String());
	if (status != B_OK) {
		_ShowError(B_TRANSLATE("Calendar could not create the note archive."),
			status);
		return;
	}
	fStatusView->SetText(B_TRANSLATE("Note archive created."));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NotesWindow::_PrintSelectedNote()
{
	const CalendarNote* note = _SelectedNote();
	if (note == NULL)
		return;

	BString jobName(note->title.c_str());
	if (jobName.IsEmpty())
		jobName = B_TRANSLATE("Calendar note");
	BPrintJob printJob(jobName.String());
	status_t status = printJob.ConfigJob();
	if (status != B_OK)
		return;

	BRect printable = printJob.PrintableRect();
	printable.OffsetTo(B_ORIGIN);
	BRect textRect(printable);
	textRect.InsetBy(24.0f, 24.0f);
	BTextView* printView = new BTextView(printable, "NotePrintView", textRect,
		B_FOLLOW_NONE, B_WILL_DRAW);
	BString contents;
	if (!note->title.empty())
		contents << note->title.c_str() << "\n\n";
	contents << note->text.c_str();
	printView->SetText(contents.String());
	printView->MakeEditable(false);
	printView->MakeSelectable(false);
	printView->SetWordWrap(true);
	printView->SetViewColor(note->red, note->green, note->blue);
	int32 brightness = note->red * 299 + note->green * 587 + note->blue * 114;
	rgb_color textColour = brightness < 128000
		? (rgb_color){255, 255, 255, 255}
		: (rgb_color){30, 30, 30, 255};
	printView->SetFontAndColor(be_plain_font, B_FONT_ALL, &textColour);
	AddChild(printView);
	printView->MoveTo(-printable.Width() - 100.0f, 0.0f);

	printJob.BeginJob();
	printJob.DrawView(printView, printView->Bounds(), B_ORIGIN);
	printJob.SpoolPage();
	if (printJob.CanContinue())
		printJob.CommitJob();
	else
		status = B_ERROR;
	RemoveChild(printView);
	delete printView;
	if (status != B_OK)
		_ShowError(B_TRANSLATE("Calendar could not print the note."), status);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
NotesWindow::_SaveModel(const NoteModel& model)
{
	status_t status = SaveCalendarNotes(model);
	if (status != B_OK) {
		_ShowError(B_TRANSLATE("Calendar could not save its notes file."), status);
		return status;
	}
	fModel = model;
	_ReloadList();
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NotesWindow::_SetSelectionEnabled(bool enabled)
{
	fEditButton->SetEnabled(enabled);
	fDeleteButton->SetEnabled(enabled);
	fDisplayButton->SetEnabled(enabled);
	fArchiveButton->SetEnabled(enabled);
	fPrintButton->SetEnabled(enabled);
}
//---------------------------------------------------------------------------------------------------------------------------------//


const CalendarNote*
NotesWindow::_SelectedNote() const
{
	int32 index = fNoteList->CurrentSelection();
	if (index < 0 || (size_t)index >= fModel.Notes().size())
		return NULL;
	return &fModel.Notes()[index];
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NotesWindow::_ShowError(const char* text, status_t status)
{
	BString message(text);
	message << "\n\n" << strerror(status);
	BAlert* alert = new BAlert(B_TRANSLATE("Notes"), message.String(),
		B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
	alert->Go();
}
//---------------------------------------------------------------------------------------------------------------------------------//
