#pragma once
#include "BackupBase.h"
#include "Storage.h"
#include "AbstrastIOManager.h"
#include "Exceptions.h"
#include "Completer.h"
#include "BackupJobExecuter.h"



class MyBCopy_API MyBCopyFile
{
public:
	//MyBCopyFile() = default;
	//MyBCopyFile() = default;
	Settings m_Settings;
	list<ParsedSources> m_Backups;
	bool LoadFromXMLFile(const wstring& srcFile);
	//bool SaveToFile(const wstring& srcFile);
private:
	class XmlLoader;
};


//
//class RestoringParams
//{
//public:
//	enum class OverwriteMode
//	{
//		Overwrite,
//		Skip,
//		DeleteAllAndWrite,//удалить все данные в целевой папке, затем распаковать в чистую
//	};
//	OverwriteMode m_OverwriteMode = OverwriteMode::Overwrite;
//	wstring m_OutputDir;
//	bool CreateDiskDirectory = true;
//
//	wstring m_StorageName;		
//	wstring m_BackupFileRel;//относительно хранилища
//
//	//wstring m_BackupTimestamp;
//};
//
//enum class ERectoreResult
//{
//	Success,
//	NoAnyBackupOnAllStorages,
//	NoSpecifiedStorage,
//	NoStorageOnSpecifiedStorage,
//};
//
//
//class RectoreResult
//{
//public:
//	RectoreResult() = default;
//	RectoreResult(ERectoreResult rectoreResult);
//	ERectoreResult m_RectoreResult;
//};






struct NotificatorSignature
{
	const MyBCopyFile* m_MyBCopyFile;
	const ParsedSources* m_ParsedSources;
};

//using ParsedSourcesIterator = list<ParsedSources>::iterator;

enum class EBackupStage
{
	//Queuing,
	//	Incremental
	Downloading,
	Extracting,
	//
	Archiving,
	Uploading		
};

enum class EStorageStage
{	
	Uploading,
	RemovingOld,
	PostUploadCheck,
	StorageFinish
};

enum class EBackupError
{
	NoError,
	WithErrors,
		
	NoFilesForBackup,
	PostUploadCheck,
	TimeStampExists,
	Download,
	Extraction,
	CreationArchive,
	NoChangesForIncrementalBackup,

	Settings_SrcPaths,
	Settings_TemporaryDir,
	Settings_Storages,
	Settings_Toolchain,
	Settings_ToolNoPassword,
	Settings_NoSystemRar,
	Settings_NoSystem7z,
	Settings_NoReferenceStorageName,

	Unknown,
};


struct Notificators
{
	using Progress = function<void(const ParsedSources&, EBackupStage)>;
	Progress m_ProgressNotificator;

	using Finish = function<void(const ParsedSources&, EBackupError)>;
	Finish m_FinishNotificator;

	using ErrorStorage = function<void(const ParsedSources&, const wstring& , EBackupError)>;
	ErrorStorage m_ErrorStorageNotificator;

	using ProgressStorage = function<void(const ParsedSources&, const wstring&, EStorageStage)>;
	ProgressStorage m_ProgressStorageNotificator;
};






class RegularSchemeHandler;

class BackupJob : public BaseJob, public Notificators
{
public:
	BackupJob(const ParsedSources& backup, const Settings& settings, const Notificators& notificators, Completer& completer);

	ParsedSources m_Backup;
	//Notificators m_Notificators;
			
	Settings m_Settings;

	Completer& m_Completer;

	shared_ptr<RegularSchemeHandler> m_Handler;	
	
	virtual void Start();
	void ValidateSettings();

protected:	
	void NotifyProgress(EBackupStage stage);
	void NotifyFinished(EBackupError error);

	void NotifyStorageError(const shared_ptr<Storage>& storage, EBackupError error);
	void NotifyStorageProgress(const shared_ptr<Storage>& storage, EStorageStage stage);

	virtual void ExceptionHandler(exception_ptr& exceptionPtr) override;
	void ExceptionStorageHandler(exception_ptr& exceptionPtr, const shared_ptr<Storage>& storage);

	void StorageFinished(const shared_ptr<Storage>& storage, bool success);

	size_t m_NumStorageSuccess;
	size_t m_NumStorageError;
};




class RegularSchemeHandler : protected BackupJob
{
public:
	RegularSchemeHandler(const BackupJob& backupJob, Completer& completer);
	virtual ~RegularSchemeHandler() = default;

	void StartBackup();
	//void Restore(const ParsedSources& backups, const RestoringParams& restoringParams = RestoringParams());	
protected:	
	void CreateTemporaryDir();
	void CreateRegularBackup();
	void StartUploading();
	virtual void CompleteUploading(shared_ptr<Storage>& storage);
	virtual void PostUploadCheck(shared_ptr<Storage>& storage);
	virtual void StartRemovingOldBackup(shared_ptr<Storage>& storage);
	

