#include "stdafx.h"
#include "Storage.h"
#include "LocalDiskIOManager.h"
#include "InternetIOManager.h"
#include "Utils.h"
#include "UtilsMyRCopy.h"


unsigned int Storage::GetNumOfDeltaFromLastRegular(const BackupBase& backup) const
{
	auto enumeratedRegulars = GetEnumeratedBackupFiles(backup);

	if (enumeratedRegulars.Regular2Deltas.empty())
		return 0;

	auto& enumeratedDeltas = enumeratedRegulars.Regular2Deltas.rbegin()->second;
	
	return enumeratedDeltas.Delta2Increments.size();
}

unsigned int Storage::GetNumOfDeltaIncrementsOfLastDeltaFromLastRegular(const BackupBase& backup) const
{
	auto enumeratedRegulars = GetEnumeratedBackupFiles(backup);

	if (enumeratedRegulars.Regular2Deltas.empty())
		return 0;

	auto& enumeratedDeltas = enumeratedRegulars.Regular2Deltas.rbegin()->second;

	if (enumeratedDeltas.Delta2Increments.empty())
		return 0;

	auto& enumeratedIncrements = enumeratedDeltas.Delta2Increments.rbegin()->second.IncrementBackupFileRels;

	return enumeratedIncrements.size();

}

LastRegularBackupInfo Storage::GetLastBackupsInfo(const BackupBase& backup) const
{
	return LastRegularBackupInfo::CreateFromBackupInfoList(GetEnumeratedBackupFiles(backup));
}

RegularBackupList Storage::GetEnumeratedBackupFiles(const BackupBase& backup) const
{
	auto it = m_BackupName2RegularBackupListCache.find(backup.m_ArchiveName);

	if (it == m_BackupName2RegularBackupListCache.end())
	{
		auto& enumed = m_BackupName2RegularBackupListCache[backup.m_ArchiveName] = DoGetEnumeratedBackupFiles(backup);
		return enumed;
	}
	else
		return it->second;
}

void Storage::UpdateCache(const BackupBase& backup)
{
	m_BackupName2RegularBackupListCache[backup.m_ArchiveName] = DoGetEnumeratedBackupFiles(backup);
}

Storage::Storage(Reserver* reserver) : m_Reserver(reserver)
{
	assert(reserver);
}




void Storage::CopyFiles(const CopyList& downloadList)
{
	for (auto& download : downloadList)
	{
		fs::copy_file(download.first, download.second);
	}
}

RegularBackupList LocalDiskStorage::DoGetEnumeratedBackupFiles(const BackupBase& backup) const
{
	wstring path =	m_Path + L"\\" + GeneralSettings::Instance().m_CloudRCopyDirName + L"\\" + backup.m_ArchiveName;
	return LocalDiskIOManager::Instance().EnumerateBackupFiles(path);
}

void LocalDiskStorage::StartDownloading(const CopyList& copylist, Completer* completer, const Completion& completion)
{
	wstring path = m_Path + L"\\" + GeneralSettings::Instance().m_CloudRCopyDirName + L"\\";
	CopyList copylist2 = copylist;

	for (auto& download : copylist2)
	{
		download.first = path + download.first;
	}

	LocalDiskIOManager::Instance().StartCopyFiles(copylist2, completer, completion);
}

void LocalDiskStorage::StartUploading(const CopyList& copylist, Completer* completer, const Completion& completion)
{
	wstring path = m_Path + L"\\" + GeneralSettings::Instance().m_CloudRCopyDirName + L"\\";

	CopyList copylist2 = copylist;

	for (auto& download : copylist2)
	{		
		download.second = path + download.second;
	}

	LocalDiskIOManager::Instance().StartCopyFiles(copylist2, completer, completion);
}

void LocalDiskStorage::StartRemovingOld(const RemoveList& removeList, Completer* completer, const Completion& completion)
{
	wstring path = m_Path + L"\\" + GeneralSettings::Instance().m_CloudRCopyDirName + L"\\";
	auto removeList2 = removeList;

	for (auto& file : removeList2)
	{
		file = path + file;
	}

	LocalDiskIOManager::Instance().StartRemoveFiles(removeList2, completer, completion);
}

RegularBackupList WebDavStorage::DoGetEnumeratedBackupFiles(const BackupBase& backup) const
{
	return InternetIOManager::Instance().EnumerateBackupFiles(this, backup);
}

void WebDavStorage::StartDownloading(const CopyList& copylist, Completer* completer, const Completion& completion)
{
	InternetCopyEvent event;
	event.storage = this;
	event.downloadList = copylist;

	InternetIOManager::Instance().StartDownloadFiles(move(event), completer, completion);
}

void WebDavStorage::StartUploading(const CopyList& copylist, Completer* completer, const Completion& completion)
{
	InternetCopyEvent event;
	event.storage = this;
	event.downloadList = copylist;

	InternetIOManager::Instance().StartUploadFiles(move(event), completer, completion);
}

void WebDavStorage::StartRemovingOld(const RemoveList& removeList, Completer* completer, const Completion& completion)
{		
	InternetRemoveEvent event;
	event.storage = this;
	event.removeList = removeList; 


	InternetIOManager::Instance().StartRemoveFiles(move(event), completer, completion);
}

void WebDavStorage::Mount(const wstring& mountPoint) const
{
	RunCMD(L"net use " + mountPoint + L" \"" + m_Url + L"\" /USER:\"" + m_UserName + L"\" \"" + m_Password + L"\" /PERSISTENT:NO");
}

bool RegularBackupList::Empty()
{
	return Regular2Deltas.empty();
}

LastRegularBackupInfo LastRegularBackupInfo::CreateFromBackupInfoList(const RegularBackupList& backup)
{
	LastRegularBackupInfo result;
	
	if (!backup.Regular2Deltas.empty())
	{
		auto& regularInfo = *backup.Regular2Deltas.rbegin();
		result.LastRegularFilename = regularInfo.first;

		if (!regularInfo.second.Delta2Increments.empty())
		{
			auto& deltaInfo = *regularInfo.second.Delta2Increments.rbegin();

			result.LastDeltaFilename = deltaInfo.first;
			result.IncrementFilenames = deltaInfo.second.IncrementBackupFileRels;
		}
	}

	return result;
}
