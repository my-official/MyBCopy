#include "MyBCopy_pch.h"
#include "Reserver.h"
#include "Utils.h"
#include "InternetIOManager.h"
#include "BackupJobExecuter.h"
#include "UtilsMyBCopy.h"
#include "UtilsWindows.h"
#include "XmlLoader.h"
#include "Tool.h"
#include "WebDavMountManager.h"



//const wstring DestListFile = "DestList.txt";

//const wstring VeraCryptContainerFilePath = "J:\\";





void Reserver::Schtask()
{
	//if (system(nullptr) == 0)
	//	throw EXCEPTION(ErrnoException());

	//wstring rarCmd = "schtasks /Create /SC DAILY /TN myBackup /TR /F  ";

	//wcout << rarCmd << endl;
	
	//if (system(rarCmd.c_str()) != 0)
	//	throw EXCEPTION(ErrnoException());
}







bool Reserver::IsUserSpecificXMLExist()
{
	return fs::exists(GetUserSpecificXMLPath());
}

fs::path Reserver::GetUserSpecificXMLPath()
{
	return GetCurrentUserHomePath() / scm_UserSpecificXMLFilename;
}


const fs::path Reserver::scm_UserSpecificXMLFilename = TEXT("MyReserveCopies.mybcopy");

Settings Reserver::OverrideDefaultSettings(const MyBCopyFile& myBCopyFile, const ParsedSources& backup)
{
	auto settings = Settings::Override(myBCopyFile.m_Settings, backup.m_Settings);

	if (settings.m_TemporaryDir.empty())
		settings.m_TemporaryDir = fs::temp_directory_path();

	if (!settings.m_SchemeParams)
	{
		settings.m_SchemeParams = make_shared<RegularScheme>();
		settings.m_SchemeParams->m_NumStoredOldRegularBackups = 0;
	}	

	for (auto& tool : settings.m_Toolchain)
	{
		if (tool->m_ExecutablePath.empty())
			tool->m_ExecutablePath = tool->GetSystemDefaultExecutablePath();
	}

	if (!settings.m_Verification.is_initialized())
		settings.m_Verification = false;
	
	return settings;
}

//void Reserver::Backup(const list<MyBCopyFile>& files)
//{
//	for (auto& file : files)
//		BackupInternal(file);
//	ExecuteAndClearAllJobs();
//}

void Reserver::Backup(const MyBCopyFile& file, const Notificators& notificators /*= Notificators()*/)
{	
	AddBackupJobs(file, notificators);
	Execute();
	WebDavMountManager::Instance().EraseUnused();
}

//void Reserver::Backup(const list<ParsedSources>& backups)
//{
//	for (auto& backup : backups)
//		BackupInternal(backup);
//	ExecuteAndClearAllJobs();
//}

//void Reserver::Backup(const ParsedSources& backup)
//{	
//	BackupInternal(backup);
//	ExecuteAndClearAllJobs();
//	
//}
void Reserver::AddBackupJobs(const MyBCopyFile& file, const Notificators& notificators /*= Notificators()*/)
{
	auto backups = file.m_Backups;
	for (auto& backup : file.m_Backups)
	{		
		try
		{				
			auto settings = OverrideDefaultSettings(file, backup);
			AddBackupJob(backup, settings, notificators);
		}
		catch (BackupException& e)
		{
			if (notificators.m_FinishNotificator)
			{
				notificators.m_FinishNotificator(backup, e.m_Error);
			}
		}
		catch (...)
		{
			if (notificators.m_FinishNotificator)
			{
				notificators.m_FinishNotificator(backup, EBackupError::Unknown);
			}
		}
	}
}

void Reserver::AddBackupJob(const ParsedSources& backup, const Settings& settings, const Notificators& notificators /*= Notificators()*/)
{	
	m_Jobs.push_back(make_shared<BackupJob>(backup, settings, notificators, m_Completer) );
}

//
//void Reserver::Verify(const list<MyBCopyFile>& backups)
//{
//	for (auto& backup : backups)
//		Verify(backup);
//}
//
//void Reserver::Verify(const MyBCopyFile& backups)
//{
//
//}
//
//void Reserver::Verify(const list<ParsedSources>& backups)
//{
//	for (auto& backup : backups)
//		Verify(backup);
//}
//
//void Reserver::Verify(const ParsedSources& backups)
//{
//
//}





