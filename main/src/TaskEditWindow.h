/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef TASK_EDIT_WINDOW_H
#define TASK_EDIT_WINDOW_H


#include <Messenger.h>
#include <Window.h>


class BButton;
class BTextControl;


const uint32 kTaskEditSaved = 'ktes';
const uint32 kTaskEditClosed = 'ktec';


class TaskEditWindow : public BWindow
{
public:
					TaskEditWindow(BMessenger target, int32 taskIndex,
						const char* schedule, const char* command);

	virtual void	MessageReceived(BMessage* message);
	virtual bool	QuitRequested();

private:
	void			_Save();

	static const uint32 kSavePressed = 1000;
	static const uint32 kCancelPressed = 1001;
	static const uint32 kPresetSelected = 1002;

	BMessenger		fTarget;
	int32			fTaskIndex;
	BTextControl*	fScheduleControl;
	BTextControl*	fCommandControl;
	BButton*		fSaveButton;
};


#endif
