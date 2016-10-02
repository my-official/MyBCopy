#include "stdafx.h"
#include "Reserver.h"
#include "Utils.h"
#include "InternetIOManager.h"
#include "BackupJobExecuter.h"
#include "UtilsMyRCopy.h"


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






void Reserver::RegularBackups()
{
	for (auto& backup : m_Backups)
	{
		BackupJob backupJob(&m_BackupJobExecuter, GeneralSettings::Instance().m_TemporaryDir + L"\\" + backup.m_ArchiveName);
		backupJob.Backup = backup;		
		backupJob.UploadToStorages = m_Storages;
		
		m_BackupJobs.emplace_back(move(backupJob));
	}	
}

void Reserver::DeltaBackup_SpecificPerStorage()
{
	for (auto& storage : m_Storages)
	{
		for (auto& backup : m_Backups)
		{
			DeltaBackup_Internal(storage, backup);			
		}
	}
}

void Reserver::DeltaBackup_UseReferenceStorage()
{
	auto referenceStorage_it = GetReferenceStorageIterator();

	shared_ptr<Storage>& referenceStorage = *referenceStorage_it;

	for (auto& backup : m_Backups)
	{
		DeltaBackup_Internal(referenceStorage, backup);			
	}
}


void Reserver::DeltaBackup_Internal(shared_ptr<Storage>& storage, ParsedBackup& backup)
{
	BackupJob backupJob(&m_BackupJobExecuter, GeneralSettings::Instance().m_TemporaryDir + L"\\" + backup.m_ArchiveName);
	backupJob.Backup = backup;

	auto enumeratedList = storage->GetEnumeratedBackupFiles(backup);

	if (!enumeratedList.Empty())
	{
		auto numOfDeltas = storage->GetNumOfDeltaFromLastRegular(backup);



		if (GeneralSettings::Instance().m_MaxDeltaBackups >= numOfDeltas)
		{
			auto numOfDeltaIncrements = storage->GetNumOfDeltaIncrementsOfLastDeltaFromLastRegular(backup);

			if (GeneralSettings::Instance().m_MaxDeltaIncrements > numOfDeltaIncrements)
			{
				AddForCreation_IncrementalBackup(storage, backup, backupJob);
			}
			else
			{
				if (GeneralSettings::Instance().m_MaxDeltaBackups > numOfDeltas)
				{
					AddForCreation_DeltaBackup(storage, backup, backupJob);
				}
				else
				{
					//AddForCreation_RegularBackup(backup);
				}				
			}			
		}
		else
		{
			//AddForCreation_RegularBackup();				
		}
	}

	backupJob.UploadToStorages = m_Storages;
	m_BackupJobs.emplace_back(move(backupJob));
}

std::list< std::shared_ptr<Storage> >::iterator Reserver::GetReferenceStorageIterator()
{
	auto referenceStorage_it = find_if(m_Storages.begin(), m_Storages.end(), [&](const shared_ptr<Storage>& a)->bool
	{
		return a->m_Name == GeneralSettings::Instance().m_ReferenceStorage;
	});

	if (referenceStorage_it == m_Storages.end())
		throw EXCEPTION( SettingsError(L"No reference storage") );
	return referenceStorage_it;
}


void Reserver::LoadStorage(const wstring& storageName, const INI::WLevel& section)
{
	const wstring& storageType = section.values.at(L"StorageType");

	if (storageType == L"WebDav")
	{		
		auto hlResult = make_shared<WebDavStorage>(this);
		hlResult->m_Name = storageName;
		hlResult->m_UserName = section.values.at(L"UserName");
		hlResult->m_Password = section.values.at(L"Password");
		hlResult->m_Url = section.values.at(L"Url");

		m_Storages.emplace_back(move(hlResult));
		return;
	}

	if (storageType == L"LocalDisk")
	{
		auto hlResult = make_shared<LocalDiskStorage>(this);
		hlResult->m_Name = storageName;
		hlResult->m_Path = section.values.at(L"Path");

		m_Storages.emplace_back(move(hlResult));
		return;
	}	
}




void Reserver::LoadStorages()
{
	auto& iniSections = m_SettingsIni.top().sections;

	for (auto& section : iniSections)
	{
		const wstring& sectionName = section.first;
		if (sectionName == L"General")
			continue;

		if (!section.second.values.count(L"StorageType"))
			continue;

		LoadStorage(sectionName,section.second);	
	}	
}



