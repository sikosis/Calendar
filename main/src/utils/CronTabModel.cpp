/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */

#include "CronTabModel.h"

#include <ctype.h>


void
CronTabModel::Load(const std::string& contents)
{
	fLines.clear();
	size_t start = 0;
	while (start < contents.length()) {
		size_t end = contents.find('\n', start);
		if (end == std::string::npos) {
			fLines.push_back(contents.substr(start));
			break;
		}

		fLines.push_back(contents.substr(start, end - start));
		start = end + 1;
	}

	_RefreshTasks();
}
//---------------------------------------------------------------------------------------------------------------------------------//


std::string
CronTabModel::Serialize() const
{
	std::string contents;
	for (size_t index = 0; index < fLines.size(); index++) {
		contents += fLines[index];
		contents += '\n';
	}
	return contents;
}
//---------------------------------------------------------------------------------------------------------------------------------//


const std::vector<CronTask>&
CronTabModel::Tasks() const
{
	return fTasks;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
CronTabModel::AddTask(
	const std::string& schedule, const std::string& command)
{
	if (!IsValidSchedule(schedule) || !IsValidCommand(command))
		return false;

	fLines.push_back(_Trim(schedule) + " " + _Trim(command));
	_RefreshTasks();
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
CronTabModel::UpdateTask(size_t taskIndex, const std::string& schedule,
	const std::string& command)
{
	if (taskIndex >= fTasks.size() || !IsValidSchedule(schedule)
		|| !IsValidCommand(command)) {
		return false;
	}

	fLines[fTasks[taskIndex].lineIndex]
		= _Trim(schedule) + " " + _Trim(command);
	_RefreshTasks();
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
CronTabModel::RemoveTask(size_t taskIndex)
{
	if (taskIndex >= fTasks.size())
		return false;

	fLines.erase(fLines.begin() + fTasks[taskIndex].lineIndex);
	_RefreshTasks();
	return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
CronTabModel::IsValidSchedule(const std::string& schedule)
{
	std::string value = _Trim(schedule);
	if (value.empty() || value.find('\n') != std::string::npos
		|| value.find('\r') != std::string::npos) {
		return false;
	}

	if (value[0] == '@')
		return value.find_first_of(" \t") == std::string::npos;

	size_t position = 0;
	int fields = 0;
	while (position < value.length()) {
		while (position < value.length()
			&& isspace(static_cast<unsigned char>(value[position]))) {
			position++;
		}
		if (position >= value.length())
			break;

		fields++;
		while (position < value.length()
			&& !isspace(static_cast<unsigned char>(value[position]))) {
			position++;
		}
	}

	return fields == 5;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
CronTabModel::IsValidCommand(const std::string& command)
{
	std::string value = _Trim(command);
	return !value.empty() && value.find('\n') == std::string::npos
		&& value.find('\r') == std::string::npos;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void
CronTabModel::_RefreshTasks()
{
	fTasks.clear();
	for (size_t index = 0; index < fLines.size(); index++) {
		CronTask task;
		if (_ParseTaskLine(fLines[index], task)) {
			task.lineIndex = index;
			fTasks.push_back(task);
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
CronTabModel::_ParseTaskLine(const std::string& line, CronTask& task)
{
	std::string value = _Trim(line);
	if (value.empty() || value[0] == '#' || _IsEnvironmentLine(value))
		return false;

	if (value[0] == '@') {
		size_t separator = value.find_first_of(" \t");
		if (separator == std::string::npos)
			return false;

		task.schedule = value.substr(0, separator);
		task.command = _Trim(value.substr(separator));
		return IsValidSchedule(task.schedule) && IsValidCommand(task.command);
	}

	size_t position = 0;
	task.schedule.clear();
	for (int field = 0; field < 5; field++) {
		while (position < value.length()
			&& isspace(static_cast<unsigned char>(value[position]))) {
			position++;
		}
		if (position >= value.length())
			return false;

		size_t start = position;
		while (position < value.length()
			&& !isspace(static_cast<unsigned char>(value[position]))) {
			position++;
		}
		if (field > 0)
			task.schedule += " ";
		task.schedule += value.substr(start, position - start);
	}

	task.command = _Trim(value.substr(position));
	return IsValidSchedule(task.schedule) && IsValidCommand(task.command);
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool
CronTabModel::_IsEnvironmentLine(const std::string& line)
{
	if (line.empty()
		|| !(isalpha(static_cast<unsigned char>(line[0])) || line[0] == '_')) {
		return false;
	}

	size_t position = 1;
	while (position < line.length()
		&& (isalnum(static_cast<unsigned char>(line[position]))
			|| line[position] == '_')) {
		position++;
	}
	while (position < line.length()
		&& isspace(static_cast<unsigned char>(line[position]))) {
		position++;
	}
	return position < line.length() && line[position] == '=';
}
//---------------------------------------------------------------------------------------------------------------------------------//


std::string
CronTabModel::_Trim(const std::string& value)
{
	size_t start = 0;
	while (start < value.length()
		&& isspace(static_cast<unsigned char>(value[start]))) {
		start++;
	}

	size_t end = value.length();
	while (end > start
		&& isspace(static_cast<unsigned char>(value[end - 1]))) {
		end--;
	}
	return value.substr(start, end - start);
}
//---------------------------------------------------------------------------------------------------------------------------------//
