#include "stdafx.h"
#include "UtilsMyRCopy.h"
#include "Storage.h"
#include "Reserver.h"





std::wstring NowTimestampAsString()
{
	time_t rawtime;
	struct tm  timeinfo;
	char buffer[80];

	time(&rawtime);

	if (localtime_s(&timeinfo, &rawtime) != 0)
		throw EXCEPTION(ErrnoException());

	strftime(buffer, 80, "%F_%H-%M", &timeinfo);
	return wstring_convert< codecvt<wchar_t, char, mbstate_t> >().from_bytes(buffer);
}



std::wstring ExtractTimestampFromFilename(const wstring& filename)
{
	wregex validationMask(L_BkTimeStampMask L"_.+\\.7z");

	if (!regex_match(filename, validationMask))
		throw EXCEPTION(BaseException(L"wrong filename of regular backup archive. Filename == " + filename));
		

	return filename.substr(0, wcslen(L_BkTimeStampExample));
}

std::wstring ExtractRegularTimestampFromDeltaBackup(const wstring& filename)
{
	wregex validationMask(L_BkTimeStampMask L"_delta_from_" L_BkTimeStampMask L"\\.7z");

	if (!regex_match(filename, validationMask))
		throw EXCEPTION(BaseException(L"wrong filename of regular backup archive. Filename == " + filename));

	return filename.substr(wcslen(L_BkTimeStampExample) + wcslen( L"_delta_from_"), wcslen(L_BkTimeStampExample));
}

std::wstring ExtractDeltaTimeStampFromIncrementDirName(const wstring& incrementDirName)
{
	wregex validationMask(L_BkTimeStampMask L"_increments_from_" L_BkTimeStampMask);

	if (!regex_match(incrementDirName, validationMask))
		throw EXCEPTION(BaseException(L"wrong incrementDirName == " + incrementDirName));

	return incrementDirName.substr(wcslen(L_BkTimeStampExample) + wcslen(L"_increments_from_"), wcslen(L_BkTimeStampExample));
}



std::wstring MakeRegularArchiveFileName(const wstring& nowTimestamp, const wstring& ext /*= L".7z"*/)
{
	return nowTimestamp + L"_regular" + ext;
}

std::wstring MakeDeltaArchiveFileName(const wstring& nowTimestamp, const wstring& prevRegularTimestamp, const wstring& ext /*= L".7z"*/)
{
	return nowTimestamp + L"_delta_from_" + prevRegularTimestamp + ext;
}

std::wstring MakeIncrementArchiveFileName(const wstring& nowTimestamp, const wstring& ext /*= L".7z"*/)
{
	return nowTimestamp + L"_increment" + ext;
}

