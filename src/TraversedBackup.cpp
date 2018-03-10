#include "MyBCopy_pch.h"
#include "TraversedBackup.h"
#include "Utils.h"
#include "ParsedSources.h"
#include "Exceptions.h"




//
//void TraversedBackup::TraversePathsWildcard(const vector<wstring>& paths)
//{
//	for (auto& filePath : paths)
//	{
//		auto pos = filePath.find_last_of(L"*?");
//		if (pos != wstring::npos)
//		{
//			wregex mask( wildcard2wregex(fs::path(filePath).filename()) );
//
//			fs::path parentPath = fs::path(filePath).parent_path();
//
//			if (!fs::is_directory(parentPath))
//				continue;
//
//			for (auto& p : fs::directory_iterator(parentPath))
//			{
//				if (fs::is_directory(p.path()))
//				{
//					TraversePathRegex(p.path(), &mask);
//				}
//				else
//				{
//					if (!fs::is_regular_file(p.path()))
//						continue;
//
//					if (!regex_match(wstring(p.path()), mask))
//						continue;
//
//					AddFileEntryPerRootPath(p.path());
//				}
//			}
//		}
//		else
//		{
//			if (fs::is_directory(filePath))
//			{
//				/*result.m_Directories.insert(filePath);
//				auto prevSize = result.m_Files.size();*/
//				TraversePathRegex(filePath);
//				/*if (prevSize == result.m_Files.size())
//				result.m_Directories.insert(filePath);*/
//			}
//			else
//			{
//				if (!fs::is_regular_file(filePath))
//					continue;
//
//				AddFileEntryPerRootPath(filePath);
//			}
//		}
//	}
//}
//
//void TraversedBackup::TraversePathRegex(const wstring& path, std::wregex* mask /*= nullptr*/)
//{
//	for (auto& p : fs::recursive_directory_iterator(path))
//	{
//		auto& filePath = p.path();
//
//		if (!fs::is_regular_file(filePath))
//			continue;
//
//		if (mask)
//		{
//			if (!regex_match(wstring(filePath.filename()), *mask))
//				continue;
//		}
//
//		AddFileEntryPerRootPath(filePath);
//	}
//}

void TraversePathRelativity(const wstring& path, FileEntries& fileEntries)
{
	for (auto& p : fs::recursive_directory_iterator(path))
	{
		auto& filePath = p.path();

		if (!fs::is_regular_file(filePath))
			continue;

		wstring filePathStr = filePath;
		fileEntries.m_Files.insert(filePathStr.substr(path.length() + 1));
	}
}


FileEntriesDifference FileEntriesDifference::CompareFileEntries(wstring pathToOldDistr, const FileEntries& oldDistEntries, wstring pathToNewDistr, const FileEntries& newDistEntries)
{
	FileEntriesDifference result;

	set_difference(newDistEntries.m_Directories.begin(), newDistEntries.m_Directories.end(), oldDistEntries.m_Directories.begin(), oldDistEntries.m_Directories.end(), back_inserter(result.NewDirectories));
	set_difference(oldDistEntries.m_Directories.begin(), oldDistEntries.m_Directories.end(), newDistEntries.m_Directories.begin(), newDistEntries.m_Directories.end(), back_inserter(result.RemoveDirectories));

	set_difference(newDistEntries.m_Files.begin(), newDistEntries.m_Files.end(), oldDistEntries.m_Files.begin(), oldDistEntries.m_Files.end(), back_inserter(result.NewFiles));
	set_difference(oldDistEntries.m_Files.begin(), oldDistEntries.m_Files.end(), newDistEntries.m_Files.begin(), newDistEntries.m_Files.end(), back_inserter(result.RemoveFiles));

	list<wstring> intersection;
	set_intersection(oldDistEntries.m_Files.begin(), oldDistEntries.m_Files.end(), newDistEntries.m_Files.begin(), newDistEntries.m_Files.end(), back_inserter(intersection));

	//if (!pathToOldDistr.empty())
	//	pathToOldDistr += L"\\";

	//if (!pathToNewDistr.empty())
	//	pathToNewDistr += L"\\";

	copy_if(make_move_iterator(intersection.begin()), make_move_iterator(intersection.end()), back_inserter(result.DiffFiles), [pathToOldDistr, pathToNewDistr](const wstring& file)->bool
	{
		wstring src = pathToOldDistr + file;
		wstring dest = pathToNewDistr + file;
		return FileCmp(src, dest) != 0;
	});

	return result;
}



