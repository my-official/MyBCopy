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



struct DeltaBackupList
{		
	map<wstring, IncrementBackupList > Delta2Increments;
};

//template <>
//struct less<RegularBackupInfo>
//{
//	bool operator() (const RegularBackupInfo& x, const RegularBackupInfo& y) const { return x.Filename < y.Filename; }
//	typedef RegularBackupInfo first_argument_type;
//	typedef RegularBackupInfo second_argument_type;
//	typedef bool result_type;
//};

struct RegularBackupList : public BackupBase
{	
	map<wstring, DeltaBackupList > Regular2Deltas;
	virtual bool Empty() override;
};


struct LastRegularBackupInfo
{
	wstring LastRegularFilename;
	wstring LastDeltaFilename;	
	set<wstring> IncrementFilenames;
	static LastRegularBackupInfo CreateFromBackupInfoList(const RegularBackupList& backup);
};



////////////////////////////


using CopyList = list< pair<wstring, wstring> >; //откуда-куда
using RemoveList = list< wstring >; //путь

class BackupJobExecuter;
struct BackupJob;
class Reserver;
class Completer;
class Completion;

class Storage
{
public:
	Storage(Reserver* reserver);
	virtual ~Storage() = default;

	wstring m_Name;
	unsigned int GetNumOfDeltaFromLastRegular(const BackupBase& backup) const;
	unsigned int GetNumOfDeltaIncrementsOfLastDeltaFromLastRegular(const BackupBase& backup) const;

	
	LastRegularBackupInfo GetLastBackupsInfo(const BackupBase& backup) const;
	RegularBackupList GetEnumeratedBackupFiles(const BackupBase& backup) const;

	void UpdateCache(const BackupBase& backup);

	
	virtual void StartDownloading(const CopyList& copylist, Completer* completer, const Completion& completion) = 0;
	virtual void StartUploading(const CopyList& copylist, Completer* completer, const Completion& completion) = 0;
	virtual void StartRemovingOld(const RemoveList& removeList, Completer* completer, const Completion& completion) = 0;

//	using Callback = function< Completer<BackupJob *>*, BackupJob*, exception_ptr>;
//	virtual void StartDownloading2(const CopyList& copylist, const Callback& callback) = 0;
protected:
	Reserver* m_Reserver;
	virtual void CopyFiles(const CopyList& downloadList);
	virtual RegularBackupList DoGetEnumeratedBackupFiles(const BackupBase& backup) const = 0;
	
private:
	mutable map<wstring, RegularBackupList> m_BackupName2RegularBackupListCache;

};

class LocalDiskStorage : public Storage
{
public:
	using Storage::Storage;

	wstring m_Path;

	virtual RegularBackupList DoGetEnumeratedBackupFiles(const BackupBase& backup) const override;

	virtual void StartDownloading(const CopyList& copylist, Completer* completer, const Completion& completion) override;
	virtual void StartUploading(const CopyList& copylist, Completer* completer, const Completion& completion) override;
	virtual void StartRemovingOld(const RemoveList& removeList, Completer* completer, const Completion& completion) override;
};

class WebDavStorage : public Storage
{
public:
	using Storage::Storage;

	wstring m_UserName;
	wstring m_Password;
	wstring m_Url;

	virtual RegularBackupList DoGetEnumeratedBackupFiles(const BackupBase& backup) const override;
	virtual void StartDownloading(const CopyList& copylist, Completer* completer, const Completion& completion) override;
	virtual void StartUploading(const CopyList& copylist, Completer* completer, const Completion& completion) override;
	virtual void StartRemovingOld(const RemoveList& removeList, Completer* completer, const Completion& completion) override;
	void Mount(const wstring& mountPoint) const;	
};