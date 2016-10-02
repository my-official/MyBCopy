#include "stdafx.h"
#include "LocalDiskIOManager.h"
#include "Utils.h"
#include "UtilsMyRCopy.h"

LocalDiskIOManager::LocalDiskIOManager()
{
	StartEventLoop();
}

void LocalDiskIOManager::StartCopyFiles(const CopyList& downloadList, Completer* completer, const Completion& completion)
{
	LocalDiskCopyEvent event;
	event.downloadList = downloadList;	

	IOManagerEvent e(EventType::Downloading, move(event), completer, completion);
	PushEvent(move(e));
}


void LocalDiskIOManager::StartRemoveFiles(const RemoveList& removeList, Completer* completer, const Completion& completion)
{
	LocalDiskRemoveEvent event;
	event.removeList = removeList;

	IOManagerEvent e(EventType::Delete,move(event), completer, completion);
	PushEvent(move(e));
}

RegularBackupList LocalDiskIOManager::EnumerateBackupFiles(const wstring& backupPath)
{
	RegularBackupList result;

	LocalDiskEnumEvent event;
	event.path = backupPath;

	event.lastBackupsList = &result;


	Completion completion;
	completion.job = nullptr;

	Completer completer;

	IOManagerEvent e(EventType::Enumeration, move(event), &completer, completion);
	completer.RegisterWait();
	
	PushEvent(move(e));

	completer.WaitForComplete(completion);

	completion.RethrowException();
	
	return result;
}

bool LocalDiskIOManager::HandleEvent(IOManagerEvent& e)
{	
	switch (e.m_Type)
	{
		case EventType::Downloading:
		case EventType::Uploading:
		{
			auto& event = *static_cast<LocalDiskCopyEvent*>(e.m_SpecificData.get());

			for (auto& download : event.downloadList)
			{
				CopyFileAndPrint(download.first, download.second, false);
			}			
		}
		break;		
		case EventType::Delete:
		{
			auto& event = *static_cast<LocalDiskRemoveEvent*>(e.m_SpecificData.get());
			
			for (auto& file : event.removeList)
			{
				RemoveFileAndPrint(file);
			}
		}
		break;
		case EventType::Enumeration:
		{
			auto& event = *static_cast<LocalDiskEnumEvent*>(e.m_SpecificData.get());
			*event.lastBackupsList = EnumBackupFiles(event.path);
		}
		break;				
		default: assert(0);
	}
	
	return true;
}

