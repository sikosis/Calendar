/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <DateTime.h>
#include <Window.h>


class BMenu;
class BMenuBar;
class Event;
class EventTabView;
class EventWindow;
class MainView;
class NotesWindow;
class Preferences;
class PreferenceWindow;
class SidePanelView;
class TasksWindow;


static const uint32 kMenuAppPref = 'kmap';
static const uint32 kMenuCategoryEdit = 'kmce';
static const uint32 kMenuHelp = 'kmhl';
static const uint32 kMenuSyncGCAL = 'kmsg';
static const uint32 kMenuTasks = 'kmts';
static const uint32 kMenuNotes = 'kmnt';
static const uint32 kMenuNewNote = 'kmnn';

static const int kAddEventMessage = 1005;


class MainWindow : public BWindow
{
public:
					MainWindow();
	virtual void	MessageReceived(BMessage* message);
	virtual bool	QuitRequested();

	void			StartNotificationThread();
	void			StopNotificationThread();
	BDate			SelectedDate() const;
	int32			SelectedView() const;

private:
	void			_InitInterface();
	void			_LaunchEventManager(Event* event, entry_ref* ref = NULL,
					const char* eventId = NULL, bool duplicate = false);
	void			_OpenHelp();
	void			_SyncWithPreferences();
	void			_UpdateEventsView();
	void			_SetEventListPopUpEnabled(bool state);
	BDate			_GetSelectedCalendarDate() const;
	void			_ToggleEventMenu(BMessage* msg);

	static const int kMenuAppClose = 1000;
	static const int kMenuAppQuit = 1001;


	MainView*		fMainView;
	EventWindow*	fEventWindow;
	TasksWindow*	fTasksWindow;
	NotesWindow*	fNotesWindow;
	BMenuBar*		fMenuBar;
	BMenu*			fAppMenu;
	BMenu*			fEventMenu;
	BMenu*			fCategoryMenu;
	BMenu*			fNotesMenu;
	BMenu*			fViewMenu;
	BMenu*			fSyncMenu;
	SidePanelView*	fSidePanelView;
	EventTabView*	fEventsView;
	thread_id		fNotificationThread;
};

#endif
