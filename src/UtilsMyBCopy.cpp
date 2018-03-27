#include "MyBCopy_pch.h"
#include "UtilsMyBCopy.h"
#include "Storage.h"
#include "Reserver.h"
#include "UtilsWindows.h"





std::wstring NowTimestampAsString()
{
	time_t rawtime;
	struct tm  timeinfo;
	char buffer[80];

	time(&rawtime);

	if (localtime_s(&timeinfo, &rawtime) != 0)
		throw EXCEPTION(ErrnoException());

	strftime(buffer, 80, "%F_%H-%M-%S", &timeinfo);
	return wstring_convert< codecvt<wchar_t, char, mbstate_t> >().from_bytes(buffer);
}



std::wstring ExtractTimestampFromFilename(const wstring& filename)
{
	wregex validationMask(L_BkTimeStampMask LR"(_\w+\.\w+)");

	if (!regex_match(filename, validationMask))
		throw EXCEPTION(BaseException(L"wrong filename of regular backup archive. Filename == " + filename));
		

	return filename.substr(0, wcslen(L_BkTimeStampExample));
}


std::wstring ExtractRegularTimeStampFromIncrementDirName(const wstring& incrementDirName)
{
	wregex validationMask(L_BkTimeStampMask L"_increments_from_" L_BkTimeStampMask);

	if (!regex_match(incrementDirName, validationMask))
		throw EXCEPTION(BaseException(L"wrong incrementDirName == " + incrementDirName));

	return incrementDirName.substr(wcslen(L_BkTimeStampExample) + wcslen(L"_increments_from_"), wcslen(L_BkTimeStampExample));
}



std::wstring MakeRegularArchiveFileBaseName(const wstring& nowTimestamp)
{
	return nowTimestamp + L"_regular";
}


std::wstring MakeIncrementArchiveFileBaseName(const wstring& nowTimestamp)
{
	return nowTimestamp + L"_increment";
}

std::wstring MakeIncrementArchivesDirName(const wstring& nowTimestamp, const wstring& regularTimestamp)
{
	return nowTimestamp + L"_increments_from_" + regularTimestamp;
}



std::wstring GetFreeMountPoint()
{
	wchar_t  drives[26];

	iota(drives, drives + 26, L'A');
		
	DWORD logicalDrivesMask = GetLogicalDrives();

	if (logicalDrivesMask == 0)
	{
		throw EXCEPTION( WinException() );
	}
		
	for (int c = 15, size = 26; c < size; c++)
	{
		if (logicalDrivesMask & (1 << c))
			continue;

		wstring result;
		result += drives[c];
		result += L":";

		TCHAR szDeviceName[80];
		DWORD cchBuff = sizeof(szDeviceName);
		DWORD  connectionState = WNetGetConnection(result.c_str(), szDeviceName, &cchBuff);
		
		if (connectionState != ERROR_NOT_CONNECTED)
		{
			continue;
		}

		return result;
	}

	for (int c = 0, size = 15; c < size; c++)
	{
		if (logicalDrivesMask & (1 << c))
			continue;

		wstring result;
		result += drives[c];
		result += L":";

		TCHAR szDeviceName[80];
		DWORD cchBuff = sizeof(szDeviceName);
		DWORD  connectionState = WNetGetConnection(result.c_str(), szDeviceName, &cchBuff);

		if (connectionState != ERROR_NOT_CONNECTED)
		{
			continue;
		}

		return result;
	}

	throw EXCEPTION(BaseException(L"No free mount point"));
}



void CollectIncrementsForRegular(const wstring& timestampRegular, list<wstring>& increments, Branch2IncrementListMap& branchMap)
{
	for (auto increment_it = increments.begin(); increment_it != increments.end(); )
	{
		wstring& incrementFileRel = *increment_it;
		wstring incrementDirName = fs::path(incrementFileRel).parent_path();
		wstring timestampRegularFromIncrementDirName = ExtractRegularTimeStampFromIncrementDirName(incrementDirName);
		if (timestampRegularFromIncrementDirName == timestampRegular)
		{
			IncrementBackupList& incrementList = branchMap[incrementDirName];
			incrementList.IncrementBackupFileRels.emplace(move(incrementFileRel));
			increment_it = increments.erase(increment_it);
		}
		else
		{
			++increment_it;
		}
	}
}

RegularBackupList EnumBackupFiles(const wstring& path)
{
	RegularBackupList result;
	if (!fs::exists(path))
		return result;

	wregex regularMask(L_BkTimeStampMask L"_regular");	
	wregex incrementDirMask(L_BkTimeStampMask L"_increments_from_" L_BkTimeStampMask);
	wregex incrementFileMask(L_BkTimeStampMask L"_increment");

	list<wstring> increments;		

	for (auto& p : fs::directory_iterator(path))
	{
		if (fs::is_directory(p.path()))
		{
			if (!regex_match(wstring(p.path().filename()), incrementDirMask))
				continue;

			for (auto& incrementFile : fs::directory_iterator(p.path()))
			{
				if (!regex_match(wstring(incrementFile.path().stem()), incrementFileMask))
					continue;

				fs::path incrementWithRelPath = p.path().filename() / incrementFile.path().filename();
				increments.push_back(incrementWithRelPath);
			}
		}
		else
		{
			if (!fs::is_regular_file(p.path()))
				continue;

			if (!regex_match(wstring(p.path().stem()), regularMask))
				continue;
			
			result.Regular2Branches.emplace(p.path().filename(), IncrementBranchList());					
		}
	}

	//////////

	for (auto& regular : result.Regular2Branches)
	{
		wstring timestampRegular = ExtractTimestampFromFilename(regular.first);
		Branch2IncrementListMap& branchMap = regular.second.Branch2Increments;

		CollectIncrementsForRegular(timestampRegular, increments, branchMap);		
	}

	//////
	while (!increments.empty())
	{
		wstring& incrementFileRel = increments.front();
		wstring incrementDirName = fs::path(incrementFileRel).parent_path();
		wstring timestampRegularFromIncrementDirName = ExtractRegularTimeStampFromIncrementDirName(incrementDirName);

		IncrementBranchList& lostRegular = result.LostRegular2Branches[MakeRegularArchiveFileBaseName(timestampRegularFromIncrementDirName) + fs::path(incrementFileRel).extension().wstring()];

		Branch2IncrementListMap& branchMap = lostRegular.Branch2Increments;

		CollectIncrementsForRegular(timestampRegularFromIncrementDirName, increments, branchMap);
	}

		

	return result;
}

