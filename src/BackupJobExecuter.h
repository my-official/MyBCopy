#pragma once

#include "BackupBase.h"
#include "Storage.h"
#include "Completer.h"
#include "Utils.h"


struct BackupJob
{
	ParsedBackup Backup;
	TmpDir TmpWorkingDir;

	shared_ptr<Storage> DownloadFromStorage;
	wstring RegularFileName;
	wstring DeltaFileName;
	set<wstring> IncrementFileNames;

	list< shared_ptr<Storage> > UploadToStorages;

	wstring UploadSrcFile;
	wstring UploadDestFileRelative;
	Archived7zBackup Archived7z;

	//exception_ptr ExceptionPtr;

	//enum class EStage
	//{
	//	Enumerate,
	//	Download,
	//	Upload,
	//	RemoveOld
	//};

	//EStage Stage;

	using BackupFunc = function<void(Completion& complition)>;
	
	queue<BackupFunc> Functions;

	BackupJob(BackupJobExecuter* backupJobExecuter, const wstring& tmpPath);
};

class Reserver;

class BackupJobExecuter
{
public:
	BackupJobExecuter(Reserver* reserver);
	bool operator()();
public:
	Reserver* GetReserver() const;
	
private:
	Reserver* m_Reserver;
	list< BackupJob >& m_BackupJobs;
	
public:
	Completer m_Completer;
private:
	list< BackupJob* >								m_DownloadList;
	list< pair<BackupJob*, shared_ptr<Storage> > >	m_UploadingList;
	list< pair<BackupJob*, shared_ptr<Storage> > >	m_RemoveOldList;



	friend struct BackupJob;

	void StartDownloadingAllJobs();	
	void StartDownloadingForJob(BackupJob& job);
	void StartUploadingForJob(BackupJob& job);
	void StartRemoveOldArchives(Completion& completion);

	
	void CreateBackup(BackupJob& job);
	
	void CreateRegularBackup(BackupJob& job);
	void CreateDeltaBackup(BackupJob& job);
	void CreateIncrementalBackup(BackupJob& job);

	
	void MakeBackupArchiveFile(const TraversedEntries& actualData, const wstring& newBackupArchiveFileNameBase, const wstring& tmpWorkingDir, Archived7zBackup& archive7z);
	void Extract(const wstring& tmpWorkingDir, const wstring& filename, const wstring& destDir);

	void PrintException(exception_ptr& e);
};