bool FileEntriesDifference::IsEmpty() const
{
	return /*NewDirectories.empty() && RemoveDirectories.empty() && */DiffFiles.empty() && NewFiles.empty()/* && RemoveFiles.empty()*/;
}



TraversedBackup TraversedBackup::CreateViaTraverseParsedBackup(ParsedSources parsedBackup, const Settings& settings)
{
	parsedBackup.NormalizeRecursive();

	TraversedBackup result;
	static_cast<BackupBase&>(result) = static_cast<BackupBase&>(parsedBackup);
	result.m_Settings = settings;


	for (auto& srcPath : parsedBackup.m_SrcPaths)
	{
		Traverser traverser(result);
		traverser.SortFiltersAndTraverse(srcPath);
	}

	//result.TraversePathsWildcard(parsedBackup.m_SrcFiles);

	//TraversedEntriesBackup exclusion;
	//exclusion.TraversePathsWildcard(parsedBackup.m_ExclFiles);

	//result.Subtract(exclusion);
	return result;
}

TraversedBackup TraversedBackup::CreateViaTraverseExtractionDir(const wstring& path)
{
	TraversedBackup result;
	for (auto& p : fs::directory_iterator(path))
	{
		wstring dirPath = p.path();
		if (!fs::is_directory(p.path()))
		{
			throw EXCEPTION(BaseException(L"wrong data inextraction dir"));
		}

		FileEntries& fileEntries = result.m_MountPoint2FileEntries[dirPath + L"\\"];

		TraversePathRelativity(dirPath, fileEntries);
	}

	return result;
}

bool TraversedBackup::Empty()
{
	return m_MountPoint2FileEntries.empty();
}

std::list<std::wstring> TraversedBackup::ToFileList() const
{
	std::list<std::wstring> result;
	for (auto& m2fe : m_MountPoint2FileEntries)
	{
		const set<wstring>& files = m2fe.second.m_Files;
		for (auto& srcfile : files)
		{
			result.emplace_back(m2fe.first + srcfile);
		}
	}
	return result;
}

void TraversedBackup::SaveToRarInclusionFile(const wstring& file) const
{
	wofstream outfile(file);

	if (!outfile)
		throw EXCEPTION(ErrnoException());

	for (auto& m2fe : m_MountPoint2FileEntries)
	{
		const set<wstring>& files = m2fe.second.m_Files;
		for (auto& srcfile : files)
		{
			outfile << L"\"" << m2fe.first << srcfile << L"\"" << endl;
		}
	}
}

//
//void TraversedBackup::Subtract(const TraversedBackup& exclusion)
//{
//	for (auto& m2fe : m_MountPoint2FileEntries)
//	{
//		auto it = exclusion.m_MountPoint2FileEntries.find(m2fe.first);
//
//		if (it == exclusion.m_MountPoint2FileEntries.end())
//			continue;
//
//		auto& files = m2fe.second.m_Files;
//		auto& exclFiles = it->second.m_Files;
//
//		set< wstring > result;
//		set_difference(files.begin(), files.end(), exclFiles.begin(), exclFiles.end(), inserter(result, result.end()));
//		files = move(result);
//	}
//}