RemoveList MakeRemoveListForOldRegularBackup(const RegularBackupList& enumeratedList, unsigned int numStoredOldRegularBackups)
{
	RemoveList result;

	if (enumeratedList.Regular2Branches.empty())
		return result;
		

	if (numStoredOldRegularBackups == 0)
		return result;
	
	if (enumeratedList.Regular2Branches.size() + enumeratedList.LostRegular2Branches.size() <= numStoredOldRegularBackups)
		return result;
	
	for (auto& lostRegular : enumeratedList.LostRegular2Branches)
	{
		const auto& branchList = lostRegular.second;

		for (auto& branch : branchList.Branch2Increments)
		{
			result.push_back(branch.first);
		}
	}
		

	if (enumeratedList.Regular2Branches.size() <= numStoredOldRegularBackups)
		return result;

	auto end = enumeratedList.Regular2Branches.begin();
	advance(end, enumeratedList.Regular2Branches.size() - numStoredOldRegularBackups);
	

	for (auto it = enumeratedList.Regular2Branches.begin(); it != end; ++it)
	{
		wstring file = it->first;
		result.push_back(file);
				
		const auto& branchList = it->second;

		for (auto& branch : branchList.Branch2Increments)
		{
			result.push_back(branch.first);
						
			/*auto& incrementBackupFileRels = branch.second.IncrementBackupFileRels;
			if (!incrementBackupFileRels.empty())
			{
				file = wstring(fs::path(*incrementBackupFileRels.begin()).parent_path());
				result.push_back(file);
			}*/
		}
	}
	
	return result;
}

fs::file_time_type cvtFilelastWriteTime(const string& dur)
{
	fs::file_time_type result;
	regex maskDay(R"(.+[^\S\r\n]day)");
	if (regex_match(dur, maskDay))
	{
		fs::file_time_type epoch;
		auto duration = epoch.time_since_epoch();
		//	duration.count() -= stoi(dur);

		result = fs::file_time_type(duration);
		return result;
	}


	//regex maskWeek(R"(.+[^\S\r\n]week)");
	//if (regex_match(dur, maskDay))
	//{
	//	result = stoi(dur);
	//	return result;
	//}
	//regex maskMonth(".+[^\S\r\n]month");

	//regex maskYear(".+[^\S\r\n]year");
	//
	return result;
}

fs::file_time_type date2file_time_type(const wstring& nowTimestamp)
{
	struct tm timeDate;
	memset(&timeDate, 0, sizeof(timeDate));


	assert(nowTimestamp.length() >= wcslen(L_BkTimeStampExample));

	timeDate.tm_year = std::stoi(nowTimestamp.substr(0, 4)) - 1900;
	timeDate.tm_mon = std::stoi(nowTimestamp.substr(5, 2)) - 1;
	timeDate.tm_mday = std::stoi(nowTimestamp.substr(8, 2));

	if (nowTimestamp.length() >= 16)
	{
		timeDate.tm_hour = std::stoi(nowTimestamp.substr(11, 2));
		timeDate.tm_min = std::stoi(nowTimestamp.substr(14, 2));
		timeDate.tm_sec = std::stoi(nowTimestamp.substr(16, 2));
	}

	timeDate.tm_isdst = -1;

	time_t tt = std::mktime(&timeDate);

	return chrono::system_clock::from_time_t(tt);
}




std::wstring GetSystemRarExeFilePath()
{
	auto path = GetValidPathFromRegAnyBitness(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WinRAR", L"exe");
	if (path.empty())
	{
		path = GetValidPathFromRegAnyBitness(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WinRAR", L"exe64");
	}

	if (!path.empty())
	{
		//		path = fs::path(path).parent_path();
		path = fs::canonical(path);
		if (fs::exists(path))
		{
			return path;
		}
	}

	return wstring();
}

std::wstring GetSystem7zExeFilePath()
{
	auto path = GetValidPathFromRegAnyBitness(HKEY_LOCAL_MACHINE, L"SOFTWARE\\7-Zip", L"Path");
	if (path.empty())
	{
		path = GetValidPathFromRegAnyBitness(HKEY_LOCAL_MACHINE, L"SOFTWARE\\7-Zip", L"Path64");
	}

	if (!path.empty())
	{
		path = fs::canonical(path) / L"7z.exe";
		if (fs::exists(path))
		{
			return path;
		}
	}

	return wstring();
}