	wstring MakeBackupArchiveFile(const TraversedBackup& actualData, const wstring& newBackupArchiveFileNameBase);	
protected:
	wstring m_NowTimestamp;
	
	unique_ptr<TmpDir> m_TmpWorkingDir;
	wstring m_TmpBackupDir;

	wstring m_UploadSrcFile;
	wstring m_UploadDestFileRelative;
};




class IncrementalSchemeHandler : public RegularSchemeHandler
{
public:
	using RegularSchemeHandler::RegularSchemeHandler;
	bool TryStartBackup();
	//void Restore(const ParsedSources& backups, const RestoringParams& restoringParams = RestoringParams());	
protected:	
	void StartDownloading();
	void CreateIncrementBackup();
	
		
	virtual void PostUploadCheck(shared_ptr<Storage>& storage) override;

	void NewIncrement();
	void NewIncrementBranch();

	CopyList GetDownloadList() const;

	void Extract(const fs::path& filename, const wstring& oldContentDir, bool rarOverwrite = false, bool rarRestoreAbsPath = false, bool rarRestoreDisk = true);
private:		
	shared_ptr<Storage> m_ReferenceStorage;
	wstring m_RegularFileName;
	set<wstring> m_IncrementFileNames;
};


//class RestoreJob : public BaseJob
//{
//public:
//	using BaseJob::BaseJob;
//	ParsedSources m_Backup;
//	Settings m_Settings;
//	RestoringParams m_RestoringParams;
//
//	CopyList GetDownloadList() const override;
//};









class MyBCopy_API Reserver : private BackupJobExecuter
{
public:
	//Reserver();
	

	//const Settings& GetGlobalSettings() const;

	//template <template <typename T, typename ...TArgs> typename OutCont, template <typename T, typename ...TArgs> typename InCont>
	//static OutCont<MyBCopyFile> LoadMyBCopyFiles(const InCont<wstring>& files)
	//{
	//	OutCont<MyBCopyFile> result;
	//	for (auto& file : files)
	//	{
	//		/*MyBCopyFile myBCopyFile 
	//			result.push_back(move(myBCopyFile));*/
	//		*inserter(result, result.end()) = LoadMyBCopyFile(file);			
	//	}
	//	return result;
	//}	

	static bool IsUserSpecificXMLExist();
	static fs::path GetUserSpecificXMLPath();	
	static const fs::path scm_UserSpecificXMLFilename;


	/*void Backup(const list<MyBCopyFile>& files);
	void Restore(const list<MyBCopyFile>& files, const RestoringParams& restoringParams = RestoringParams());
	void Verify(const list<MyBCopyFile>& files);*/

	/*static Settings OverrideDefaultSettings(const ParsedSources& backup);*/
	

	void Backup(const MyBCopyFile& file, const Notificators& notificators = Notificators());
	//void Restore(const MyBCopyFile& file, const RestoringParams& restoringParams = RestoringParams());
	//void Verify(const MyBCopyFile& file);
			
	//void Backup(const list<ParsedSources>& backups);
	//void Restore(const list<ParsedSources>& backups, const RestoringParams& restoringParams = RestoringParams());
	//void Verify(const list<ParsedSources>& backups);

	//void Backup(const ParsedSources& backup);
	//RectoreResult Restore(const ParsedSources& backups, const RestoringParams& restoringParams = RestoringParams());
	//void Verify(const ParsedSources& backups);	

	void AddBackupJob(const ParsedSources& backup, const Settings& settings, const Notificators& notificators = Notificators());
	void AddBackupJobs(const MyBCopyFile& file, const Notificators& notificators = Notificators());
			
private:	
	static Settings OverrideDefaultSettings(const MyBCopyFile& myBCopyFile, const ParsedSources& backup);
	//list< RestoreJob > m_RestoreJobs;
		
	void Schtask();
	
	
		
	

	//void RestoreInternal(const MyBCopyFile& file, const RestoringParams& restoringParams);
	//RectoreResult RestoreInternal(const ParsedSources& backup, const RestoringParams& restoringParams);
	//RectoreResult ValidateRestoreParams(const ParsedSources& backup, const Settings& settings, RestoringParams& restoringParams);
	//void StartDownloadingForRestoring(RestoreJob& job);
	//void RestoreBackup(RestoreJob& job);	
};










class MyBCopy_API BackupException : public CachedMessageException
{
public:	
	BackupException(const wstring& backupName, EBackupError error);
	BackupException(const BackupBase& backup, EBackupError error);
	
	
	wstring m_BackupName;
	EBackupError m_Error;
protected:
	virtual void FormattedMessageLine() override;
};