//Reserver::Reserver() : m_BackupJobExecuter(this)
//{
//
//}



//
//const Settings& Reserver::GetGlobalSettings() const
//{
//	return m_GlobalSettings;
//}



void IncrementalSchemeHandler::NewIncrement()
{	
	auto lastBackupInfo = m_ReferenceStorage->GetLastBackupsInfo(m_Backup);

	m_RegularFileName = lastBackupInfo.LastRegularFilename;	
	m_IncrementFileNames = lastBackupInfo.LastIncrementFileRels;
}

void IncrementalSchemeHandler::NewIncrementBranch()
{
	auto lastBackupInfo = m_ReferenceStorage->GetLastBackupsInfo(m_Backup);
	m_RegularFileName = lastBackupInfo.LastRegularFilename;
}




//
//void Reserver::Restore(const list<MyBCopyFile>& files, const RestoringParams& restoringParams /*= RestoringParams()*/)
//{
//	for (auto& file : files)
//		RestoreInternal(file, restoringParams);
//	ExecuteAndClearAllJobs();
//}

//void Reserver::Restore(const MyBCopyFile& file, const RestoringParams& restoringParams /*= RestoringParams()*/)
//{	
//	RestoreInternal(file, restoringParams);
//	ExecuteAndClearAllJobs();
//	m_RestoreJobs.clear();
//}
//
//void Reserver::Restore(const list<ParsedSources>& backups, const RestoringParams& restoringParams /*= RestoringParams()*/)
//{
//	for (auto& backup : backups)
//		RestoreInternal(backup, restoringParams);
//	ExecuteAndClearAllJobs();
//	m_RestoreJobs.clear();
//}
//
//RectoreResult Reserver::Restore(const ParsedSources& backup, const RestoringParams& restoringParams /*= RestoringParams()*/)
//{
//	RestoreInternal(backup, restoringParams);
//	ExecuteAndClearAllJobs();
//	
//	return ERectoreResult::Success;
//}
//

//
//void Reserver::RestoreInternal(const MyBCopyFile& file, const RestoringParams& restoringParams)
//{
//	auto backups = file.m_Backups;
//	for (auto& backup : backups)
//	{
//		backup.m_Settings = Settings::Override(file.m_Settings, backup.m_Settings);
//		RestoreInternal(backup, restoringParams);
//	}	
//}
//
//RectoreResult Reserver::RestoreInternal(const ParsedSources& backup, const RestoringParams& _restoringParams)
//{
//	Settings overridableSettings = Settings::Override(DefaultSettings::Instance(), backup.m_Settings);
//
//	string paramName;
//	if (overridableSettings.HasUninitializedParam(&paramName))
//	{
//		throw EXCEPTION(SettingsException(backup, paramName) );
//	}
//
//	Settings settings(NowTimestampAsString(), overridableSettings);
//
//	ValidateSettings(backup, settings);
//
//	RestoringParams restoringParams = _restoringParams;
//
//	RectoreResult result = ValidateRestoreParams(backup, settings, restoringParams);
//
//	if (result.m_RectoreResult != ERectoreResult::Success)
//	{
//		return result;
//	}
//
//	auto storage = settings.GetStorageByName(restoringParams.m_StorageName);
//
//	LastRegularBackupInfo lastBackupInfo = storage->GetLastBackupsInfo(backup);
//
//	//wstring lastFile = lastBackupInfo.GetLastFile();
//
//
//	RestoreJob job(settings.m_TemporaryDir + L"\\" + backup.m_Name);
//	job.m_DownloadFromStorage = settings.GetStorageByName(restoringParams.m_StorageName);
//	job.m_RegularFileName = backup.m_Name + L"\\" + lastBackupInfo.LastRegularFilename;	
//	for (auto& incrementFile : lastBackupInfo.LastIncrementFileRels)
//	{
//		job.m_IncrementFileNames.insert(backup.m_Name + L"\\" + incrementFile);
//	}
//
//
//	job.m_Settings = settings;
//	job.m_RestoringParams = restoringParams;
//	
//	m_RestoreJobs.emplace_back(move(job));
//	StartDownloadingForRestoring(m_RestoreJobs.back());
//
//	return result;
//}