//
//ArchivedRarBackup::ArchivedRarBackup(const TraversedBackup& backup, const wstring& tmpWorkingDir, const wstring& newBackupArchiveFileNameBase)
//	: BackupAndSettings(static_cast<const BackupAndSettings&>(backup)),
//	m_InclusionListFile(tmpWorkingDir + L"\\" + newBackupArchiveFileNameBase + L"_inclusion.txt"),
//	m_ArchiveRarFile(tmpWorkingDir + L"\\" + newBackupArchiveFileNameBase + L".rar")
//{
//	fs::create_directories(tmpWorkingDir);
//
//	backup.SaveToRarInclusionFile(m_InclusionListFile);
//
//	wstring cmd = L"\"\"" + m_Settings.m_RarPath + L"\" a -hp\"" + m_Settings.m_RarPassword + L"\" -idq -m5 -ep3 -r -s -t -y -- \"" + m_ArchiveRarFile + L"\" @\"" + m_InclusionListFile + L"\"\"";
//
//	RunCMD_except(cmd);
//}
//
//ArchivedRarBackup::~ArchivedRarBackup()
//{
//	if (Empty())
//		return;
//
//	if (fs::exists(m_InclusionListFile))
//	{
//		fs::remove(m_InclusionListFile);
//	}
//	if (fs::exists(m_ArchiveRarFile))
//	{
//		fs::remove(m_ArchiveRarFile);
//	}
//}
//
//std::wstring ArchivedRarBackup::GetArchiveRarFile() const
//{
//	return m_ArchiveRarFile;
//}
//
//bool ArchivedRarBackup::Empty()
//{
//	return m_ArchiveRarFile.empty() || m_InclusionListFile.empty();
//}
//
//void ArchivedRarBackup::ExtractToDir(const wstring& destDir, const wstring& passwd, bool rarOverwrite /*= false*/, bool rarRestoreAbsPath /*= false*/, bool rarRestoreDisk /*= true*/)
//{
//	wstring overwrite = rarOverwrite ? L"+" : L"-";
//
//
//	wstring cmd = L"\"\"" + m_Settings.m_RarPath + L"\" x -p\"" + passwd + L"\" -o" + overwrite + L" ";
//
//	if (rarRestoreAbsPath)
//	{
//		cmd += rarRestoreDisk ? L"-ep3" : L"-ep2";
//	}
//
//	cmd = L" -y -- \"" + m_ArchiveRarFile + L"\" * ";
//
//	if (!rarRestoreAbsPath)
//	{
//		cmd += L"\"" + destDir + L"\\\"\"";
//	}
//
//	RunCMD_except(cmd);
//}
//
//
//Archived7zBackup::~Archived7zBackup()
//{
//	if (Empty())
//		return;
//
//	if (fs::exists(m_Archive7zFile))
//	{
//		fs::remove(m_Archive7zFile);
//	}
//}
//
//std::wstring Archived7zBackup::Archive7zFile() const
//{
//	return m_Archive7zFile;
//}
//
//bool Archived7zBackup::Empty()
//{
//	return m_Archive7zFile.empty();
//}
//
//
//void Archived7zBackup::AssignAndCompress(const ArchivedRarBackup& backup, const wstring& tmpWorkingDir, const wstring& newBackupArchiveFileNameBase)
//{
//	static_cast<BackupAndSettings&>(*this) = static_cast<const BackupAndSettings&>(backup);
//	m_Archive7zFile = tmpWorkingDir + L"\\" + newBackupArchiveFileNameBase + L".7z";
//	RunCMD_except(L"\"\"" + m_Settings.m_7zPath + L"\" a -p\"" + m_Settings.m_7zPassword + L"\" -mhe=on -mx=0 -bso0 -bsp0 -y \"" + m_Archive7zFile + L"\" \"" + backup.GetArchiveRarFile() + L"\"\"");
//	RunCMD_except(L"\"\"" + m_Settings.m_7zPath + L"\" t -p\"" + m_Settings.m_7zPassword + L"\" -bso0 -bsp0 -y \"" + m_Archive7zFile + L"\" *\"");
//}
//
//void Archived7zBackup::CreateForExtraction(const wstring& file, const BackupAndSettings& backup, Archived7zBackup& archive7z)
//{
//	static_cast<BackupAndSettings&>(archive7z) = static_cast<const BackupAndSettings&>(backup);
//	archive7z.m_Archive7zFile = file;
//}
//
//void Archived7zBackup::ExtractRarBackupFrom7zArchive(const wstring& passwd, ArchivedRarBackup& archiveRar)
//{
//	static_cast<BackupAndSettings&>(archiveRar) = static_cast<const BackupAndSettings&>(*this);
//	RunCMD_except(L"\"\"" + m_Settings.m_7zPath + L"\" x -p\"" + passwd + L"\" -o\"" + wstring(fs::path(m_Archive7zFile).parent_path()) + L"\" -y \"" + m_Archive7zFile + L"\"\"");
//	archiveRar.m_ArchiveRarFile = wstring(fs::path(m_Archive7zFile).parent_path() / fs::path(m_Archive7zFile).stem()) + L".rar";
//}


