#include "MyBCopy_pch.h"
#include "Storage.h"
#include "LocalDiskIOManager.h"
#include "InternetIOManager.h"
#include "Utils.h"
#include "UtilsMyBCopy.h"


unsigned int Storage::GetNumBranchesFromLastRegular(const BackupBase& backup) const
{
	auto enumeratedRegulars = GetEnumeratedBackupFiles(backup);

	if (enumeratedRegulars.Regular2Branches.empty())
		return 0;

	auto& enumeratedBranches = enumeratedRegulars.Regular2Branches.rbegin()->second;
	
	return enumeratedBranches.Branch2Increments.size();
}

unsigned int Storage::GetLastChainLengthFromLastRegular(const BackupBase& backup) const
{
	auto enumeratedRegulars = GetEnumeratedBackupFiles(backup);

	if (enumeratedRegulars.Regular2Branches.empty())
		return 0;

	auto& enumeratedBranches = enumeratedRegulars.Regular2Branches.rbegin()->second;

	if (enumeratedBranches.Branch2Increments.empty())
		return 0;

	auto& enumeratedIncrements = enumeratedBranches.Branch2Increments.rbegin()->second.IncrementBackupFileRels;

	return enumeratedIncrements.size();

}

LastRegularBackupInfo Storage::GetLastBackupsInfo(const BackupBase& backup) const
{
	return LastRegularBackupInfo::CreateFromBackupInfoList(GetEnumeratedBackupFiles(backup));
}

RegularBackupList Storage::GetEnumeratedBackupFiles(const BackupBase& backup) const
{
	return DoGetEnumeratedBackupFiles(backup);	
}





RegularBackupList LocalDiskStorage::DoGetEnumeratedBackupFiles(const BackupBase& backup) const
{	
	wstring path =	m_Path + L"\\" + backup.m_Name;
	return LocalDiskIOManager::Instance().EnumerateBackupFiles(path);
}

void LocalDiskStorage::StartDownloading(const CopyList& copylist, Completer* completer, const Completion& completion)
{
	wstring path = m_Path + L"\\";
	CopyList copylist2 = copylist;

	for (auto& download : copylist2)
	{
		download.first = path + download.first;
	}

	LocalDiskIOManager::Instance().StartCopyFiles(copylist2, completer, completion);
}

void LocalDiskStorage::StartUploading(const CopyList& copylist, Completer* completer, const Completion& completion)
{
	wstring path = m_Path + L"\\";

	CopyList copylist2 = copylist;

	for (auto& download : copylist2)
	{		
		download.second = path + download.second;
	}

	LocalDiskIOManager::Instance().StartCopyFiles(copylist2, completer, completion);
}

void LocalDiskStorage::StartRemoving(const RemoveList& removeList, Completer* completer, const Completion& completion)
{
	wstring path = m_Path + L"\\";
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

void WebDavStorage::StartRemoving(const RemoveList& removeList, Completer* completer, const Completion& completion)
{		
	InternetRemoveEvent event;
	event.storage = this;
	event.removeList = removeList; 


	InternetIOManager::Instance().StartRemoveFiles(move(event), completer, completion);
}

bool RegularBackupList::Empty()
{
	return Regular2Branches.empty();
}

size_t RegularBackupList::TotalNumFiles() const
{
	size_t result = Regular2Branches.size();
	for (auto& r2d : Regular2Branches)
	{
		auto& branch2Increments = r2d.second.Branch2Increments;
		result += branch2Increments.size();
		for (auto& d2i : branch2Increments)
		{
			result += d2i.second.IncrementBackupFileRels.size();
		}
	}
	return result;
}

bool RegularBackupList::HasFileWithTimeStamp(const wstring& timestamp)
{
	for (auto& r2b : Regular2Branches)
	{
		wstring regularTimestamp = ExtractTimestampFromFilename(r2b.first);
		if (regularTimestamp == timestamp)
			return true;

		auto& branch2increments = r2b.second.Branch2Increments;

		for (auto& b2i : branch2increments)
		{
			for (auto increment : b2i.second.IncrementBackupFileRels)
			{
				wstring incrementTimestamp = ExtractTimestampFromFilename(fs::path(increment).filename());
				if (incrementTimestamp == timestamp)
					return true;
			}
		}
	}

	return false;
}

LastRegularBackupInfo LastRegularBackupInfo::CreateFromBackupInfoList(const RegularBackupList& backup)
{
	LastRegularBackupInfo result;
	
	if (!backup.Regular2Branches.empty())
	{
		auto& regularInfo = *backup.Regular2Branches.rbegin();
		result.LastRegularFilename = regularInfo.first;

		if (!regularInfo.second.Branch2Increments.empty())
		{
			auto& branchInfo = *regularInfo.second.Branch2Increments.rbegin();						
			result.LastIncrementFileRels = branchInfo.second.IncrementBackupFileRels;
		}
	}

	return result;
}

bool LastRegularBackupInfo::Empty() const
{
	return LastRegularFilename.empty() && LastIncrementFileRels.empty();
}

std::wstring LastRegularBackupInfo::GetLastFile() const
{	
	if (LastIncrementFileRels.empty())
	{
		return LastRegularFilename;
	}
	else
	{
		return *LastIncrementFileRels.rbegin();
	}	
}

std::wstring LastRegularBackupInfo::GetLastBackupTimestamp() const
{
	auto fileRel = GetLastFile();
	wstring filename = fs::path(fileRel).filename();
	return  ExtractTimestampFromFilename(filename);
}

fs::file_time_type LastRegularBackupInfo::GetLastBackupTime() const
{	
	return  date2file_time_type(GetLastBackupTimestamp());
}
