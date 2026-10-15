/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "TasksWindow.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <string>

#include <Alert.h>
#include <Button.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <ScrollView.h>
#include <String.h>
#include <StringItem.h>

#include "TaskEditWindow.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TasksWindow"


static char
LowerCharacter(char character)
{
	return (char)tolower(static_cast<unsigned char>(character));
}
//---------------------------------------------------------------------------------------------------------------------------------//


static status_t
RunCommand(char* const arguments[], BString& output)
{
	int outputPipe[2];
	if (pipe(outputPipe) != 0)
		return errno;

	pid_t child = fork();
	if (child < 0) {
		status_t error = errno;
		close(outputPipe[0]);
		close(outputPipe[1]);
		return error;
	}

	if (child == 0) {
		close(outputPipe[0]);
		dup2(outputPipe[1], STDOUT_FILENO);
		dup2(outputPipe[1], STDERR_FILENO);
		close(outputPipe[1]);
		setenv("NO_COLOR", "1", 1);
		execvp(arguments[0], arguments);
		const char* error = strerror(errno);
		write(STDERR_FILENO, error, strlen(error));
		write(STDERR_FILENO, "\n", 1);
		_exit(127);
	}

	close(outputPipe[1]);
	char buffer[1024];
	ssize_t bytesRead;
	while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0)
		output.Append(buffer, (int32)bytesRead);
	close(outputPipe[0]);

	int waitStatus = 0;
	while (waitpid(child, &waitStatus, 0) < 0 && errno == EINTR) {
	}
	if (WIFEXITED(waitStatus) && WEXITSTATUS(waitStatus) == 0)
		return B_OK;
	return B_ERROR;
}
//---------------------------------------------------------------------------------------------------------------------------------//


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
	_SetSelectionEnabled(false);
	fNewButton->SetEnabled(false);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.Add(scroll)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(fDeleteButton)
			.AddGlue()
			.Add(fNewButton)
			.Add(fEditButton)
			.End()
		.End();

	CenterOnScreen();
	_LoadCrontab();
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
		case kTaskSelected:
			_SetSelectionEnabled(fTaskList->CurrentSelection() >= 0);
			break;
		case kTaskEditSaved:
		{
			int32 taskIndex = message->GetInt32("taskIndex", -1);
			const char* schedule = message->GetString("schedule", "");
			const char* command = message->GetString("command", "");
			CronTabModel updated = fModel;
			bool changed = taskIndex < 0
				? updated.AddTask(schedule, command)
				: updated.UpdateTask(taskIndex, schedule, command);
			status_t status = changed ? _InstallModel(updated) : B_BAD_VALUE;
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
TasksWindow::_LoadCrontab()
{
	char command[] = "/boot/system/bin/crontab";
	char listOption[] = "-l";
	char* arguments[] = {command, listOption, NULL};
	BString output;
	status_t status = RunCommand(arguments, output);
	if (status != B_OK) {
		std::string lowerOutput(output.String());
		std::transform(lowerOutput.begin(), lowerOutput.end(),
			lowerOutput.begin(), LowerCharacter);
		if (lowerOutput.find("no crontab") == std::string::npos) {
			BString text(B_TRANSLATE("Calendar could not read your crontab."));
			if (!output.IsEmpty()) {
				text << "\n\n";
				text << output;
			}
			BAlert* alert = new BAlert(B_TRANSLATE("Tasks"), text.String(),
				B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL,
				B_WARNING_ALERT);
			alert->Go();
			return;
		}
		output.Truncate(0);
	}

	fModel.Load(output.String());
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

	const std::vector<CronTask>& tasks = fModel.Tasks();
	for (size_t index = 0; index < tasks.size(); index++) {
		BString label(tasks[index].schedule.c_str());
		label << "    " << tasks[index].command.c_str();
		fTaskList->AddItem(new BStringItem(label.String()));
	}
	_SetSelectionEnabled(false);
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
		B_TRANSLATE("Delete the selected task from your crontab?"),
		B_TRANSLATE("Cancel"), B_TRANSLATE("Delete"), NULL,
		B_WIDTH_AS_USUAL, B_WARNING_ALERT);
	alert->SetShortcut(0, B_ESCAPE);
	if (alert->Go() != 1)
		return;

	CronTabModel updated = fModel;
	if (updated.RemoveTask(taskIndex))
		_InstallModel(updated);
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
TasksWindow::_InstallModel(const CronTabModel& model)
{
	char temporaryPath[] = "/tmp/Calendar-crontab-XXXXXX";
	int file = mkstemp(temporaryPath);
	if (file < 0) {
		BString text(B_TRANSLATE("Calendar could not create a temporary "
			"crontab file."));
		text << "\n\n" << strerror(errno);
		BAlert* alert = new BAlert(B_TRANSLATE("Tasks"), text.String(),
			B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT);
		alert->Go();
		return B_ERROR;
	}

	std::string contents = model.Serialize();
	size_t written = 0;
	while (written < contents.length()) {
		ssize_t result
			= write(file, contents.data() + written, contents.length() - written);
		if (result <= 0) {
			int error = result < 0 ? errno : EIO;
			close(file);
			unlink(temporaryPath);
			BString text(B_TRANSLATE("Calendar could not write the temporary "
				"crontab file."));
			text << "\n\n" << strerror(error);
			BAlert* alert = new BAlert(B_TRANSLATE("Tasks"), text.String(),
				B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL,
				B_WARNING_ALERT);
			alert->Go();
			return B_ERROR;
		}
		written += result;
	}
	close(file);

	char command[] = "/boot/system/bin/crontab";
	char* arguments[] = {command, temporaryPath, NULL};
	BString output;
	status_t status = RunCommand(arguments, output);
	unlink(temporaryPath);
	if (status != B_OK) {
		BString text(B_TRANSLATE("Calendar could not install your crontab."));
		if (!output.IsEmpty()) {
			text << "\n\n";
			text << output;
		}
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
}
//---------------------------------------------------------------------------------------------------------------------------------//
