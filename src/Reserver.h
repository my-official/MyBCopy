#pragma once
#include "BackupBase.h"
#include "Storage.h"
#include "AbstrastIOManager.h"







enum class DeltaBackupMode
{
	Incremental,
	FromRegularOnly
};

class GeneralSettings : public Singleton<GeneralSettings>
{
public:
	GeneralSettings();

	wstring m_ExePath;
	unsigned int m_MaxNumOfOldArchives;
	unsigned int m_MinNumOfOldRegularArchives;
	wstring m_CloudRCopyDirName;
	wstring m_CloudShareDisk;
	
	wstring m_TemporaryDir;
	wstring m_NowTimestamp;
	wstring m_ReferenceStorage;

	unsigned int m_MaxDeltaBackups;
	unsigned int m_MaxDeltaIncrements;	


	void Load(INI::WParser& settingsIni);
	
};

class Reserver
{
public:
	Reserver();
	virtual ~Reserver();

	
	bool Process();
		
	INI::WParser m_SettingsIni;

	deque<ParsedBackup> m_Backups;
	list< shared_ptr<Storage> > m_Storages;
	list< shared_ptr<Storage> >::iterator m_ReferenceStorageIterator;

	list< BackupJob > m_BackupJobs;
	BackupJobExecuter m_BackupJobExecuter;
private:
	void LoadStorages();
	void LoadStorage(const wstring& storageName, const INI::WLevel& section);


	void Schtask();
		
	void RegularBackups();
	void DeltaBackup_SpecificPerStorage();
	void DeltaBackup_UseReferenceStorage();	
	void DeltaBackup_Internal(shared_ptr<Storage>& storage, ParsedBackup& backup);

	list< shared_ptr<Storage> >::iterator GetReferenceStorageIterator();

		

	void AddForCreation_RegularBackup(shared_ptr<Storage>& storage, ParsedBackup& backup)	{	}
	void AddForCreation_IncrementalBackup(shared_ptr<Storage>& storage, ParsedBackup& backup, BackupJob& backupJob);
	void AddForCreation_DeltaBackup(shared_ptr<Storage>& storage, ParsedBackup& backup, BackupJob& backupJob);
		
};


class ParseException : public CachedMessageException
{
public:
	ParseException(const wstring& file, size_t line);

	wstring m_File;
	size_t m_Line;
protected:
	virtual void FormattedMessageLine() override;
};

class SettingsError : public CachedMessageException
{
public:
	SettingsError(const wstring& msg);
protected:
	virtual void FormattedMessageLine()
	{
	}
};