//RectoreResult Reserver::ValidateRestoreParams(const ParsedSources& backup, const Settings& settings, RestoringParams& restoringParams)
//{
//	if (restoringParams.m_StorageName.empty())
//	{
//		map<wstring, LastRegularBackupInfo> storageName2LastBackupInfo;
//		for (auto& storage : settings.m_Storages)
//		{
//			LastRegularBackupInfo lastBackupInfo = storage->GetLastBackupsInfo(backup);
//			if (lastBackupInfo.Empty())
//				continue;
//			storageName2LastBackupInfo[storage->m_Name] = lastBackupInfo;
//		}
//
//		if (storageName2LastBackupInfo.empty())
//		{
//			return ERectoreResult::NoAnyBackupOnAllStorages;
//		}
//
//		auto it = max_element(storageName2LastBackupInfo.begin(), storageName2LastBackupInfo.end(), [&](const pair<wstring, LastRegularBackupInfo>& lhs, const pair<wstring, LastRegularBackupInfo>& rhs)->bool
//		{
//			fs::file_time_type lhsMaxTime = lhs.second.GetLastBackupTime();
//			fs::file_time_type rhsMaxTime = rhs.second.GetLastBackupTime();
//			return lhsMaxTime < rhsMaxTime;
//		});
//		
//		restoringParams.m_StorageName = it->first;
//		restoringParams.m_BackupFileRel = it->second.GetLastFile();
//	}
//
//	auto storage = settings.GetStorageByName(restoringParams.m_StorageName);
//	if (!storage)
//	{
//		return ERectoreResult::NoSpecifiedStorage;
//	}
//
//	if (restoringParams.m_BackupFileRel.empty())
//	{
//		LastRegularBackupInfo lastBackupInfo = storage->GetLastBackupsInfo(backup);
//
//		if (lastBackupInfo.Empty())
//		{
//			return ERectoreResult::NoStorageOnSpecifiedStorage;
//		}
//
//		restoringParams.m_BackupFileRel = lastBackupInfo.GetLastFile();
//	}
//
//
//
//	return ERectoreResult::Success;
//}

//void Reserver::StartDownloadingForRestoring(RestoreJob& job)
//{	
//	Completion completion;	
//	completion.CallbackFunction = [this, &job](Completion& complition)
//	{
//		RestoreBackup(job);
//	};
//
//	if (job.m_DownloadFromStorage)
//	{
//		CopyList downloadList = job.GetDownloadList();
//
//		job.m_DownloadFromStorage->StartDownloading(downloadList, &m_Completer, completion);
//
//
//		m_Completer.RegisterWait();
//	}
//	else
//	{
//		m_Completer.RegisterWait();
//		m_Completer.CompleteSignaled(completion);
//	}
//}
//
//void Reserver::RestoreBackup(RestoreJob& job)
//{
//	wstring oldRegularArchiveFileName = job.m_RegularFileName;
//		
//	wstring oldExtractedDataDir = job.m_RestoringParams.m_OutputDir;
//
//	TraversedBackup backup;
//	static_cast<BackupBase&>(backup) = job.m_Backup;
//	backup.m_Settings = job.m_Settings;
//
//
//	Extract(job.m_TmpDir, job.m_RegularFileName, oldExtractedDataDir, backup, true, job.m_RestoringParams.m_OutputDir.empty(), job.m_RestoringParams.CreateDiskDirectory);
//		
//	for (auto it = job.m_IncrementFileNames.crbegin(); it != job.m_IncrementFileNames.crend(); ++it)
//	{
//		const wstring& incrementArchiveFileName = *it;
//		Extract(job.m_TmpDir, incrementArchiveFileName, oldExtractedDataDir, backup, true, job.m_RestoringParams.m_OutputDir.empty(), job.m_RestoringParams.CreateDiskDirectory);
//	}	
//}



