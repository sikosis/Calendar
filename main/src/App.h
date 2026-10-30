/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef APP_H
#define APP_H


#include <Application.h>
#include <Path.h>


class CategoryWindow;
class MainWindow;
class Preferences;
class PreferenceWindow;
class EventSyncWindow;


extern const char* kAppName;
extern const char* kSignature;
extern const uint32 kQuitCalendarCompletely;


class App : public BApplication
{
public:
					App();
					~App();

	void			ArgvReceived(int32 argc, char** argv);
	status_t		GetSupportedSuites(BMessage* message);
	void			MessageReceived(BMessage* message);
	void			RefsReceived(BMessage* message);
	void			ReadyToRun();
	BHandler*		ResolveSpecifier(BMessage* message, int32 index,
					BMessage* specifier, int32 form, const char* property);

	void			AboutRequested();
	bool			QuitRequested();
	Preferences*	GetPreferences();
	MainWindow*		mainWindow();
	CategoryWindow*	categoryWindow();


private:
	bool			_HandleScriptingMessage(BMessage* message);
	void			_InstallDeskbarReplicant();
	void			_QuitCompletely();

	MainWindow*		fMainWindow;
	PreferenceWindow* fPreferenceWindow;
	CategoryWindow*	fCategoryWindow;
	EventSyncWindow* fEventSyncWindow;

	Preferences*	fPreferences;
	BPath			fPreferencesFile;
};

#endif
