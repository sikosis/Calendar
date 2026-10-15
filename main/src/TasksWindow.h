/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef TASKS_WINDOW_H
#define TASKS_WINDOW_H


#include <Messenger.h>
#include <Window.h>

#include "CronTabModel.h"


class BButton;
class BListView;
class TaskEditWindow;


const uint32 kTasksWindowQuitting = 'ktwq';


class TasksWindow : public BWindow
{
public:
					TasksWindow(BMessenger owner);

	virtual void	MessageReceived(BMessage* message);
	virtual bool	QuitRequested();

private:
	void			_LoadCrontab();
	void			_ReloadList();
	void			_OpenEditor(int32 taskIndex);
	void			_DeleteSelectedTask();
	status_t		_InstallModel(const CronTabModel& model);
	void			_SetSelectionEnabled(bool enabled);

	static const uint32 kNewPressed = 1000;
	static const uint32 kEditPressed = 1001;
	static const uint32 kDeletePressed = 1002;
	static const uint32 kTaskSelected = 1003;

	BMessenger		fOwner;
	BListView*		fTaskList;
	BButton*		fNewButton;
	BButton*		fEditButton;
	BButton*		fDeleteButton;
	TaskEditWindow* fEditWindow;
	CronTabModel	fModel;
	bool			fLoaded;
};


#endif