//void Reserver::CreateVeraCryptContainer(wstring veraCryptContainerFile)
//{
//	//const wstring newPath = R"(C:\Program Files\VeraCrypt)";
//
//	//wstring cmd = "cd //D " + VeraCryptContainerFilePath + R"(" && "VeraCrypt Format.exe" /create ")" + veraCryptContainerFileName + R"(" /size 25M /password "a b û ) " /encryption Twofish /filesystem FAT /force /silent)";
//

//}



BackupException::BackupException(const wstring& backupName, EBackupError error)
	: m_BackupName(backupName)
	, m_Error(error)
{

}

BackupException::BackupException(const BackupBase& backup, EBackupError error)
	: m_BackupName(backup.m_Name)
	, m_Error(error)
{

}

void BackupException::FormattedMessageLine()
{
	TextLine(L"Backup name is " + m_BackupName + L". ErrorCode is " + to_wstring(int(m_Error)));
}




//
//RectoreResult::RectoreResult(ERectoreResult rectoreResult) : m_RectoreResult(rectoreResult)
//{
//
//}


//
//
//BaseJob::BaseJob(const wstring& tmpDirPath, const wstring& backupSubDirName /*= L""*/)
//	: m_TmpWorkingDir(tmpDirPath),
//	m_TmpDir(m_TmpWorkingDir.GetPath() + (backupSubDirName.empty() ? L"" : L"\\" + backupSubDirName))
//{
//	//Functions.push([backupJobExecuter](Completion& complition)
//	//{
//	//	BackupJob& job = *complition.job;
//	//	backupJobExecuter->CreateBackup(job);	
//	//	backupJobExecuter->StartUploadingForJob(job);
//	//});
//
//
//	/*Functions.push([backupJobExecuter](Completion& complition)
//	{
//	backupJobExecuter->StartRemoveOldArchives(complition);
//	});*/
//}


BackupJob::BackupJob(const ParsedSources& backup, const Settings& settings, const Notificators& notificators, Completer& completer)
	: Notificators(notificators)
	, m_Backup(backup)
	, m_Settings(settings)
	, m_Completer(completer)
	, m_NumStorageSuccess(0)
	, m_NumStorageError(0)
{

}

CopyList IncrementalSchemeHandler::GetDownloadList() const
{
	assert(!m_RegularFileName.empty());

	CopyList downloadList;

	wstring backupRemotePath = m_Backup.m_Name + L"\\";

	downloadList.push_back(make_pair(backupRemotePath + m_RegularFileName, m_TmpBackupDir + L"\\" + m_RegularFileName));

	for (auto& incrementFileName : m_IncrementFileNames)
	{
		downloadList.push_back(make_pair(backupRemotePath + incrementFileName, m_TmpBackupDir + L"\\" + fs::path(incrementFileName).filename().wstring()));
	}

	return downloadList;
}

void IncrementalSchemeHandler::Extract(const fs::path& filename, const wstring& oldContentDir, bool rarOverwrite /*= false*/, bool rarRestoreAbsPath /*= false*/, bool rarRestoreDisk /*= true*/)
{
	try
	{
		fs::path inFilename = filename;
		fs::path inFile = m_TmpBackupDir / inFilename;
		
		if (m_Settings.m_Toolchain.size() > 1)
		{
			auto rend_it = --m_Settings.m_Toolchain.rend();

			for (auto tool_it = m_Settings.m_Toolchain.rbegin(); tool_it != rend_it; ++tool_it)
			{
				auto& tool = *tool_it;

				inFilename = wstring(filename.stem()) + tool->GetFileTypeExt();
				inFile = m_TmpBackupDir / inFilename;

				fs::path inInFilename = L"in_" + wstring(filename.stem()) + tool->GetFileTypeExt();
				fs::path inInFile = m_TmpBackupDir / inInFilename;

				fs::rename(inFile, inInFile);
				tool->Decrypt(inInFile, m_TmpBackupDir);				
				fs::remove(inInFile);			
			}

			inFilename = wstring(filename.stem()) + m_Settings.m_Toolchain.front()->GetFileTypeExt();
			inFile = m_TmpBackupDir / inFilename;
		}

		{
			auto& tool = m_Settings.m_Toolchain.front();
			tool->DecryptAndDecompress(inFile, oldContentDir);		
			fs::remove(inFile);
		}

	}
	catch (...)
	{
		throw EXCEPTION(BackupException(m_Backup, EBackupError::Extraction));
	}
}

