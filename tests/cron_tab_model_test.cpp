#include "CronTabModel.h"

#include <assert.h>


int
main()
{
	CronTabModel model;
	model.Load("# keep this comment\nPATH=/boot/system/bin\n"
		"15 9 * * 1-5 /boot/home/config/non-packaged/bin/backup\n"
		"@daily echo daily\n\n");

	assert(model.Tasks().size() == 2);
	assert(model.Tasks()[0].schedule == "15 9 * * 1-5");
	assert(model.Tasks()[0].command
		== "/boot/home/config/non-packaged/bin/backup");
	assert(model.Tasks()[1].schedule == "@daily");
	assert(model.Tasks()[1].command == "echo daily");

	assert(model.UpdateTask(0, "30 10 * * 1-5", "echo updated"));
	assert(model.AddTask("@reboot", "echo started"));
	assert(model.RemoveTask(1));
	assert(model.Tasks().size() == 2);
	assert(model.Serialize().find("# keep this comment\n") == 0);
	assert(model.Serialize().find("PATH=/boot/system/bin\n")
		!= std::string::npos);
	assert(model.Serialize().find("30 10 * * 1-5 echo updated\n")
		!= std::string::npos);
	assert(model.Serialize().find("@reboot echo started\n")
		!= std::string::npos);

	assert(CronTabModel::IsValidSchedule("0 12 * * *"));
	assert(CronTabModel::IsValidSchedule("@weekly"));
	assert(!CronTabModel::IsValidSchedule("0 12 * *"));
	assert(!CronTabModel::IsValidSchedule("@daily extra"));
	assert(CronTabModel::IsValidCommand("echo hello"));
	assert(!CronTabModel::IsValidCommand(""));
	assert(!CronTabModel::IsValidCommand("echo one\necho two"));
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//
