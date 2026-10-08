CXX ?= c++
CXXFLAGS ?= -std=c++98 -Wall -Wextra -Werror

.PHONY: test clean

test: reminder_time_test clock_time_test time_interval_test
	./reminder_time_test
	./clock_time_test
	./time_interval_test

reminder_time_test: reminder_time_test.cpp ../main/src/utils/ReminderTime.cpp
	$(CXX) $(CXXFLAGS) -I../main/src/utils $^ -o $@

clock_time_test: clock_time_test.cpp ../main/src/utils/ClockTime.cpp
	$(CXX) $(CXXFLAGS) -I../main/src/utils $^ -o $@

time_interval_test: time_interval_test.cpp ../main/src/utils/TimeInterval.cpp
	$(CXX) $(CXXFLAGS) -I../main/src/utils $^ -o $@

clean:
	rm -f reminder_time_test clock_time_test time_interval_test
