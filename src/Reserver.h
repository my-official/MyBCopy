#pragma once


struct Backup
{
	string ArchiveName;
	string EncryptedFile;
	deque<string> SrcFiles;
	deque<string> ExclFiles;


};

class Reserver
{
public:
	Reserver();
	virtual ~Reserver();

	void Process();
	

private:
	unsigned int m_MaxNumOfOldArchives;
	string m_RCopyDirPath;
	string m_ExePath;
	string m_TemporaryDir;
	string m_Now;

	INI::Parser m_SettingsIni; 
		
		
	deque<Backup> m_Backups;


	static string GetThisProgramExeDirPath();

	

	void AddPath(const string& newpath);

	string Now();
	
	void LoadSourcesFromFile(const string& srcFile);	
	void ParsePath(const string& line, const string& srcFile, size_t currLine);	

	
	void PrepareSourceFileListForRar(Backup& backup);
	void MakeRarArchive(const Backup& backup);
	void Make7zArchive(const Backup& backup);
	void CleanupRar(Backup& backup);

	void UploadToCloud(const string& IniFileSectionName);	
	void MountWebDavShare(const string& IniFileSectionName);
	void RemoveOldArchives(const Backup& backup);
	void UnmountWebDavShare();
		
	
	string GetTemporaryDirPath(const Backup& backup) const;
	string GetTemporaryRarFile(const Backup& backup) const;
	string GetTemporaryRarSourceListFile(const Backup& backup) const;
	string GetTemporaryRarExclusionListFile(const Backup& backup) const;

	string GetTemporary7zEncryptedFile(const Backup& backup) const;

	string GetCloudDestPath(const Backup& backup) const;
	string GetCloudDestFile(const Backup& backup) const;


	void Schtask();

	static void RunCMD(const string& cmd);
	
};


class ParseException : public CachedMessageExeption
{
public:
	ParseException(const string& file, size_t line);

	string m_File;
	size_t m_Line;			
protected:	
	virtual void FormattedMessageLine() override;
	
};