std::wstring MakeIncrementArchivesDirName(const wstring& nowTimestamp, const wstring& prevDeltaTimestamp)
{
	return nowTimestamp + L"_increments_from_" + prevDeltaTimestamp;
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
		if ( !(logicalDrivesMask & (1 << c)))
		{
			wstring result;
			result += drives[c];
			result += L":";
			return result;
		}
	}

	for (int c = 0, size = 15; c < size; c++)
	{
		if ( !(logicalDrivesMask & (1 << c)))
		{
			wstring result;
			result += drives[c];
			result += L":";
			return result;
		}
	}

	throw EXCEPTION(BaseException(L"No free mount point"));
}
//
//LastRegularBackupInfo EnumLastBackups(const wstring& path)
//{
//	wregex regularMask(L_BkTimeStampMask L"_regular\\..*");
//	wregex deltaMask(L_BkTimeStampMask L"_delta_from_" L_BkTimeStampMask "\\..*");
//	wregex incrementMask(L_BkTimeStampMask L"_increments_from_" L_BkTimeStampMask);
//
//	LastRegularBackupInfo result;
//
//	for (auto& p : fs::directory_iterator(path))
//	{
//		if (fs::is_directory(p.path()))
//		{
//			if (!regex_match(wstring(p.path().filename()), incrementMask))
//				continue;
//
//			result.Increments.insert(p.path().filename());
//		}
//		else
//		{
//			if (!fs::is_regular_file(p.path()))
//				continue;
//
//			if (regex_match(wstring(p.path().filename()), regularMask))
//			{
//				result.Regulars.insert(p.path().filename());
//			}
//			else
//			{
//				if (!regex_match(wstring(p.path().filename()), deltaMask))
//					continue;
//
//				result.DeltasFromLastRegular.insert(p.path().filename());
//			}
//		}
//	}
//
//	//////////
//
//	decltype(result.DeltasFromLastRegular) deltas;
//
//	const wstring& lastRegular = *result.Regulars.rbegin();
//	deltaMask.assign((L_BkTimeStampMask L"_delta_from_") + ExtractBackupTimestampFromFilename(lastRegular) + L"\\..*");
//
//	copy_if(make_move_iterator(result.DeltasFromLastRegular.begin()), make_move_iterator(result.DeltasFromLastRegular.end()), inserter(deltas, deltas.end()), [&deltaMask](const wstring& e)->bool
//	{
//		return regex_match(e, deltaMask);
//	});
//
//	result.DeltasFromLastRegular = move(deltas);
//
//	//////////
//
//	decltype(result.Increments) increments;
//
//	const wstring& lastDelta = *result.DeltasFromLastRegular.rbegin();
//	incrementMask.assign((L_BkTimeStampMask L"_increments_from_") + ExtractBackupTimestampFromFilename(lastDelta));
//
//	copy_if(make_move_iterator(result.Increments.begin()), make_move_iterator(result.Increments.end()), inserter(increments, increments.end()), [&incrementMask](const wstring& e)->bool
//	{
//		return regex_match(e, incrementMask);
//	});
//
//	result.Increments.clear();
//	incrementMask.assign(L_BkTimeStampMask L"_increment\\..*");
//
//	for (auto& p : fs::directory_iterator(path + L"\\" + *increments.rbegin()))
//	{
//		if (!fs::is_regular_file(p.path()))
//			continue;
//
//		if (!regex_match(wstring(p.path().filename()), incrementMask))
//			continue;
//
//		result.Increments.insert(p.path().filename());
//	}
//
//	return result;
//}




RegularBackupList EnumBackupFiles(const wstring& path)
{
	RegularBackupList result;
	if (!fs::exists(path))
		return result;

	wregex regularMask(L_BkTimeStampMask L"_regular\\.7z");
	wregex deltaMask(L_BkTimeStampMask L"_delta_from_" L_BkTimeStampMask L"\\.7z");
	wregex incrementDirMask(L_BkTimeStampMask L"_increments_from_" L_BkTimeStampMask);
	wregex incrementFileMask(L_BkTimeStampMask L"_increment\\.7z");

	set<wstring> deltas;
	set<wstring> increments;
		

	for (auto& p : fs::directory_iterator(path))
	{
		if (fs::is_directory(p.path()))
		{
			if (!regex_match(wstring(p.path().filename()), incrementDirMask))
				continue;

			for (auto& incrementFile : fs::directory_iterator(p.path()))
			{
				if (!regex_match(wstring(incrementFile.path().filename()), incrementFileMask))
					continue;

				fs::path incrementWithRelPath = p.path().filename() / incrementFile.path().filename();
				increments.insert(incrementWithRelPath);
			}
		}
		else
		{
			if (!fs::is_regular_file(p.path()))
				continue;

			if (regex_match(wstring(p.path().filename()), regularMask))
			{
				result.Regular2Deltas.emplace(p.path().filename(), DeltaBackupList());
			}
			else
			{
				if (!regex_match(wstring(p.path().filename()), deltaMask))
					continue;

				deltas.insert(p.path().filename());
			}
		}
	}

	//////////

	for (auto& regular : result.Regular2Deltas)
	{
		wstring timestampRegular = ExtractTimestampFromFilename(regular.first);

		for (auto delta_it = deltas.begin(); delta_it != deltas.end(); )
		{
			const wstring& deltaFileName = *delta_it;
			wstring timestampRegularFromDelta = ExtractRegularTimestampFromDeltaBackup(deltaFileName);
			if (timestampRegularFromDelta == timestampRegular)
			{
				IncrementBackupList deltaBackupInfo;

				wstring timestampDelta = ExtractTimestampFromFilename(deltaFileName);


				for (auto increment_it = increments.begin(); increment_it != increments.end(); )
				{
					const wstring& incrementFileRel = *increment_it;
					wstring incrementDirName = fs::path(incrementFileRel).parent_path();
					wstring timestampDeltaFromIncrementDirName = ExtractDeltaTimeStampFromIncrementDirName(incrementDirName);
					if (timestampDeltaFromIncrementDirName == timestampDelta)
					{
						deltaBackupInfo.IncrementBackupFileRels.insert(incrementFileRel);
						increment_it = increments.erase(increment_it);
					}
					else
					{
						++increment_it;
					}
				}

				pair<wstring, IncrementBackupList> delta2IncrementList;
				delta2IncrementList.first = deltaFileName;
				delta2IncrementList.second = move(deltaBackupInfo);

				regular.second.Delta2Increments.emplace(move(delta2IncrementList));
				delta_it = deltas.erase(delta_it);
			}
			else
			{
				++delta_it;
			}
		}
	}


	return result;
}

