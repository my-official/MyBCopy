#include "MyBCopy_pch.h"
#include "InternetIOManager.h"
#include "Utils.h"
#include "UtilsMyBCopy.h"
#include "WebDavMountManager.h"



InternetIOManager::InternetIOManager()
{
	SetIndependentUploadThread(true);
}

bool InternetIOManager::GetIndependentUploadThread() const
{
	unique_lock<std::recursive_mutex> lck(m_UploadingThreadMutex);
	return m_UploadingThread != nullptr;
}


void InternetIOManager::SetIndependentUploadThread(bool independent)
{
	unique_lock<std::recursive_mutex> lck(m_UploadingThreadMutex);

	bool changed =  ( independent  == bool(m_UploadingThread) );
	if (!changed)
		return;
	

	if (independent)
	{		 
		m_UploadingThread = make_unique<InternetIOThread>( );
	}
	else
	{
		m_UploadingThread.reset();		
	}
}

void InternetIOManager::StartDownloadFiles(InternetCopyEvent&& event, Completer* completer, const Completion& completion)
{
	IOManagerEvent e(EventType::Downloading, move(event), completer, completion);
	PushEvent(move(e));
}

void InternetIOManager::StartUploadFiles(InternetCopyEvent&& event, Completer* completer, const Completion& completion)
{	
	IOManagerEvent e(EventType::Uploading, move(event), completer, completion);

	unique_lock<std::recursive_mutex> lck(m_UploadingThreadMutex);
	if (m_UploadingThread)
	{
		m_UploadingThread->PushEvent(move(e));
	}
	else
	{
		PushEvent(move(e));
	}	
}


void InternetIOManager::StartRemoveFiles(InternetRemoveEvent&& event, Completer* completer, const Completion& completion)
{
	IOManagerEvent e(EventType::Remove, move(event), completer, completion);
	PushEvent(move(e));
}


RegularBackupList InternetIOManager::EnumerateBackupFiles(const WebDavStorage* storage, const BackupBase& backup)
{
	RegularBackupList result;

	InternetEnumEvent event;
	event.path = backup.m_Name;
	event.storage = storage;

	event.lastBackupsList = &result;


	Completion completion = Completion::CreateEmpty();

	Completer completer;

	IOManagerEvent e(EventType::Enumeration, move(event), &completer, completion);
	completer.RegisterWait();

	unique_lock<std::recursive_mutex> lck(m_UploadingThreadMutex);

	bool independent = GetIndependentUploadThread();
	SetIndependentUploadThread(false);
		
	PushEvent(move(e));	
	completer.WaitForComplete(completion);
	SetIndependentUploadThread(independent);

	completion.RethrowException();

	return result;
}


InternetIOThread::InternetIOThread()
{
	StartEventLoop();
}

bool InternetIOThread::HandleEvent(IOManagerEvent& e)
{
	InternetEventData& data = *(InternetEventData*)e.m_SpecificData.get();

	auto mount =  move( WebDavMountManager::Instance().Mount(data.storage->m_Url, data.storage->m_UserName, data.storage->m_Password) );
	m_Mount = &mount;



	switch (e.m_Type)		
	{
		case EventType::Downloading:		
		{
			auto& event = *static_cast<InternetCopyEvent*>(e.m_SpecificData.get());
			ProcessDownloading(event);
		}
		break;
		
		case EventType::Uploading:
		{
			auto& event = *static_cast<InternetCopyEvent*>(e.m_SpecificData.get());
			ProcessUploading(event);
		}
		break;
		case EventType::Remove:
		{
			auto& event = *static_cast<InternetRemoveEvent*>(e.m_SpecificData.get());
			ProcessRemoving(event);
		}
		break;
		case EventType::Enumeration: 
		{
			auto& event = *static_cast<InternetEnumEvent*>(e.m_SpecificData.get());
			ProcessEnumeration(event);
		}
		break;
		default: assert(0);
	}
		
	return true;
}


void InternetIOThread::ProcessDownloading(InternetCopyEvent& event)
{
	wstring cloudDirName = m_Mount->MountPoint() + L"\\";
	if (!event.storage->m_BackupDirName.empty())
		cloudDirName += event.storage->m_BackupDirName + L"\\";

	for (auto& download : event.downloadList)
	{
		download.first = cloudDirName + download.first;

		_copy_file(download.first, download.second);		
	}
}

void InternetIOThread::ProcessUploading(InternetCopyEvent& event)
{
	wstring cloudDirName = m_Mount->MountPoint() + L"\\";
	if (!event.storage->m_BackupDirName.empty())
		cloudDirName += event.storage->m_BackupDirName + L"\\";

	for (auto& download : event.downloadList)
	{
		download.second = cloudDirName + download.second;

		_copy_file(download.first, download.second);
	}
}

void InternetIOThread::ProcessRemoving(InternetRemoveEvent& event)
{
	wstring cloudDirName = m_Mount->MountPoint() + L"\\";
	if (!event.storage->m_BackupDirName.empty())
		cloudDirName += event.storage->m_BackupDirName + L"\\";

	for (auto& file : event.removeList)
	{
		file = cloudDirName + file;

		_force_remove_all(file);
	}
}


void InternetIOThread::ProcessEnumeration(InternetEnumEvent& event)
{
	wstring path = m_Mount->MountPoint() + L"\\";
	if (!event.storage->m_BackupDirName.empty())
		path += event.storage->m_BackupDirName + L"\\";
	path += event.path;
	*event.lastBackupsList = EnumBackupFiles(path) ;
}

