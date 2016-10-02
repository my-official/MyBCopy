#pragma once
#include "Reserver.h"
#include "BackupBase.h"
#include "BackupJobExecuter.h"
#include "AbstrastDiskManager.h"

struct HDDManagerEvent
{
	CopyList downloadList;
	BackupJobExecuter* backupJobExecuter;
	BackupJob* backupJob;
	bool Uploading;
};

class HDDManager : public AbstrastDiskManager<HDDManager, HDDManagerEvent>
{
public:	
	void StartCopyFiles(const CopyList& downloadList, BackupJobExecuter* backupJobExecuter, BackupJob* backupJob, bool uploading);
private:
	virtual bool HandleEvent(HDDManagerEvent& event) override;
	virtual void SendQuit() override;
};