void RemoveOldBackupFiles(const RegularBackupList& enumeratedList, const wstring& parantPath)
{
	const auto MaxNumOfOldArchives = GeneralSettings::Instance().m_MaxNumOfOldArchives;
	//const auto MinNumOfOldRegularArchives = GeneralSettings::Instance()->m_MinNumOfOldRegularArchives;
	
	
	if (enumeratedList.Regular2Deltas.size() >= MaxNumOfOldArchives)
	{
		auto it = enumeratedList.Regular2Deltas.begin();
		const auto& deltaList = it->second;

		for (unsigned int c = 0; c < enumeratedList.Regular2Deltas.size() + 1 - MaxNumOfOldArchives; c++)
		{
			wstring file = parantPath + L"\\" + it->first;
			fs::remove(file);

			for (auto& delta : deltaList.Delta2Increments)
			{
				file = parantPath + L"\\" + delta.first;
				fs::remove(file);

				if (!delta.second.IncrementBackupFileRels.empty())
				{
					file = parantPath + L"\\" + wstring( fs::path(*delta.second.IncrementBackupFileRels.begin()).parent_path() );
					fs::remove_all(file);
				}				
			}
		}
	}
}


RemoveList MakeRemoveListForOldBackupFiles(const RegularBackupList& enumeratedList)
{
	const auto MaxNumOfOldArchives = GeneralSettings::Instance().m_MaxNumOfOldArchives;
	//const auto MinNumOfOldRegularArchives = GeneralSettings::Instance()->m_MinNumOfOldRegularArchives;

	RemoveList result;
	if (enumeratedList.Regular2Deltas.size() >= MaxNumOfOldArchives)
	{
		auto it = enumeratedList.Regular2Deltas.begin();
		const auto& deltaList = it->second;

		for (unsigned int c = 0; c < enumeratedList.Regular2Deltas.size() + 1 - MaxNumOfOldArchives; c++)
		{
			wstring file = it->first;
			result.push_back(file);
			

			for (auto& delta : deltaList.Delta2Increments)
			{
				file = delta.first;
				result.push_back(file);

				if (!delta.second.IncrementBackupFileRels.empty())
				{
					file = wstring(fs::path(*delta.second.IncrementBackupFileRels.begin()).parent_path());
					result.push_back(file);					
				}
			}
		}
	}
	return result;
}
