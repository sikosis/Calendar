/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * Copyright 2014-2017 Kacper Kasper, kacperkasper@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 */


#include "Preferences.h"

#include <Alert.h>
#include <Catalog.h>
#include <File.h>
#include <Message.h>
#include <String.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Preferences"


void
Preferences::Load(const char* filename)
{
	BFile* file = new BFile(filename, B_READ_ONLY);
	status_t result = file->InitCheck();
	switch (result) {
		case B_BAD_VALUE:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Configuration file"),
				B_TRANSLATE(
					"Couldn't open configuration file because the path is not "
					"specified. "
					"It usually means that the programmer made a mistake. "
					"There is nothing you can do about it. "
					"Your personal settings will not be loaded. Sorry."),
				B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL,
				B_WARNING_ALERT);
			alert->Go();
			break;
		}
		case B_PERMISSION_DENIED:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Configuration file"),
				B_TRANSLATE(
					"Couldn't open configuration file because "
					"permission was denied. "
					"It usually means that you don't have read "
					"permissions to your settings directory. "
					"If you want to have your personal settings "
					"loaded, check your OS documentation "
					"to find out which directory it is and try "
					"changing its permissions."),
				B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL,
				B_WARNING_ALERT);
			alert->Go();
			break;
		}
		case B_NO_MEMORY:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Configuration file"),
				B_TRANSLATE(
					"There is not enough memory available on your "
					"system to load the "
					"configuration file. If you want to have your "
					"personal settings loaded, try "
					"closing a few applications and restart Calendar."),
				B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL,
				B_WARNING_ALERT);
			alert->Go();
			break;
		}
		default:
			break;
	}

	BMessage storage;
	if (result == B_OK)
		storage.Unflatten(file);

	fStartOfWeekOffset = storage.GetInt32("startOfWeekOffset", 0);
	fSelectedTab = storage.GetInt32("selectedTab", 0);
	fViewMode = storage.GetInt32("viewMode", 0);
	fHeaderVisible = storage.GetBool("headerVisible", false);
	fFirstDeletion = storage.GetBool("firstDeletion", true);
	fYearProgressVisible = storage.GetBool("yearProgressVisible", true);
	fLifeProgressVisible = storage.GetBool("lifeProgressVisible", false);
	fLifeBirthYear = storage.GetInt32("lifeBirthYear", 0);
	fLifeExpectancy = storage.GetInt32("lifeExpectancy", 90);
	fDefaultCategory
		= storage.GetString("defaultCategory", B_TRANSLATE("Default"));
	fBarColour = ui_color(B_KEYBOARD_NAVIGATION_COLOR);
	const void* colourData = NULL;
	ssize_t colourSize = 0;
	if (storage.FindData("barColour", B_RGB_COLOR_TYPE, &colourData,
			&colourSize) == B_OK
		&& colourSize == sizeof(rgb_color)) {
		fBarColour = *(const rgb_color*) colourData;
		fBarColour.alpha = 255;
	}
	fYearProgressColour = ui_color(B_STATUS_BAR_COLOR);
	colourData = NULL;
	colourSize = 0;
	if (storage.FindData("yearProgressColour", B_RGB_COLOR_TYPE, &colourData,
			&colourSize) == B_OK
		&& colourSize == sizeof(rgb_color)) {
		fYearProgressColour = *(const rgb_color*) colourData;
		fYearProgressColour.alpha = 255;
	}
	fLifeProgressColour = ui_color(B_STATUS_BAR_COLOR);
	colourData = NULL;
	colourSize = 0;
	if (storage.FindData("lifeProgressColour", B_RGB_COLOR_TYPE, &colourData,
			&colourSize) == B_OK
		&& colourSize == sizeof(rgb_color)) {
		fLifeProgressColour = *(const rgb_color*) colourData;
		fLifeProgressColour.alpha = 255;
	}
	fMainWindowRect = storage.GetRect("mainWindowRect", BRect());
	fEventWindowRect = storage.GetRect("eventWindowRect", BRect());

	delete file;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