TraversedBackup CompareBackups(const TraversedBackup& oldDistEntries, const TraversedBackup& newDistEntries)
{
	TraversedBackup result;
	result.m_Settings = newDistEntries.m_Settings;


	auto&  oldMountPoint2FileEntries = oldDistEntries.m_MountPoint2FileEntries;
	auto& newMountPoint2FileEntries = newDistEntries.m_MountPoint2FileEntries;

	for (auto& m2fe : newMountPoint2FileEntries)
	{
		auto& newMountPoint = m2fe.first;

		auto it = find_if(oldMountPoint2FileEntries.begin(), oldMountPoint2FileEntries.end(), [&newMountPoint](const pair<wstring, FileEntries>& e)->bool
		{
			wstring oldMountPoint = e.first;
			auto len = oldMountPoint.length();

			oldMountPoint = oldMountPoint.substr(len - 3);
			oldMountPoint[1] = L':';
			return oldMountPoint == newMountPoint;
		});


		if (it == oldMountPoint2FileEntries.end())
		{
			result.m_MountPoint2FileEntries[newMountPoint] = m2fe.second;
		}
		else
		{
			auto fileDiff = FileEntriesDifference::CompareFileEntries(it->first, it->second, m2fe.first, m2fe.second);

			if (!fileDiff.IsEmpty())
			{
				auto& resultFileEntries = result.m_MountPoint2FileEntries[newMountPoint];

				copy(fileDiff.DiffFiles.begin(), fileDiff.DiffFiles.end(), inserter(resultFileEntries.m_Files, resultFileEntries.m_Files.end()));
				copy(fileDiff.NewFiles.begin(), fileDiff.NewFiles.end(), inserter(resultFileEntries.m_Files, resultFileEntries.m_Files.end()));
			}
		}
	}

	return result;
}

TraversedBackup::Traverser::Traverser(TraversedBackup& backup)
	: m_Backup(backup)
{

}


void TraversedBackup::Traverser::AddFileEntryPerRootPath(const fs::path& filepath)
{
	wstring mountPoint = filepath.root_path();
	FileEntries& fileEntries = m_Backup.m_MountPoint2FileEntries[mountPoint];
	fileEntries.m_Files.insert(filepath.relative_path());
}

void TraversedBackup::Traverser::FunctionalRecursion(const SrcPathDesc& srcPath, const fs::path& originPath /*= fs::path()*/)
{
	Traverser recursion(m_Backup);
	recursion.SortFiltersAndTraverse(srcPath, originPath);
}



void TraversedBackup::Traverser::SortFiltersAndTraverse(const SrcPathDesc& srcPath, const fs::path& originPath /*= fs::path()*/)
{
	SortFilters(srcPath);	
	Traverse(srcPath, originPath);
}

void TraversedBackup::Traverser::Traverse(const SrcPathDesc& srcPath, const fs::path& originPath /*= fs::path()*/)
{
	fs::path srcPathAbsolute = (!originPath.empty() ? originPath / fs::path(srcPath.m_Path) : fs::path(srcPath.m_Path));

	if (!fs::exists(srcPathAbsolute))
	{
		return;
	}			

	if (!srcPath.m_Include)
	{
		for (auto& subSrcPath : srcPath.m_SubSrcPathDescs)
		{
			FunctionalRecursion(subSrcPath, originPath);
		}
		return;
	}

	if (fs::is_directory(srcPathAbsolute))
	{
		for (auto& p : fs::directory_iterator(srcPathAbsolute))
		{
			fs::path entityPath = p.path();
			wstring entityName = entityPath.filename();

			auto ret = find_if(srcPath.m_SubSrcPathDescs.begin(), srcPath.m_SubSrcPathDescs.end(), [&entityName](const SrcPathDesc& e)->bool
			{
				return e.m_Path == entityName;
			});

			if (ret != srcPath.m_SubSrcPathDescs.end())
			{
				FunctionalRecursion(*ret, originPath);
				continue;
			}


			if (fs::is_directory(entityPath))
			{
				if (srcPath.m_Depth == 0)
				{
					ret = find_if(srcPath.m_SubSrcPathDescs.begin(), srcPath.m_SubSrcPathDescs.end(), [&entityName](const SrcPathDesc& e)->bool
					{
						return *fs::path(e.m_Path).begin() == entityName;
					});

					if (ret != srcPath.m_SubSrcPathDescs.end())
					{
						FunctionalRecursion(*ret, originPath);
					}

					continue;
				}

				if (!DirFilters(entityPath))
					continue;

				///Наследование			
				SrcPathDesc inheritedSrcPath = SrcPathDesc::CreateViaInheritance(entityName, srcPath);
				Traverse(inheritedSrcPath, entityPath.parent_path());
			}
			else
			{
				/*		if (!fs::is_regular_file(entityPath))
							throw EXCEPTION(BaseException(L"Unknown filesystem element " + wstring(entityPath)));*/

				if (!srcPath.m_Include)
					return;

				if (!FileFilters(entityPath))
					return;

				AddFileEntryPerRootPath(entityPath);
			}
		}
	}
	else
	{
		if (!FileFilters(srcPathAbsolute))
			return;

		AddFileEntryPerRootPath(srcPathAbsolute);
	}
}