bool Reserver::Process()
{
	if (_wsystem(nullptr) == 0)
		throw EXCEPTION(ErrnoException());

	GeneralSettings::Instance().Load(m_SettingsIni);
	LoadStorages();

	if (m_Storages.empty())
	{
		WriteToConsole(L"No files for backup");
		return true;
	}

	ParsedBackup::LoadBackupSourcesFromFile(m_SettingsIni.top()(L"General")[L"SourcesListFile"], m_Backups);
	
	if (m_Backups.empty())
	{
		WriteToConsole(L"No files for backup");		
		return true;
	}

	AddPath(m_SettingsIni.top()(L"Rar")[L"DirPath"]);
	AddPath(m_SettingsIni.top()(L"7z")[L"DirPath"]);
		
	WriteToConsole(wstring(L"TemporaryDir is ") + GeneralSettings::Instance().m_TemporaryDir);
	WriteToConsole(wstring(L"MaxNumOfOldArchives ") + to_wstring(GeneralSettings::Instance().m_MaxNumOfOldArchives) );
	
	if (!fs::exists(GeneralSettings::Instance().m_TemporaryDir))
	{
		WriteToConsole(L"TemporaryDir not found");		
		throw EXCEPTION(WinException());
	}

	

	if (GeneralSettings::Instance().m_MaxDeltaBackups > 0)
	{
		if (!GeneralSettings::Instance().m_ReferenceStorage.empty())
		{
			//вариант 1
			//Эталонное хранилище
			DeltaBackup_UseReferenceStorage();
		}
		else
		{	//вариант 2
			//Создание разностного бекапа для каждого хранилища
			DeltaBackup_SpecificPerStorage();
		}
	}
	else
	{
		RegularBackups();
	}

	
	return m_BackupJobExecuter();
}



Reserver::Reserver()
	: m_SettingsIni( (GeneralSettings::Instance().m_ExePath + L"\\Settings.ini").c_str() ),
	m_BackupJobExecuter(this)
{
}


Reserver::~Reserver()
{
}



void Reserver::AddForCreation_IncrementalBackup(shared_ptr<Storage>& storage, ParsedBackup& backup, BackupJob& backupJob)
{	
	backupJob.DownloadFromStorage = storage;

	auto lastBackupInfo = storage->GetLastBackupsInfo(backup);

	backupJob.RegularFileName = lastBackupInfo.LastRegularFilename;
	backupJob.DeltaFileName = lastBackupInfo.LastDeltaFilename;
	backupJob.IncrementFileNames = lastBackupInfo.IncrementFilenames;
}

void Reserver::AddForCreation_DeltaBackup(shared_ptr<Storage>& storage, ParsedBackup& backup, BackupJob& backupJob)
{
	backupJob.DownloadFromStorage = storage;

	auto lastBackupInfo = storage->GetLastBackupsInfo(backup);

	backupJob.RegularFileName = lastBackupInfo.LastRegularFilename;	
}

ParseException::ParseException(const wstring& file, size_t line) : m_File(file) , m_Line(line)
{

}

void ParseException::FormattedMessageLine()
{
	TextLine(L"Parse error at " + m_File + L":" + to_wstring(m_Line));
}



//void Reserver::CreateVeraCryptContainer(wstring veraCryptContainerFile)
//{
//	//const wstring newPath = R"(C:\Program Files\VeraCrypt)";
//
//	//wstring cmd = "cd //D " + VeraCryptContainerFilePath + R"(" && "VeraCrypt Format.exe" /create ")" + veraCryptContainerFileName + R"(" /size 25M /password "a b ы ) " /encryption Twofish /filesystem FAT /force /silent)";
//

//}

SettingsError::SettingsError(const wstring& msg)
{
	TextLine(msg);
}




void GeneralSettings::Load(INI::WParser& settingsIni)
{
	m_MaxNumOfOldArchives = stoul( settingsIni.top()(L"General")[L"MaxNumOfOldArchives"] );
	m_MinNumOfOldRegularArchives = stoul(settingsIni.top()(L"General")[L"MinNumOfOldRegularArchives"]);
	m_CloudRCopyDirName = settingsIni.top()(L"General")[L"CloudRCopyDirName"];
	m_CloudShareDisk = settingsIni.top()(L"General")[L"CloudShareDisk"];
	m_TemporaryDir = settingsIni.top()(L"General")[L"TemporaryDir"];
	m_NowTimestamp = NowTimestampAsString();
	m_ReferenceStorage = settingsIni.top()(L"General")[L"ReferenceStorage"];
	m_MaxDeltaBackups = stoul(settingsIni.top()(L"General")[L"MaxDeltaBackups"]);

	m_MaxDeltaIncrements = stoul(settingsIni.top()(L"General")[L"MaxDeltaIncrements"]);
}

GeneralSettings::GeneralSettings() : m_ExePath(GetThisProgramExeDirPath())
{
	INI::WParser settingsIni((m_ExePath + L"\\Settings.ini").c_str());	
	Load(settingsIni);
}