void BackupJob::Start()
{		
	ValidateSettings();
	
	RegularScheme* scheme = m_Settings.m_SchemeParams.get();
	IncrementalScheme* incrementalScheme = dynamic_cast<IncrementalScheme*>(scheme);
		
	if (incrementalScheme)
	{
		auto incrementalHandler = make_shared<IncrementalSchemeHandler>(*this, m_Completer);
		if (incrementalHandler->TryStartBackup())
		{
			m_Handler = incrementalHandler;
			return;
		}
	}
	
	m_Handler = make_shared<RegularSchemeHandler>(*this, m_Completer);
	m_Handler->StartBackup();
}

void BackupJob::ValidateSettings()
{
	if (m_Backup.m_SrcPaths.empty())
	{
		throw EXCEPTION(BackupException(m_Backup, EBackupError::Settings_SrcPaths));
	}

	if (m_Settings.m_TemporaryDir.empty())
	{
		throw EXCEPTION(BackupException(m_Backup, EBackupError::Settings_TemporaryDir));
	}

	if (!fs::exists(m_Settings.m_TemporaryDir))
	{
		throw EXCEPTION(BackupException(m_Backup, EBackupError::Settings_TemporaryDir));
	}

	if (m_Settings.m_Storages.empty())
	{
		throw EXCEPTION(BackupException(m_Backup, EBackupError::Settings_Storages));
	}

	if (m_Settings.m_Toolchain.empty())
	{
		throw EXCEPTION(BackupException(m_Backup, EBackupError::Settings_Toolchain));
	}

	for (auto& tool : m_Settings.m_Toolchain)
	{
		if (tool->m_Password.empty())
			throw EXCEPTION(BackupException(m_Backup, EBackupError::Settings_ToolNoPassword));

		if (tool->m_ExecutablePath.empty())
			throw EXCEPTION(BackupException(m_Backup, tool->ErrorCode_NoExecutable()));
	}
}

//CopyList RestoreJob::GetDownloadList() const
//{
//	assert(!m_RegularFileName.empty());
//
//	CopyList downloadList;
//
//	wstring backupRemotePath = m_Settings.m_BackupDirName + L"\\" + m_Backup.m_Name + L"\\";
//
//	downloadList.push_back(make_pair(backupRemotePath + m_RegularFileName, m_TmpDir + L"\\" + m_RegularFileName));
//
//	for (auto& incrementFileName : m_IncrementFileNames)
//	{
//		downloadList.push_back(make_pair(backupRemotePath + incrementFileName, m_TmpDir + L"\\" + incrementFileName));
//	}
//
//	return downloadList;
//}



RegularSchemeHandler::RegularSchemeHandler(const BackupJob& backupJob, Completer& completer)
	: BackupJob(backupJob)
	, m_NowTimestamp(NowTimestampAsString())
{

}

void RegularSchemeHandler::StartBackup()
{
	CreateTemporaryDir();
	NotifyProgress(EBackupStage::Archiving);
	CreateRegularBackup();
	StartUploading();
}



//
//void RegularSchemeHandler::Restore(const ParsedSources& backups, const RestoringParams& restoringParams /*= RestoringParams()*/)
//{
//
//}



void RegularSchemeHandler::StartUploading()
{
	NotifyProgress(EBackupStage::Uploading);	

	CopyList upload;
	upload.emplace_back(m_UploadSrcFile, m_UploadDestFileRelative);

	for (auto& storage : m_Settings.m_Storages)
	{	
		NotifyStorageProgress(storage, EStorageStage::Uploading);		 
				
		Completion completion(
			[this, &storage]()
			{
				CompleteUploading(storage);
			},
			[this, storage](exception_ptr& exceptionPtr)
			{
				ExceptionStorageHandler(exceptionPtr, storage);
			});
		
		auto enumeratedList = storage->GetEnumeratedBackupFiles(m_Backup);

		if (enumeratedList.HasFileWithTimeStamp(m_NowTimestamp))
		{
			NotifyStorageError(storage, EBackupError::TimeStampExists);
			StorageFinished(storage, false);
			continue;
		}			

		storage->StartUploading(upload, &m_Completer,completion);
		m_Completer.RegisterWait();
	}
}



