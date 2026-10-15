/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "TaskEditWindow.h"

#include <string.h>

#include <Alert.h>
#include <Button.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <StringView.h>
#include <TextControl.h>

#include "TaskModel.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TaskEditWindow"


TaskEditWindow::TaskEditWindow(BMessenger target, int32 taskIndex,
	const char* schedule, const char* command)
	:
	BWindow(BRect(), taskIndex < 0 ? B_TRANSLATE("New task")
								 : B_TRANSLATE("Edit task"),
		B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE
			| B_AUTO_UPDATE_SIZE_LIMITS),
	fTarget(target),
	fTaskIndex(taskIndex)
{
	fScheduleControl = new BTextControl("Schedule", B_TRANSLATE("Schedule:"),
		schedule, NULL);
	fCommandControl = new BTextControl(
		"Command", B_TRANSLATE("Command:"), command, NULL);
	fSaveButton = new BButton(
		"Save", B_TRANSLATE("Save"), new BMessage(kSavePressed));
	BPopUpMenu* presetMenu = new BPopUpMenu("SchedulePreset");
	const char* presetLabels[] = {
		B_TRANSLATE("Every minute"), B_TRANSLATE("Hourly"),
		B_TRANSLATE("Daily"), B_TRANSLATE("Weekly"),
		B_TRANSLATE("Monthly"), B_TRANSLATE("At startup")
	};
	const char* presetSchedules[] = {
		"* * * * *", "@hourly", "@daily", "@weekly", "@monthly", "@reboot"
	};
	bool markedPreset = false;
	for (size_t index = 0; index < 6; index++) {
		BMessage* preset = new BMessage(kPresetSelected);
		preset->AddString("schedule", presetSchedules[index]);
		BMenuItem* item = new BMenuItem(presetLabels[index], preset);
		presetMenu->AddItem(item);
		if (strcmp(schedule, presetSchedules[index]) == 0) {
			item->SetMarked(true);
			markedPreset = true;
		}
	}
	BMenuItem* customItem = new BMenuItem(
		B_TRANSLATE("Custom"), new BMessage(kPresetSelected));
	presetMenu->AddItem(customItem);
	if (!markedPreset)
		customItem->SetMarked(true);
	BMenuField* presetField = new BMenuField(
		"SchedulePresetField", B_TRANSLATE("Common schedule:"), presetMenu);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(presetField)
		.Add(fScheduleControl)
		.Add(new BStringView("ScheduleHelp",
			B_TRANSLATE("Use five schedule fields, for example 0 9 * * 1-5, "
				"or a nickname such as @daily.")))
		.Add(fCommandControl)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.AddGlue()
			.Add(new BButton(
				"Cancel", B_TRANSLATE("Cancel"), new BMessage(kCancelPressed)))
			.Add(fSaveButton)
			.End()
		.End();

	SetDefaultButton(fSaveButton);
	fScheduleControl->SetExplicitMinSize(BSize(420, B_SIZE_UNSET));
	fScheduleControl->MakeFocus(true);
	CenterOnScreen();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TaskEditWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kSavePressed:
			_Save();
			break;
		case kCancelPressed:
			PostMessage(B_QUIT_REQUESTED);
			break;
		case kPresetSelected:
		{
			const char* schedule = message->GetString("schedule", NULL);
			if (schedule != NULL)
				fScheduleControl->SetText(schedule);
			fScheduleControl->MakeFocus(true);
			break;
		}
		default:
			BWindow::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
TaskEditWindow::QuitRequested()
{
	fTarget.SendMessage(kTaskEditClosed);
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TaskEditWindow::_Save()
{
	const char* schedule = fScheduleControl->Text();
	const char* command = fCommandControl->Text();
	if (!TaskModel::IsValidSchedule(schedule)) {
		BAlert* alert = new BAlert(B_TRANSLATE("Invalid schedule"),
			B_TRANSLATE("Enter five schedule fields or one nickname such "
				"as @daily."),
			B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->Go();
		fScheduleControl->MakeFocus(true);
		return;
	}
	if (!TaskModel::IsValidCommand(command)) {
		BAlert* alert = new BAlert(B_TRANSLATE("Invalid command"),
			B_TRANSLATE("Enter the command that this task should run."),
			B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->Go();
		fCommandControl->MakeFocus(true);
		return;
	}

	BMessage save(kTaskEditSaved);
	save.AddInt32("taskIndex", fTaskIndex);
	save.AddString("schedule", schedule);
	save.AddString("command", command);
	BMessage reply;
	status_t result = fTarget.SendMessage(&save, &reply);
	if (result == B_OK && reply.GetInt32("status", B_ERROR) == B_OK)
		PostMessage(B_QUIT_REQUESTED);
}
//---------------------------------------------------------------------------------------------------------------------------------//
