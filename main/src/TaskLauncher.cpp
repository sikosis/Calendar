/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "TaskLauncher.h"

#include <stdlib.h>
#include <string.h>

#include <OS.h>


static int32
RunTask(void* data)
{
	char* command = static_cast<char*>(data);
	int result = system(command);
	free(command);
	return result;
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t
LaunchCalendarTask(const char* command)
{
	if (command == NULL || command[0] == '\0')
		return B_BAD_VALUE;

	char* commandCopy = strdup(command);
	if (commandCopy == NULL)
		return B_NO_MEMORY;

	thread_id thread = spawn_thread(
		RunTask, "Calendar task", B_NORMAL_PRIORITY, commandCopy);
	if (thread < B_OK) {
		free(commandCopy);
		return thread;
	}

	status_t status = resume_thread(thread);
	if (status != B_OK) {
		kill_thread(thread);
		free(commandCopy);
	}
	return status;
}
//---------------------------------------------------------------------------------------------------------------------------------//
