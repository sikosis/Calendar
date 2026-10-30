#include <assert.h>

#include "NoteModel.h"


int
main()
{
	NoteModel model;
	assert(!model.AddNote(0, "Title", "", 255, 255, 0));
	assert(!model.AddNote(1, "  ", "\n\t", 255, 255, 0));
	assert(model.AddNote(1, "Shopping", "Milk", 255, 240, 120));
	assert(!model.AddNote(1, "Duplicate", "", 1, 2, 3));
	assert(model.Notes().size() == 1);

	const CalendarNote* note = model.FindNote(1);
	assert(note != NULL);
	assert(note->title == "Shopping");
	assert(note->text == "Milk");
	assert(note->red == 255);

	assert(model.UpdateNote(1, "", "Milk and bread", 100, 110, 120));
	note = model.FindNote(1);
	assert(note != NULL && note->title.empty());
	assert(note->text == "Milk and bread");
	assert(note->green == 110);
	assert(!model.UpdateNote(99, "Missing", "", 0, 0, 0));

	assert(model.RemoveNote(1));
	assert(!model.RemoveNote(1));
	assert(model.Notes().empty());
	return 0;
}
