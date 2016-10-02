#pragma once
#include "Reserver.h"
#include "BackupBase.h"
#include "BackupJobExecuter.h"
#include "AbstrastIOManager.h"



struct LocalDiskCopyEvent : public CopyEvent, public EventTypeSpecificData
{
};

struct LocalDiskRemoveEvent : public RemoveEvent, public EventTypeSpecificData
{
	
};

struct LocalDiskEnumEvent : public EnumEvent, public EventTypeSpecificData
{
};


class LocalDiskIOManager : public AbstrastIOManager<LocalDiskIOManager, IOManagerEvent>
{
public:		
	LocalDiskIOManager();
	void StartCopyFiles(const CopyList& downloadList, Completer* completer, const Completion& completion);
	void StartRemoveFiles(const RemoveList& removeList, Completer* completer, const Completion& completion);
	RegularBackupList EnumerateBackupFiles(const wstring& backupPath);
private:
	virtual bool HandleEvent(IOManagerEvent& event) override;	
};