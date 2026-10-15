/*
 * Copyight 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "App.h"

#include <AboutWindow.h>
#include <Catalog.h>
#include <Deskbar.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <LocaleRoster.h>
#include <NodeInfo.h>
#include <PropertyInfo.h>
#include <Roster.h>
#include <String.h>

#include <locale.h>
#include <stdio.h>

#include "CalendarDeskbarView.h"
#include "CategoryWindow.h"
#include "EventTabView.h"
#include "EventWindow.h"
#include "MainWindow.h"
#include "PreferenceWindow.h"
#include "Preferences.h"
#include "ScriptingUtils.h"
#include "SidePanelView.h"

const char* kAppName = B_TRANSLATE_SYSTEM_NAME("Calendar");
const char* kSignature = "application/x-vnd.calendar";
const char* const kVersion = "0.2.0";

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "App"


static property_info kCalendarProperties[] = {
	{ "Version", { B_GET_PROPERTY }, { B_DIRECT_SPECIFIER },
		B_TRANSLATE_MARK("Return the Calendar version") },
	{ "SelectedDate", { B_GET_PROPERTY, B_SET_PROPERTY },
		{ B_DIRECT_SPECIFIER },
		B_TRANSLATE_MARK("Get or set the selected date in YYYY-MM-DD format") },
	{ "View", { B_GET_PROPERTY, B_SET_PROPERTY }, { B_DIRECT_SPECIFIER },
		B_TRANSLATE_MARK("Get or set the day, week or month view") },
	{ "Today", { B_EXECUTE_PROPERTY }, { B_DIRECT_SPECIFIER },
		B_TRANSLATE_MARK("Go to today's date") },
	{ "NewEvent", { B_EXECUTE_PROPERTY }, { B_DIRECT_SPECIFIER },
		B_TRANSLATE_MARK("Open a new event window") },
	{ "ManageCategories", { B_EXECUTE_PROPERTY }, { B_DIRECT_SPECIFIER },
		B_TRANSLATE_MARK("Open the category manager") },
	{ 0 }
};


static void
SendScriptingReply(BMessage* request, status_t error, const char* result = NULL)
{
	BMessage reply(B_REPLY);
	if (result != NULL)
		reply.AddString("result", result);
	reply.AddInt32("error", error);
	request->SendReply(&reply);
}
//---------------------------------------------------------------------------------------------------------------------------------//


App::App()
	:
	BApplication(kSignature),
	fPreferenceWindow(NULL),
	fCategoryWindow(NULL),
	fPreferences(NULL),
	fEventSyncWindow(NULL)
{
	BPath settingsPath;
	find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath);
	settingsPath.Append(kAppName);
	BDirectory preferencesDir(settingsPath.Path());
	if (preferencesDir.InitCheck() == B_ENTRY_NOT_FOUND)
		preferencesDir.CreateDirectory(settingsPath.Path(), &preferencesDir);


	fPreferencesFile.SetTo(&preferencesDir, "settings");
	fPreferences = new Preferences();
	fPreferences->Load(fPreferencesFile.Path());
	fPreferences->fSettingsPath = settingsPath;

	fMainWindow = new MainWindow();
	fMainWindow->Show();
}


App::~App()
{
	fPreferences->Save(fPreferencesFile.Path());
	delete fPreferences;
}


void
App::ReadyToRun()
{
	_InstallDeskbarReplicant();
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
App::GetSupportedSuites(BMessage* message)
{
	message->AddString("suites", "suite/vnd.Sikosis-Calendar");
	BPropertyInfo propertyInfo(kCalendarProperties);
	message->AddFlat("messages", &propertyInfo);
	return BApplication::GetSupportedSuites(message);
}
//---------------------------------------------------------------------------------------------------------------------------------//


BHandler*
App::ResolveSpecifier(BMessage* message, int32 index, BMessage* specifier,
	int32 form, const char* property)
{
	BPropertyInfo propertyInfo(kCalendarProperties);
	if (propertyInfo.FindMatch(message, 0, specifier, form, property) >= 0)
		return this;

	return BApplication::ResolveSpecifier(
		message, index, specifier, form, property);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
App::_InstallDeskbarReplicant()
{
	BDeskbar deskbar;
	if (!deskbar.IsRunning() || deskbar.HasItem(kCalendarDeskbarItemName))
		return;

	app_info info;
	if (GetAppInfo(&info) == B_OK)
		deskbar.AddItem(&info.ref);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
App::AboutRequested()
{
	const char* currentContributors[] = {
		"Sikosis",
		NULL
	};
	const char* pastContributors[] = {
		"Akshay Agarwal",
		"Humdinger",
		"Jaidyn Levesque",
		"Harshit Sharma",
		"Jérôme Duval",
		"malbx",
		"Scott McCreary",
		"Fredrik Modéen",
		"Bach Nguyen",
		"janus2",
		"Augustin Cavalier",
		"Barmendu Chakraborty",
		NULL
	};

	BAboutWindow* aboutW = new BAboutWindow(kAppName, kSignature);
	BString version(B_TRANSLATE("Version"));
	version << " " << kVersion;
	aboutW->SetVersion(version.String());
	aboutW->AddDescription(
		B_TRANSLATE("A calendar application to manage your appointments."));
	aboutW->AddCopyright(2017, "Akshay Agarwal");
	aboutW->AddText(
		B_TRANSLATE("Current Contributors"), currentContributors);
	aboutW->AddText(B_TRANSLATE("Past Contributors"), pastContributors);
	aboutW->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
App::QuitRequested()
{
	fPreferences->fMainWindowRect = fMainWindow->Frame();

	if (fMainWindow->Lock())
		fMainWindow->Quit();
	return true;
}


Preferences*
App::GetPreferences()
{
	return fPreferences;
}


MainWindow*
App::mainWindow()
{
	return fMainWindow;
}


CategoryWindow*
App::categoryWindow()
{
	return fCategoryWindow;
}


bool
App::_HandleScriptingMessage(BMessage* message)
{
	int32 specifierIndex;
	BMessage specifier;
	BString property;
	if (message->GetCurrentSpecifier(&specifierIndex, &specifier) != B_OK
		|| specifier.FindString("property", &property) != B_OK) {
		return false;
	}

	if (message->what == B_GET_PROPERTY && property == "Version") {
		SendScriptingReply(message, B_OK, kVersion);
		return true;
	}

	if (message->what == B_GET_PROPERTY && property == "SelectedDate") {
		if (!fMainWindow->Lock()) {
			SendScriptingReply(message, B_ERROR);
			return true;
		}

		BDate date = fMainWindow->SelectedDate();
		fMainWindow->Unlock();
		char value[11];
		snprintf(value, sizeof(value), "%04ld-%02ld-%02ld",
			(long)date.Year(), (long)date.Month(), (long)date.Day());
		SendScriptingReply(message, B_OK, value);
		return true;
	}

	if (message->what == B_SET_PROPERTY && property == "SelectedDate") {
		const char* value;
		int year;
		int month;
		int day;
		if (message->FindString("data", &value) != B_OK
			|| !ParseIsoDate(value, year, month, day)) {
			SendScriptingReply(message, B_BAD_VALUE);
			return true;
		}

		BDate date(year, month, day);
		if (!date.IsValid()) {
			SendScriptingReply(message, B_BAD_VALUE);
			return true;
		}

		BMessage command(kSelectionMessage);
		command.AddInt32("year", year);
		command.AddInt32("month", month);
		command.AddInt32("day", day);
		SendScriptingReply(message, fMainWindow->PostMessage(&command), value);
		return true;
	}

	if (message->what == B_GET_PROPERTY && property == "View") {
		if (!fMainWindow->Lock()) {
			SendScriptingReply(message, B_ERROR);
			return true;
		}

		int32 selectedView = fMainWindow->SelectedView();
		fMainWindow->Unlock();
		SendScriptingReply(message, B_OK, CalendarViewName(selectedView));
		return true;
	}

	if (message->what == B_SET_PROPERTY && property == "View") {
		const char* value;
		int view;
		if (message->FindString("data", &value) != B_OK
			|| !ParseCalendarView(value, view)) {
			SendScriptingReply(message, B_BAD_VALUE);
			return true;
		}

		BMessage command(kChangeListTab);
		command.AddInt32("tab", view);
		SendScriptingReply(
			message, fMainWindow->PostMessage(&command), CalendarViewName(view));
		return true;
	}

	if (message->what == B_EXECUTE_PROPERTY && property == "Today") {
		SendScriptingReply(
			message, fMainWindow->PostMessage(kSetCalendarToCurrentDate));
		return true;
	}

	if (message->what == B_EXECUTE_PROPERTY && property == "NewEvent") {
		SendScriptingReply(message, fMainWindow->PostMessage(kAddEventMessage));
		return true;
	}

	if (message->what == B_EXECUTE_PROPERTY
		&& property == "ManageCategories") {
		SendScriptingReply(message, PostMessage(kMenuCategoryEdit));
		return true;
	}

	return false;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
App::MessageReceived(BMessage* message)
{
	if ((message->what == B_GET_PROPERTY
			|| message->what == B_SET_PROPERTY
			|| message->what == B_EXECUTE_PROPERTY)
		&& _HandleScriptingMessage(message)) {
		return;
	}

	switch (message->what) {

		case kMenuAppPref:
		{
			if (fPreferenceWindow == NULL) {
				fPreferenceWindow
					= new PreferenceWindow(fMainWindow->Frame(), fPreferences);
				fPreferenceWindow->Show();
			}

			fPreferenceWindow->Activate();
			break;
		}

		case kMenuCategoryEdit:
		{
			if (fCategoryWindow == NULL) {
				fCategoryWindow = new CategoryWindow();
				fCategoryWindow->Show();
			}

			fCategoryWindow->Activate();
			break;
		}

		case kCategoryWindowQuitting:
			fCategoryWindow = NULL;
			break;

		/*case kMenuSyncGCAL:
		{
			if (fEventSyncWindow == NULL) {
				fEventSyncWindow = new EventSyncWindow();
				fEventSyncWindow->Show();
			}

			fEventSyncWindow->Activate();
			break;
		}

		case kEventSyncWindowQuitting:
			fEventSyncWindow = NULL;
			break;*/

		/* NOTE:
		** Current Google Calendar Synchronization is non-working, hence are
		** these above cases. But removing them is not a wise choice, as they
		** can serve as a starting point in implementing the same functionality.
		*/

		case kPreferenceWindowQuitting:
			fPreferenceWindow = NULL;
			break;

		case kAppPreferencesChanged:
			fMainWindow->PostMessage(message);
			break;

		case B_REFS_RECEIVED:
			RefsReceived(message);
			break;

		case B_LOCALE_CHANGED:
			fMainWindow->PostMessage(message);
			break;

		default:
		{
			BApplication::MessageReceived(message);
			break;
		}
	}
}


