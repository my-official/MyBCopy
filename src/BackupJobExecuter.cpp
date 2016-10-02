#include "stdafx.h"
#include "BackupJobExecuter.h"
#include "Reserver.h"
#include "Utils.h"
#include "UtilsMyRCopy.h"
#include "Completer.h"


BackupJobExecuter::BackupJobExecuter(Reserver* reserver) : m_Reserver(reserver), m_BackupJobs(reserver->m_BackupJobs)
{
}





bool BackupJobExecuter::operator()()
{
	StartDownloadingAllJobs();			

	Completion completion;
	
	while (m_Completer.WaitForComplete(completion))
	{						
		if (completion.m_ExceptionPtr)
		{			
			PrintException(completion.m_ExceptionPtr);
		}
		else
		{
			BackupJob* job = completion.job;
			auto& functions = job->Functions;
			
			if (!functions.empty())
			{
				functions.front()(completion);
				functions.pop();
			}
			
		}	
	}


	return true;
}

Reserver* BackupJobExecuter::GetReserver() const
{
	return m_Reserver;
}

void BackupJobExecuter::StartRemoveOldArchives(Completion& completion)
{
	auto storage = completion.storage;
	auto job = completion.job;

	auto enumeratedList = storage->GetEnumeratedBackupFiles(job->Backup);
	auto removeList = MakeRemoveListForOldBackupFiles(enumeratedList);

	for (auto& file : removeList)
	{
		file = job->Backup.m_ArchiveName + L"\\" + file;
	}

	storage->StartRemovingOld(removeList, &m_Completer, completion);
	m_Completer.RegisterWait();
}

void BackupJobExecuter::StartDownloadingAllJobs()
{
	for (auto& job : m_BackupJobs)
	{
		StartDownloadingForJob(job);

	}

	/*for (auto& job : m_BackupJobs)
	{
		job.Functions.front()(job);		
	}*/
}




void BackupJobExecuter::StartDownloadingForJob(BackupJob& job)
{
	Completion completion;
	completion.job = &job;

	if (job.DownloadFromStorage)
	{
		assert(!job.RegularFileName.empty());

		CopyList downloadList;

		downloadList.push_back(make_pair(job.Backup.m_ArchiveName + L"\\" + job.RegularFileName, job.TmpWorkingDir.GetPath() + L"\\" + job.RegularFileName));

		if (!job.DeltaFileName.empty())
		{
			downloadList.push_back(make_pair(job.Backup.m_ArchiveName + L"\\" + job.DeltaFileName, job.TmpWorkingDir.GetPath() + L"\\" + job.DeltaFileName));

			for (auto& incrementFileName : job.IncrementFileNames)
			{
				downloadList.push_back(make_pair(job.Backup.m_ArchiveName + L"\\" + incrementFileName, job.TmpWorkingDir.GetPath() + L"\\" + incrementFileName));
			}
		}
		
		
		job.DownloadFromStorage->StartDownloading(downloadList, &m_Completer, completion);


		m_Completer.RegisterWait();
	}
	else
	{
		m_Completer.RegisterWait();
		m_Completer.CompleteSignaled(completion);
	}
}







void BackupJobExecuter::StartUploadingForJob(BackupJob& job)
{
	CopyList upload;
	upload.emplace_back(job.UploadSrcFile, job.UploadDestFileRelative);

	for (auto& storage : job.UploadToStorages)
	{
		Completion completion;
		completion.job = &job;
		completion.storage = storage.get();

		storage->StartUploading(upload, &m_Completer, completion);
		m_Completer.RegisterWait();	
	}
}






void BackupJobExecuter::CreateBackup(BackupJob& job)
{
	if (job.RegularFileName.empty())
	{
		CreateRegularBackup(job);
	}
	else
	{
		if (job.DeltaFileName.empty())
		{
			CreateDeltaBackup(job);
		}
		else
		{
			CreateIncrementalBackup(job);
		}
	}
}

void BackupJobExecuter::CreateRegularBackup(BackupJob& job)
{
	TraversedParesedBackup actualData(job.Backup);
	if (actualData.Empty())
		return;

	const wstring& nowTimestamp = GeneralSettings::Instance().m_NowTimestamp;

	wstring newBackupArchiveFileNameBase = MakeRegularArchiveFileName(nowTimestamp, L"");
	MakeBackupArchiveFile(actualData, newBackupArchiveFileNameBase, job.TmpWorkingDir.GetPath(), job.Archived7z);

	job.UploadSrcFile = job.TmpWorkingDir.GetPath() + L"\\" + newBackupArchiveFileNameBase + L".7z";
	job.UploadDestFileRelative = job.Backup.m_ArchiveName + L"\\" + newBackupArchiveFileNameBase + L".7z";

	StartUploadingForJob(job);
}

void BackupJobExecuter::CreateDeltaBackup(BackupJob& job)
{
	wstring oldRegularArchiveFileName = job.RegularFileName;
	wstring oldExtractedDataDir = job.TmpWorkingDir.GetPath() + L"\\content";

	Extract(job.TmpWorkingDir.GetPath(), job.RegularFileName, oldExtractedDataDir);

	TraversedPathBackup oldData(oldExtractedDataDir);

	TraversedParesedBackup actualData(job.Backup);
	if (actualData.Empty())
		return;
	

	BackupDifference diff = BackupDifference::Compare(oldData, actualData);

	actualData = TraversedParesedBackup();

	diff.ToTraversedEntries(actualData);
	if (actualData.Empty())
		return;

	const wstring& nowTimestamp = GeneralSettings::Instance().m_NowTimestamp;

	wstring newBackupArchiveFileNameBase = MakeDeltaArchiveFileName(nowTimestamp, ExtractTimestampFromFilename(oldRegularArchiveFileName), L"");
	MakeBackupArchiveFile(actualData, newBackupArchiveFileNameBase, job.TmpWorkingDir.GetPath(), job.Archived7z);
		
	job.UploadSrcFile = job.TmpWorkingDir.GetPath() + L"\\" + newBackupArchiveFileNameBase + L".7z";
	job.UploadDestFileRelative = job.Backup.m_ArchiveName + L"\\" + newBackupArchiveFileNameBase + L".7z";

	StartUploadingForJob(job);
}

