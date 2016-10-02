#include "stdafx.h"
#include "BackupBase.h"
#include "Utils.h"
#include "BackupJobExecuter.h"
#include "LocalDiskIOManager.h"
#include "InternetIOManager.h"


BackupBase::BackupBase(const wstring& archiveName) : m_ArchiveName(archiveName)
{

}


void ParsedBackup::LoadBackupSourcesFromFile(const wstring& srcFile, deque<ParsedBackup>& backups)
{
	wifstream srcFileStream;


	fs::path srcFileFullPath(srcFile);

	if (srcFileFullPath.is_absolute())
		srcFileStream.open(srcFile);
	else
		srcFileStream.open(GeneralSettings::Instance().m_ExePath + L"\\" + srcFile);

	if (!srcFileStream)
		return;

	size_t currLine = 0;
	wstring line;

	while (!srcFileStream.eof())
	{
		getline(srcFileStream, line);
		++currLine;

		wsmatch matches;

		if (!regex_search(line, matches, wregex(L"\\S")))
			continue;

		if (matches.size() != 1)
			throw EXCEPTION(ParseException(srcFile, currLine));

		auto& match = matches[0];

		switch (match.str()[0])
		{
			case L';': break;
			case L'#':
			{
				wcmatch includedFileMatches;
				if (!regex_match(line.c_str() + matches.position() + match.length(), includedFileMatches, wregex(LR"regex(include\s(\w.*)(?=\r?$))regex")))
					throw EXCEPTION(ParseException(srcFile, currLine));

				if (includedFileMatches.size() != 2)
					throw EXCEPTION(ParseException(srcFile, currLine));

				LoadBackupSourcesFromFile(includedFileMatches[1].str(), backups);

			}
			break;
			case L'<':
			{
				wcmatch sectionNameMatches;
				if (!regex_match(line.c_str() + matches.position() + match.length(), sectionNameMatches, wregex(LR"((.+)>)")))
					throw EXCEPTION(ParseException(srcFile, currLine));

				if (sectionNameMatches.size() != 2)
					throw EXCEPTION(ParseException(srcFile, currLine));

				backups.emplace_back();
				ParsedBackup& backup = backups.back();
				backup.m_ArchiveName = sectionNameMatches[1];
			}
			break;

			default:
			{
				if (backups.empty())
					throw EXCEPTION(ParseException(srcFile, currLine));

				ParsePath(backups, line.c_str() + matches.position(), srcFile, currLine);
			}
			break;
		}
	}
}

bool ParsedBackup::Empty()
{
	return m_SrcFiles.empty();
}



void ParsedBackup::SaveToRarInclusionFile(const wstring& file) const
{
	wofstream outfile(file);

	if (!outfile)
		throw EXCEPTION(ErrnoException());

	for (auto& srcfile : m_SrcFiles)
	{
		outfile << L"\"" << srcfile << L"\"" << endl;
	}

	//if (!backup.m_ExclFiles.empty())
	//{
	//	outfile.close();

	//	outfile.open(GetTemporaryRarExclusionListFile(backup));

	//	if (!outfile)
	//		throw EXCEPTION(ErrnoException());

	//	for (auto& srcfile : backup.m_ExclFiles)
	//	{
	//		outfile << L"\"" << srcfile << L"\"" << endl;
	//	}
	//}
}