void RegularSchemeHandler::CompleteUploading(shared_ptr<Storage>& storage)
{
	PostUploadCheck(storage);
	StartRemovingOldBackup(storage);	
}

void RegularSchemeHandler::CreateRegularBackup()
{
	wstring newBackupArchiveFileNameBase = MakeRegularArchiveFileBaseName(m_NowTimestamp);

	TraversedBackup traversedBackup = TraversedBackup::CreateViaTraverseParsedBackup(m_Backup, m_Settings);

	if (traversedBackup.Empty())
	{
		throw EXCEPTION(BackupException(m_Backup, EBackupError::NoFilesForBackup));
	}

	wstring srcFilename = MakeBackupArchiveFile(traversedBackup, newBackupArchiveFileNameBase);	

	m_UploadSrcFile = m_TmpBackupDir + L"\\" + srcFilename;
	m_UploadDestFileRelative = m_Backup.m_Name + L"\\" + srcFilename;
}

void RegularSchemeHandler::StartRemovingOldBackup(shared_ptr<Storage>& storage)
{
	NotifyStorageProgress(storage, EStorageStage::RemovingOld);

	auto enumeratedList = storage->GetEnumeratedBackupFiles(m_Backup);

	unsigned int numStoredOldRegularBackups = static_pointer_cast<RegularScheme>(m_Settings.m_SchemeParams)->m_NumStoredOldRegularBackups;

	auto removeList = MakeRemoveListForOldRegularBackup(enumeratedList, numStoredOldRegularBackups);

	if (removeList.empty())
	{
		NotifyStorageProgress(storage, EStorageStage::StorageFinish);
		StorageFinished(storage, true);
		return;
	}

	for (auto& file : removeList)
	{
		file = m_Backup.m_Name + L"\\" + file;
	}

	Completion completion(
				[this, &storage]()
			{
				NotifyStorageProgress(storage, EStorageStage::StorageFinish);
				StorageFinished(storage, true);
			},

				[this, storage](exception_ptr& exceptionPtr)
			{
				ExceptionStorageHandler(exceptionPtr, storage);
			});

	storage->StartRemoving(removeList, &m_Completer, completion);
	m_Completer.RegisterWait();
}

void RegularSchemeHandler::PostUploadCheck(shared_ptr<Storage>& storage)
{
	NotifyStorageProgress(storage, EStorageStage::PostUploadCheck);
		
	auto fileList = storage->GetLastBackupsInfo(m_Backup);
	wstring lastRegularFileAtStorage = m_Backup.m_Name + L"\\" + fileList.LastRegularFilename;
	
	if (lastRegularFileAtStorage != m_UploadDestFileRelative)
		throw EXCEPTION(BackupException(m_Backup, EBackupError::PostUploadCheck));
}

std::wstring RegularSchemeHandler::MakeBackupArchiveFile(const TraversedBackup& actualData, const wstring& newBackupArchiveFileNameBase)
{
	try
	{
		wstring inFilename;
		wstring inFile;

		auto tool_it = m_Settings.m_Toolchain.begin();
		{
			auto& tool = *tool_it;
			inFilename = newBackupArchiveFileNameBase + tool->GetFileTypeExt();
			inFile = m_TmpBackupDir + L"\\" + inFilename;
			tool->CompressAndEncrypt(actualData.ToFileList(), m_TmpBackupDir, inFile);
		}


		for (++tool_it; tool_it != m_Settings.m_Toolchain.end(); ++tool_it)
		{
			auto& tool = *tool_it;

			const wstring outFilename = L"out_" + newBackupArchiveFileNameBase + tool->GetFileTypeExt();
			const wstring outFile = m_TmpBackupDir + L"\\" + outFilename;

			tool->Encrypt(inFile, outFile);

			fs::remove(inFile);

			inFilename = newBackupArchiveFileNameBase + tool->GetFileTypeExt();
			inFile = m_TmpBackupDir + L"\\" + inFilename;
			fs::rename(outFile, inFile);
		}

		return inFilename;

	}
	catch (...)
	{
		throw EXCEPTION(BackupException(m_Backup, EBackupError::CreationArchive));
	}
}