bool TraversedBackup::Filtering::DirFilters(const fs::path& entityPath)
{
	return ApplyFilters(entityPath, m_IncludeDirFilters, m_ExcludeDirFilters);
}

bool TraversedBackup::Filtering::FileFilters(const fs::path& entityPath)
{
	return ApplyFilters(entityPath, m_IncludeFileFilters, m_ExcludeFileFilters);
}

bool TraversedBackup::Filtering::ApplyFilters(const fs::path& entityPath, const vector<Filters::FilterBase* >& entrySpecificInclusionFilters, const vector< Filters::FilterBase* >& entrySpecificExclusionFilters)
{
	bool include = true;

	if (!m_IncludeEntryFilters.empty())
	{
		include = any_of(m_IncludeEntryFilters.begin(), m_IncludeEntryFilters.end(), [&entityPath](const Filters::FilterBase* filter)->bool
		{
			return filter->operator()(entityPath);
		});
	}

	if (!entrySpecificInclusionFilters.empty())
	{
		include |= any_of(entrySpecificInclusionFilters.begin(), entrySpecificInclusionFilters.end(), [&entityPath](const Filters::FilterBase* filter)->bool
		{
			return filter->operator()(entityPath);
		});
	}

	if (!include)
		return false;

	bool exclude = false;

	if (!m_ExcludeEntryFilters.empty())
	{
		exclude = any_of(m_ExcludeEntryFilters.begin(), m_ExcludeEntryFilters.end(), [&entityPath](const Filters::FilterBase* filter)->bool
		{
			return filter->operator()(entityPath);
		});
	}

	if (exclude)
		return false;
	
	if (!entrySpecificExclusionFilters.empty())
	{
		exclude = any_of(entrySpecificExclusionFilters.begin(), entrySpecificExclusionFilters.end(), [&entityPath](const Filters::FilterBase* filter)->bool
		{
			return filter->operator()(entityPath);
		});
	}

	if (exclude)
		return false;

	return true;
}

void TraversedBackup::Filtering::SortFilters(const SrcPathDesc& srcPath)
{
	for (auto& _filter : srcPath.m_Filters)
	{
		Filters::FilterBase* filter = _filter.get();
		vector< Filters::FilterBase* >* filters = GetFiltersVector(filter);
		filters->push_back(filter);
	}
}

std::vector<Filters::FilterBase *>* TraversedBackup::Filtering::GetFiltersVector(Filters::FilterBase* filter)
{
	bool inclusion = filter->IsInclusion();

	switch (filter->GetEntryType())
	{
		case Filters::EntryType::Entry: return inclusion ? &m_IncludeEntryFilters : &m_ExcludeEntryFilters;
		case Filters::EntryType::Dir: return inclusion ? &m_IncludeDirFilters : &m_ExcludeDirFilters;
		case Filters::EntryType::File: return inclusion ? &m_IncludeFileFilters : &m_ExcludeFileFilters;
		default:
		assert(0);
		return nullptr;
	}
}
