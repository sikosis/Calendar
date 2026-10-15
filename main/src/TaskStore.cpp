/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "TaskStore.h"

#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>
#include <String.h>

#include <string>
#include <vector>

#include "TaskModel.h"


status_t
FindCalendarTasksPath(BPath& path)
{
	status_t status = find_directory(B_USER_SETTINGS_DIRECTORY, &path);
	if (status != B_OK)
		return status;

	status = path.Append("Calendar");
	if (status != B_OK)
		return status;

	BDirectory directory(path.Path());
	if (directory.InitCheck() == B_ENTRY_NOT_FOUND) {
		BPath settingsPath(path);
		settingsPath.GetParent(&path);
		BDirectory settingsDirectory(path.Path());
		status = settingsDirectory.CreateDirectory("Calendar", &directory);
		if (status != B_OK && status != B_FILE_EXISTS)
			return status;
		path = settingsPath;
		path.Append("Calendar");
	}

	return path.Append("tasks");
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
LoadCalendarTasks(TaskModel& model)
{
	BPath path;
	status_t status = FindCalendarTasksPath(path);
	if (status != B_OK)
		return status;

	BFile file(path.Path(), B_READ_ONLY);
	status = file.InitCheck();
	if (status == B_ENTRY_NOT_FOUND) {
		model.Load("");
		return B_OK;
	}
	if (status != B_OK)
		return status;

	off_t size;
	status = file.GetSize(&size);
	if (status != B_OK)
		return status;

	std::vector<char> buffer((size_t)size);
	size_t totalRead = 0;
	while (totalRead < (size_t)size) {
		ssize_t bytesRead
			= file.Read(&buffer[totalRead], (size_t)size - totalRead);
		if (bytesRead < 0)
			return (status_t)bytesRead;
		if (bytesRead == 0)
			break;
		totalRead += bytesRead;
	}

	model.Load(std::string(buffer.begin(), buffer.begin() + totalRead));
	return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
SaveCalendarTasks(const TaskModel& model)
{
	BPath path;
	status_t status = FindCalendarTasksPath(path);
	if (status != B_OK)
		return status;

	BPath parentPath;
	path.GetParent(&parentPath);
	BPath temporaryPath(parentPath);
	BString temporaryLeaf(path.Leaf());
	temporaryLeaf << ".new";
	temporaryPath.Append(temporaryLeaf.String());

	BFile file(temporaryPath.Path(),
		B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	status = file.InitCheck();
	if (status != B_OK)
		return status;

	std::string contents = model.Serialize();
	size_t totalWritten = 0;
	while (totalWritten < contents.length()) {
		ssize_t bytesWritten = file.Write(contents.data() + totalWritten,
			contents.length() - totalWritten);
		if (bytesWritten < 0)
			return (status_t)bytesWritten;
		if (bytesWritten == 0)
			return B_IO_ERROR;
		totalWritten += bytesWritten;
	}
	status = file.Sync();
	if (status != B_OK)
		return status;

	BEntry temporary(temporaryPath.Path());
	BDirectory parent(parentPath.Path());
	return temporary.MoveTo(&parent, path.Leaf(), true);
}
//---------------------------------------------------------------------------------------------------------------------------------//