void BackupJob::NotifyFinished(EBackupError error)
{
	if (m_FinishNotificator)
	{
		m_FinishNotificator(m_Backup, error);
	}
}

void BackupJob::NotifyProgress(EBackupStage stage)
{
	if (m_ProgressNotificator)
	{
		m_ProgressNotificator(m_Backup, stage);
	}
}

void BackupJob::NotifyStorageError(const shared_ptr<Storage>& storage, EBackupError error)
{
	if (m_ErrorStorageNotificator)
	{
		m_ErrorStorageNotificator(m_Backup, storage->m_Name, error);
	}
}

void BackupJob::NotifyStorageProgress(const shared_ptr<Storage>& storage, EStorageStage stage)
{
	if (m_ProgressStorageNotificator)
	{
		m_ProgressStorageNotificator(m_Backup, storage->m_Name, stage);
	}
}

void BackupJob::ExceptionHandler(exception_ptr& exceptionPtr)
{
	try
	{
		rethrow_exception(exceptionPtr);
	}	
	catch (BackupException& e)
	{
		NotifyFinished(e.m_Error);
	}
	catch (...)
	{
		NotifyFinished(EBackupError::Unknown);		
	}
}


void BackupJob::ExceptionStorageHandler(exception_ptr& exceptionPtr, const shared_ptr<Storage>& storage)
{
	try
	{
		rethrow_exception(exceptionPtr);
	}	
	catch (BackupException& e)
	{
		NotifyStorageError(storage,e.m_Error);
	}
	catch (...)
	{
		NotifyStorageError(storage, EBackupError::Unknown);		
	}
	StorageFinished(storage, false);
}

void BackupJob::StorageFinished(const shared_ptr<Storage>& storage, bool success)
{
	if (success)			
		++m_NumStorageSuccess;	
	else
		++m_NumStorageError;

	if (m_NumStorageSuccess + m_NumStorageError == m_Settings.m_Storages.size())
	{
		if (m_NumStorageError > 0)
			NotifyFinished(EBackupError::WithErrors);
		else
			NotifyFinished(EBackupError::NoError);
	}
}

void RegularSchemeHandler::CreateTemporaryDir()
{
	m_TmpWorkingDir = make_unique<TmpDir>(m_Settings.m_TemporaryDir + L"\\" + m_Backup.m_Name);
	m_TmpBackupDir = m_TmpWorkingDir->GetPath();
}

bool IncrementalSchemeHandler::TryStartBackup()
{	
	const IncrementalScheme*  incrementalScheme = static_cast<const IncrementalScheme*>(m_Settings.m_SchemeParams.get());
	if (incrementalScheme->m_NumIncrementBranches == 0)
		return false;

	m_ReferenceStorage = m_Settings.GetStorageByName(incrementalScheme->m_ReferenceStorageName);

	if (!m_ReferenceStorage)
		throw EXCEPTION(BackupException(m_Backup, EBackupError::Settings_NoReferenceStorageName));	
		
	auto enumeratedList = m_ReferenceStorage->GetEnumeratedBackupFiles(m_Backup);

	if (enumeratedList.Empty())
		return false;
			

	auto numBranches = m_ReferenceStorage->GetNumBranchesFromLastRegular(m_Backup);

	if (incrementalScheme->m_NumIncrementBranches < numBranches)
		return false;
	
	
	auto lenIncrementChain = m_ReferenceStorage->GetLastChainLengthFromLastRegular(m_Backup);

	if (incrementalScheme->m_LengthIncrementChain > lenIncrementChain || incrementalScheme->m_LengthIncrementChain == 0)
	{		
		NewIncrement();
		StartDownloading();
	}
	else
	{
		if (incrementalScheme->m_NumIncrementBranches > numBranches || incrementalScheme->m_NumIncrementBranches == 0)
		{
			NewIncrementBranch();
			StartDownloading();
		}
		else
		{
			return false;
		}
	}
	
	return true;
}



