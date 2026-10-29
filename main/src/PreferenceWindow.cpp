/*
 * Copyright 2017 Akshay Agarwal, agarwal.akshay.akshay8@gmail.com
 * All rights reserved. Distributed under the terms of the MIT license.
 * Contributed by:
 *	Humdinger <humdingerb@gmail.com>, 2021
 */


#include "PreferenceWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <ColorControl.h>
#include <ColorMenuItem.h>
#include <DateTime.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Message.h>
#include <PopUpMenu.h>
#include <SeparatorView.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>
#include <TextView.h>
#include <Window.h>

#include <stdlib.h>

#include "App.h"
#include "Category.h"
#include "Preferences.h"
#include "QueryDBManager.h"
#include "SidePanelView.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PreferenceWindow"


static int32
ParsePositiveNumber(const char* text, int32 maximum)
{
	if (text == NULL || text[0] == '\0')
		return 0;

	char* end = NULL;
	long value = strtol(text, &end, 10);
	if (end == text || *end != '\0' || value <= 0 || value > maximum)
		return 0;

	return (int32)value;
}
//---------------------------------------------------------------------------------------------------------------------------------//


PreferenceWindow::PreferenceWindow(BRect frame, Preferences* preferences)
	:
	BWindow(BRect(), B_TRANSLATE("Settings"), B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	fCurrentPreferences = preferences;

	fStartPreferences = new Preferences();
	*fStartPreferences = *fCurrentPreferences;

	fTempPreferences = new Preferences();
	*fTempPreferences = *fCurrentPreferences;

	fDBManager = new QueryDBManager();

	_InitInterface();

	CenterIn(frame);
	_SyncPreferences(fCurrentPreferences);
}


PreferenceWindow::~PreferenceWindow()
{
	delete fStartPreferences;
	delete fTempPreferences;
}


