#include "stdafx.h"
#include "HDDManager.h"

void HDDManager::StartCopyFiles(const CopyList& downloadList, BackupJobExecuter* backupJobExecuter, BackupJob* backupJob, bool uploading)
{
	HDDManagerEvent event;
	event.downloadList = downloadList;
	event.backupJobExecuter = backupJobExecuter;
	event.backupJob = backupJob;
	event.Uploading = uploading;

	PushEvent(event);
}

bool HDDManager::HandleEvent(HDDManagerEvent& event)
{
	if (!event.backupJobExecuter)
		return false;

	for (auto& download : event.downloadList)
	{
		fs::copy_file(download.first, download.second);
	}

	if (event.Uploading)
		event.backupJobExecuter->m_UploadCompleter.CompleteSignaled(event.backupJob);
	else
		event.backupJobExecuter->m_DownloadCompleter.CompleteSignaled(event.backupJob);
	
	return true;
}

void HDDManager::SendQuit()
{
	HDDManagerEvent event;	
	event.backupJobExecuter = nullptr;

	PushEvent(event);
}
