#pragma once
#include "BackupBase.h"





struct IncrementBackupList
{	
	set<wstring> IncrementBackupFileRels;	
};

//template <>
//struct less<DeltaBackupInfo>
//{
//	bool operator() (const DeltaBackupInfo& x, const DeltaBackupInfo& y) const { return x.Filename < y.Filename; }
//	typedef DeltaBackupInfo first_argument_type;
//	typedef DeltaBackupInfo second_argument_type;
//	typedef bool result_type;
//};

using Branch2IncrementListMap = map<wstring, IncrementBackupList >;

struct IncrementBranchList
{		
	Branch2IncrementListMap Branch2Increments;
};

//template <>
//struct less<RegularBackupInfo>
//{
//	bool operator() (const RegularBackupInfo& x, const RegularBackupInfo& y) const { return x.Filename < y.Filename; }
//	typedef RegularBackupInfo first_argument_type;
//	typedef RegularBackupInfo second_argument_type;
//	typedef bool result_type;
//};

using Regular2BranchesMap = map<wstring, IncrementBranchList >;

struct RegularBackupList : public BackupBase
{		
	Regular2BranchesMap Regular2Branches;
	Regular2BranchesMap LostRegular2Branches;
	virtual bool Empty() override;
	size_t TotalNumFiles() const;
	bool HasFileWithTimeStamp(const wstring& timestamp);
};


struct LastRegularBackupInfo
{
	wstring LastRegularFilename;	
	set<wstring> LastIncrementFileRels;
	static LastRegularBackupInfo CreateFromBackupInfoList(const RegularBackupList& backup);

	bool Empty() const;
	wstring GetLastFile() const;
	wstring GetLastBackupTimestamp() const;
	fs::file_time_type GetLastBackupTime() const;	
};



////////////////////////////


using CopyList = list< pair<wstring, wstring> >; //откуда-куда
using RemoveList = list< wstring >; //путь

class BackupJobExecuter;
class Reserver;
class Completer;
class Completion;

class MyBCopy_API Storage
{
public:	
	virtual ~Storage() = default;

	wstring m_Name;
	unsigned int GetNumBranchesFromLastRegular(const BackupBase& backup) const;
	unsigned int GetLastChainLengthFromLastRegular(const BackupBase& backup) const;

	
	LastRegularBackupInfo GetLastBackupsInfo(const BackupBase& backup) const;
	RegularBackupList GetEnumeratedBackupFiles(const BackupBase& backup) const;
		

	
	virtual void StartDownloading(const CopyList& copylist, Completer* completer, const Completion& completion) = 0;
	virtual void StartUploading(const CopyList& copylist, Completer* completer, const Completion& completion) = 0;
	virtual void StartRemoving(const RemoveList& removeList, Completer* completer, const Completion& completion) = 0;

//	using Callback = function< Completer<BackupJob *>*, BackupJob*, exception_ptr>;
//	virtual void StartDownloading2(const CopyList& copylist, const Callback& callback) = 0;
protected:		
	virtual RegularBackupList DoGetEnumeratedBackupFiles(const BackupBase& backup) const = 0;	


};

class MyBCopy_API LocalDiskStorage : public Storage
{
public:
	using Storage::Storage;

	wstring m_Path;

	virtual RegularBackupList DoGetEnumeratedBackupFiles(const BackupBase& backup) const override;

	virtual void StartDownloading(const CopyList& copylist, Completer* completer, const Completion& completion) override;
	virtual void StartUploading(const CopyList& copylist, Completer* completer, const Completion& completion) override;
	virtual void StartRemoving(const RemoveList& removeList, Completer* completer, const Completion& completion) override;
};

class MyBCopy_API WebDavStorage : public Storage
{
public:
	using Storage::Storage;

	wstring m_UserName;
	wstring m_Password;
	wstring m_Url;
	wstring m_BackupDirName;

	virtual RegularBackupList DoGetEnumeratedBackupFiles(const BackupBase& backup) const override;
	virtual void StartDownloading(const CopyList& copylist, Completer* completer, const Completion& completion) override;
	virtual void StartUploading(const CopyList& copylist, Completer* completer, const Completion& completion) override;
	virtual void StartRemoving(const RemoveList& removeList, Completer* completer, const Completion& completion) override;	
};