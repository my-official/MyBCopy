#pragma once
#include "BackupBase.h"




struct FileEntries
{
	set< wstring > m_Directories;
	set< wstring > m_Files;
};

void TraversePathRelativity(const wstring& path, FileEntries& fileEntries);


struct FileEntriesDifference
{
	vector< wstring > NewDirectories;
	vector< wstring > RemoveDirectories;

	vector< wstring > DiffFiles;
	vector< wstring > NewFiles;
	vector< wstring > RemoveFiles;

	static FileEntriesDifference CompareFileEntries(wstring pathToOldDistr, const FileEntries& oldDistEntries, wstring pathToNewDistr, const FileEntries& newDistEntries);
	bool IsEmpty() const;
};


class ParsedSources;






class SrcPathDesc;

namespace Filters
{
	class FilterBase;
}

class MyBCopy_API TraversedBackup : public BackupBase
{
	//private:
	//	TraversedBackup() = default;
public:
	static TraversedBackup CreateViaTraverseParsedBackup(ParsedSources parsedBackup, const Settings& settings);
	static TraversedBackup CreateViaTraverseExtractionDir(const wstring& path);

	map<wstring, FileEntries> m_MountPoint2FileEntries;

	virtual bool Empty() override;
	list<wstring> ToFileList() const;
	virtual void SaveToRarInclusionFile(const wstring& file) const;

	class MyBCopy_API Filtering
	{
	public:
		virtual ~Filtering() = default;
		
		void SortFilters(const SrcPathDesc& srcPath);		

		bool DirFilters(const fs::path& entityPath);
		bool FileFilters(const fs::path& entityPath);		
	protected:		
		bool ApplyFilters(const fs::path& entityPath, const vector<Filters::FilterBase* >& entrySpecificInclusionFilters, const vector< Filters::FilterBase* >& entrySpecificExclusionFilters);
	private:
		vector< Filters::FilterBase* >	m_IncludeEntryFilters;
		vector< Filters::FilterBase* >	m_ExcludeEntryFilters;

		vector< Filters::FilterBase* >	m_IncludeDirFilters;
		vector< Filters::FilterBase* >	m_ExcludeDirFilters;

		vector< Filters::FilterBase* >	m_IncludeFileFilters;
		vector< Filters::FilterBase* >	m_ExcludeFileFilters;
		vector<Filters::FilterBase *>* GetFiltersVector(Filters::FilterBase* filter);
	};


	class MyBCopy_API Traverser : public Filtering
	{
	public:
		Traverser(TraversedBackup&);
				
		void SortFiltersAndTraverse(const SrcPathDesc& srcPath, const fs::path& originPath = fs::path());		
	protected:
		virtual void AddFileEntryPerRootPath(const fs::path& filepath);
	private:
		void Traverse(const SrcPathDesc& srcPath, const fs::path& originPath = fs::path());
		void FunctionalRecursion(const SrcPathDesc& srcPath, const fs::path& originPath = fs::path());		
		
	
		TraversedBackup& m_Backup;
	};
};



TraversedBackup CompareBackups(const TraversedBackup& oldDistEntries, const TraversedBackup& newDistEntries);

//////////////////////////////

//
//class ArchivedRarBackup : public BackupAndSettings
//{
//public:
//	ArchivedRarBackup() = default;
//	ArchivedRarBackup(const TraversedBackup& backup, const wstring& tmpWorkingDir, const wstring& newBackupArchiveFileNameBase);
//
//	ArchivedRarBackup(const ArchivedRarBackup&) = delete;
//	ArchivedRarBackup& operator= (const ArchivedRarBackup&) = delete;
//
//	~ArchivedRarBackup();
//	std::wstring GetArchiveRarFile() const;
//	virtual bool Empty() override;
//
//	void ExtractToDir(const wstring& destDir, const wstring& passwd, bool rarOverwrite = false, bool rarRestoreAbsPath = false, bool rarRestoreDisk = true);
//
//	wstring  m_ArchiveRarFile;
//private:
//	wstring  m_InclusionListFile;
//};
//
//
//class Archived7zBackup : public BackupAndSettings
//{
//public:
//	Archived7zBackup() = default;
//	Archived7zBackup(const Archived7zBackup&) = delete;
//	Archived7zBackup& operator= (const Archived7zBackup&) = delete;
//
//	Archived7zBackup(Archived7zBackup&&) noexcept(false) = default;
//	Archived7zBackup& operator= (Archived7zBackup&&)  noexcept(false) = default;
//
//	~Archived7zBackup() noexcept;
//
//	std::wstring Archive7zFile() const;
//	virtual bool Empty() override;
//
//	wstring  m_Archive7zFile;
//
//	void AssignAndCompress(const ArchivedRarBackup& backup, const wstring& tmpWorkingDir, const wstring& newBackupArchiveFileNameBase);
//
//
//	static void CreateForExtraction(const wstring& file, const BackupAndSettings& backup, Archived7zBackup& archive7z);
//	void ExtractRarBackupFrom7zArchive(const wstring& passwd, ArchivedRarBackup& archiveRar);
//};
//