void ParsedBackup::TraverseFiles(const deque<wstring>& files, TraversedBackup1& result)
{
	//for (auto& srcpath : files)
	//{
	//	auto pos = srcpath.rfind(L'*');
	//	if (pos != wstring::npos)
	//	{
	//		wstring regexp = wildcard2regex(fs::path(srcpath).filename());
	//		wregex mask(regexp);

	//		fs::path parentPath = fs::path(srcpath).parent_path();

	//		if (!fs::is_directory(parentPath))
	//			continue;

	//		for (auto& p : fs::directory_iterator(parentPath))
	//		{
	//			if (fs::is_directory(p.path()))
	//			{
	//				TraversedBackupEntry dirEntry;

	//				dirEntry.m_Name = p.path();
	//				dirEntry.m_Type = TraversedBackupEntryType::Directory;
	//				result.TraverseDirectory(p.path(), dirEntry, &mask);

	//				result.m_Entries.emplace(move(dirEntry));
	//			}
	//			else
	//			{
	//				if (!fs::is_regular_file(p.path()))
	//					continue;

	//				if (!regex_match(wstring(p.path()), mask))
	//					continue;

	//				TraversedBackupEntry fileEntry;
	//				fileEntry.m_Name = p.path();
	//				fileEntry.m_Type = TraversedBackupEntryType::File;
	//				result.m_Entries.emplace(move(fileEntry));
	//			}
	//		}
	//	}
	//	else
	//	{
	//		if (fs::is_directory(srcpath))
	//		{
	//			TraversedBackupEntry dirEntry;

	//			dirEntry.m_Name = srcpath;
	//			dirEntry.m_Type = TraversedBackupEntryType::Directory;
	//			result.TraverseDirectory(srcpath, dirEntry);

	//			result.m_Entries.emplace(move(dirEntry));
	//		}
	//		else
	//		{
	//			if (!fs::is_regular_file(srcpath))
	//				continue;

	//			TraversedBackupEntry fileEntry;

	//			fileEntry.m_Name = srcpath;
	//			fileEntry.m_Type = TraversedBackupEntryType::File;
	//			result.m_Entries.emplace(move(fileEntry));
	//		}
	//	}
	//}
}


void ParsedBackup::ParsePath(deque<ParsedBackup>& backups, const wstring& line, const wstring& srcFile, size_t currLine)
{
	auto pos = line.rfind(L'\\');

	if (pos == wstring::npos)
		throw EXCEPTION(ParseException(srcFile, currLine));

	if (pos == 0)
		throw EXCEPTION(ParseException(srcFile, currLine));

	wstring path = line.substr(0, pos);
	if (path.empty())
		throw EXCEPTION(ParseException(srcFile, currLine));

	bool exclusion = path[0] == L'-';

	if (exclusion)
	{
		if (path.size() == 1)
			throw EXCEPTION(ParseException(srcFile, currLine));

		path = path.substr(1);
	}

	if (pos + 1 < line.size())
	{
		wstring filetypes = line.substr(pos + 1);

		ForEachNonEmptySubword(filetypes, L';', [&](const wchar_t* filetype, wstring::size_type filetypeSize)
		{
			if (filetype[0] == '-')
			{
				if (filetypeSize == 1)
					throw EXCEPTION(ParseException(srcFile, currLine));

				if (exclusion)
					throw EXCEPTION(ParseException(srcFile, currLine));

				filetype = filetype + 1;
				backups.back().m_ExclFiles.push_back(path + wstring(L"\\") + wstring(filetype, filetypeSize - 1));
			}
			else
			{
				if (exclusion)
					backups.back().m_ExclFiles.push_back(path + wstring(L"\\") + wstring(filetype, filetypeSize));
				else
					backups.back().m_SrcFiles.push_back(path + wstring(L"\\") + wstring(filetype, filetypeSize));
			}
		});
	}
	else
	{
		if (exclusion)
			backups.back().m_ExclFiles.push_back(path);
		else
			backups.back().m_SrcFiles.push_back(path);
	}
}

void TraversedEntries::ParsedBackup_TraversePathsAndMasks(const deque<wstring>& filelist)
{
	for (auto& filePath : filelist)
	{
		auto pos = filePath.rfind(L'*');
		if (pos != wstring::npos)
		{
			wstring regexp = wildcard2regex(fs::path(filePath).filename());
			wregex mask(regexp);

			fs::path parentPath = fs::path(filePath).parent_path();

			if (!fs::is_directory(parentPath))
				continue;

			for (auto& p : fs::directory_iterator(parentPath))
			{
				if (fs::is_directory(p.path()))
				{
					ParsedBackup_TraversePath(p.path(), &mask);
				}
				else
				{
					if (!fs::is_regular_file(p.path()))
						continue;

					if (!regex_match(wstring(p.path()), mask))
						continue;

					m_Files.insert(p.path());
				}
			}
		}
		else
		{
			if (fs::is_directory(filePath))
			{
				/*result.m_Directories.insert(filePath);
				auto prevSize = result.m_Files.size();*/
				ParsedBackup_TraversePath(filePath);
				/*if (prevSize == result.m_Files.size())
				result.m_Directories.insert(filePath);*/
			}
			else
			{
				if (!fs::is_regular_file(filePath))
					continue;

				m_Files.insert(filePath);
			}
		}
	}
}