void BackupJobExecuter::CreateIncrementalBackup(BackupJob& job)
{
	wstring oldRegularArchiveFileName = job.RegularFileName;	
	wstring oldExtractedDataDir = job.TmpWorkingDir.GetPath() + L"\\content";
		
	Extract(job.TmpWorkingDir.GetPath(), job.RegularFileName, oldExtractedDataDir);
	Extract(job.TmpWorkingDir.GetPath(), job.DeltaFileName, oldExtractedDataDir);

	for (const wstring& incrementArchiveFileName : job.IncrementFileNames)
	{
		Extract(job.TmpWorkingDir.GetPath(), incrementArchiveFileName, oldExtractedDataDir);
	}


	TraversedPathBackup oldData(oldExtractedDataDir);

	TraversedParesedBackup actualData(job.Backup);
	if (actualData.Empty())
		return;

	BackupDifference diff = BackupDifference::Compare(oldData, actualData);

	actualData = TraversedParesedBackup();

	diff.ToTraversedEntries(actualData);
	if (actualData.Empty())
		return;

	const wstring& nowTimestamp = GeneralSettings::Instance().m_NowTimestamp;

	wstring newBackupArchiveFileNameBase = MakeIncrementArchiveFileName(nowTimestamp, L"");
	MakeBackupArchiveFile(actualData, newBackupArchiveFileNameBase, job.TmpWorkingDir.GetPath(), job.Archived7z);
	

	wstring incrementsDirName;
	if (job.IncrementFileNames.empty())
	{
		incrementsDirName = MakeIncrementArchivesDirName(nowTimestamp, ExtractTimestampFromFilename(job.DeltaFileName));
	}
	else
	{
		wstring oldIncrementTimeStamp = ExtractTimestampFromFilename(  fs::path(*job.IncrementFileNames.begin()).filename() );
		incrementsDirName = MakeIncrementArchivesDirName(oldIncrementTimeStamp, ExtractTimestampFromFilename(job.DeltaFileName));
	}

	job.UploadSrcFile = job.TmpWorkingDir.GetPath() + L"\\" + newBackupArchiveFileNameBase + L".7z";
	job.UploadDestFileRelative = job.Backup.m_ArchiveName + L"\\" + incrementsDirName + L"\\" + newBackupArchiveFileNameBase + L".7z";

	StartUploadingForJob(job);
}

void BackupJobExecuter::MakeBackupArchiveFile(const TraversedEntries& actualData, const wstring& newBackupArchiveFileNameBase, const wstring& tmpWorkingDir, Archived7zBackup& archive7z)
{	
	wstring rarFile = tmpWorkingDir + L"\\" + newBackupArchiveFileNameBase + L".rar";
	ArchivedRarBackup archiveRar(actualData, tmpWorkingDir, newBackupArchiveFileNameBase, m_Reserver->m_SettingsIni.top()(L"Rar")[L"Password"]);
	archive7z.AssignAndCompress(archiveRar, tmpWorkingDir, newBackupArchiveFileNameBase, m_Reserver->m_SettingsIni.top()(L"7z")[L"Password"]);
	
}

void BackupJobExecuter::Extract(const wstring& tmpWorkingDir, const wstring& filename, const wstring& destDir)
{
	Archived7zBackup archive7z;	
	Archived7zBackup::CreateForExtraction(tmpWorkingDir + L"\\" + filename, archive7z);
	ArchivedRarBackup archiveRar;
	archive7z.ExtractRarBackup(m_Reserver->m_SettingsIni.top()(L"7z")[L"Password"], archiveRar);
	archiveRar.ExtractToDir(destDir, m_Reserver->m_SettingsIni.top()(L"Rar")[L"Password"]);
}

void BackupJobExecuter::PrintException(exception_ptr& e)
{
	try
	{
		rethrow_exception(e);
	}
	catch (BaseException& e)
	{
		WriteToConsole(wstring(L"BaseException: ") + e.ErrorMessage());
	}
	catch (range_error& e)
	{
		WriteToConsole(string("range_error: ") + e.what());
	}
	catch (std::exception& e)
	{
		WriteToConsole(string("std::exception: ") + e.what());
	}
	catch (...)
	{
		WriteToConsole(L"unknown exception");		
	}
}

BackupJob::BackupJob(BackupJobExecuter* backupJobExecuter, const wstring& tmpPath) : TmpWorkingDir(tmpPath)
{
	Functions.push([backupJobExecuter](Completion& complition)
	{
		BackupJob& job = *complition.job;
		backupJobExecuter->CreateBackup(job);		
	});

	//Functions.push([backupJobExecuter](BackupJob& job)
	//{
	//	
	//});

	Functions.push([backupJobExecuter](Completion& complition)
	{
		backupJobExecuter->StartRemoveOldArchives(complition);
	});
}
