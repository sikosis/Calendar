/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "TasksWindow.h"

#include <string.h>

#include <string>

#include <Alert.h>
#include <Button.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <ScrollView.h>
#include <String.h>
#include <StringItem.h>
#include <StringView.h>

#include "TaskEditWindow.h"
#include "TaskLauncher.h"
#include "TaskStore.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TasksWindow"


TasksWindow::TasksWindow(BMessenger owner)
	:
	BWindow(BRect(), B_TRANSLATE("Tasks"), B_TITLED_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS),
	fOwner(owner),
	fEditWindow(NULL),
	fLoaded(false)
{
	fTaskList = new BListView("TaskList", B_SINGLE_SELECTION_LIST);
	fTaskList->SetSelectionMessage(new BMessage(kTaskSelected));
	fTaskList->SetInvocationMessage(new BMessage(kEditPressed));
	BScrollView* scroll = new BScrollView(
		"TaskScroll", fTaskList, B_WILL_DRAW, true, true);
	scroll->SetExplicitMinSize(BSize(560, 280));

	fNewButton = new BButton("New", B_TRANSLATE("New" B_UTF8_ELLIPSIS),
		new BMessage(kNewPressed));
	fEditButton = new BButton("Edit", B_TRANSLATE("Edit" B_UTF8_ELLIPSIS),
		new BMessage(kEditPressed));
	fDeleteButton = new BButton(
		"Delete", B_TRANSLATE("Delete"), new BMessage(kDeletePressed));
	fRunButton = new BButton(
		"RunNow", B_TRANSLATE("Run now"), new BMessage(kRunPressed));
	fStatusView = new BStringView("TaskStatus", "");
	_SetSelectionEnabled(false);
	fNewButton->SetEnabled(false);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(scroll)
		.Add(fStatusView)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fDeleteButton)
			.Add(fRunButton)
			.AddGlue()
			.Add(fNewButton)
			.Add(fEditButton)
			.End()
		.End();

	CenterOnScreen();
	_LoadTasks();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TasksWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kNewPressed:
			_OpenEditor(-1);
			break;
		case kEditPressed:
			_OpenEditor(fTaskList->CurrentSelection());
			break;
		case kDeletePressed:
			_DeleteSelectedTask();
			break;
		case kRunPressed:
			_RunSelectedTask();
			break;
		case kTaskSelected:
			_SetSelectionEnabled(fTaskList->CurrentSelection() >= 0);
			break;
		case kTaskEditSaved:
		{
			int32 taskIndex = message->GetInt32("taskIndex", -1);
			const char* schedule = message->GetString("schedule", "");
			const char* command = message->GetString("command", "");
			TaskModel updated = fModel;
			bool changed = taskIndex < 0
				? updated.AddTask(schedule, command)
				: updated.UpdateTask(taskIndex, schedule, command);
			status_t status = changed ? _SaveModel(updated) : B_BAD_VALUE;
			BMessage reply;
			reply.AddInt32("status", status);
			message->SendReply(&reply);
			break;
		}
		case kTaskEditClosed:
			fEditWindow = NULL;
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
TasksWindow::QuitRequested()
{
	if (fEditWindow != NULL) {
		fEditWindow->Activate();
		return false;
	}

	fOwner.SendMessage(kTasksWindowQuitting);
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TasksWindow::_LoadTasks()
{
	status_t status = LoadCalendarTasks(fModel);
	if (status != B_OK) {
		BString text(B_TRANSLATE("Calendar could not read its task file."));
		text << "\n\n" << strerror(status);
		BAlert* alert = new BAlert(B_TRANSLATE("Tasks"), text.String(),
			B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL,
			B_WARNING_ALERT);
		alert->Go();
		return;
	}

	fLoaded = true;
	fNewButton->SetEnabled(true);
	_ReloadList();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TasksWindow::_ReloadList()
{
	BListItem* item;
	while ((item = fTaskList->RemoveItem((int32)0)) != NULL)
		delete item;

	const std::vector<CalendarTask>& tasks = fModel.Tasks();
	for (size_t index = 0; index < tasks.size(); index++) {
		BString label(tasks[index].schedule.c_str());
		label << "    " << tasks[index].command.c_str();
		fTaskList->AddItem(new BStringItem(label.String()));
	}
	_SetSelectionEnabled(false);
	BString status(B_TRANSLATE("Scheduled tasks: %count%. The Deskbar calendar checks "
		"once per minute."));
	BString count;
	count << (int32)tasks.size();
	status.ReplaceFirst("%count%", count.String());
	fStatusView->SetText(status.String());
	fTaskList->Invalidate();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TasksWindow::_OpenEditor(int32 taskIndex)
{
	if (!fLoaded)
		return;
	if (fEditWindow != NULL) {
		fEditWindow->Activate();
		return;
	}

	const char* schedule = "";
	const char* command = "";
	if (taskIndex >= 0 && (size_t)taskIndex < fModel.Tasks().size()) {
		schedule = fModel.Tasks()[taskIndex].schedule.c_str();
		command = fModel.Tasks()[taskIndex].command.c_str();
	}
	fEditWindow
		= new TaskEditWindow(BMessenger(this), taskIndex, schedule, command);
	fEditWindow->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TasksWindow::_DeleteSelectedTask()
{
	int32 taskIndex = fTaskList->CurrentSelection();
	if (taskIndex < 0)
		return;

	BAlert* alert = new BAlert(B_TRANSLATE("Delete task"),
		B_TRANSLATE("Delete the selected task from Calendar?"),
		B_TRANSLATE("Cancel"), B_TRANSLATE("Delete"), NULL,
		B_WIDTH_AS_USUAL, B_WARNING_ALERT);
	alert->SetShortcut(0, B_ESCAPE);
	if (alert->Go() != 1)
		return;

	TaskModel updated = fModel;
	if (updated.RemoveTask(taskIndex))
		_SaveModel(updated);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TasksWindow::_RunSelectedTask()
{
	int32 taskIndex = fTaskList->CurrentSelection();
	if (taskIndex < 0 || (size_t)taskIndex >= fModel.Tasks().size())
		return;

	status_t status
		= LaunchCalendarTask(fModel.Tasks()[taskIndex].command.c_str());
	if (status == B_OK) {
		fStatusView->SetText(B_TRANSLATE("Task launched."));
		return;
	}

	BString text(B_TRANSLATE("Calendar could not launch the selected task."));
	text << "\n\n" << strerror(status);
	BAlert* alert = new BAlert(B_TRANSLATE("Tasks"), text.String(),
		B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
	alert->Go();
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
TasksWindow::_SaveModel(const TaskModel& model)
{
	status_t status = SaveCalendarTasks(model);
	if (status != B_OK) {
		BString text(B_TRANSLATE("Calendar could not save its task file."));
		text << "\n\n" << strerror(status);
		BAlert* alert = new BAlert(B_TRANSLATE("Tasks"), text.String(),
			B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->Go();
		return status;
	}

	fModel = model;
	_ReloadList();
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
TasksWindow::_SetSelectionEnabled(bool enabled)
{
	fEditButton->SetEnabled(enabled);
	fDeleteButton->SetEnabled(enabled);
	fRunButton->SetEnabled(enabled);
}
//---------------------------------------------------------------------------------------------------------------------------------//