void TraversedEntries::ParsedBackup_TraversePath(const wstring& path, std::wregex* mask /*= nullptr*/)
{
	
	for (auto& p : fs::recursive_directory_iterator(path))
	{
		auto& filepath = p.path();		

		if (!fs::is_regular_file(filepath))
			continue;
				
		if (mask)
		{			
			if (!regex_match(wstring(filepath.filename()), *mask))
				continue;
		}

		wstring filepathStr(filepath);		
		m_Files.insert(filepathStr);
	}
}



void BackupDifference::FilesCompare(const TraversedPathBackup& oldDistEntries, const TraversedEntries& newDistEntries, BackupDifference& patch)
{
	list<wstring> intersection;
	set_intersection(oldDistEntries.m_Files.begin(), oldDistEntries.m_Files.end(), newDistEntries.m_Files.begin(), newDistEntries.m_Files.end(), back_inserter(intersection));

	for (auto& file : intersection)
	{
		wstring src = oldDistEntries.m_OriginPath + L"\\" + file[0];
		src += L"_" + file.substr(2);
		if (FileCmp(src, file))
		{
			patch.DiffFiles.push_back(file);
		}
	}
}

BackupDifference BackupDifference::Compare(const TraversedPathBackup& oldDistEntries, const TraversedEntries& newDistEntries)
{
	BackupDifference result;

	set_difference(newDistEntries.m_Directories.begin(), newDistEntries.m_Directories.end(), oldDistEntries.m_Directories.begin(), oldDistEntries.m_Directories.end(), back_inserter(result.NewDirectories));
	set_difference(oldDistEntries.m_Directories.begin(), oldDistEntries.m_Directories.end(), newDistEntries.m_Directories.begin(), newDistEntries.m_Directories.end(), back_inserter(result.RemoveDirectories));

	set_difference(newDistEntries.m_Files.begin(), newDistEntries.m_Files.end(), oldDistEntries.m_Files.begin(), oldDistEntries.m_Files.end(), back_inserter(result.NewFiles));
	set_difference(oldDistEntries.m_Files.begin(), oldDistEntries.m_Files.end(), newDistEntries.m_Files.begin(), newDistEntries.m_Files.end(), back_inserter(result.RemoveFiles));

	FilesCompare(oldDistEntries, newDistEntries, result);

	return result;
}

void BackupDifference::ToTraversedEntries(TraversedEntries& actualData)
{	
	copy(DiffFiles.begin(),DiffFiles.end(), inserter(actualData.m_Files, actualData.m_Files.end()));
	copy(NewFiles.begin(), NewFiles.end(), inserter(actualData.m_Files, actualData.m_Files.end()));
}


bool TraversedEntries::Empty()
{
	return m_Directories.empty() && m_Files.empty();
}

void TraversedEntries::SaveToRarInclusionFile(const wstring& file) const
{
	wofstream outfile(file);

	if (!outfile)
		throw EXCEPTION(ErrnoException());

	for (auto& srcfile : m_Files)
	{
		outfile << L"\"" << srcfile << L"\"" << endl;
	}
}

//TraversedEntries TraversedEntries::CreateFromParsedBackupTraverse(const ParsedBackup& ParsedBackup)
//{
//	TraversedEntries result;
//	result.m_OriginPath = path;
//	result.m_ArchiveName = ParsedBackup.m_ArchiveName;
//	result.ParsedBackup_TraversePathsAndMasks(ParsedBackup.m_SrcFiles);
//
//	TraversedEntries exclusion;
//	exclusion.ParsedBackup_TraversePathsAndMasks(ParsedBackup.m_ExclFiles);
//
//	result.Subtract(exclusion);
//
//	return result;
//}
//
//TraversedEntries TraversedEntries::CreateFromPathTraverse(const wstring& path)
//{
//	TraversedEntries result;
//	result.m_OriginPath = path;
//	result.ParsedBackup_TraversePath(path);
//	return result;
//}

void TraversedEntries::Subtract(const TraversedEntries& exclusion)
{
	set< wstring > result;
	set_difference(m_Files.begin(), m_Files.end(), exclusion.m_Files.begin(), exclusion.m_Files.end(), inserter(result, result.end()));
	m_Files = move(result);
}


