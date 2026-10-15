/*
 * Copyright 2026 Calendar contributors
 * All rights reserved. Distributed under the terms of the MIT License.
 */
#ifndef TASK_STORE_H
#define TASK_STORE_H


#include <SupportDefs.h>


class BPath;
class TaskModel;


status_t FindCalendarTasksPath(BPath& path);
status_t LoadCalendarTasks(TaskModel& model);
status_t SaveCalendarTasks(const TaskModel& model);


#endif