void IncrementalSchemeHandler::StartDownloading()
{
	CreateTemporaryDir();
	NotifyProgress(EBackupStage::Downloading);

	Completion completion(
		[this]()
		{
			CreateIncrementBackup();
			StartUploading();
		},

		[this](exception_ptr& exceptionPtr)
		{
			ExceptionHandler(exceptionPtr);		
		});

	assert(m_ReferenceStorage);

	CopyList downloadList = GetDownloadList();
	m_ReferenceStorage->StartDownloading(downloadList, &m_Completer, completion);
	m_Completer.RegisterWait();
}

void IncrementalSchemeHandler::CreateIncrementBackup()
{
	NotifyProgress(EBackupStage::Extracting);	

	wstring oldContentDir = m_TmpBackupDir + L"\\content";
	if (!fs::create_directories(oldContentDir))
	{
		throw EXCEPTION(BackupException(m_Backup, EBackupError::CreationArchive));
	}

	for (auto it = m_IncrementFileNames.crbegin(); it != m_IncrementFileNames.crend(); ++it)
	{
		const wstring& incrementArchiveFileName = *it;
		Extract(incrementArchiveFileName, oldContentDir);
	}
		
	Extract(m_RegularFileName, oldContentDir);
	
	TraversedBackup oldData = TraversedBackup::CreateViaTraverseExtractionDir(oldContentDir);


	NotifyProgress(EBackupStage::Archiving);

	TraversedBackup traversedBackup = TraversedBackup::CreateViaTraverseParsedBackup(m_Backup, m_Settings);

	if (traversedBackup.Empty())
	{
		throw EXCEPTION(BackupException(m_Backup, EBackupError::NoFilesForBackup));
	}

	TraversedBackup actualData = CompareBackups(oldData, traversedBackup);

	if (actualData.Empty())
	{
		throw EXCEPTION(BackupException(m_Backup, EBackupError::NoChangesForIncrementalBackup));		
	}
	
	wstring newBackupArchiveFileNameBase = MakeIncrementArchiveFileBaseName(m_NowTimestamp);
	wstring srcFilename = MakeBackupArchiveFile(actualData, newBackupArchiveFileNameBase);		
	
	_force_remove_all(oldContentDir);

	wstring incrementsDirName;
	if (m_IncrementFileNames.empty())
	{
		incrementsDirName = MakeIncrementArchivesDirName(m_NowTimestamp, ExtractTimestampFromFilename(m_RegularFileName));
	}
	else
	{
		wstring firstIncrementTimeStamp = ExtractTimestampFromFilename(fs::path(*m_IncrementFileNames.begin()).filename());
		incrementsDirName = MakeIncrementArchivesDirName(firstIncrementTimeStamp, ExtractTimestampFromFilename(m_RegularFileName));
	}

	m_UploadSrcFile = m_TmpBackupDir + L"\\" + srcFilename;
	m_UploadDestFileRelative = m_Backup.m_Name + L"\\" + incrementsDirName + L"\\" + srcFilename;
}




void IncrementalSchemeHandler::PostUploadCheck(shared_ptr<Storage>& storage)
{
	NotifyStorageProgress(storage, EStorageStage::PostUploadCheck);

	LastRegularBackupInfo lastBackupInfo = storage->GetLastBackupsInfo(m_Backup);
	if (lastBackupInfo.LastIncrementFileRels.empty())
		throw EXCEPTION(BackupException(m_Backup, EBackupError::PostUploadCheck));

	wstring lastIncrementFileAtStorage = m_Backup.m_Name + L"\\" + *lastBackupInfo.LastIncrementFileRels.rbegin();

	if (lastIncrementFileAtStorage != m_UploadDestFileRelative)
		throw EXCEPTION(BackupException(m_Backup, EBackupError::PostUploadCheck));
}



bool MyBCopyFile::LoadFromXMLFile(const wstring& srcFile)
{
	try
	{
		MyBCopyFile result = XmlLoader(srcFile).Load();
		*this = result;
		return true;
	}
	catch (...)
	{
		return false;
	}
}
//
//bool MyBCopyFile::SaveToFile(const wstring& srcFile)
//{
//	try
//	{		
//		return SaveMyBCopyFile(srcFile, *this);;
//	}
//	catch (...)
//	{
//		return false;
//	}
//}
//
