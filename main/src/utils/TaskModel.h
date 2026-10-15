/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef TASK_MODEL_H
#define TASK_MODEL_H


#include <stddef.h>

#include <string>
#include <vector>


struct CalendarTask {
	size_t		lineIndex;
	std::string	schedule;
	std::string	command;
};


class TaskModel
{
public:
	void				Load(const std::string& contents);
	std::string			Serialize() const;

	const std::vector<CalendarTask>& Tasks() const;
	bool				AddTask(const std::string& schedule,
							const std::string& command);
	bool				UpdateTask(size_t taskIndex,
							const std::string& schedule,
							const std::string& command);
	bool				RemoveTask(size_t taskIndex);

	static bool			IsValidSchedule(const std::string& schedule);
	static bool			IsValidCommand(const std::string& command);

private:
	void				_RefreshTasks();
	static bool			_ParseTaskLine(const std::string& line,
							CalendarTask& task);
	static bool			_IsEnvironmentLine(const std::string& line);
	static std::string	_Trim(const std::string& value);

	std::vector<std::string> fLines;
	std::vector<CalendarTask> fTasks;
};


#endif