Preferences::Save(const char* filename)
{
	BFile* file
		= new BFile(filename, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	status_t result = file->InitCheck();
	switch (result) {
		case B_BAD_VALUE:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Configuration file"),
				B_TRANSLATE(
					"Couldn't open configuration file because the path is not "
					"specified. "
					"It usually means that the programmer made a mistake. "
					"There is nothing you can do about it. "
					"Your personal settings will not be loaded. Sorry."),
				B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL,
				B_WARNING_ALERT);
			alert->Go();
			break;
		}
		case B_PERMISSION_DENIED:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Configuration file"),
				B_TRANSLATE(
					"Couldn't open configuration file because "
					"permission was denied. "
					"It usually means that you don't have read "
					"permissions to your settings directory. "
					"If you want to have your personal settings "
					"loaded, check your OS documentation "
					"to find out which directory it is and try "
					"changing its permissions."),
				B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL,
				B_WARNING_ALERT);
			alert->Go();
			break;
		}
		case B_NO_MEMORY:
		{
			BAlert* alert = new BAlert(B_TRANSLATE("Configuration file"),
				B_TRANSLATE(
					"There is not enough memory available on your "
					"system to save the configuration file. If you "
					"want to have your personal settings saved, try "
					"closing a few applications and try again."),
				B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL,
				B_WARNING_ALERT);
			alert->Go();
			break;
		}
		default:
			break;
	}

	BMessage storage;
	storage.AddInt32("startOfWeekOffset", fStartOfWeekOffset);
	storage.AddInt32("selectedTab", fSelectedTab);
	storage.AddInt32("viewMode", fViewMode);
	storage.AddBool("headerVisible", fHeaderVisible);
	storage.AddBool("firstDeletion", fFirstDeletion);
	storage.AddBool("yearProgressVisible", fYearProgressVisible);
	storage.AddBool("lifeProgressVisible", fLifeProgressVisible);
	storage.AddInt32("lifeBirthYear", fLifeBirthYear);
	storage.AddInt32("lifeExpectancy", fLifeExpectancy);
	storage.AddString("defaultCategory", fDefaultCategory);
	storage.AddData(
		"barColour", B_RGB_COLOR_TYPE, &fBarColour, sizeof(fBarColour));
	storage.AddData("yearProgressColour", B_RGB_COLOR_TYPE,
		&fYearProgressColour, sizeof(fYearProgressColour));
	storage.AddData("lifeProgressColour", B_RGB_COLOR_TYPE,
		&fLifeProgressColour, sizeof(fLifeProgressColour));
	storage.AddRect("mainWindowRect", fMainWindowRect);
	storage.AddRect("eventWindowRect", fEventWindowRect);

	storage.Flatten(file);

	delete file;
}
//---------------------------------------------------------------------------------------------------------------------------------//


Preferences&
Preferences::operator=(const Preferences& p)
{
	fSettingsPath = p.fSettingsPath;
	fStartOfWeekOffset = p.fStartOfWeekOffset;
	fViewMode = p.fViewMode;
	fSelectedTab = p.fSelectedTab;
	fHeaderVisible = p.fHeaderVisible;
	fFirstDeletion = p.fFirstDeletion;
	fDefaultCategory = p.fDefaultCategory;
	fBarColour = p.fBarColour;
	fYearProgressVisible = p.fYearProgressVisible;
	fYearProgressColour = p.fYearProgressColour;
	fLifeProgressVisible = p.fLifeProgressVisible;
	fLifeBirthYear = p.fLifeBirthYear;
	fLifeExpectancy = p.fLifeExpectancy;
	fLifeProgressColour = p.fLifeProgressColour;
	fMainWindowRect = p.fMainWindowRect;
	fEventWindowRect = p.fEventWindowRect;

	return *this;
}
//---------------------------------------------------------------------------------------------------------------------------------//