void
PreferenceWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {

		case kStartOfWeekChangeMessage:
		{
			BMenuItem* item = fDayOfWeekMenu->FindMarked();
			int32 index = fDayOfWeekMenu->IndexOf(item);
			fTempPreferences->fStartOfWeekOffset = index;
			_PreferencesModified();
			break;
		}

		case kShowWeekChangeMessage:
		{
			bool state = fWeekNumberHeaderCB->Value() == B_CONTROL_ON;
			fTempPreferences->fHeaderVisible = state;
			_PreferencesModified();
			break;
		}

		case kApplyPreferencesMessage:
		{
			int32 currentYear = BDate::CurrentDate(B_LOCAL_TIME).Year();
			if (fTempPreferences->fLifeProgressVisible
				&& (fTempPreferences->fLifeBirthYear <= 0
					|| fTempPreferences->fLifeBirthYear > currentYear
					|| fTempPreferences->fLifeExpectancy <= 0)) {
				BAlert* alert = new BAlert(
					B_TRANSLATE("Invalid life progress settings"),
					B_TRANSLATE("Enter a birth year no later than the current "
						"year and an expected age between 1 and 200."),
					B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL,
					B_WARNING_ALERT);
				alert->Go();
				if (fTempPreferences->fLifeBirthYear <= 0
					|| fTempPreferences->fLifeBirthYear > currentYear) {
					fLifeBirthYearText->MakeFocus(true);
				} else
					fLifeExpectancyText->MakeFocus(true);
				break;
			}

			*fCurrentPreferences = *fTempPreferences;
			fApplyButton->SetEnabled(false);
			BMessage changed(kAppPreferencesChanged);
			be_app->PostMessage(&changed);
			break;
		}

		case kRevertPreferencesMessage:
		{
			*fTempPreferences = *fStartPreferences;
			fRevertButton->SetEnabled(false);
			fApplyButton->SetEnabled(true);
			_SyncPreferences(fTempPreferences);
			break;
		}

		case kDefaultCategoryChangeMessage:
		{
			BMenuItem* item = fDefaultCatMenu->FindMarked();
			fTempPreferences->fDefaultCategory = BString(item->Label());
			_PreferencesModified();
			break;
		}

		case kBarColourChangeMessage:
			fTempPreferences->fBarColour = fBarColourPicker->ValueAsColor();
			_PreferencesModified();
			break;

		case kYearProgressVisibilityChangeMessage:
			fTempPreferences->fYearProgressVisible
				= fYearProgressVisibleCB->Value() == B_CONTROL_ON;
			fYearProgressColourPicker->SetEnabled(
				fTempPreferences->fYearProgressVisible);
			_PreferencesModified();
			break;

		case kYearProgressColourChangeMessage:
			fTempPreferences->fYearProgressColour
				= fYearProgressColourPicker->ValueAsColor();
			_PreferencesModified();
			break;

		case kLifeProgressVisibilityChangeMessage:
			fTempPreferences->fLifeProgressVisible
				= fLifeProgressVisibleCB->Value() == B_CONTROL_ON;
			fLifeBirthYearText->SetEnabled(
				fTempPreferences->fLifeProgressVisible);
			fLifeExpectancyText->SetEnabled(
				fTempPreferences->fLifeProgressVisible);
			_PreferencesModified();
			break;

		case kLifeProgressValueChangeMessage:
			fTempPreferences->fLifeBirthYear
				= ParsePositiveNumber(fLifeBirthYearText->Text(), 9999);
			fTempPreferences->fLifeExpectancy
				= ParsePositiveNumber(fLifeExpectancyText->Text(), 200);
			_PreferencesModified();
			break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
PreferenceWindow::QuitRequested()
{
	be_app->PostMessage(kPreferenceWindowQuitting);
	return true;
}


void
PreferenceWindow::_InitInterface()
{
	fDayOfWeekMenu = new BPopUpMenu("DayOfWeekMenu");
	fDefaultCatMenu = new BPopUpMenu("DefaultCatMenu");

	const char* startOfWeekItems[] = {B_TRANSLATE("Locale based"),
		B_TRANSLATE("Monday"), B_TRANSLATE("Tuesday"), B_TRANSLATE("Wednesday"),
		B_TRANSLATE("Thursday"), B_TRANSLATE("Friday"), B_TRANSLATE("Saturday"),
		B_TRANSLATE("Sunday"), NULL};
	for (int i = 0; startOfWeekItems[i]; ++i)
		fDayOfWeekMenu->AddItem(new BMenuItem(
			startOfWeekItems[i], new BMessage(kStartOfWeekChangeMessage)));
	fDayOfWeekMenu->ItemAt(0)->SetMarked(true);
	fDayOfWeekMenuField = new BMenuField(
		"DayOfWeekMenu", B_TRANSLATE("First day of the week:"), fDayOfWeekMenu);

	CategoryList* categories = fDBManager->GetAllCategories(((App*) be_app)->GetPreferences()->fDefaultCategory);
	for (int i = 0; i < categories->CountItems(); i++) {
		Category* category = categories->ItemAt(i);
		fDefaultCatMenu->AddItem(new BColorMenuItem(category->GetName(),
			new BMessage(kDefaultCategoryChangeMessage), category->GetColor()));
	}
	fDefaultCatMenu->ItemAt(0)->SetMarked(true);
	fDefaultCatMenuField = new BMenuField(
		"DefaultCatMenu", B_TRANSLATE("Default category:"), fDefaultCatMenu);

	fWeekNumberHeaderCB = new BCheckBox("WeekNumberHeader",
		B_TRANSLATE("Show week numbers in calendar"),
		new BMessage(kShowWeekChangeMessage));
	fWeekNumberHeaderCB->SetValue(B_CONTROL_OFF);
	fBarColourPicker = new BColorControl(B_ORIGIN, B_CELLS_32x8, 1,
		"BarColourPicker", new BMessage(kBarColourChangeMessage));
	fYearProgressVisibleCB = new BCheckBox("YearProgressVisible",
		B_TRANSLATE("Show year progress"),
		new BMessage(kYearProgressVisibilityChangeMessage));
	fYearProgressColourPicker = new BColorControl(B_ORIGIN, B_CELLS_32x8, 1,
		"YearProgressColourPicker",
		new BMessage(kYearProgressColourChangeMessage));
	fLifeProgressVisibleCB = new BCheckBox("LifeProgressVisible",
		B_TRANSLATE("Show life progress"),
		new BMessage(kLifeProgressVisibilityChangeMessage));
	fLifeBirthYearText = new BTextControl("LifeBirthYear",
		B_TRANSLATE("Birth year:"), NULL, NULL);
	fLifeExpectancyText = new BTextControl("LifeExpectancy",
		B_TRANSLATE("Live to age:"), NULL, NULL);
	fLifeBirthYearText->SetModificationMessage(
		new BMessage(kLifeProgressValueChangeMessage));
	fLifeExpectancyText->SetModificationMessage(
		new BMessage(kLifeProgressValueChangeMessage));
	for (uint32 character = 0; character < '0'; character++) {
		fLifeBirthYearText->TextView()->DisallowChar(character);
		fLifeExpectancyText->TextView()->DisallowChar(character);
	}
	for (uint32 character = '9' + 1; character < 255; character++) {
		fLifeBirthYearText->TextView()->DisallowChar(character);
		fLifeExpectancyText->TextView()->DisallowChar(character);
	}

	fApplyButton = new BButton(
		B_TRANSLATE("Apply"), new BMessage(kApplyPreferencesMessage));
	fRevertButton = new BButton(
		B_TRANSLATE("Revert"), new BMessage(kRevertPreferencesMessage));

	fApplyButton->SetEnabled(false);
	fRevertButton->SetEnabled(false);

	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_DEFAULT_SPACING)
		.AddGrid(B_USE_DEFAULT_SPACING, B_USE_SMALL_SPACING)
		.SetInsets(
			B_USE_SMALL_INSETS, B_USE_SMALL_INSETS, B_USE_SMALL_INSETS, 0)
		.AddMenuField(fDefaultCatMenuField, 0, 0)
		.AddMenuField(fDayOfWeekMenuField, 0, 1)
		.Add(fWeekNumberHeaderCB, 0, 2, 2)
		.Add(new BStringView(
			"BarColourLabel", B_TRANSLATE("Bar colour:")), 0, 3)
		.Add(fBarColourPicker, 1, 3)
		.Add(fYearProgressVisibleCB, 0, 4, 2)
		.Add(new BStringView("YearProgressColourLabel",
			B_TRANSLATE("Year progress colour:")), 0, 5)
		.Add(fYearProgressColourPicker, 1, 5)
		.Add(fLifeProgressVisibleCB, 0, 6, 2)
		.Add(fLifeBirthYearText, 0, 7, 2)
		.Add(fLifeExpectancyText, 0, 8, 2)
		.End()
		.Add(new BSeparatorView(B_HORIZONTAL))
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
		.SetInsets(
			B_USE_SMALL_INSETS, 0, B_USE_SMALL_INSETS, B_USE_SMALL_INSETS)
		.Add(fRevertButton)
		.AddGlue()
		.Add(fApplyButton)
		.End()
		.End();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
PreferenceWindow::_SyncPreferences(Preferences* preferences)
{
	if (preferences->fHeaderVisible == true)
		fWeekNumberHeaderCB->SetValue(B_CONTROL_ON);
	else
		fWeekNumberHeaderCB->SetValue(B_CONTROL_OFF);

	BMenuItem* item = fDayOfWeekMenu->ItemAt(preferences->fStartOfWeekOffset);
	item->SetMarked(true);
	fBarColourPicker->SetValue(preferences->fBarColour);
	fYearProgressVisibleCB->SetValue(preferences->fYearProgressVisible
		? B_CONTROL_ON : B_CONTROL_OFF);
	fYearProgressColourPicker->SetValue(preferences->fYearProgressColour);
	fYearProgressColourPicker->SetEnabled(preferences->fYearProgressVisible);
	fLifeProgressVisibleCB->SetValue(preferences->fLifeProgressVisible
		? B_CONTROL_ON : B_CONTROL_OFF);
	BString birthYear;
	if (preferences->fLifeBirthYear > 0)
		birthYear << preferences->fLifeBirthYear;
	fLifeBirthYearText->SetText(birthYear.String());
	BString lifeExpectancy;
	lifeExpectancy << preferences->fLifeExpectancy;
	fLifeExpectancyText->SetText(lifeExpectancy.String());
	fLifeBirthYearText->SetEnabled(preferences->fLifeProgressVisible);
	fLifeExpectancyText->SetEnabled(preferences->fLifeProgressVisible);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
PreferenceWindow::_PreferencesModified()
{
	fApplyButton->SetEnabled(true);
	fRevertButton->SetEnabled(true);
}
