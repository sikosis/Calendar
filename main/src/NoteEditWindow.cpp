/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "NoteEditWindow.h"

#include <stddef.h>

#include <Alert.h>
#include <Button.h>
#include <Catalog.h>
#include <ColorControl.h>
#include <ColorMenuItem.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <StringView.h>
#include <TextControl.h>
#include <TextView.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NoteEditWindow"


NoteEditWindow::NoteEditWindow(BMessenger target, const CalendarNote* note)
	:
	BWindow(BRect(), note == NULL ? B_TRANSLATE("New note")
								: B_TRANSLATE("Edit note"),
		B_TITLED_WINDOW, B_AUTO_UPDATE_SIZE_LIMITS),
	fTarget(target),
	fNoteId(note != NULL ? note->id : 0)
{
	fTitleControl = new BTextControl("NoteTitle", B_TRANSLATE("Title:"),
		note != NULL ? note->title.c_str() : "", NULL);
	fTextView = new BTextView("NoteText", B_WILL_DRAW);
	fTextView->SetText(note != NULL ? note->text.c_str() : "");
	fTextView->SetExplicitMinSize(BSize(420, 180));
	BScrollView* scroll = new BScrollView(
		"NoteTextScroll", fTextView, B_WILL_DRAW, false, true);

	fColourControl = new BColorControl(B_ORIGIN, B_CELLS_32x8, 1,
		"NoteColour", new BMessage(kCustomColourSelected));
	rgb_color colour = {255, 244, 128, 255};
	if (note != NULL) {
		colour.red = note->red;
		colour.green = note->green;
		colour.blue = note->blue;
	}
	fColourControl->SetValue(colour);

	const char* presetNames[] = {
		B_TRANSLATE("Red"), B_TRANSLATE("Yellow"), B_TRANSLATE("Blue"),
		B_TRANSLATE("Green"), B_TRANSLATE("Cyan"), B_TRANSLATE("Magenta"),
		B_TRANSLATE("Orange"), B_TRANSLATE("Purple"), B_TRANSLATE("Pink"),
		B_TRANSLATE("Brown"), B_TRANSLATE("Grey"), B_TRANSLATE("White"),
		B_TRANSLATE("Black")
	};
	const rgb_color presetColours[] = {
		{255, 0, 0, 255}, {255, 255, 0, 255}, {0, 0, 255, 255},
		{0, 192, 0, 255}, {0, 255, 255, 255}, {255, 0, 255, 255},
		{255, 165, 0, 255}, {128, 0, 128, 255}, {255, 105, 180, 255},
		{150, 90, 50, 255}, {160, 160, 160, 255}, {255, 255, 255, 255},
		{0, 0, 0, 255}
	};
	fColourPresetMenu = new BPopUpMenu("NoteColourPreset");
	fColourPresetMenu->SetRadioMode(true);
	fColourPresetMenu->SetLabelFromMarked(true);
	bool matchedPreset = false;
	const size_t presetCount = sizeof(presetNames) / sizeof(presetNames[0]);
	for (size_t index = 0; index < presetCount; index++) {
		BMessage* presetMessage = new BMessage(kColourPresetSelected);
		presetMessage->AddInt32("red", presetColours[index].red);
		presetMessage->AddInt32("green", presetColours[index].green);
		presetMessage->AddInt32("blue", presetColours[index].blue);
		BMenuItem* item = new BColorMenuItem(
			presetNames[index], presetMessage, presetColours[index]);
		fColourPresetMenu->AddItem(item);
		if (colour.red == presetColours[index].red
			&& colour.green == presetColours[index].green
			&& colour.blue == presetColours[index].blue) {
			item->SetMarked(true);
			matchedPreset = true;
		}
	}
	fColourPresetMenu->AddSeparatorItem();
	fCustomColourItem = new BMenuItem(
		B_TRANSLATE("Custom"), new BMessage(kCustomColourSelected));
	fColourPresetMenu->AddItem(fCustomColourItem);
	if (!matchedPreset)
		fCustomColourItem->SetMarked(true);
	BMenuField* colourPresetField = new BMenuField("NoteColourPresetField",
		B_TRANSLATE("Colour preset:"), fColourPresetMenu);

	fSaveButton = new BButton(
		"Save", B_TRANSLATE("Save"), new BMessage(kSavePressed));
	BButton* cancelButton = new BButton(
		"Cancel", B_TRANSLATE("Cancel"), new BMessage(kCancelPressed));

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(fTitleControl)
		.Add(new BStringView("NoteTextLabel", B_TRANSLATE("Note:")))
		.Add(scroll)
		.Add(colourPresetField)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(new BStringView(
				"NoteColourLabel", B_TRANSLATE("Custom colour:")))
			.Add(fColourControl)
			.End()
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.AddGlue()
			.Add(cancelButton)
			.Add(fSaveButton)
			.End()
		.End();

	SetDefaultButton(fSaveButton);
	fTitleControl->MakeFocus(true);
	CenterOnScreen();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NoteEditWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kSavePressed:
			_Save();
			break;
		case kCancelPressed:
			PostMessage(B_QUIT_REQUESTED);
			break;
		case kColourPresetSelected:
		{
			rgb_color colour = {
				(uint8)message->GetInt32("red", 255),
				(uint8)message->GetInt32("green", 244),
				(uint8)message->GetInt32("blue", 128), 255};
			fColourControl->SetValue(colour);
			break;
		}
		case kCustomColourSelected:
			if (fColourPresetMenu->FindMarked() != NULL)
				fColourPresetMenu->FindMarked()->SetMarked(false);
			fCustomColourItem->SetMarked(true);
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
NoteEditWindow::QuitRequested()
{
	fTarget.SendMessage(kNoteEditClosed);
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
NoteEditWindow::_Save()
{
	const char* title = fTitleControl->Text();
	const char* text = fTextView->Text();
	if (!NoteModel::IsValid(title, text)) {
		BAlert* alert = new BAlert(B_TRANSLATE("Empty note"),
			B_TRANSLATE("Enter a title or some note text."),
			B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL,
			B_WARNING_ALERT);
		alert->Go();
		fTitleControl->MakeFocus(true);
		return;
	}

	rgb_color colour = fColourControl->ValueAsColor();
	BMessage save(kNoteEditSaved);
	save.AddInt64("noteId", fNoteId);
	save.AddString("title", title);
	save.AddString("text", text);
	save.AddInt32("red", colour.red);
	save.AddInt32("green", colour.green);
	save.AddInt32("blue", colour.blue);
	BMessage reply;
	status_t result = fTarget.SendMessage(&save, &reply);
	if (result == B_OK && reply.GetInt32("status", B_ERROR) == B_OK)
		PostMessage(B_QUIT_REQUESTED);
}
//---------------------------------------------------------------------------------------------------------------------------------//
