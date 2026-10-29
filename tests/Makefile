CXX ?= c++
CXXFLAGS ?= -std=c++98 -Wall -Wextra -Werror

.PHONY: test clean

test: reminder_time_test clock_time_test time_interval_test scripting_utils_test \
	task_model_test task_schedule_test year_progress_test
	./reminder_time_test
	./clock_time_test
	./time_interval_test
	./scripting_utils_test
	./task_model_test
	./task_schedule_test
	./year_progress_test

reminder_time_test: reminder_time_test.cpp ../main/src/utils/ReminderTime.cpp
	$(CXX) $(CXXFLAGS) -I../main/src/utils $^ -o $@

clock_time_test: clock_time_test.cpp ../main/src/utils/ClockTime.cpp
	$(CXX) $(CXXFLAGS) -I../main/src/utils $^ -o $@

time_interval_test: time_interval_test.cpp ../main/src/utils/TimeInterval.cpp
	$(CXX) $(CXXFLAGS) -I../main/src/utils $^ -o $@

scripting_utils_test: scripting_utils_test.cpp ../main/src/utils/ScriptingUtils.cpp
	$(CXX) $(CXXFLAGS) -I../main/src/utils $^ -o $@

task_model_test: task_model_test.cpp ../main/src/utils/TaskModel.cpp \
		../main/src/utils/TaskSchedule.cpp
	$(CXX) $(CXXFLAGS) -I../main/src/utils $^ -o $@

task_schedule_test: task_schedule_test.cpp ../main/src/utils/TaskSchedule.cpp
	$(CXX) $(CXXFLAGS) -I../main/src/utils $^ -o $@

year_progress_test: year_progress_test.cpp ../main/src/utils/YearProgress.cpp
	$(CXX) $(CXXFLAGS) -I../main/src/utils $^ -o $@

clean:
	rm -f reminder_time_test clock_time_test time_interval_test \
		scripting_utils_test task_model_test task_schedule_test \
		year_progress_test