void
App::RefsReceived(BMessage* message)
{
	int i = 0;
	entry_ref ref;
	BFile file;
	BNodeInfo info;
	char type[B_FILE_NAME_LENGTH];

	while (message->HasRef("refs", i)) {
		BMessage msg = BMessage(B_REFS_RECEIVED);
		message->FindRef("refs", i++, &ref);
		msg.AddRef("refs", &ref);

		file.SetTo(&ref, B_READ_ONLY);
		info.SetTo(&file);
		info.GetType(type);

		if (BString(type) == BString("application/x-calendar-event")
			|| BString(type) == BString("text/calendar"))
			fMainWindow->PostMessage(&msg);
		else if (BString(type) == BString("application/x-calendar-category")) {
			MessageReceived(new BMessage(kMenuCategoryEdit));
			fCategoryWindow->PostMessage(&msg);
		}
	}
}


void
App::ArgvReceived(int32 argc, char** argv)
{
	BMessage message(B_REFS_RECEIVED);

	for (int32 i = 1; i < argc; i++) {
		BEntry entry(argv[i]);
		entry_ref ref;
		if (entry.Exists() && entry.GetRef(&ref) == B_OK)
			message.AddRef("refs", &ref);
	}

	RefsReceived(&message);
}


int
main()
{
	setlocale(LC_ALL, "");

	App app;
	app.Run();
	return 0;
}
