#pragma once



class BackupBase
{
public:
	BackupBase() = default;
	BackupBase(const wstring& archiveName);
	virtual ~BackupBase() = default;

	wstring m_ArchiveName;
	virtual bool Empty() = 0;	
};


/////////////////////////////
class TraversedBackup1;

class ParsedBackup : public BackupBase
{
public:
	ParsedBackup() = default;
	static void LoadBackupSourcesFromFile(const wstring& srcFile, deque<ParsedBackup>& backups);	

	deque<wstring> m_SrcFiles;
	deque<wstring> m_ExclFiles;

	virtual bool Empty() override;
	virtual void SaveToRarInclusionFile(const wstring& file) const;

	TraversedBackup1 Traverse();

private:
	void TraverseFiles(const deque<wstring>& files, TraversedBackup1& result);
	static void ParsePath(deque<ParsedBackup>& backups, const wstring& line, const wstring& srcFile, size_t currLine);
};


////////////////////////////




class TraversedEntries : public BackupBase
{
public:
	set< wstring > m_Directories;
	set< wstring > m_Files;

	virtual bool Empty() override;
	virtual void SaveToRarInclusionFile(const wstring& file) const;

	void Subtract(const TraversedEntries& exclusion);

protected:
	void ParsedBackup_TraversePathsAndMasks(const deque<wstring>& filelist);
	virtual void ParsedBackup_TraversePath(const wstring& path, std::wregex* mask = nullptr);
};


class TraversedParesedBackup : public TraversedEntries
{
public:	
	TraversedParesedBackup() = default;
	TraversedParesedBackup(const ParsedBackup& parsedBackup);
};


class TraversedPathBackup : public TraversedEntries
{
public:
	TraversedPathBackup(const wstring& path);
	wstring			m_OriginPath;
protected:
	virtual void ParsedBackup_TraversePath(const wstring& path, std::wregex* mask = nullptr) override final;
};



struct BackupDifference
{
	vector< wstring > NewDirectories;
	vector< wstring > RemoveDirectories;

	vector< wstring > DiffFiles;
	vector< wstring > NewFiles;
	vector< wstring > RemoveFiles;

	static BackupDifference Compare(const TraversedPathBackup& oldDistEntries, const TraversedEntries& newDistEntries);
	void ToTraversedEntries(TraversedEntries& actualData);

	static void FilesCompare(const TraversedPathBackup& oldDistEntries, const TraversedEntries& newDistEntries, BackupDifference& patch);
};

//////////////////////////////


class ArchivedRarBackup : public BackupBase
{
public:	
	ArchivedRarBackup() = default;
	ArchivedRarBackup(const TraversedEntries& backup, const wstring& tmpWorkingDir, const wstring& newBackupArchiveFileNameBase, const wstring& passwd);

	ArchivedRarBackup(const ArchivedRarBackup& ) = delete;
	ArchivedRarBackup& operator= (const ArchivedRarBackup&) = delete;

	~ArchivedRarBackup();
	std::wstring ArchiveRarFile() const;
	virtual bool Empty() override;

	void ExtractToDir(const wstring& destDir, const wstring& passwd);

	wstring  m_ArchiveRarFile;
private:	
	wstring  m_InclusionListFile;	
};


class Archived7zBackup : public BackupBase
{
public:
	Archived7zBackup() = default;
	Archived7zBackup(const Archived7zBackup&) = delete;
	Archived7zBackup& operator= (const Archived7zBackup&) = delete;

	Archived7zBackup(Archived7zBackup&&) noexcept(false) = default;
	Archived7zBackup& operator= (Archived7zBackup&&)  noexcept(false) = default;
	
	~Archived7zBackup() noexcept;

	std::wstring Archive7zFile() const;
	virtual bool Empty() override;
	
	wstring  m_Archive7zFile;	
	
	void AssignAndCompress(const ArchivedRarBackup& backup, const wstring& tmpWorkingDir, const wstring& newBackupArchiveFileNameBase, const wstring& passwd);


	static void CreateForExtraction(const wstring& file, Archived7zBackup& archive7z);
	void ExtractRarBackup(const wstring& passwd, ArchivedRarBackup& archiveRar);
};