ArchivedRarBackup::ArchivedRarBackup(const TraversedEntries& backup, const wstring& tmpWorkingDir, const wstring& newBackupArchiveFileNameBase, const wstring& passwd)
	: BackupBase(backup.m_ArchiveName),
	m_InclusionListFile(tmpWorkingDir + L"\\" + newBackupArchiveFileNameBase + L"_inclusion.txt"),
	m_ArchiveRarFile(tmpWorkingDir + L"\\" + newBackupArchiveFileNameBase + L".rar")
{
	fs::create_directories(tmpWorkingDir);

	backup.SaveToRarInclusionFile(m_InclusionListFile);
	
	wstring cmd = L"rar a -hp\"" + passwd + L"\" -idq -m5 -ep3 -r -s -t -y -- \"" + m_ArchiveRarFile + L"\" @\"" + m_InclusionListFile + L"\"";

	RunCMD(cmd);
}

ArchivedRarBackup::~ArchivedRarBackup()
{	
	if (Empty())
		return;

	if (fs::exists(m_InclusionListFile))
	{
		fs::remove(m_InclusionListFile);
	}
	if (fs::exists(m_ArchiveRarFile))
	{
		fs::remove(m_ArchiveRarFile);
	}	
}

std::wstring ArchivedRarBackup::ArchiveRarFile() const
{
	return m_ArchiveRarFile;
}

bool ArchivedRarBackup::Empty()
{
	return m_ArchiveRarFile.empty() || m_InclusionListFile.empty();
}

void ArchivedRarBackup::ExtractToDir(const wstring& destDir, const wstring& passwd)
{
	wstring cmd = L"rar x -p\"" + passwd + L"\" -o+ -y -- \"" + m_ArchiveRarFile + L"\" * \"" + destDir + L"\\\"";

	RunCMD(cmd);
}


Archived7zBackup::~Archived7zBackup()
{
	if (Empty())
		return;

	if (fs::exists(m_Archive7zFile))
	{
		fs::remove(m_Archive7zFile);
	}
}

std::wstring Archived7zBackup::Archive7zFile() const
{
	return m_Archive7zFile;
}

bool Archived7zBackup::Empty()
{
	return m_Archive7zFile.empty();
}


void Archived7zBackup::AssignAndCompress(const ArchivedRarBackup& backup, const wstring& tmpWorkingDir, const wstring& newBackupArchiveFileNameBase, const wstring& passwd)
{
	m_ArchiveName = backup.m_ArchiveName;
	m_Archive7zFile = tmpWorkingDir + L"\\" + newBackupArchiveFileNameBase + L".7z";
	RunCMD(L"7z a -p\"" + passwd + L"\" -mhe=on -mx=0 -bso0 -bsp0 -y \"" + m_Archive7zFile + L"\" \"" + backup.ArchiveRarFile() + L"\"");
	RunCMD(L"7z t -p\"" + passwd + L"\" -bso0 -bsp0 -y \"" + m_Archive7zFile + L"\" *");
}

void Archived7zBackup::CreateForExtraction(const wstring& file, Archived7zBackup& archive7z)
{
	Archived7zBackup result;
	archive7z.m_Archive7zFile = file;	
}

void Archived7zBackup::ExtractRarBackup(const wstring& passwd, ArchivedRarBackup& archiveRar)
{
	RunCMD(L"7z x -p\"" + passwd + L"\" -o\"" + wstring( fs::path(m_Archive7zFile).parent_path() ) + L"\" -y \"" + m_Archive7zFile + L"\"");
	archiveRar.m_ArchiveRarFile = wstring( fs::path(m_Archive7zFile).parent_path() / fs::path(m_Archive7zFile).stem() ) + L".rar";
}

TraversedParesedBackup::TraversedParesedBackup(const ParsedBackup& parsedBackup)
{
	ParsedBackup_TraversePathsAndMasks(parsedBackup.m_SrcFiles);

	TraversedParesedBackup exclusion;
	exclusion.ParsedBackup_TraversePathsAndMasks(parsedBackup.m_ExclFiles);

	Subtract(exclusion);
}

TraversedPathBackup::TraversedPathBackup(const wstring& path) : m_OriginPath(path)
{
	ParsedBackup_TraversePath(m_OriginPath);
}

void TraversedPathBackup::ParsedBackup_TraversePath(const wstring& path, std::wregex* mask /*= nullptr*/)
{
	TraversedEntries::ParsedBackup_TraversePath(path, mask);
	
	auto pathLen = m_OriginPath.length();
	decltype(m_Files) files;

	for (wstring file : m_Files)
	{
		file = file.substr(pathLen + 1);
		file[1] = L':';
		files.emplace(move(file));
	}

	m_Files.swap(files);		
}
