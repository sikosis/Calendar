/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef CRON_TAB_MODEL_H
#define CRON_TAB_MODEL_H


#include <stddef.h>

#include <string>
#include <vector>


struct CronTask {
	size_t		lineIndex;
	std::string	schedule;
	std::string	command;
};


class CronTabModel
{
public:
	void				Load(const std::string& contents);
	std::string			Serialize() const;

	const std::vector<CronTask>& Tasks() const;
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
							CronTask& task);
	static bool			_IsEnvironmentLine(const std::string& line);
	static std::string	_Trim(const std::string& value);

	std::vector<std::string> fLines;
	std::vector<CronTask> fTasks;
};


#endif
